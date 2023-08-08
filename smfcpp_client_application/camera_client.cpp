#include "sensor.hpp"

int main(int argc, char * argv[])
{
    auto camera_client = smfcpp::CameraClient::make_shared(
        "camera_client", argv[1]);
    camera_client->run(cv::Size(512, 384));
}