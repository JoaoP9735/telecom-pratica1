#ifndef V21_HPP
#define V21_HPP

#include <functional>
#include <deque>
#include "config.hpp"
#include <math.h>

class V21_RX
{
public:
    V21_RX(float omega_m, float omega_s, std::function<void(const unsigned int *, unsigned int)> get_digital_samples);
    void demodulate(const float *in_analog_samples, unsigned int n);
private:
    float omega_m = 2 * M_PI * 1650;
    float omega_s = 2 * M_PI * 1850;
    
    float lp_num[3];
    float lp_den[3];

    std::deque<float> sample_buffer;
    float vspace_r_buffer, vspace_i_buffer;
    float vmark_r_buffer, vmark_i_buffer;
    float buffer_decisao[2];
    float buffer_filtrada[2];
    float R;
    float fs;
    float T;
    int L; 
    enum {
        IDLE,
        CARRIER
    } state;
    unsigned int low_state;

    std::function<void(const unsigned int *, unsigned int)> get_digital_samples;
};

class V21_TX
{
public:
    V21_TX(float omega_m, float omega_s) :omega_m(omega_m),omega_s(omega_s),phase(0.f) {};
    void modulate(const unsigned int *in_digital_samples, float *out_analog_samples, unsigned int n);
private:
    float omega_m, omega_s;
    float phase;
};

#endif