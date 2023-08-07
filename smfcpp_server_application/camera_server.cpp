#include "sensor_server.hpp"

#include <executor.hpp>

int main(int argc, char * argv[])
{
    auto camera_server = smfcpp::CameraServer::make_shared(
        "camera_server", 10001);
    camera_server->run();
    printf("hello, world\n");

    smfcpp::SingleThreadedExecutor exec;
    exec.add_node(camera_server);
    exec.spin();
    
    return 0;
}