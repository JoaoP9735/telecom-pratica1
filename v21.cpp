#include <math.h>
#include <numbers>
#include "v21.hpp"

void V21_RX::demodulate(const float *in_analog_samples, unsigned int n)
{
    unsigned int num_symbols = n / SAMPLES_PER_SYMBOL;
    unsigned int digital_samples[num_symbols];

    for (unsigned int symbol = 0; symbol < num_symbols; ++symbol) {
        float sum_mark = 0.0f;
        float sum_space = 0.0f;

        for (unsigned int i = 0; i < SAMPLES_PER_SYMBOL; ++i) {
            unsigned int index = symbol * SAMPLES_PER_SYMBOL + i;
            float sample = in_analog_samples[index];

            float ref_mark = sin(phase_mark);
            float ref_space = sin(phase_space);

            sum_mark += sample * ref_mark;
            sum_space += sample * ref_space;

            phase_mark += 2 * std::numbers::pi * freq_mark / fs;
            phase_space += 2 * std::numbers::pi * freq_space / fs;

            if (phase_mark > 2 * std::numbers::pi)
                phase_mark -= 2 * std::numbers::pi;
            if (phase_space > 2 * std::numbers::pi)
                phase_space -= 2 * std::numbers::pi;
        }

        float energy_mark = sum_mark * sum_mark;
        float energy_space = sum_space * sum_space;

        digital_samples[symbol] = (energy_mark > energy_space) ? 1 : 0;
    }

    get_digital_samples(digital_samples, num_symbols);
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
