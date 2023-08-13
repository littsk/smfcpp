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
    const char * outputUrl)
: m_outputUrl(outputUrl)
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

int CameraClient::run(const cv::Size size)
{
    int width = static_cast<int>(size.width);
    int height = static_cast<int>(size.height);
    double fps = 30;

    avformat_network_init();
    AVFormatContext* m_octx = NULL;

    avformat_alloc_output_context2(&m_octx, NULL, "flv", m_outputUrl);
    if (!m_octx) {
        std::cerr << "Could not create output context" << std::endl;
        return -1;
    }

    // Change to use libx264 directly
    AVCodec* codec = avcodec_find_encoder_by_name("libx264");
    if (!codec) {
        std::cerr << "Codec libx264 not found" << std::endl;
        return -1;
    }
    AVStream* stream = avformat_new_stream(m_octx, codec);

    // Set codec context parameters.
    AVCodecContext* m_ctx = avcodec_alloc_context3(codec);
    m_ctx->bit_rate = 400000;
    m_ctx->width = width;
    m_ctx->height = height;
    m_ctx->time_base = (AVRational){1, static_cast<int>(fps)};
    m_ctx->gop_size = 12;
    m_ctx->pix_fmt = AV_PIX_FMT_YUV420P;

    // Some formats want stream headers to be separate.
    if (m_octx->oformat->flags & AVFMT_GLOBALHEADER) {
        m_ctx->flags |= AV_CODEC_FLAG_GLOBAL_HEADER;
    }

    AVDictionary *opts = NULL;
    av_dict_set(&opts, "preset", "veryfast", 0);
    av_dict_set(&opts, "tune", "zerolatency", 0);

    avcodec_open2(m_ctx, codec, &opts);

    avcodec_parameters_from_context(stream->codecpar, m_ctx);

    av_dump_format(m_octx, 0, m_outputUrl, 1);

    if (!(m_octx->oformat->flags & AVFMT_NOFILE)) {
        avio_open(&m_octx->pb, m_outputUrl, AVIO_FLAG_WRITE);
    }

    if(avformat_write_header(m_octx, NULL) < 0){
        std::cout << "avformat write header error" << std::endl;
    }

    AVFrame* frame = av_frame_alloc();
    AVPacket * pkt = av_packet_alloc();
    int y_size = m_ctx->width * m_ctx->height;

    // Allocate image data buffer.
    uint8_t* picture_buf = (uint8_t *)av_malloc(y_size * 3 / 2);
    av_image_fill_arrays(frame->data, frame->linesize, picture_buf, m_ctx->pix_fmt, m_ctx->width, m_ctx->height, 1);

    frame->format = m_ctx->pix_fmt;
    frame->width = m_ctx->width;
    frame->height = m_ctx->height;

    SwsContext* sws_ctx = sws_getContext(m_ctx->width, m_ctx->height, AV_PIX_FMT_BGR24, m_ctx->width, m_ctx->height, AV_PIX_FMT_YUV420P, SWS_BICUBIC, NULL, NULL, NULL);
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
            cv::imshow("test", matImage);
            cv::waitKey(1000 / (fps + 3));

            const int stride[] = { static_cast<int>(matImage.step[0]) };
            sws_scale(sws_ctx, &matImage.data, stride, 0, m_ctx->height, frame->data, frame->linesize);

            frame->pts = av_gettime();

            avcodec_send_frame(m_ctx, frame);
            int ret = avcodec_receive_packet(m_ctx, pkt);
            if (ret == 0) {
                pkt->stream_index = stream->index;
                pkt->dts = pkt->pts;

                av_interleaved_write_frame(m_octx, pkt);
                av_packet_unref(pkt);
            } else {
                // handle the error
                // const char* error_msg = av_err2str(ret);
                std::cerr << "Error receiving packet" << std::endl;
            }

            //在成功调用CameraGetImageBuffer后，必须调用CameraReleaseImageBuffer来释放获得的buffer。
            //否则再次调用CameraGetImageBuffer时，程序将被挂起一直阻塞，直到其他线程中调用CameraReleaseImageBuffer来释放了buffer
            CameraReleaseImageBuffer(hCamera,pbyBuffer);
        }
    }

    av_write_trailer(m_octx);

    // Clean up.
    avio_closep(&m_octx->pb);
    avcodec_free_context(&m_ctx);
    av_frame_free(&frame);
    av_free(picture_buf);
    av_dict_free(&opts); // Free the options dictionary

    if (!(m_octx->oformat->flags & AVFMT_NOFILE)) {
        avio_closep(&m_octx->pb);
    }

    avformat_free_context(m_octx);

    return 0;
}

}