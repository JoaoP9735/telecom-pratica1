#include "uart.hpp"

void UART_RX::put_samples(const unsigned int *buffer, unsigned int n)
{
    static enum State { IDLE, START_BIT, WAIT, RECEIVING, STOP_BIT } state = IDLE;
    static unsigned int sample_count = 0;
    static unsigned int bit_index = 0;
    static unsigned int wait_50 = 0;
    static uint8_t current_byte = 0;
    static std::deque<unsigned int> history;

    for (unsigned int i = 0; i < n; ++i) {
        unsigned int sample = buffer[i];

        // Armazena histórico das últimas 30 amostras
        history.push_back(sample);
        if (history.size() > 30) {
            history.pop_front();
        }

        switch (state) {
            case IDLE:
                // Espera pelo início do start bit
                if (sample == 0 && history.size() == 30) {
                    int low_count = 0;
                    for (unsigned int s : history) {
                        if (s == 0) low_count++;
                    }

                    if (low_count >= 25 && history.front() == 0) {
                        // Detecção de start bit válida
                        sample_count = 0;
                        bit_index = 0;
                        current_byte = 0;
                        wait_50 = 0;
                        state = WAIT;
                    }
                }
                break;

            case WAIT:
                wait_50++;
                if(wait_50 == 50){
                    sample_count = 0;
                    bit_index = 0;
                    current_byte = 0;
                    state = RECEIVING;
                }
            case RECEIVING:
                sample_count++;
                if (sample_count == (bit_index + 1) * 160) {
                    // Amostra no meio do símbolo do bit atual
                    current_byte |= (sample << bit_index);
                    bit_index++;

                    if (bit_index == 8) {
                        state = STOP_BIT;
                    }
                }
                break;

            case STOP_BIT:
                sample_count++;
                if (sample_count == 9 * 160) {
                    // Ignora valor do stop bit (assume válido)
                    get_byte(current_byte);
                    state = IDLE;
                    history.clear();
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
