#include "sensor.hpp"

#include <hik/HCNetSDK.h>

#include <cstdlib>

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
    int m_user_id;
    std::string m_ip;
    std::string m_user_name;
    std::string m_password;
    NET_DVR_DEVICEINFO_V40 m_device_info;

    bool init(std::string ip, std::string user_name, std::string password, unsigned short port){
        long lUserID;
        //login
        NET_DVR_USER_LOGIN_INFO struLoginInfo = {0};
        NET_DVR_DEVICEINFO_V40 struDeviceInfoV40 = {0};
        struLoginInfo.bUseAsynLogin = false;

        struLoginInfo.wPort = port;
        memcpy(struLoginInfo.sDeviceAddress, ip.c_str(), NET_DVR_DEV_ADDRESS_MAX_LEN);
        memcpy(struLoginInfo.sUserName, user_name.c_str(), NAME_LEN);
        memcpy(struLoginInfo.sPassword, password.c_str(), NAME_LEN);

        lUserID = NET_DVR_Login_V40(&struLoginInfo, &struDeviceInfoV40);

        if (lUserID < 0)
        {
            std::cout << "Login error, " << NET_DVR_GetLastError() << ". " << struLoginInfo.sDeviceAddress << " may not a camera" << std::endl;
            return false;
        }

        std::cout << "succ" << " " << ip << std::endl;
        this->m_user_id = lUserID;
        this->m_ip = ip;
        this->m_user_name = user_name;
        this->m_password = password;
        this->m_device_info = struDeviceInfoV40;
        return true;
    }
};

template<CameraType T>
CameraClient<T>::CameraClient(
    const std::string & name, 
    const std::string & outputUrl)
: m_outputUrl(outputUrl),
  m_impl(new CameraClient<T>::Impl())
{
}

template<CameraType T>
bool CameraClient<T>::init(const CameraConfig & camera_config){
    return m_impl->init(camera_config.ip, camera_config.user_name, camera_config.password, camera_config.port);
}

template<CameraType T>
int CameraClient<T>::run(){
    std::string rtsp_addr = "rtsp://" + m_impl->m_user_name + ":" + m_impl->m_password + "@" + m_impl->m_ip + ":554/h264/ch1/main/av_stream";
    std::string cmd = "./build/smfcpp_client_application/hik_push " + rtsp_addr + " " + m_outputUrl;

    std::cout << cmd << std::endl;
    while(std::system(cmd.c_str()) != 0){
        std::cerr << "camera: " + m_impl->m_ip + " opened error" << std::endl;
        sleep(10);
    }

    return 0;
}

REGISTER_TEMPLATE(CameraType::HikVision)
}