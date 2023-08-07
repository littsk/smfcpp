#ifndef SMFCPP__SENSOR_SERVER_HPP_
#define SMFCPP__SENSOR_SERVER_HPP_

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
        const std::string & file_name = "../log/logfile");

    virtual ~UartServer();
    virtual void recv_callback(
        std::vector<uint8_t> & data) override;
    

private:
    log * logfile;
};

class CameraServer: public tcp::Server
{
public:
    SMFCPP_SMART_PTR_DEFINITIONS(CameraServer)
    CameraServer(
        const std::string & name, 
        int port,
        size_t max_files = MAX_FILES);

    virtual ~CameraServer() = default;

    virtual void recv_callback(
        std::vector<uint8_t> & data) override;

private:
    size_t m_max_files;
};

}

#endif