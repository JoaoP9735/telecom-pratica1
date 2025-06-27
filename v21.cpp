#include <cstdio>
#include <numbers>
#include "v21.hpp"

constexpr float r = 0.99;

V21_RX::V21_RX(float omega_m, float omega_s, std::function<void(const unsigned int *, unsigned int)> get_digital_samples) : omega_m(omega_m),
                                                                                                                            omega_s(omega_s),
                                                                                                                            get_digital_samples(get_digital_samples)
{
    for (int i = 0; i < SAMPLES_PER_SYMBOL; i++)
        this->sample_buffer.push_front(0.f);
    vspace_r_buffer = 0.f;
    vspace_i_buffer = 0.f;
    vmark_r_buffer = 0.f;
    vmark_i_buffer = 0.f;
    buffer_decisao[0] = 0.f;
    buffer_decisao[1] = 0.f;
    buffer_filtrada[0] = 0.f;
    buffer_filtrada[1] = 0.f;
    low_state = 0;
    R = 30;
    fs = 48000;
    T = 1 / fs;
    L = SAMPLES_PER_SYMBOL;
    
    this->lp_num[0] = 0.00844269f;
    this->lp_num[1] = 0.01688539f;
    this->lp_num[2] = 0.00844269f;

    this->lp_den[0] = 1.0f;
    this->lp_den[1] = -1.72377617f;
    this->lp_den[2] = 0.75754694f;
};

void V21_RX::demodulate(const float *in_analog_samples, unsigned int n)
{
    unsigned int digital_samples[n];

    for (int i = 0; i < n; i++)
    {
        this->sample_buffer.push_front(in_analog_samples[i]);

        float vspace_r = sample_buffer[0] - pow(r, L) * cos(omega_s * L * T) * sample_buffer[L] + r * cos(omega_s * T) * vspace_r_buffer - r * sin(omega_s * T) * vspace_i_buffer;
        float vspace_i = -pow(r, L) * sin(omega_s * L * T) * sample_buffer[L] + r * cos(omega_s * T) * vspace_i_buffer + r * sin(omega_s * T) * vspace_r_buffer;
        float vmark_r = sample_buffer[0] - pow(r, L) * cos(omega_m * L * T) * sample_buffer[L] + r * cos(omega_m * T) * vmark_r_buffer - r * sin(omega_m * T) * vmark_i_buffer;
        float vmark_i = -pow(r, L) * sin(omega_m * L * T) * sample_buffer[L] + r * cos(omega_m * T) * vmark_i_buffer + r * sin(omega_m * T) * vmark_r_buffer;

        float decisao = vmark_r * vmark_r + vmark_i * vmark_i - vspace_r * vspace_r - vspace_i * vspace_i;

        float decisao_filtro =
            (lp_num[0] * decisao + lp_num[1] * buffer_decisao[0] + lp_num[2] * buffer_decisao[1] - lp_den[1] * buffer_filtrada[0] - lp_den[2] * buffer_filtrada[1]) / lp_den[0];

        switch (state)
        {
        case IDLE:
            if (abs(decisao_filtro) > 120)
            {
                digital_samples[i] = decisao_filtro > 0 ? 1 : 0;
                low_state = 0;
                this->state = CARRIER;
            }
            else
                digital_samples[i] = 1;

            break;

        case CARRIER:
            if (abs(decisao_filtro) < 60)
                low_state++;
            else
                low_state = 0;

            if (low_state >= 50)
            {
                digital_samples[i] = 1;
                state = IDLE;
            }
            else
                digital_samples[i] = decisao_filtro > 0 ? 1 : 0;

            break;

        default:
            break;
        }

        this->sample_buffer.pop_back();
        this->vspace_r_buffer = vspace_r;
        this->vspace_i_buffer = vspace_i;
        this->vmark_r_buffer = vmark_r;
        this->vmark_i_buffer = vmark_i;
        this->buffer_decisao[1] = this->buffer_decisao[0];
        this->buffer_decisao[0] = decisao;
        this->buffer_filtrada[1] = this->buffer_filtrada[0];
        this->buffer_filtrada[0] = decisao_filtro;
    }

    get_digital_samples(digital_samples, n);
}

void V21_TX::modulate(const unsigned int *in_digital_samples, float *out_analog_samples, unsigned int n)
{
    while (n--)
    {
        *out_analog_samples++ = sin(phase);
        phase += (*in_digital_samples++ ? omega_m : omega_s) * SAMPLING_PERIOD;

        // evita que phase cresça indefinidamente, o que causaria perda de precisão
        phase = remainder(phase, 2 * M_PI);
    }
}