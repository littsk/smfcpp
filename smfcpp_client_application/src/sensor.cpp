#include "sensor.hpp"

#include <crc.hpp>

#include <sstream>
#include <iomanip>

#include <sys/stat.h>

static uint8_t air_acquire[8] = {0x02, 0x03, 0x00, 0x00, 0x00, 0x05, 0x00, 0x00};
static uint8_t soil_acquire[8] = {0x01, 0x03, 0x00, 0x00, 0x00, 0x04, 0x00, 0x00};
static uint8_t addr1to2[8]    = {0x01, 0x06, 0x07, 0xd0, 0x00, 0x02, 0x00, 0x00}; //给设备更换地址使用
CRC::Mod<uint16_t> mod(0x8005, 0xffff, 0x0000, true, true);

namespace smfcpp{

UartClient::UartClient(
    const std::string & name, 
    const std::string & ip_address, 
    const int port,
    const std::string & uart_file,
    const int collect_interval)
: tcp::Client(name, ip_address, port)
{
    m_uart_port = new Uart(uart_file.c_str(), 4800, 8, 'N', 1);
    collect_timer = this->create_timer(
        std::chrono::seconds(collect_interval), std::bind(&UartClient::collect_data, this));
}

UartClient::~UartClient(){
    delete m_uart_port;
}

void UartClient::collect_data(){
    std::vector<uint8_t> m_sensor_data;

    CRC::crc_comlete(air_acquire, 6, mod);
    CRC::crc_comlete(soil_acquire, 6, mod);

    // air data
    m_uart_port->send(air_acquire, 8);
    m_uart_port->receive(m_sensor_data.data(), 15);
    float hum = (int16_t)((m_sensor_data[3] << 8) | m_sensor_data[4]) / 10.0, 
          tem = (int16_t)((m_sensor_data[5] << 8) | (m_sensor_data[6])) / 10.0,
          co2 = (uint16_t)((m_sensor_data[7] << 8) | (m_sensor_data[8])),
          light = (uint32_t)((m_sensor_data[9] << 24) | (m_sensor_data[10] << 16) | (m_sensor_data[11] << 8) | (m_sensor_data[12]));
    std::stringstream air_ss;
    air_ss << "humidity:" << std::fixed << std::setprecision(2) << hum << "%,"
        << "temperature:" << std::fixed << std::setprecision(2) << tem << "centigrade,"
        << "co2:" << std::fixed << std::setprecision(2) << co2 << "ppm,"
        << "light:" << std::fixed << std::setprecision(2) << light << "lux";
    std::string air_str = air_ss.str();
    std::vector<uint8_t> air_data_vec(air_str.begin(), air_str.end());
    send_data(air_data_vec);     // send_data_to_server

    // soil_data
    m_uart_port->send(soil_acquire, 8);
    m_uart_port->receive(m_sensor_data.data(), 15);
    float soil_hum = (int16_t)((m_sensor_data[3] << 8) | m_sensor_data[4]) / 10.0, 
          soil_tem = (int16_t)((m_sensor_data[5] << 8) | (m_sensor_data[6])) / 10.0,
          soil_ec = (uint16_t)((m_sensor_data[7] << 8) | (m_sensor_data[8])),
          soil_ph = (uint16_t)((m_sensor_data[9] << 8) | (m_sensor_data[10])) / 10.0;
    std::stringstream soil_ss;
    soil_ss << "soil humidity:" << std::fixed << std::setprecision(2) << soil_hum << "%,"
        << "soil temperature:" << std::fixed << std::setprecision(2) << soil_tem << "centigrade,"
        << "soil electrical conductivity:" << std::fixed << std::setprecision(2) << soil_ec << "us/cm,"
        << "PH:" << std::fixed << std::setprecision(2) << soil_ph;
    std::string soil_str = soil_ss.str();
    std::vector<uint8_t> soil_data_vec(soil_str.begin(), soil_str.end());
    send_data(soil_data_vec); // send_data_to_server
}

CameraClient::CameraClient(const std::string & name, 
        const std::string & ip_address, 
        const int port)
: tcp::Client(name, ip_address, port)
{
    CameraSdkInit(1);

    // 枚举设备，并建立设备列表
    iStatus = CameraEnumerateDevice(&tCameraEnumList, &iCameraCounts);
    printf("Enumerate state = %d\n", iStatus);
    printf("Camera count = %d\n", iCameraCounts); //specify the number of camera that are found
    if(iCameraCounts == 0){
        std::cerr<<"There is no camera detected!"<<std::endl;
        return;
    }

    // 相机初始化。初始化成功后，才能调用任何其他相机相关的操作接口
    iStatus = CameraInit(&tCameraEnumList,-1,-1,&hCamera);
    // 初始化失败
    printf("Init state = %d\n", iStatus);
    if (iStatus != CAMERA_STATUS_SUCCESS) {
        std::cerr<<"Init failed!"<<std::endl;
        return;
    }


    // 获得相机的特性描述结构体。该结构体中包含了相机可设置的各种参数的范围信息。决定了相关函数的参数
    CameraGetCapability(hCamera,&tCapability);

    // 让SDK进入工作模式，开始接收来自相机发送的图像数据。如果当前相机是触发模式，则需要接收到触发帧以后才会更新图像。    
    CameraPlay(hCamera);

    /*  
    其他的相机参数设置
        例如 CameraSetExposureTime   CameraGetExposureTime  设置/读取曝光时间
        CameraSetImageResolution  CameraGetImageResolution 设置/读取分辨率
        CameraSetGamma、CameraSetConrast、CameraSetGain等设置图像伽马、对比度、RGB数字增益等等。
    更多的参数的设置方法，参考MindVision_Demo。本例程只是为了演示如何将SDK中获取的图像，转成OpenCV的图像格式,以便调用OpenCV的图像处理函数进行后续开发
    */

    if(tCapability.sIspCapacity.bMonoSensor){
        channel=1;
        CameraSetIspOutFormat(hCamera,CAMERA_MEDIA_TYPE_MONO8);
    }
    else{
        channel=3;
        CameraSetIspOutFormat(hCamera,CAMERA_MEDIA_TYPE_BGR8);
    }

    CameraSetAeState(hCamera, auto_expose);  // whether to set exposure time manually
    if(!auto_expose && CameraSetExposureTime(hCamera, exposure_time) != CAMERA_STATUS_SUCCESS){
        printf("Expoture time set wrong!\n");
    }

    printf("Camera Catch Node Lauched!\n");
    img_data.resize(tCapability.sResolutionRange.iHeightMax*tCapability.sResolutionRange.iWidthMax*3);
}

void CameraClient::run(const std::string video_save_path, 
    const int fps_span, 
    const cv::Size size)
{
    Client::connect_to_server();
    // fps_span, how long to save
    cv::VideoWriter output_video;
    int t = 0;
    while(1){
        int i_frames = 0;
        std::string str_time(24, 0);
        time_t timer;
        time(&timer); 
        strncpy(str_time.data(), asctime(localtime(&timer)), 24);
        std::string pth = video_save_path + " " + str_time + ".mp4";
        output_video.open(pth, cv::VideoWriter::fourcc('m', 'p', '4', 'v'), 30.0, size);
        while(1){   
            if(CameraGetImageBuffer(hCamera,&sFrameInfo,&pbyBuffer,1000) == CAMERA_STATUS_SUCCESS){
                CameraImageProcess(hCamera, pbyBuffer, img_data.data(), &sFrameInfo);


                //although the argument's unit is different from the stamp that has unit of nanosecond, it could also tell the order.
                
                cv::Mat matImage(
                        cv::Size(sFrameInfo.iWidth,sFrameInfo.iHeight), 
                        sFrameInfo.uiMediaType == CAMERA_MEDIA_TYPE_MONO8 ? CV_8UC1 : CV_8UC3,
                        img_data.data()
                        );
                cv::resize(matImage, matImage, size);
                output_video.write(matImage);
                cv::imshow("test", matImage);
                cv::waitKey(30);
                //在成功调用CameraGetImageBuffer后，必须调用CameraReleaseImageBuffer来释放获得的buffer。
                //否则再次调用CameraGetImageBuffer时，程序将被挂起一直阻塞，直到其他线程中调用CameraReleaseImageBuffer来释放了buffer
                CameraReleaseImageBuffer(hCamera,pbyBuffer);
            }
            if(++i_frames == fps_span){
                output_video.release();
                // this->send_video(pth.c_str());
                // cv::waitKey(30);
                std::thread send_thd(&CameraClient::send_video, this, pth);
                send_thd.detach();
                break;
            }
        }
    }
}

void CameraClient::send_video(const std::string path, 
    std::chrono::seconds delay)
{
    std::unique_lock<std::timed_mutex> lock(m_mutex, std::defer_lock);
    if(lock.try_lock_for(delay)){
        struct stat buf;
        int video_fd = open(path.c_str(), O_RDONLY);
        if(video_fd < 0){
            perror("open");
        }
        fstat(video_fd, &buf);
        uint32_t video_size = buf.st_size;
        std::vector<uint8_t> video_data;
        video_data.resize(video_size);
        read(video_fd, video_data.data(), video_size);
        this->send_data(video_data);
        close(video_fd);
    }
    remove(path.c_str());
}

}