#include "uart.hpp"

void UART_RX::put_samples(const unsigned int *buffer, unsigned int n)
{
    for (unsigned int i = 0; i < n; i++) {
        this->history.push_front(buffer[i]);
        if (this->history[0] == 0)
            this->low_count++;
        if (this->history[30] == 0)
            this->low_count--;

        switch (state) {
            case IDLE:
                if (low_count >= 25 && this->history[80] == 0) {
                    this->sample_count = 0; 
                    this->current_byte = 0;
                    this->bit_index = 0;
                    this->state = RECEIVING;
                }
                

            case RECEIVING:
                this->sample_count++;
                if (this->sample_count == (bit_index+1) * 160) {
                    // Amostra no meio do símbolo do bit atual
                    this->current_byte |= (this->history[0] << this->bit_index);
                    this->bit_index++;

                    if (this->bit_index == 8) {
                        this->state = STOP_BIT;
                        this->sample_count = 0;
                    }

                }
            break;

            case STOP_BIT:
                this->sample_count++;
                if (this->sample_count == 160) {
                    this->get_byte(this->current_byte);
                    this->state = IDLE;
                }
                break;
        }
    }
}


void UART_TX::put_byte(uint8_t byte)
{
    samples_mutex.lock();
    put_bit(0);  // start bit
    for (int i = 0; i < 8; i++) {
        put_bit(byte & 1);
        byte >>= 1;
    }
    put_bit(1);  // stop bit
    samples_mutex.unlock();
}

void UART_TX::get_samples(unsigned int *buffer, unsigned int n)
{
    samples_mutex.lock();
    std::vector<unsigned int>::size_type i = 0;
    while (!samples.empty() && i < n) {
        buffer[i++] = samples.front();
        samples.pop_front();
    }
    samples_mutex.unlock();

    while (i < n) {
        // idle
        buffer[i++] = 1;
    }
}

void UART_TX::put_bit(unsigned int bit)
{
    for (int i = 0; i < SAMPLES_PER_SYMBOL; i++) {
        samples.push_back(bit);
    }
}
