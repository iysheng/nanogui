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

NAMESPACE_BEGIN(nanogui)

int VideoView::video_draw_handler(void *object)
{
    VideoView *p_video_obj = (VideoView *)object;
    AVFormatContext *p_avformat_context = NULL;
    AVCodecParameters *p_avcodec_parameter = NULL;
    AVCodecContext *p_avcodec_context = NULL;
    const AVCodec *p_avcodec = NULL;
    AVDictionary* options = NULL;

    enum AVPixelFormat src_fix_fmt;
    enum AVPixelFormat dst_fix_fmt;

    AVPacket packet;
    struct SwsContext* sws_clx = NULL;
    AVFrame* p_frame = NULL;
    int video_stream_index;
    int value;
    Vector2i sdl_rect;

    p_video_obj->mStatus = R_VIDEO_RUNNING;

    if (!strlen(p_video_obj->mSrcUrl))
    {
        return -1;
    }

    p_avformat_context = avformat_alloc_context();
    if (!p_avformat_context)
    {
        red_debug_lite("Failed avformat_alloc_context\n");
        return -1;
    }

    value = av_dict_set(&options, "rtsp_transport", "tcp", 0);
    /* 修改超时时间，单位是 ms */
    value = av_dict_set(&options, "timeout", "5000", 0);
    if (value < 0)
    {
        red_debug_lite("Failed av_dict_set %d\n", value);
        return -2;
    }

    char errbuf[128];
    red_debug_lite("src_file=%s\n", p_video_obj->mSrcUrl);
re_open:
    value = avformat_open_input(&p_avformat_context, p_video_obj->mSrcUrl, NULL, &options);

    if (value)
    {
        av_strerror(value, errbuf, sizeof(errbuf));
        red_debug_lite("Failed open av input:%d  %s\n", value, errbuf);
        sleep(1);
        goto re_open;
        return -3;
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

    src_fix_fmt = p_avcodec_context->pix_fmt;
    dst_fix_fmt = AV_PIX_FMT_RGB32;//AVCOL_PRI_BT709;//AV_PIX_FMT_RGB32;//AVCOL_PRI_BT709;
    p_frame = av_frame_alloc();

    if (!p_frame)
    {
        red_debug_lite("Failed alloc AVFrame\n");
        return -7;
    }

    Vector2i top_left = Vector2i(p_video_obj->pixel_to_pos(Vector2f(0.f, 0.f))),
             size     = Vector2i(p_video_obj->pixel_to_pos(Vector2f(p_video_obj->image()->size())) - Vector2f(top_left));

    sws_clx = sws_getContext(
        p_avcodec_context->width,
        p_avcodec_context->height,
        p_avcodec_context->pix_fmt,
        size[0],
        size[1],
        dst_fix_fmt,
        SWS_BILINEAR,
        NULL,
        NULL,
        NULL);


    red_debug_lite("w=%d h=%d", size[0], size[1]);

    value = av_image_alloc(p_video_obj->m_pixels, p_video_obj->m_pitch, size[0], size[1], dst_fix_fmt, 1);
    if (value < 0)
    {
        red_debug_lite("Failed create av image\n");
        return -12;
    }
    else
    {
        red_debug_lite("dst memory size=%d\n", value);
    }

    p_video_obj->mStatus = R_VIDEO_INITLED;
just_draw:
    while (1)
    {
        if (av_read_frame(p_avformat_context, &packet) < 0)
        {
            red_debug_lite("Failed get frame 00000000000\n");
            break;
        }

        if (packet.stream_index == video_stream_index)
        {
            int response = avcodec_send_packet(p_avcodec_context, &packet);

            if (response < 0) {
              red_debug_lite("Error while sending a packet to the decoder\n");
              return response;
            }

            int frame_finished;
            frame_finished = avcodec_receive_frame(p_avcodec_context, p_frame);

            if (!frame_finished)
            {
                sws_scale(sws_clx, (const uint8_t * const *)p_frame->data, p_frame->linesize, 0, p_avcodec_context->height, p_video_obj->m_pixels, p_video_obj->m_pitch);
                av_packet_unref(&packet);
            }
            else
            {
                red_debug_lite("Error frame-finished=%d\n", frame_finished);
            }
        }
    }
exit:
    /* 标记状态为未初始化 */
    p_video_obj->mStatus = R_VIDEO_UNINITLED;
    if (p_frame)
    {
        av_frame_free(&p_frame);
    }

    /* 释放内存 */
    av_freep(&p_video_obj->m_pixels[0]);

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

VideoView::VideoView(Widget* parent):ImageView(parent),
    m_thread(nullptr), mSrcUrl("rtsp://admin:jariled123@192.168.100.64")
{
    Window * wnd = parent->window();
    int hh = wnd->theme()->m_window_header_height;
    Screen* screen = dynamic_cast<Screen*>(wnd->parent());
    assert(screen);
    /* TODO create a thread to get image data */
#if 0
    if (!mTexture)
    {
        /* 默认创建一个 window size - 20 大小的窗口 */
        mTexture = SDL_CreateTexture(screen->sdlRenderer(), SDL_PIXELFORMAT_ARGB8888, SDL_TEXTUREACCESS_STREAMING, wnd->size().x, wnd->size().y - hh);
        red_debug_lite("w=%d h=%d", wnd->size().x - 10, wnd->size().y - hh);
    }
#endif

    if (m_thread)
    {
        red_debug_lite("wait previous thread done");
        ///SDL_WaitThread(m_thread, NULL);
        red_debug_lite("new thread start");
        return;
    }
    m_thread = new std::thread(VideoView::video_draw_handler, mSrcUrl);
    m_thread->detach();
}

VideoView::~VideoView() {}

/* 图像绘制函数 */
void VideoView::draw(NVGcontext *ctx)
{
    ImageView::draw(ctx);

#if 0
    SDL_Point ap = getAbsolutePos();

    const Screen* screen = dynamic_cast<const Screen*>(this->window()->parent());
    assert(screen);
    Vector2f screenSize = screen->size().tofloat();
    Vector2f scaleFactor = imageSizeF().cquotient(screenSize) * mScale;
    /* 转换为浮点数 */
    Vector2f positionInScreen = absolutePosition().tofloat();
    Vector2f positionAfterOffset = positionInScreen + mOffset;
    /* 计算系数 */
    Vector2f imagePosition = positionAfterOffset.cquotient(screenSize);

    /* 测试打印为 0 */
    //red_debug_lite("mOffset(%f,%f)", mOffset.x, mOffset.y);
    if (mStatus == R_VIDEO_INITLED)
    {
        if (mTexture)
        {
          Vector2i borderPosition = Vector2i{ ap.x, ap.y } + mOffset.toint();
          /* 图像大小 */
          Vector2i borderSize = scaledImageSizeF().toint();

          SDL_Rect br{ borderPosition.x + 1, borderPosition.y + 1,  borderSize.x - 2, borderSize.y - 2 };

          PntRect r = srect2pntrect(br);
          PntRect wr = { ap.x, ap.y, ap.x + width(), ap.y + height() };

          if (r.x1 <= wr.x1) r.x1 = wr.x1;
          if (r.x2 >= wr.x2) r.x2 = wr.x2;
          if (r.y1 <= wr.y1) r.y1 = wr.y1;
          if (r.y2 >= wr.y2) r.y2 = wr.y2;

          int ix = 0, iy = 0;
          int iw = r.x2 - r.x1;
          int ih = r.y2 - r.y1;
          if (positionAfterOffset.x <= ap.x)
          {
            ix = ap.x - positionAfterOffset.x;
            iw = mImageSize.x- ix;
            positionAfterOffset.x = absolutePosition().x;
          }
          if (positionAfterOffset.y <= ap.y)
          {
            iy = ap.y - positionAfterOffset.y;
            ih = mImageSize.y - iy;
            positionAfterOffset.y = absolutePosition().y;
          }
          SDL_Rect imgrect{ix, iy, iw, ih};
          SDL_Rect rect{
              (int)std::round(positionAfterOffset.x),
              (int)std::round(positionAfterOffset.y),
           mImageSize.x,
           mImageSize.y,
         };
          //red_debug_lite("%d %d %d %d", rect.x, rect.y, rect.w, rect.h);
          /* 绘制一帧的数据信息 */
          SDL_UpdateTexture(mTexture, NULL, m_pixels[0], m_pitch[0]);
          SDL_RenderCopy(renderer, mTexture, NULL, &rect);
          return;
        }
    }
    else if (mStatus == R_VIDEO_UNINITLED)
    {
        if (m_thread)
        {
            red_debug_lite("wait previous thread done");
            SDL_WaitThread(m_thread, NULL);
            red_debug_lite("new thread start");
        }
        m_thread = SDL_CreateThread(VideoView::video_draw_handler, mSrcUrl, this);
    }
    else
    {
        return;
    }

    drawWidgetBorder(renderer, ap);
    drawImageBorder(renderer, ap);

    if (helpersVisible())
        drawHelpers(renderer);
#endif
}

NAMESPACE_END(nanogui)
