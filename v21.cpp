#include <math.h>
#include <numbers>
#include "v21.hpp"


void V21_RX::demodulate(const float *in_analog_samples, unsigned int n)
 {
     unsigned int digital_samples[n];

    for (unsigned int i = 0; i < n; ++i) {
        float sample = in_analog_samples[i];

        // Gera senoides locais (coerência)
        float ref_mark = sin(phase_mark);
        float ref_space = sin(phase_space);

        phase_mark += 2 * std::numbers::pi * freq_mark / fs;
        phase_space += 2 * std::numbers::pi * freq_space / fs;

        if (phase_mark > 2 * std::numbers::pi) 
            phase_mark -= 2 * std::numbers::pi;
        if (phase_space > 2 * std::numbers::pi) 
            phase_space -= 2 * std::numbers::pi;

        // Multiplicação coerente
        float out_mark = sample * ref_mark;
        float out_space = sample * ref_space;

        // Detecção de energia (energia relativa)
        float diff = out_mark * out_mark - out_space * out_space;

        // Filtro passa-baixa
        lp_filter_state = (1 - alpha) * lp_filter_state + alpha * diff;

        // Detecção de ausência de portadora
        float power = out_mark * out_mark + out_space * out_space;
        if (power < 1e-4) {
            digital_samples[i] = 1;  // sem portadoras
        } else {
            digital_samples[i] = (lp_filter_state > 0) ? 1 : 0;
        }
    }

    get_digital_samples(digital_samples, n);
 }

void V21_TX::modulate(const unsigned int *in_digital_samples, float *out_analog_samples, unsigned int n)
{
    while (n--) {
        *out_analog_samples++ = sin(phase);
        phase += (*in_digital_samples++ ? omega_mark : omega_space) * SAMPLING_PERIOD;

        // evita que phase cresça indefinidamente, o que causaria perda de precisão
        phase = remainder(phase, 2*std::numbers::pi);
    }
}
