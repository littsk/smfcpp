#include <iostream>
#include <uart.hpp>
#include <crc.hpp>



static uint8_t get_status[12] = {0x01, 0x03, 0x00, 0x00, 0x00, 0x01, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00};
CRC::Mod<uint16_t> mod(0x8005, 0xffff, 0x0000, true, true);


int main(int argc, char * argv[]){
    std::vector<uint8_t> m_sensor_data(16, 0);

    auto uart_terminal = new Uart("/dev/ttyUSB0", 19200, 8, 'N', 0);
    CRC::crc_comlete(get_status, 6, mod);
    for(int i = 0; i < 8; ++ i){
        printf("%x\n", get_status[i]);
    }
    uart_terminal->send(get_status, 8);

    sleep(1);
    
    uart_terminal->receive(m_sensor_data.data(), 8);

    sleep(1);

    for(int i = 0; i < 16; ++i){
        std::cout << (int)m_sensor_data[i] << std::endl;
    }
    return 0;
}