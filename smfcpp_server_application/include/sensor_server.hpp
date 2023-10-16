#ifndef SMFCPP__SENSOR_SERVER_HPP_
#define SMFCPP__SENSOR_SERVER_HPP_

#include <filesystem>

#include <tcp.hpp>
#include <log.hpp>

#define MAX_FILES 100 //最多存储多少个视频文件

namespace smfcpp 
{
class UartServer: public tcp::Server
{
public: 
    SMFCPP_SMART_PTR_DEFINITIONS(UartServer)
    UartServer(
        const std::string & name, 
        int port,
        const std::string & log_path);

    virtual ~UartServer() override;
    virtual void recv_callback(
        std::vector<uint8_t> & data) override;
    

private:
    std::filesystem::path m_log_path;
};

}

#endif