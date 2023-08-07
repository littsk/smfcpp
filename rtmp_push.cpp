#include <iostream>
#include <opencv2/opencv.hpp>
#include <librtmp/rtmp.h>

#include <libavcodec/avcodec.h>
#include <libavutil/opt.h>
#include <libavutil/avutil.h>

using namespace std;
using namespace cv;

#define RTMP_PUSH_URL "rtmp://127.0.0.1/live/test"

int main()
{
    // 打开 RTMP 服务器连接
    RTMP* rtmp = RTMP_Alloc();
    RTMP_Init(rtmp);
    if (!RTMP_SetupURL(rtmp, RTMP_PUSH_URL))
    {
        cout << "RTMP_SetupURL failed." << endl;
        return -1;
    }
    RTMP_EnableWrite(rtmp);

    if (!RTMP_Connect(rtmp, NULL))
    {
        cout << "RTMP_Connect failed." << endl;
        return -1;
    }
    if (!RTMP_ConnectStream(rtmp, 0))
    {
        cout << "RTMP_ConnectStream failed." << endl;
        return -1;
    }

    // 打开本地摄像头
    VideoCapture capture(0);
    if (!capture.isOpened())
    {
        cout << "Failed to open camera." << endl;
        return -1;
    }

    // 设置视频编码参数
    int width = capture.get(CV_CAP_PROP_FRAME_WIDTH);
    int height = capture.get(CV_CAP_PROP_FRAME_HEIGHT);
    int fps = 25;
    int bitrate = 1000000;
    AVCodecID codec_id = AV_CODEC_ID_H264;

    // 创建编码器并打开编码器上下文
    AVCodec* codec = avcodec_find_encoder(codec_id);
    if (!codec)
    {
        cout << "Failed to find encoder." << endl;
        return -1;
    }
    AVCodecContext* codec_ctx = avcodec_alloc_context3(codec);
    codec_ctx->codec_id = codec_id;
    codec_ctx->width = width;
    codec_ctx->height = height;
    codec_ctx->time_base = { 1, fps };
    codec_ctx->gop_size = fps;
    codec_ctx->bit_rate = bitrate;
    codec_ctx->pix_fmt = AV_PIX_FMT_YUV420P;
    av_opt_set(codec_ctx->priv_data, "preset", "fast", 0);
    avcodec_open2(codec_ctx, codec, NULL);

    // 分配视频帧和缓存空间
    AVFrame* frame = av_frame_alloc();
    frame->width = width;
    frame->height = height;
    frame->format = AV_PIX_FMT_YUV420P;
    av_frame_get_buffer(frame, 32);

    // 编码并推流
    Mat img, yuv;
    while (capture.read(img))
    {
        // 转换为 YUV 格式
        cvtColor(img, yuv, COLOR_BGR2YUV_I420);
        frame->data[0] = yuv.data;
        frame->data[1] = yuv.data + width * height;
        frame->data[2] = yuv.data + width * height * 5 / 4;

        // 设置时间戳并编码
        frame->pts = av_rescale_q(frame->pts, codec_ctx->time_base, codec_ctx->time_base);
        int ret = avcodec_send_frame(codec_ctx, frame);
        if (ret < 0)
        {
            cout << "avcodec_send_frame failed." << endl;
            break;
                }

    // 推送编码后的数据
    AVPacket pkt;
    av_init_packet(&pkt);
    while (avcodec_receive_packet(codec_ctx, &pkt) == 0)
    {
        pkt.stream_index = 0;
        pkt.pts = av_rescale_q(pkt.pts, codec_ctx->time_base, rtmp->streams[0]->time_base);
        pkt.dts = av_rescale_q(pkt.dts, codec_ctx->time_base, rtmp->streams[0]->time_base);
        pkt.duration = av_rescale_q(pkt.duration, codec_ctx->time_base, rtmp->streams[0]->time_base);
        RTMP_SendPacket(rtmp, &pkt, 1);
        av_packet_unref(&pkt);
    }

    // 检查键盘输入，按 Q 键退出
    if (waitKey(1) == 'q')
    {
        break;
    }
}

// 释放资源
avcodec_close(codec_ctx);
avcodec_free_context(&codec_ctx);
av_frame_free(&frame);
RTMP_Close(rtmp);
RTMP_Free(rtmp);

return 0;
}


