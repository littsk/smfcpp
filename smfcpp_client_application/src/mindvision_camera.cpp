#include "sensor.hpp"
#include <CameraApi.h>
#include <opencv2/video/video.hpp>
#include <opencv2/opencv.hpp>
namespace smfcpp{

template<CameraType T>
CameraClient<T>::~CameraClient() = default;

// template<CameraType T>
// bool CameraClient<T>::is_open(){
//     return m_is_open;
// }

template<CameraType T>
struct CameraClient<T>::Impl {
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
};

template<CameraType T>
CameraClient<T>::CameraClient(const std::string & name, 
    const std::string & outputUrl)
: m_outputUrl(outputUrl),
  m_impl(new CameraClient<T>::Impl())
{
    CameraSdkInit(1);

    // 枚举设备，并建立设备列表
    m_impl->iStatus = CameraEnumerateDevice(&m_impl->tCameraEnumList, &m_impl->iCameraCounts);
    printf("Enumerate state = %d\n", m_impl->iStatus);
    printf("Camera count = %d\n", m_impl->iCameraCounts); //specify the number of camera that are found
    if(m_impl->iCameraCounts == 0){
        std::cerr<<"There is no camera detected!"<<std::endl;
        return;
    }

    // 相机初始化。初始化成功后，才能调用任何其他相机相关的操作接口
    m_impl->iStatus = CameraInit(&m_impl->tCameraEnumList,-1,-1,&m_impl->hCamera);
    // 初始化失败
    printf("Init state = %d\n", m_impl->iStatus);
    if (m_impl->iStatus != CAMERA_STATUS_SUCCESS) {
        std::cerr<<"Init failed!"<<std::endl;
        return;
    }


    // 获得相机的特性描述结构体。该结构体中包含了相机可设置的各种参数的范围信息。决定了相关函数的参数
    CameraGetCapability(m_impl->hCamera,&m_impl->tCapability);

    // 让SDK进入工作模式，开始接收来自相机发送的图像数据。如果当前相机是触发模式，则需要接收到触发帧以后才会更新图像。    
    CameraPlay(m_impl->hCamera);

    /*  
    其他的相机参数设置
        例如 CameraSetExposureTime   CameraGetExposureTime  设置/读取曝光时间
        CameraSetImageResolution  CameraGetImageResolution 设置/读取分辨率
        CameraSetGamma、CameraSetConrast、CameraSetGain等设置图像伽马、对比度、RGB数字增益等等。
    更多的参数的设置方法，参考MindVision_Demo。本例程只是为了演示如何将SDK中获取的图像，转成OpenCV的图像格式,以便调用OpenCV的图像处理函数进行后续开发
    */

    if(m_impl->tCapability.sIspCapacity.bMonoSensor){
        m_impl->channel=1;
        CameraSetIspOutFormat(m_impl->hCamera,CAMERA_MEDIA_TYPE_MONO8);
    }
    else{
        m_impl->channel=3;
        CameraSetIspOutFormat(m_impl->hCamera,CAMERA_MEDIA_TYPE_BGR8);
    }

    CameraSetAeState(m_impl->hCamera, m_impl->auto_expose);  // whether to set exposure time manually
    if(!m_impl->auto_expose && CameraSetExposureTime(m_impl->hCamera, m_impl->exposure_time) != CAMERA_STATUS_SUCCESS){
        printf("Expoture time set wrong!\n");
    }

    printf("Camera Catch Node Lauched!\n");
    m_impl->img_data.resize(m_impl->tCapability.sResolutionRange.iHeightMax * m_impl->tCapability.sResolutionRange.iWidthMax*3);
}

template<CameraType T>
int CameraClient<T>::run()
{
    const cv::Size size = cv::Size(512, 384);
    int width = static_cast<int>(size.width);
    int height = static_cast<int>(size.height);
    double fps = 30;

    avformat_network_init();
    AVFormatContext* m_octx = NULL;

    avformat_alloc_output_context2(&m_octx, NULL, "flv", m_outputUrl.c_str());
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

    av_dump_format(m_octx, 0, m_outputUrl.c_str(), 1);

    if (!(m_octx->oformat->flags & AVFMT_NOFILE)) {
        avio_open(&m_octx->pb, m_outputUrl.c_str(), AVIO_FLAG_WRITE);
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
        if(CameraGetImageBuffer(m_impl->hCamera,&m_impl->sFrameInfo,&m_impl->pbyBuffer,1000) == CAMERA_STATUS_SUCCESS){
            CameraImageProcess(m_impl->hCamera, m_impl->pbyBuffer, m_impl->img_data.data(), &m_impl->sFrameInfo);


            //although the argument's unit is different from the stamp that has unit of nanosecond, it could also tell the order.
            
            cv::Mat matImage(
                    cv::Size(m_impl->sFrameInfo.iWidth,m_impl->sFrameInfo.iHeight), 
                    m_impl->sFrameInfo.uiMediaType == CAMERA_MEDIA_TYPE_MONO8 ? CV_8UC1 : CV_8UC3,
                    m_impl->img_data.data()
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
            CameraReleaseImageBuffer(m_impl->hCamera, m_impl->pbyBuffer);
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

REGISTER_TEMPLATE(smfcpp::CameraType::MindVision)

}