#include "sensor.hpp"

#include <hik/HCNetSDK.h>

#define  HPR_OK 0
#define  HPR_ERROR -1

namespace smfcpp{

template<CameraType T>
CameraClient<T>::~CameraClient() = default;

// template<CameraType T>
// bool CameraClient<T>::is_open(){
//     return m_is_open;
// }

template<CameraType T>
struct CameraClient<T>::Impl{
    bool init(std::string ip, std::string user_name, std::string password, unsigned short port){
        long lUserID;
        //login
        NET_DVR_USER_LOGIN_INFO struLoginInfo = {0};
        NET_DVR_DEVICEINFO_V40 struDeviceInfoV40 = {0};
        struLoginInfo.bUseAsynLogin = false;

        struLoginInfo.wPort = 8000;
        memcpy(struLoginInfo.sDeviceAddress, ip.c_str(), NET_DVR_DEV_ADDRESS_MAX_LEN);
        memcpy(struLoginInfo.sUserName, "admin", NAME_LEN);
        memcpy(struLoginInfo.sPassword, "ys88889999", NAME_LEN);

        lUserID = NET_DVR_Login_V40(&struLoginInfo, &struDeviceInfoV40);

        if (lUserID < 0)
        {
            std::cout << "niema" << std::endl;
            std::cout << "Login error, " << NET_DVR_GetLastError() << ". " << struLoginInfo.sDeviceAddress << " may not a camera" << std::endl;
            return 0;
        }

        std::cout << "succ" << " " << ip << std::endl;

        return 1;
    }
    
    int m_user_id;
    std::string m_ip;
    NET_DVR_DEVICEINFO_V40 m_device_info;
};

template<CameraType T>
CameraClient<T>::CameraClient(
    const std::string & name, 
    const char * outputUrl)
: m_outputUrl(outputUrl)
{
}

template<CameraType T>
bool CameraClient<T>::init(const CameraConfig & camera_config){
    return m_impl->init(camera_config.ip, camera_config.user_name, camera_config.password, camera_config.port);
}

REGISTER_TEMPLATE(CameraType::HikVision)
}