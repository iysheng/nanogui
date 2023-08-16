/*
    sdl_gui/videoview.cpp -- Widget used to display images.

    The image view widget was contributed by Stefan Ivanov.

    Based on NanoGUI by Wenzel Jakob <wenzel@inf.ethz.ch>.
    Adaptation for SDL by Dalerank <dalerankn8@gmail.com>

    All rights reserved. Use of this source code is governed by a
    BSD-style license that can be found in the LICENSE.txt file.
*/

#include <nanogui/videoview.h>
#include <nanogui/renderpass.h>
#include <nanogui/shader.h>
#include <nanogui/texture.h>
#include <nanogui/screen.h>
#include <nanogui/opengl.h>
#include <nanogui_resources.h>
#include <nanogui/window.h>
#include <nanogui/theme.h>
#include <cmath>

#include <unistd.h>

#include <sys/ioctl.h>
#include <linux/sockios.h>
#include <net/route.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>

NAMESPACE_BEGIN(nanogui)

#if 1
/* 固定大小为 520 * 286 */
#define VIDEO_SHOW_FIXED_WIDTH    520
#define VIDEO_SHOW_FIXED_HEIGH    286

#define VIDEO_TRACK_FIXED_WIDTH    1920
#define VIDEO_TRACK_FIXED_HEIGH    1080

#define ERROR -1

//#define DEBUG_MPP_DECODER

#ifdef DEBUG_MPP_DECODER
void dump_file(char * prefix, char *data, int data_len)
{
    static int counts;
    char file_name[64] = {0};
    FILE * yuv_file_fd;

    printf("yuv raw data_len=%d", data_len);
    if (counts > 20)
        return;
    snprintf(file_name, 64, "/tmp/%s%d.yuv", prefix, counts++);

    yuv_file_fd = fopen(file_name, "w+b");
    fwrite(data, 1, data_len, yuv_file_fd);
    fclose(yuv_file_fd);
}
#endif

void _do_reopen_prepare(AVFrame* p_frame, AVFormatContext *p_avformat_context, AVCodecContext *p_avcodec_context)
{
    int value;
    char errbuf[128];
    printf("reopen prepare 555555555555555555\n");
    if (p_frame) {
        av_frame_free(&p_frame);
    }

    value = avcodec_close(p_avcodec_context);
    if (value) {
        av_strerror(value, errbuf, sizeof(errbuf));
        printf("Failed close av input:%d  %s\n", value, errbuf);
    }
    avformat_close_input(&p_avformat_context);
    avcodec_free_context(&p_avcodec_context);
    avformat_free_context(p_avformat_context);
    printf("reopen prepare 666666666666666666\n");
}

int VideoView::video_draw_handler(void *object)
{
    VideoView *p_video_obj = (VideoView *)object;
    AVFormatContext *p_avformat_context = NULL;
    AVCodecParameters *p_avcodec_parameter = NULL;
    AVCodecContext *p_avcodec_context = NULL;
    const AVCodec *p_avcodec = NULL;
    AVDictionary* options = NULL;
    int options_need_set = 0;
	int got_picture = 0;
	 static struct SwsContext *img_convert_ctx;

    AVPacket packet;
    AVFrame* p_frame = NULL;
    AVFrame* p_frame_rgb = NULL;
    int video_stream_index;
    int value;
    int ret;
    Vector2i sdl_rect;

    ///p_video_obj->mStatus = R_VIDEO_RUNNING;

    if (!strlen(p_video_obj->mSrcUrl)) {
        return -1;
    }

    char errbuf[128];
    printf("red say src_file=%s\n", p_video_obj->mSrcUrl);
re_open:
    if (options_need_set) {
        value = av_dict_set(&options, "rtsp_transport", "tcp", 0);
        /* 修改超时时间，单位是 ms */
        value = av_dict_set(&options, "timeout", "5000", 0);
        value = av_dict_set(&options, "buffer_size", "10240000", 0);
        if (value < 0) {
            printf("Failed av_dict_set %d\n", value);
            return -2;
        }
        options_need_set = 0;
    }

	avformat_network_init();
	av_register_all();

    if (!p_avformat_context)
	{
        p_avformat_context = avformat_alloc_context();
		printf("alloc context success\n");
	}
    if (!p_avformat_context) {
        printf("Failed avformat_alloc_context\n");
        return -1;
    }

    value = avformat_open_input(&p_avformat_context, p_video_obj->mSrcUrl, NULL, &options);

    if (value) {
        av_strerror(value, errbuf, sizeof(errbuf));
        printf("Red Failed open av input:%d  %s src=%s\n", value, errbuf, p_video_obj->mSrcUrl);
        sleep(1);
        goto re_open;
        //return -3;
    } else {
        printf("Open input success\n");
    }

    value = avformat_find_stream_info(p_avformat_context, NULL);
    if (value) {
        printf("Failed find stream info\n");
        return -4;
    }

    for (int i = 0; i < p_avformat_context->nb_streams; i++) { // find video stream posistion/index.
        if (p_avformat_context->streams[i]->codecpar->codec_type == AVMEDIA_TYPE_VIDEO) {
            video_stream_index = i;
            break;
        }
    }

    if (video_stream_index == -1) {
        printf("Failed get video stream\n");
        return -5;
    }

    p_avcodec_parameter = p_avformat_context->streams[video_stream_index]->codecpar;
	/* decoder */
    p_avcodec = avcodec_find_decoder(p_avcodec_parameter->codec_id);
    if (!p_avcodec) {
        printf("Failed get avcodec format\n");
        return -6;
    }

    p_avcodec_context = avcodec_alloc_context3(p_avcodec);
    if (!p_avcodec_context) {
        printf("Failed create avcodec context\n");
        return -7;
    }
    value = avcodec_parameters_to_context(p_avcodec_context, p_avcodec_parameter);
    if (value < 0) {
        printf("Failed init avcodec context\n");
        return -8;
    }

    if ((value = avcodec_open2(p_avcodec_context, p_avcodec, NULL)) < 0) {
        av_log(NULL, AV_LOG_ERROR, "Cannot open video decoder\n");
        return value;
    }
    av_dump_format(p_avformat_context, 0, p_video_obj->mSrcUrl, 0);

    p_frame = av_frame_alloc();
    p_frame_rgb = av_frame_alloc();

    if (!p_frame) {
        printf("Failed alloc AVFrame\n");
        return -7;
    }

	/* 格式转换，返回一个格式转换的上下文，错误的话返回为空 */
    img_convert_ctx = sws_getContext(p_avcodec_context->width, p_avcodec_context->height,
            p_avcodec_context->pix_fmt, VIDEO_SHOW_FIXED_WIDTH, VIDEO_SHOW_FIXED_HEIGH,
            AV_PIX_FMT_BGR32, SWS_BICUBIC, NULL, NULL, NULL);

    if (!p_video_obj->m_pixels)
        p_video_obj->m_pixels = (uint8_t *)malloc(4 * VIDEO_SHOW_FIXED_WIDTH * VIDEO_SHOW_FIXED_HEIGH * 8);

    if (!p_video_obj->m_pixels) {
        printf("Failed malloc memory for mpp\n");
        return -12;
    } else {
        printf("malloc memory 4 mpp size=%.1f\n", 4 * VIDEO_SHOW_FIXED_WIDTH * VIDEO_SHOW_FIXED_HEIGH * 8);
    }

    avpicture_fill((AVPicture *)p_frame_rgb, p_video_obj->m_pixels, AV_PIX_FMT_BGR32,
            VIDEO_SHOW_FIXED_WIDTH, VIDEO_SHOW_FIXED_HEIGH);

just_draw:
    while (1) {
        ret = av_read_frame(p_avformat_context, &packet);
#ifdef DEBUG_MPP_DECODER
        /*
         * TODO dump av_paket to file
         * */
        dump_file("hikraw", (char *)packet.data, packet.size);
#endif
        if (ret < 0) {
            printf("Failed get frame 00000000000 %d\n", p_video_obj->m_no_frame_counts++);
            avcodec_send_packet(p_avcodec_context, NULL);
            if (p_video_obj->m_no_frame_counts < 100)
                continue;
            else {
                p_video_obj->m_no_frame_counts = 0;
                p_video_obj->mStatus = R_VIDEO_UNINITLED;
                _do_reopen_prepare(p_frame, p_avformat_context, p_avcodec_context);
                p_frame = NULL;
                p_avformat_context = NULL;
                p_avcodec_context = NULL;
                /* 需要重新使能设置 options
                 * 因为 avformat_open_input 成功后会释放 otions 的相关资源
                 * */
                options_need_set = 1;
                goto re_open;
            }
        } else if (packet.stream_index != video_stream_index) {
            printf("No video frame 11111111111\n");
        } else {
            p_video_obj->m_no_frame_counts = 0;
            if (p_video_obj->mStatus != R_VIDEO_RUNNING)
                p_video_obj->mStatus = R_VIDEO_INITLED;

            ret = avcodec_decode_video2(p_avcodec_context, p_frame, &got_picture, &packet);

            if (ret < 0) {
                printf("decode error.\n");
                continue;
            }

            if (got_picture) {
                sws_scale(img_convert_ctx,
                        (uint8_t const * const *) p_frame->data,
                        p_frame->linesize, 0, p_avcodec_context->height, p_frame_rgb->data,
                        p_frame_rgb->linesize);
            }
        }
        av_packet_unref(&packet);
    }
exit:
    /* 标记状态为未初始化 */
    p_video_obj->mStatus = R_VIDEO_UNINITLED;
    if (p_frame) {
        av_frame_free(&p_frame);
    }

    /* 释放内存 */
    free(p_video_obj->m_pixels);
#ifdef LED3000_MPP_H265
    free(p_video_obj->m_crop4h265);
#endif

    value = avcodec_close(p_avcodec_context);
    if (value) {
        av_strerror(value, errbuf, sizeof(errbuf));
        printf("Failed open av input:%d  %s\n", value, errbuf);
    }
    avformat_close_input(&p_avformat_context);
    avcodec_free_context(&p_avcodec_context);
    avformat_free_context(p_avformat_context);

    return 0;
}

VideoView::VideoView(Widget* parent): ImageView(parent), m_texture(nullptr), m_pixels(nullptr), m_crop4h265(nullptr),
    m_thread(nullptr), mSrcUrl("rtsp://admin:jariled123@192.168.91.150"), m_no_frame_counts(0)
{
    Window * wnd = parent->window();
    Screen* screen = dynamic_cast<Screen*>(wnd->parent());
    assert(screen);
    if (!m_texture) {
        /* 默认创建一个 window size - 20 大小的窗口 */
        m_texture = new Texture(
            Texture::PixelFormat::RGBA,
            Texture::ComponentFormat::UInt8,
            m_size,
            Texture::InterpolationMode::Trilinear,
            Texture::InterpolationMode::Nearest);
    }

    m_thread = new std::thread(VideoView::video_draw_handler, this);
    m_thread->detach();
}

VideoView::~VideoView() {}

/* 图像绘制函数 */
void VideoView::draw(NVGcontext *ctx)
{
    if (mStatus == R_VIDEO_INITLED) {
        if (m_texture) {
            if (m_fixed_size != Vector2i(0)) {
                m_texture->resize(m_fixed_size);
            }
            m_texture->upload(m_pixels);
            ImageView::set_image(m_texture);
            mStatus = R_VIDEO_RUNNING;
        }
    } else if (mStatus == R_VIDEO_RUNNING) {
        m_texture->upload(m_pixels);
    }
    ImageView::draw(ctx);
}

bool VideoView::mouse_button_event(const Vector2i &p, int button, bool down, int modifiers)
{
    return true;
}
#endif
NAMESPACE_END(nanogui)
