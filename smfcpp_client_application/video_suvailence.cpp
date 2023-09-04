#include <camera_clients_console.hpp>
#include "hik/HCNetSDK.h"


int main(){
    smfcpp::CameraClientsConsole<smfcpp::CameraType::HikVision> console("test");
    console.run();
}