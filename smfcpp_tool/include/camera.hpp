#ifndef CAMERA_HPP
#define CAMERA_HPP
#include <thread>
#include <algorithm>

#include "CameraApi.h"
#include "opencv2/opencv.hpp"
#include "sys/stat.h"

#include <mutex>

#include "tcp.hpp"

class camera_sensor{
public:
    camera_sensor(const char * ip_address, const char * port){
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



        if((sockfd = socket(AF_INET, SOCK_STREAM, 0)) == -1){
            perror("socket");
        }
        struct sockaddr_in servaddr = {0};
        servaddr.sin_family = AF_INET;
        struct hostent * h;
        if((h = gethostbyname(ip_address)) == NULL){
            printf("host name error");
        }
        memcpy(&servaddr.sin_addr, h->h_addr, h->h_length);
        servaddr.sin_port = htons(atoi(port));
        if(connect(sockfd, (struct sockaddr *)&servaddr, sizeof(servaddr)) != 0){
            perror("connect");
        }

    }


    void record(const std::string video_path, const int fps_span, const cv::Size size){
        // fps_span, how long to save
        cv::VideoWriter output_video;
        int t = 0;
        while(1){
            int i_frames = 0;
            std::string str_time(24, 0);
            time_t timer;
            time(&timer); 
            strncpy(str_time.data(), asctime(localtime(&timer)), 24);
            std::string pth = video_path + " " + str_time + ".mp4";
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
                    std::thread send_thd(&camera_sensor::send_video, this, pth);
                    send_thd.detach();
                    break;
                }
            }
        }
    }

    void send_video(const std::string path){
        struct stat buf;
        int video_fd = open(path.c_str(), O_RDONLY);
        if(video_fd < 0){
            perror("open");
        }
        fstat(video_fd, &buf);
        uint32_t video_size = buf.st_size;
        tcp::head h(tcp::other, video_size);
        std::vector<uint8_t> video_data;
        video_data.resize(video_size);
        read(video_fd, video_data.data(), video_size);

        std::lock_guard<std::mutex> guard(this->fdmtx);
        h.send(this->sockfd);
        tcp::sendn(this->sockfd, video_data.data(), video_size);

        close(video_fd);
        remove(path.c_str());
    }

    void keep_alive(){
        tcp::head h(tcp::heart, 0);
        std::lock_guard<std::mutex> guard(this->fdmtx);
        h.send(this->sockfd);
    }

    ~camera_sensor(){
        close(this->sockfd);
    }

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

    int sockfd;
    std::mutex fdmtx;
};


#endif