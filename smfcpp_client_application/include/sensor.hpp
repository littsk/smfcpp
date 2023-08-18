#ifndef SMFCPP__SENSOR_HPP_
#define SMFCPP__SENSOR_HPP_

#include <tcp.hpp>
#include <uart.hpp>
#include <iostream>

#include <opencv2/opencv.hpp>
#include <opencv2/video/video.hpp>

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

template<CameraType type>
class CameraClient{
public:
    SMFCPP_SMART_PTR_DEFINITIONS(CameraClient)

    CameraClient(const std::string & name, 
        const char * outputUrl);

    virtual ~CameraClient();

    int run(const cv::Size size);


    // /**
    //  * \param delay means that if the time exceeds the specified duration, 
    //  *        then the system will give up sending the video.
    // */
    // void send_video(const std::string path, std::chrono::seconds delay);

private:
    struct Impl;
    std::unique_ptr<Impl> m_impl;

    std::timed_mutex m_mutex; //mutex for send video
    const char * m_outputUrl;
};

#define REGISTER_TEMPLATE(T) template class CameraClient<T>;

}

#endif
