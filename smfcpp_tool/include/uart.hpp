#ifndef UART_HPP
#define UART_HPP

#include <iostream>
#include <stdexcept>
#include <vector>
#include <string>
#include <signal.h>
#include <unistd.h>
#include <memory>

#ifdef __APPLE__
#include <termios.h>
#elif defined(__linux__)
#include <termio.h>
#endif

#include <string.h>

#include "fcntl.h"

class Uart{
public:
    class error : public std::runtime_error{
    public:
        error(const std::string & str):runtime_error(str){}
        virtual ~error() noexcept {}
    };

    Uart(const char * file_pth, int nSpeed, int nBits, char nEvent, int nStop);

    int get_fd() const;
    size_t send(const void *buf, size_t count);
    int receive(uint8_t * buf, size_t count, uint32_t timewait = 5000000);

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