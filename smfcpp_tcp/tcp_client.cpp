#include <iostream>
#include "tcp.hpp"
#include "executor.hpp"

void f(smfcpp::tcp::Client::SharedPtr client){
    std::string str("hello, world");
    std::vector<uint8_t> data(str.begin(), str.end());
    client->send_data(data);
}

int main(int argc, char * argv[])
{
    auto tcp_client = smfcpp::tcp::Client::make_shared("my_tcp_client", "127.0.0.1", 8000);
    tcp_client->connect_to_server();
    auto send_timer = tcp_client->create_timer(
        std::chrono::seconds(1), std::bind(f, std::ref(tcp_client)));

    smfcpp::SingleThreadedExecutor exec;
    exec.add_node(tcp_client);
    exec.spin();

    return 0;
}