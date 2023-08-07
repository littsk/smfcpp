#ifndef SMFCPP__SENSOR_HPP_
#define SMFCPP__SENSOR_HPP_

#include <tcp.hpp>
#include <uart.hpp>

#include <CameraApi.h>

#include <opencv2/opencv.hpp>
#include <opencv2/video/video.hpp>


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

class CameraClient: public tcp::Client{
public:
    SMFCPP_SMART_PTR_DEFINITIONS(CameraClient)

    CameraClient(const std::string & name, 
        const std::string & ip_address, 
        const int port);

    virtual ~CameraClient() = default;

    void run(const std::string video_save_path, 
        const int fps_span, 
        const cv::Size size);


    /**
     * \param delay means that if the time exceeds the specified duration, 
     *        then the system will give up sending the video.
    */
    void send_video(const std::string path, std::chrono::seconds delay);

private:
    int                     iCameraCounts = 1;   // how many cameras are linked to the mother board
    int                     iStatus = -1;        //
    tSdkCameraDevInfo       tCameraEnumList;     //
    int                     hCamera;             // handle of the camera
    tSdkCameraCapbility     tCapability;         // 设备描述信息
    tSdkFrameHead           sFrameInfo;
    BYTE*			        pbyBuffer;
    int                     channel = 3;         // the channel of image, usually be 3

    bool                    auto_expose = true;  // whether to auto tune the exposure time
    int                     exposure_time = 4000;// if auto_expose == false, it will work

    std::vector<uint8_t> img_data;

    std::timed_mutex m_mutex; //mutex for send video
};



}

#endif
