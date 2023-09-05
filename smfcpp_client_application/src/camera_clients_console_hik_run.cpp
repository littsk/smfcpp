#include "camera_clients_console.hpp"

namespace smfcpp{

template<CameraType T>
void CameraClientsConsole<T>::run(){
    for(auto item : m_cameras){
        item->run();
    }
    
}

REGISTER_CameraClientsConsole(CameraType::HikVision)

}