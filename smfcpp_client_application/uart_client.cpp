#include "sensor.hpp"

#include <executor.hpp>


int main(int argc, char * argv[])
{
    auto uart_client = smfcpp::UartClient::make_shared(
        "uart_client", "119.29.251.32", 9000, "/dev/ttyUSB0", 30);
    uart_client->connect_to_server();
    smfcpp::SingleThreadedExecutor exec;
    exec.add_node(uart_client);
    exec.spin();
}
