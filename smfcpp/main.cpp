#include <iostream>
#include <chrono>
#include <functional>

#include <unistd.h>

#include "node.hpp"
#include "executor.hpp"


void f(){
    printf("hello, world\n");
}

void f2(){
    printf("fuck, world\n");
}

bool check(){
    sleep(1);
    return true;
}

void recv_cb(){
    printf("recv\n");
}
int main(int argc, char * argv[]){
    auto node = smfcpp::Node::make_shared("my_node");
    // std::function<void()> func = [](){ printf("hello, world\n"); };
    auto timer1 = node->create_timer(std::chrono::seconds(1), f);
    auto timer2 = node->create_timer(std::chrono::seconds(1), f2);
    auto recver = node->create_recver(check, recv_cb);
    std::cout<<node->get_name()<<std::endl;
    smfcpp::SingleThreadedExecutor exec;
    exec.add_node(node);
    exec.spin();

    return 0;
}