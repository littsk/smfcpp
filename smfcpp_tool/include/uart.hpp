#ifndef UART_HPP
#define UART_HPP

#include <iostream>
#include <stdexcept>
#include <vector>
#include <string>
#include <memory>
#include <chrono>
#include <thread>
#include <cstring>

#include <unistd.h>
#include <fcntl.h>
#include <signal.h>

#ifdef __APPLE__
#include <termios.h>
#elif defined(__linux__)
#include <termio.h>
#endif

class Uart{
public:
    class error : public std::runtime_error{
    public:
        error(const std::string & str):runtime_error(str){}
        virtual ~error() noexcept {}
    };

    Uart(const char * file_pth, int nSpeed, int nBits, char nEvent, int nStop);

    int get_fd() const;


    void send(const void* buf, size_t count);


    /**
     * @brief Receive data from the UART port.
     *
     * This function receives data from the UART port and stores it in the provided
     * buffer. It waits for a specified amount of time for data to be available on
     * the UART port.
     *
     * @param buf The buffer to store the received data.
     * @param count The number of bytes to receive.
     * @param timewait The time to wait for data as a duration (std::chrono::milliseconds).
     *
     * @throws Uart::error if there is an error during data reception.
     */
    void receive(
        uint8_t* buf, 
        size_t count, 
        std::chrono::milliseconds timewait = std::chrono::milliseconds(100));

    virtual ~Uart();

    static std::shared_ptr<Uart> 
    create_uart(
        std::string const & file_path, 
        uint32_t n_speed,
        uint32_t n_bits,
        char check_method,
        uint32_t n_stops);

private:
    int fd, baudrate;
    fd_set fd_sets;

    int open_port(const char * file_pth);
    int set_opt(int nSpeed, int nBits, char nEvent, int nStop);
};

#endif