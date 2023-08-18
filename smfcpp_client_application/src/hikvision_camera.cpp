#include "sensor.hpp"

#include <hik/HCNetSDK.h>

namespace smfcpp{

template<CameraType T>
CameraClient<T>::~CameraClient() = default;

template<CameraType T>
struct CameraClient<T>::Impl{
    
};

template<CameraType T>
CameraClient<T>::CameraClient(
    const std::string & name, 
    const char * outputUrl)
: m_outputUrl(outputUrl)
{
}

template<CameraType T>
int CameraClient<T>::run(const cv::Size size){

    NET_DVR_Init();
    long lUserID;
    //login
    NET_DVR_USER_LOGIN_INFO struLoginInfo = {0};
    NET_DVR_DEVICEINFO_V40 struDeviceInfoV40 = {0};
    struLoginInfo.bUseAsynLogin = false;

    char ip_set[16][16] = {};
    unsigned int n_ips = 0;
    int enable_bind = false;
    NET_DVR_GetLocalIP(ip_set, &n_ips, &enable_bind);

    unsigned int err_number = NET_DVR_GetLastError();
    // printf("\n%d\n", err_number);

    cv::VideoCapture cap;
    cv::Mat matImage;
    for(int i = 0; i < n_ips; ++i){
        std::string ip_addr(ip_set[i], ip_set[i] + 16);
        std::string rtsp_addr = "rtsp://admin:ys88889999@" + ip_addr + ":554/h264/ch1/main/av_stream";
        std::cout << rtsp_addr << std::endl;
        cap = cv::VideoCapture(rtsp_addr);
        if(cap.isOpened() && cap.read(matImage)){
            break;
        }
    }

    /**
    *
    * 
    **/
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


    while(cap.isOpened()){
        bool succ = cap.read(matImage);
        if(!succ){
            std::cerr << "Error reading frame" << std::endl;
            break;
        }

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
    }

    //logout
    NET_DVR_Logout_V30(lUserID);
    NET_DVR_Cleanup();



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

REGISTER_TEMPLATE(CameraType::HikVision)


}