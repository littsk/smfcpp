#include "sensor_server.hpp"

#include <executor.hpp>

int main(int argc, char * argv[])
{
    auto uart_server = smfcpp::UartServer::make_shared(
        "uart_server", 10000, "./log/logfile");
    uart_server->run();
    
    smfcpp::SingleThreadedExecutor exec;
    exec.add_node(uart_server);
    exec.spin();
    
}