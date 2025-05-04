#ifndef V21_HPP
#define V21_HPP

#include <functional>
#include "config.hpp"
#include <cmath.h>
class V21_RX {
public:
    V21_RX(float omega_mark, float omega_space,
           std::function<void(const unsigned int *, unsigned int)> get_digital_samples)
        : omega_mark(omega_mark),
          omega_space(omega_space),
          get_digital_samples(get_digital_samples),
          freq_mark(omega_mark / (2 * M_PI)),
          freq_space(omega_space / (2 * M_PI)) {}

    void demodulate(const float *in_analog_samples, unsigned int n);

private:
    float omega_mark, omega_space;
    std::function<void(const unsigned int *, unsigned int)> get_digital_samples;
    float freq_mark, freq_space;
    static constexpr float fs = 48000.0f;

    float phase_mark = 0.0f;
    float phase_space = 0.0f;
    float lp_filter_state = 0.0f;

    const float alpha = 0.05f;
};

class V21_TX
{
public:
    V21_TX(float omega_mark, float omega_space) : omega_mark(omega_mark), omega_space(omega_space), phase(0.f) {};
    void modulate(const unsigned int *in_digital_samples, float *out_analog_samples, unsigned int n);
private:
    float omega_mark, omega_space;
    float phase;
};

#endif
