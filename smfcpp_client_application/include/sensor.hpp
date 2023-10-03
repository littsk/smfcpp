#ifndef SMFCPP__SENSOR_HPP_
#define SMFCPP__SENSOR_HPP_

#include <iostream>
#include <string>
#include <vector>
#include <filesystem>

#include <tcp.hpp>
#include <uart.hpp>

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

struct UartConfig {
    std::string server_ip;
    unsigned short server_port;
    std::uint32_t collect_interval; // Collect interval time (seconds)

    std::filesystem::path file_path;
    uint32_t baud_rate;
    uint32_t n_bits;
    uint32_t n_stops;
    char check_event;
    
    // Static member function to load configuration from the device information file
    static UartConfig get_config_from_device_info();
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
