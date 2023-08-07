#include <iostream>
#include "tcp.hpp"
#include "executor.hpp"


int main(int argc, char * argv[])
{
    auto tcp_server = smfcpp::tcp::Server::make_shared("my_tcp_server", 8000);

    tcp_server->run();

    smfcpp::SingleThreadedExecutor exec;
    exec.add_node(tcp_server);
    exec.spin();

    return 0;
}