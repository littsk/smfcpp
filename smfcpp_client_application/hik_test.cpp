#include <string.h>
#include <stdio.h>

#include <unistd.h>

#include <iostream>

#include <opencv2/opencv.hpp>

#include <hik/HCNetSDK.h>

#define  HPR_OK 0
#define  HPR_ERROR -1

#include <time.h>
using namespace std;

#include <iostream>
#include <cstdlib>
#include <cstring>
#include <arpa/inet.h>
#include <netinet/ip_icmp.h>
#include <sys/socket.h>

#include <cstdlib>

#include <fstream>
#include <regex>
#include <vector>

#include <unistd.h>
#include <arpa/inet.h>
#include <netdb.h>

struct HikCamera{
    int user_id;
    std::string ip;
    NET_DVR_DEVICEINFO_V40 device_info;
};

int login(std::string const & ip, HikCamera & hik_camera){
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
        std::cout << "Login error, " << NET_DVR_GetLastError() << ". " << ip << " may not a camera" << std::endl;
        return -1;
    }

    std::cout << "succ" << " " << ip << std::endl;

    hik_camera.ip = ip;
    hik_camera.device_info = struDeviceInfoV40;
    hik_camera.user_id = lUserID;

    return 0;
}

std::vector<HikCamera> get_all_login_device(std::vector<std::string> const & candidate_ips){

    std::vector<HikCamera> hik_cameras;
    for(std::string const & ip : candidate_ips){
        HikCamera hik_camera;
        if(login(ip, hik_camera) == 0){
            hik_cameras.push_back(hik_camera);
        }
    }

    std::cout << "find " << hik_cameras.size() << " cameras." << std::endl;

    return hik_cameras;
}

std::vector<std::string> get_candidate_ips(std::string host_ip, int subnet_mask_num) {
    std::cout << "camera detecting ..." << std::endl;

    std::vector<std::string> candidate_ips;
    std::string command = "nmap -sn --min-rtt-timeout 500ms " + host_ip + "/" + to_string(subnet_mask_num); // need change
    FILE * pipe = popen(command.c_str(), "r");
    char buffer[128];
    if(!pipe){
        perror("pipe");
        std::cerr << "pipe error" << std::endl;
    }

    while(fgets(buffer, sizeof(buffer), pipe) != nullptr){
        std::string tmp = buffer;;
        std::regex ipregex(R"(\b(\d{1,3}\.){3}\d{1,3}\b)");
        std::smatch match;
        std::regex_search(tmp, match, ipregex);
        std::cout << match.str() << std::endl;
        if(!match.str().empty()){
            candidate_ips.push_back(match.str());
        }
    }
    pclose(pipe);

    return candidate_ips;
}

int main(){
    NET_DVR_Init();

        long lUserID;
    //login
    NET_DVR_USER_LOGIN_INFO struLoginInfo = {0};
    NET_DVR_DEVICEINFO_V40 struDeviceInfoV40 = {0};
    struLoginInfo.bUseAsynLogin = false;

    struLoginInfo.wPort = 8000;
    memcpy(struLoginInfo.sDeviceAddress, "192.168.124.85", NET_DVR_DEV_ADDRESS_MAX_LEN);
    memcpy(struLoginInfo.sUserName, "admin", NAME_LEN);
    memcpy(struLoginInfo.sPassword, "ys88889999", NAME_LEN);

    lUserID = NET_DVR_Login_V40(&struLoginInfo, &struDeviceInfoV40);

    if (lUserID < 0)
    {
        std::cout << "niema" << std::endl;
        std::cout << "Login error, " << NET_DVR_GetLastError() << ". " << struLoginInfo.sDeviceAddress << " may not a camera" << std::endl;
    }

    std::cout << "succ" << " " << struLoginInfo.sDeviceAddress << std::endl;

    NET_DVR_Logout_V30(lUserID);
    // std::vector<std::string> candidate_ips = get_candidate_ips("192.168.124.10", 24);

    // std::vector<HikCamera> hik_cameras = get_all_login_device(candidate_ips);

    NET_DVR_Cleanup();
}