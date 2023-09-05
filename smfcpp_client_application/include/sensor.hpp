#ifndef SMFCPP__SENSOR_HPP_
#define SMFCPP__SENSOR_HPP_

#include <tcp.hpp>
#include <uart.hpp>

#include <iostream>
#include <string>
#include <vector>

extern "C" {
    #include <libavutil/time.h>
    #include <libavformat/avformat.h>
    #include <libavutil/timestamp.h> 
    #include <libavformat/avformat.h>    
    #include <libavutil/opt.h>
    #include <libavutil/imgutils.h>
    #include <libswscale/swscale.h>
}


namespace smfcpp{


class UartClient: public tcp::Client
{
public:
    SMFCPP_SMART_PTR_DEFINITIONS(UartClient)
    UartClient(
        const std::string & name, 
        const std::string & ip_address, 
        const int port,
        const std::string & uart_file, 
        const int collect_interval // time between two collect (second)
    );
    virtual ~UartClient();

    // collect data and send data
    void collect_data();

private:
    Uart * m_uart_port;
    smfcpp::TimerBase::SharedPtr collect_timer;
};


enum class CameraType {
    MindVision,
    HikVision
};


struct CameraConfig{
    // these for hikvision
    std::string ip;
    std::string user_name;
    std::string password;
    unsigned short port;

    // these for mindvision
};


template<CameraType type>
class CameraClient{
public:
    SMFCPP_SMART_PTR_DEFINITIONS(CameraClient)

    CameraClient(const std::string & name, 
        const std::string & outputUrl);

    virtual ~CameraClient();

    bool init(const CameraConfig & camera_config);

    int run();

    bool is_open();

    // /**
    //  * \param delay means that if the time exceeds the specified duration, 
    //  *        then the system will give up sending the video.
    // */
    // void send_video(const std::string path, std::chrono::seconds delay);

private:
    struct Impl;
    std::unique_ptr<Impl> m_impl;
    const std::string m_outputUrl;
    bool m_is_open = false;
};

#define REGISTER_TEMPLATE(T) template class CameraClient<T>;

}

#endif
