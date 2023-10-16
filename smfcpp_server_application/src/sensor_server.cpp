#include "sensor_server.hpp"

#include <regex>

#include <stdio.h>

#include <unistd.h>
#include <fcntl.h>
#include <sys/stat.h>

#include <log.hpp>
#include <glog/logging.h>

using namespace smfcpp;

UartServer::UartServer(
    const std::string & name, 
    int port,
    const std::string & log_path)
: tcp::Server(name, port),
  m_log_path(log_path)
{}

UartServer::~UartServer()
{
}

void UartServer::recv_callback(
    std::vector<uint8_t> & data)
{
    std::string recv_str(data.begin(), data.end());
    if(recv_str.find("DeviceUpdate") != std::string::npos)
    {
        // TODO deal this situation
        return;
    }
    else if(recv_str.find("SensorID") != std::string::npos)
    {   
        std::regex pattern("FarmID-(\\d+)\\.SensorID-(\\d+)\\.(\\w+)\\.(.*)");
        std::smatch matches;

        std::regex_search(recv_str, matches, pattern);
        std::string farm_id = matches[1];
        std::string sensor_id = matches[2];
        std::string sensor_type = matches[3];
        std::string rest = matches[4];

        std::filesystem::path log_dir = m_log_path / std::filesystem::path(farm_id);

        if(!std::filesystem::exists(log_dir)){
            std::filesystem::create_directories(log_dir);
        }

        std::filesystem::path log_file_pth = log_dir / std::filesystem::path(sensor_id + "_" + sensor_type + ".log");

        std::shared_ptr<log> logfile = std::make_shared<log>(log_file_pth.c_str());
        logfile->write("%s\n", rest.c_str());
    }
    else{
        LOG(ERROR) << "unknow data frame";
    }
    
}

