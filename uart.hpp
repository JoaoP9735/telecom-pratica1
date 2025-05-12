#ifndef UART_HPP
#define UART_HPP

#include <functional>
#include <deque>
#include <mutex>
#include <stdint.h>
#include "config.hpp"

class UART_RX
{
public:
    UART_RX(std::function<void(uint8_t)> get_byte) : get_byte(get_byte), bit_counter(0), state(IDLE),
                                                    sample_count(0), low_count(0), bit_index(0), current_byte(0){
                                                   for (int i = 0; i < 80; i++)
                                                    this->history.push_front(1);
                                                    }
    void put_samples(const unsigned int *buffer, unsigned int n);

private:
    std::function<void(uint8_t)> get_byte;
    unsigned int bit_counter;
    enum { IDLE, RECEIVING, STOP_BIT } state;
    unsigned int sample_count ;
    unsigned int low_count;
    unsigned int bit_index ;
    uint8_t current_byte ;
    std::deque<unsigned int> history;

};


class UART_TX
{
public:
    void put_byte(uint8_t byte);
    void get_samples(unsigned int *buffer, unsigned int n);
private:
    std::deque<unsigned int> samples;
    std::mutex samples_mutex;
    void put_bit(unsigned int bit);
};

#endif