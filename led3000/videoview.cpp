/*
    sdl_gui/videoview.cpp -- Widget used to display images.

    The image view widget was contributed by Stefan Ivanov.

    Based on NanoGUI by Wenzel Jakob <wenzel@inf.ethz.ch>.
    Adaptation for SDL by Dalerank <dalerankn8@gmail.com>

    All rights reserved. Use of this source code is governed by a
    BSD-style license that can be found in the LICENSE.txt file.
*/

#include <videoview.h>
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

#include <led3000gui.h>
#include <PolyM/include/polym/Queue.hpp>

#include "MppDecode.h"
#include "im2d_api/im2d.hpp"
#include "RgaUtils.h"
#include "rga.h"

NAMESPACE_BEGIN(nanogui)

/* 固定大小为 520 * 286 */
#define VIDEO_SHOW_FIXED_WIDTH    520
#define VIDEO_SHOW_FIXED_HEIGH    286

#define VIDEO_TRACK_FIXED_WIDTH    1920
#define VIDEO_TRACK_FIXED_HEIGH    1080

#define SRC_FORMAT RK_FORMAT_YCbCr_420_SP
#define DST_FORMAT RK_FORMAT_RGBA_8888

#define ERROR -1

float mpp_get_bpp_from_format(int format) {
    float bpp = 0;

    switch (format) {
        case RK_FORMAT_Y4:
            bpp = 0.5;
            break;
        case RK_FORMAT_BPP1:
        case RK_FORMAT_BPP2:
        case RK_FORMAT_BPP4:
        case RK_FORMAT_BPP8:
        case RK_FORMAT_YCbCr_400:
            bpp = 1;
            break;
        case RK_FORMAT_YCbCr_420_SP:
        case RK_FORMAT_YCbCr_420_P:
        case RK_FORMAT_YCrCb_420_P:
        case RK_FORMAT_YCrCb_420_SP:
        /* yuyv */
        case RK_FORMAT_YVYU_420:
        case RK_FORMAT_VYUY_420:
        case RK_FORMAT_YUYV_420:
        case RK_FORMAT_UYVY_420:
            bpp = 1.5;
            break;
        case RK_FORMAT_RGB_565:
        case RK_FORMAT_RGBA_5551:
        case RK_FORMAT_RGBA_4444:
        case RK_FORMAT_BGR_565:
        case RK_FORMAT_BGRA_5551:
        case RK_FORMAT_BGRA_4444:
        case RK_FORMAT_YCbCr_422_SP:
        case RK_FORMAT_YCbCr_422_P:
        case RK_FORMAT_YCrCb_422_SP:
        case RK_FORMAT_YCrCb_422_P:
        /* yuyv */
        case RK_FORMAT_YVYU_422:
        case RK_FORMAT_VYUY_422:
        case RK_FORMAT_YUYV_422:
        case RK_FORMAT_UYVY_422:
            bpp = 2;
            break;
        /*RK encoder requires alignment of odd multiples of 256.*/
        /*Here bpp=2 guarantee to read complete data.*/
        case RK_FORMAT_YCbCr_420_SP_10B:
        case RK_FORMAT_YCrCb_420_SP_10B:
            bpp = 2;
            break;
        case RK_FORMAT_YCbCr_422_10b_SP:
        case RK_FORMAT_YCrCb_422_10b_SP:
            bpp = 2.5;
            break;
        case RK_FORMAT_BGR_888:
        case RK_FORMAT_RGB_888:
            bpp = 3;
            break;
        case RK_FORMAT_RGBA_8888:
        case RK_FORMAT_RGBX_8888:
        case RK_FORMAT_BGRA_8888:
        case RK_FORMAT_BGRX_8888:
            bpp = 4;
            break;
        default:
            printf("Is unsupport format now, please fix \n");
            return 0;
    }

    return bpp;
}

void deInit(MppPacket *packet, MppFrame *frame, MppCtx ctx, char *buf, MpiDecLoopData *data )
{
    if (packet) {
        mpp_packet_deinit(packet);
        packet = NULL;
    }

    if (frame) {
        mpp_frame_deinit(frame);
        frame = NULL;
    }

    if (ctx) {
        mpp_destroy(ctx);
        ctx = NULL;
    }


    if (buf) {
        mpp_free(buf);
        buf = NULL;
    }


    if (data->pkt_grp) {
        mpp_buffer_group_put(data->pkt_grp);
        data->pkt_grp = NULL;
    }

    if (data->frm_grp) {
        mpp_buffer_group_put(data->frm_grp);
        data->frm_grp = NULL;
    }

    if (data->fp_output) {
        fclose(data->fp_output);
        data->fp_output = NULL;
    }

    if (data->fp_input) {
        fclose(data->fp_input);
        data->fp_input = NULL;
    }
}

int mpp_hardware_init(MpiDecLoopData *data)
{
    MPP_RET ret         = MPP_OK;
    size_t file_size    = 0;

    // base flow context
    MppCtx ctx          = NULL;
    MppApi *mpi         = NULL;

    // input / output
    MppPacket packet    = NULL;
    MppFrame  frame     = NULL;

    MpiCmd mpi_cmd      = MPP_CMD_BASE;
    MppParam param      = NULL;
    RK_U32 need_split   = 1;
//    MppPollType timeout = 5;

    // paramter for resource malloc
    RK_U32 width        = 2560;
    RK_U32 height       = 1440;
    MppCodingType type  = MPP_VIDEO_CodingAVC;

    // resources
    char *buf           = NULL;
    size_t packet_size  = 8*1024;
    MppBuffer pkt_buf   = NULL;
    MppBuffer frm_buf   = NULL;


    printf("mpi_dec_test start\n");
    memset(data, 0, sizeof(MpiDecLoopData));

    // decoder demo
    ret = mpp_create(&ctx, &mpi);

    if (MPP_OK != ret) {
        printf("mpp_create failed\n");
        deInit(&packet, &frame, ctx, buf, data);
    }

    // NOTE: decoder split mode need to be set before init
    mpi_cmd = MPP_DEC_SET_PARSER_SPLIT_MODE;
    param = &need_split;
    ret = mpi->control(ctx, mpi_cmd, param);
    if (MPP_OK != ret) {
        printf("format mpi->control failed\n");
        deInit(&packet, &frame, ctx, buf, data);
    }

    mpi_cmd = MPP_SET_INPUT_BLOCK;
    param = &need_split;
    ret = mpi->control(ctx, mpi_cmd, param);
    if (MPP_OK != ret) {
        printf("mpi->control failed\n");
        deInit(&packet, &frame, ctx, buf, data);
    }

    ret = mpp_init(ctx, MPP_CTX_DEC, type);
    if (MPP_OK != ret) {
        printf("mpp_init failed\n");
        deInit(&packet, &frame, ctx, buf, data);
    }

#if 0
   mpi_cmd = MPP_DEC_SET_OUTPUT_FORMAT;
   MppFrameFormat rgba_format = MPP_FMT_ARGB8888;//MPP_FMT_YUV420SP;// MPP_FMT_YUV420SP;//MPP_FMT_RGBA8888;
    param = &rgba_format;
    ret = mpi->control(ctx, mpi_cmd, param);
    if (MPP_OK != ret) {
        printf("mpi->control failed aaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaa\n");
        deInit(&packet, &frame, ctx, buf, data);
    }
    else
    {
        printf("mpi->control success bbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbb\n");
    }
#endif

    data->ctx            = ctx;
    data->mpi            = mpi;
    data->eos            = 0;
    data->packet_size    = packet_size;
    data->frame          = frame;
    data->frame_count    = 0;

    return 0;
}

int mpp_decode_simple(MpiDecLoopData *data, AVPacket *av_packet, char *display_buffer)
{
    RK_U32 pkt_done = 0;
    RK_U32 pkt_eos  = 0;
    RK_U32 err_info = 0;
    MPP_RET ret = MPP_OK;
    MppCtx ctx  = data->ctx;
    MppApi *mpi = data->mpi;
    MppPacket packet = NULL;
    MppFrame  frame  = NULL;
    size_t read_size = 0;
    size_t packet_size = data->packet_size;
    RK_U32 h_stride = 0;
    RK_U32 v_stride = 0;
    RK_U32 buf_size;

    MppBuffer buffer    = NULL;

    rga_buffer_t src;
    rga_buffer_t dst;
    im_rect src_rect;
    im_rect dst_rect;

    ret = mpp_packet_init(&packet, av_packet->data, av_packet->size);
    mpp_packet_set_pts(packet, av_packet->pts);

    do {
        RK_S32 times = 5;
        // send the packet first if packet is not done
        if (!pkt_done) {
            ret = mpi->decode_put_packet(ctx, packet);
            if (MPP_OK == ret)
                pkt_done = 1;
            else
                printf("送入码流出错:%d\n", ret);
        }

        int abc=0, def_index= 0;
        char file_name[32]={0};
        FILE *yuv_file_fd;
        // then get all available frame and release
        do {
            RK_S32 get_frm = 0;
            RK_U32 frm_eos = 0;

            try_again:
            /* 循环等待解码流 */
            ret = mpi->decode_get_frame(ctx, &frame);
            if (MPP_ERR_TIMEOUT == ret) {
                if (times > 0) {
                    times--;
                    msleep(2);
                    goto try_again;
                }
                printf("decode_get_frame failed too much time\n");
            }
            if (MPP_OK != ret) {
                printf("decode_get_frame failed 这里不应该出错，等了很久了 ret %d\n", ret);
                break;
            }

            /* 如果解析到一帧数据 */
            if (frame) {
                /* 信息变换帧,这个一般不会 */
                if (mpp_frame_get_info_change(frame)) {
                    RK_U32 width = mpp_frame_get_width(frame);
                    RK_U32 height = mpp_frame_get_height(frame);
                    RK_U32 hor_stride = mpp_frame_get_hor_stride(frame);
                    RK_U32 ver_stride = mpp_frame_get_ver_stride(frame);
                    RK_U32 buf_size = mpp_frame_get_buf_size(frame);

                    printf("decode_get_frame get info changed found 不应该出现信息变换帧\n");
                    printf("decoder require buffer w:h [%d:%d] stride [%d:%d] buf_size %d",
                            width, height, hor_stride, ver_stride, buf_size);

                    ret = mpp_buffer_group_get_internal(&data->frm_grp, MPP_BUFFER_TYPE_ION);
                    if (ret) {
                        printf("get mpp buffer group  failed ret %d\n", ret);
                        break;
                    }
                    mpi->control(ctx, MPP_DEC_SET_EXT_BUF_GROUP, data->frm_grp);

                    mpi->control(ctx, MPP_DEC_SET_INFO_CHANGE_READY, NULL);
                } else {
                    err_info = mpp_frame_get_errinfo(frame) | mpp_frame_get_discard(frame);
                    if (err_info) {
                        printf("解码的帧出错了 decoder_get_frame get err info:%d discard:%d.\n",
                                mpp_frame_get_errinfo(frame), mpp_frame_get_discard(frame));
                    }
                    data->frame_count++;
                    /* 如果成功解码了一帧 */
                    /*  准备显示这个 frame */
                   if (!err_info){
                    /* 获取这个帧的 buffer 信息 */
                    buffer = mpp_frame_get_buffer(frame);
                    buf_size = mpp_frame_get_buf_size(frame);
                    h_stride = mpp_frame_get_hor_stride(frame);
                    v_stride = mpp_frame_get_ver_stride(frame);
                    RK_U8 * base = NULL;
                    /* 获取内存首地址 */
                    base = (RK_U8 *)mpp_buffer_get_ptr(buffer);
                    //printf("base=%p fmt=%d hstride=%d vstride=%d size=%d\n", base, mpp_frame_get_fmt(frame), h_stride, v_stride, buf_size);
                    ///////////////////////////////
                    memset(&src_rect, 0, sizeof(src_rect));
                    memset(&dst_rect, 0, sizeof(dst_rect));
                    memset(&src, 0, sizeof(src));
                    memset(&dst, 0, sizeof(dst));
                    /* RGA 模块封装源信息 */
                    src = wrapbuffer_virtualaddr(base, h_stride, v_stride, SRC_FORMAT);
                    /* RGA 模块封装目的信息 */
                    dst = wrapbuffer_virtualaddr(display_buffer, VIDEO_SHOW_FIXED_WIDTH, VIDEO_SHOW_FIXED_HEIGH, DST_FORMAT);
                    if(dst.width == 0) {
                        printf("%s, %s\n", __FUNCTION__, imStrError());
                        return ERROR;
                    }

                    /* 处理源和目的 rect 信息 */
            ret = (MPP_RET)imcheck(src, dst, src_rect, dst_rect);
            if (IM_STATUS_NOERROR != ret) {
                printf("%d, check error! %s", __LINE__, imStrError((IM_STATUS)ret));
                return -1;
            }

            /* 缩放之前标记编码格式 */
            src.format = RK_FORMAT_YCbCr_420_SP;
            dst.format = RK_FORMAT_RGBA_8888;
            ret = (MPP_RET)imresize(src, dst);
            //printf("resizing .... %s %p\n", imStrError(ret), display_buffer);
                   }
                }
                frm_eos = mpp_frame_get_eos(frame);
                /* 清零 frame 信息 */
                mpp_frame_deinit(&frame);
                frame = NULL;
                get_frm = 1;
            }

            // try get runtime frame memory usage
            if (data->frm_grp) {
                size_t usage = mpp_buffer_group_usage(data->frm_grp);
                if (usage > data->max_usage)
                    data->max_usage = usage;
            }

            // if last packet is send but last frame is not found continue
            if (pkt_eos && pkt_done && !frm_eos) {
                msleep(1);
                continue;
            }

            if (frm_eos) {
                printf("最后一帧退出 found last frame\n");
                break;
            }

            if (data->frame_num > 0 && data->frame_count >= data->frame_num) {
                data->eos = 1;
                break;
            }

            if (get_frm)
                continue;
            break;
        } while (1);

        if (data->frame_num > 0 && data->frame_count >= data->frame_num) {
            data->eos = 1;
            printf("reach max frame number %d\n", data->frame_count);
            break;
        }

        if (pkt_done)
            break;

        /*
         * why sleep here:
         * mpi->decode_put_packet will failed when packet in internal queue is
         * full,waiting the package is consumed .Usually hardware decode one
         * frame which resolution is 1080p needs 2 ms,so here we sleep 3ms
         * * is enough.
         */
        msleep(3);
    } while (1);
    mpp_packet_deinit(&packet);

    return ret;
}

void _do_reopen_prepare(AVFrame* p_frame, AVFormatContext *p_avformat_context, AVCodecContext *p_avcodec_context)
{
    int value;
    char errbuf[128];
    red_debug_lite("reopen prepare 555555555555555555\n");
    if (p_frame)
    {
        av_frame_free(&p_frame);
    }

    value = avcodec_close(p_avcodec_context);
    if (value)
    {
        av_strerror(value, errbuf, sizeof(errbuf));
        red_debug_lite("Failed close av input:%d  %s\n", value, errbuf);
    }
    avformat_close_input(&p_avformat_context);
    avcodec_free_context(&p_avcodec_context);
    avformat_free_context(p_avformat_context);
    red_debug_lite("reopen prepare 666666666666666666\n");
}

int VideoView::video_draw_handler(void *object)
{
    VideoView *p_video_obj = (VideoView *)object;
    AVFormatContext *p_avformat_context = NULL;
    AVCodecParameters *p_avcodec_parameter = NULL;
    AVCodecContext *p_avcodec_context = NULL;
    const AVCodec *p_avcodec = NULL;
    AVDictionary* options = NULL;
    int options_need_set = 1;

    enum AVPixelFormat src_fix_fmt;
    enum AVPixelFormat dst_fix_fmt;

    AVPacket packet;
    struct SwsContext* sws_clx = NULL;
    AVFrame* p_frame = NULL;
    int video_stream_index;
    int value;
    int ret;
    Vector2i sdl_rect;
    MpiDecLoopData mpp_data;

    ///p_video_obj->mStatus = R_VIDEO_RUNNING;

    if (!strlen(p_video_obj->mSrcUrl))
    {
        return -1;
    }

    char errbuf[128];
    red_debug_lite("src_file=%s\n", p_video_obj->mSrcUrl);
re_open:
    if (options_need_set)
    {
        value = av_dict_set(&options, "rtsp_transport", "udp", 0);
        /* 修改超时时间，单位是 ms */
        value = av_dict_set(&options, "timeout", "5000", 0);
        value = av_dict_set(&options, "buffer_size", "10240000", 0);
        if (value < 0)
        {
            red_debug_lite("Failed av_dict_set %d\n", value);
            return -2;
        }
        options_need_set = 0;
    }

    if (!p_avformat_context)
        p_avformat_context = avformat_alloc_context();
    if (!p_avformat_context)
    {
        red_debug_lite("Failed avformat_alloc_context\n");
        return -1;
    }

    value = avformat_open_input(&p_avformat_context, p_video_obj->mSrcUrl, NULL, &options);

    if (value)
    {
        av_strerror(value, errbuf, sizeof(errbuf));
        red_debug_lite("Failed open av input:%d  %s src=%s\n", value, errbuf, p_video_obj->mSrcUrl);
        sleep(1);
        goto re_open;
        //return -3;
    }
    else
    {
        red_debug_lite("Open input success\n");
    }

    value = avformat_find_stream_info(p_avformat_context, NULL);
    if (value)
    {
        red_debug_lite("Failed find stream info\n");
        return -4;
    }

    for (int i = 0; i < p_avformat_context->nb_streams; i++) // find video stream posistion/index.
    {
        if (p_avformat_context->streams[i]->codecpar->codec_type == AVMEDIA_TYPE_VIDEO)
        {
            video_stream_index = i;
            break;
        }
    }

    if (video_stream_index == -1)
    {
        red_debug_lite("Failed get video stream\n");
        return -5;
    }

    p_avcodec_parameter = p_avformat_context->streams[video_stream_index]->codecpar;
    p_avcodec = avcodec_find_decoder(p_avcodec_parameter->codec_id);
    if (!p_avcodec)
    {
        red_debug_lite("Failed get avcodec format\n");
        return -6;
    }

    p_avcodec_context = avcodec_alloc_context3(p_avcodec);
    if (!p_avcodec_context)
    {
        red_debug_lite("Failed create avcodec context\n");
        return -7;
    }
    value = avcodec_parameters_to_context(p_avcodec_context, p_avcodec_parameter);
    if (value < 0)
    {
        red_debug_lite("Failed init avcodec context\n");
        return -8;
    }

    if ((value = avcodec_open2(p_avcodec_context, p_avcodec, NULL)) < 0) {
        av_log(NULL, AV_LOG_ERROR, "Cannot open video decoder\n");
        return value;
    }
    av_dump_format(p_avformat_context, 0, p_video_obj->mSrcUrl, 0);

    p_frame = av_frame_alloc();

    if (!p_frame)
    {
        red_debug_lite("Failed alloc AVFrame\n");
        return -7;
    }

    Vector2i top_left = Vector2i(p_video_obj->pixel_to_pos(Vector2f(0.f, 0.f))),
             size     = Vector2i(p_video_obj->pixel_to_pos(Vector2f(p_video_obj->size())) - Vector2f(top_left));
    if (!p_video_obj->m_pixels)
        p_video_obj->m_pixels = (uint8_t *)malloc(10 * VIDEO_SHOW_FIXED_WIDTH * VIDEO_SHOW_FIXED_HEIGH * mpp_get_bpp_from_format(DST_FORMAT));
    if (!p_video_obj->m_pixels)
    {
        red_debug_lite("Failed malloc memory for mpp\n");
        return -12;
    }
    else
    {
        red_debug_lite("malloc memory 4 mpp size=%d\n", 10 * VIDEO_SHOW_FIXED_WIDTH * VIDEO_SHOW_FIXED_HEIGH * mpp_get_bpp_from_format(DST_FORMAT));
    }
    if (!mpp_hardware_init(&mpp_data))
    {
        red_debug_lite("success mpp_hardware_init\n");
    }

just_draw:
    while (1)
    {
        ret = av_read_frame(p_avformat_context, &packet);
        if (ret < 0)
        {
            red_debug_lite("Failed get frame 00000000000 %d\n", p_video_obj->m_no_frame_counts++);
            avcodec_send_packet(p_avcodec_context, NULL);
            if (p_video_obj->m_no_frame_counts < 100)
                continue;
            else
            {
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
        }
        else if (packet.stream_index != video_stream_index)
        {
            red_debug_lite("No video frame 11111111111\n");
        }
        else
        {
            p_video_obj->m_no_frame_counts = 0;
            mpp_decode_simple(&mpp_data, &packet, (char *)p_video_obj->m_pixels);
            if (p_video_obj->mStatus != R_VIDEO_RUNNING)
                p_video_obj->mStatus = R_VIDEO_INITLED;
        }
        av_packet_unref(&packet);
    }
exit:
    /* 标记状态为未初始化 */
    p_video_obj->mStatus = R_VIDEO_UNINITLED;
    if (p_frame)
    {
        av_frame_free(&p_frame);
    }

    /* 释放内存 */
    free(p_video_obj->m_pixels);

    value = avcodec_close(p_avcodec_context);
    if (value)
    {
        av_strerror(value, errbuf, sizeof(errbuf));
        red_debug_lite("Failed open av input:%d  %s\n", value, errbuf);
    }
    avformat_close_input(&p_avformat_context);
    avcodec_free_context(&p_avcodec_context);
    avformat_free_context(p_avformat_context);

    return 0;
}

VideoView::VideoView(Widget* parent):ImageView(parent), m_texture(nullptr), m_pixels(nullptr),
    m_thread(nullptr), mSrcUrl("rtsp://admin:jariled123@192.168.100.64"), m_no_frame_counts(0)
{
    Window * wnd = parent->window();
    int hh = wnd->theme()->m_window_header_height;
    Screen* screen = dynamic_cast<Screen*>(wnd->parent());
    assert(screen);
    if (!m_texture)
    {
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
    if (mStatus == R_VIDEO_INITLED)
    {
        if (m_texture)
        {
            if (m_fixed_size != Vector2i(0))
            {
                m_texture->resize(m_fixed_size);
            }
            m_texture->upload(m_pixels);
            red_debug_lite("show %p -------------------------------------------", m_pixels);
            ImageView::set_image(m_texture);
            mStatus = R_VIDEO_RUNNING; 
        }
    }
    else if (mStatus == R_VIDEO_RUNNING)
    {
        m_texture->upload(m_pixels);
    }
    ImageView::draw(ctx);
}

bool VideoView::mouse_button_event(const Vector2i &p, int button, bool down, int modifiers) {
    char track_buffer[32] = {0};
    Vector2i track_p;

    Led3000Window * led3000Window = dynamic_cast<Led3000Window *>(window()->parent());
    if (led3000Window->getJsonValue()->devices[led3000Window->getCurrentDevice()].turntable.mode != TURNTABLE_TRACK_MODE)
        return false;
    track_p.v[0] = p.v[0] * VIDEO_TRACK_FIXED_WIDTH / VIDEO_SHOW_FIXED_WIDTH;
    track_p.v[1] = p.v[1] * VIDEO_TRACK_FIXED_HEIGH / VIDEO_SHOW_FIXED_HEIGH;
    //track_p.v[0] -= VIDEO_TRACK_FIXED_WIDTH / 2 ;
    //track_p.v[1] = VIDEO_TRACK_FIXED_HEIGH / 2 - track_p.v[1];
    snprintf(track_buffer, sizeof track_buffer, "%d,%d", track_p.v[0], track_p.v[1]);
    red_debug_lite("track buffer %s raw:%d,%d", track_buffer, p.v[0], p.v[1]);
    /* TODO 发送坐标信息进行目标追踪 */
    led3000Window->getCurrentDeviceQueue().put(PolyM::DataMsg<std::string>(POLYM_TURNTABLE_TRACK_SETTING, track_buffer));
    /* 防止发送太快驱动板无法处理 */
    usleep(100000);
    return true;
}
NAMESPACE_END(nanogui)
