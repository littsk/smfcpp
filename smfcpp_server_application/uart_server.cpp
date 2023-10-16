#include "sensor_server.hpp"

#include <executor.hpp>

#include "config.h"

int main(int argc, char * argv[])
{
    auto uart_server = smfcpp::UartServer::make_shared(
        "uart_server", 9000, LOG_DIR);
    uart_server->run();
    
    smfcpp::SingleThreadedExecutor exec;
    exec.add_node(uart_server);
    exec.spin();
}