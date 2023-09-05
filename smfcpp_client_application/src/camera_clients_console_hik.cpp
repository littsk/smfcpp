#include "camera_clients_console.hpp"

#include <hik/HCNetSDK.h>

#include <regex>
#include <string>
#include <queue>

#include <yaml-cpp/yaml.h>


namespace smfcpp{

std::vector<CameraClient<CameraType::HikVision>::SharedPtr> get_all_device_by_login(
    std::vector<std::string> const & candidate_ips,
    std::vector<std::string> const & rtmp_urls,
    std::string user_name, 
    std::string password, 
    unsigned short port)
{
    std::cout << user_name << password << port << std::endl;
    std::queue<std::string> rtmp_urls_que;
    for(auto const & url : rtmp_urls){
        rtmp_urls_que.push(url);
    }
    std::vector<CameraClient<CameraType::HikVision>::SharedPtr> hik_cameras;
    for(std::string const & ip : candidate_ips){
        CameraClient<CameraType::HikVision>::SharedPtr hik_camera;
        if(!rtmp_urls_que.empty()){
            hik_camera = CameraClient<CameraType::HikVision>::make_shared(
                "camera" + ip, rtmp_urls_que.front());
        }
        else{
            std::cerr << "rtmp_urls is not enough, please get more and fill them in to yaml" << std::endl;
        }

        CameraConfig camera_config;
        camera_config.ip = ip;
        camera_config.user_name = user_name;
        camera_config.password = password;
        camera_config.port = port;
        if(hik_camera->init(camera_config)){
            rtmp_urls_que.pop();
            hik_cameras.push_back(hik_camera);
        }
    }

    std::cout << "find " << hik_cameras.size() << " cameras." << std::endl;
    return hik_cameras;
}  

std::vector<std::string> get_candidate_ips(std::string host_ip, int subnet_mask_num) {
    std::cout << "camera detecting ..." << std::endl;

    std::vector<std::string> candidate_ips;
    std::string command = "nmap -sn --min-rtt-timeout 500ms " + host_ip + "/" + std::to_string(subnet_mask_num); // need change
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

template<CameraType T>
CameraClientsConsole<T>::CameraClientsConsole(
    const std::string & name)
: Node(name)
{
    NET_DVR_Init();
    YAML::Node config = YAML::LoadFile("Deviceinfo.yaml");
    std::string host_ip = config["HostIP"].as<std::string>();
    std::string user_name = config["UserName"].as<std::string>();
    std::string password = config["Password"].as<std::string>();
    unsigned short port = config["Port"].as<unsigned short>();
    int subnet_mask_num = config["SubnetMaskNum"].as<int>();
    std::vector<std::string> rtmp_urls = config["RtmpUrls"].as<std::vector<std::string>>();

    std::vector<std::string> candidate_ips = get_candidate_ips(host_ip, subnet_mask_num);
    m_cameras = get_all_device_by_login(candidate_ips, rtmp_urls, user_name, password, port);
}

template<CameraType T>
CameraClientsConsole<T>::~CameraClientsConsole(){
    NET_DVR_Cleanup();
}

template<CameraType T>
void CameraClientsConsole<T>::run(){
    std::vector<std::thread> thds;
    for(auto item : m_cameras){
        thds.emplace_back(&CameraClient<T>::run, item);
    }

    for(auto & thd : thds){
        thd.join();
    }
}

REGISTER_CameraClientsConsole(CameraType::HikVision)


    
}
