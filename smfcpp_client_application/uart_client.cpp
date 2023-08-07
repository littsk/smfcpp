#include "sensor.hpp"

#include <executor.hpp>


int main(int argc, char * argv[])
{
    auto uart_client = smfcpp::UartClient::make_shared(
        "uart_client", "127.0.0.1", 9000, "/dev/ttyUSB0", 30);
        
    smfcpp::SingleThreadedExecutor exec;
    exec.add_node(uart_client);
    exec.spin();
}