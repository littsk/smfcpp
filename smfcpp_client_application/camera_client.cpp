#include "sensor.hpp"

int main(int argc, char * argv[])
{
    auto camera_client = smfcpp::CameraClient::make_shared(
        "camera_client", "127.0.0.1", 9001);
    camera_client->run("./tmp/surveillance", 100, cv::Size(480, 384));
}