/*
    sdl_gui/imageview.h -- Widget used to display images.

    The image view widget was contributed by Stefan Ivanov.

    Based on NanoGUI by Wenzel Jakob <wenzel@inf.ethz.ch>.
    Adaptation for SDL by Dalerank <dalerankn8@gmail.com>

    All rights reserved. Use of this source code is governed by a
    BSD-style license that can be found in the LICENSE.txt file.
*/
/** \file */

#pragma once

#include <libavcodec/avcodec.h>
#include <libavformat/avformat.h>
#include <libavutil/dict.h>
#include <libavutil/imgutils.h>
#include <libavutil/pixfmt.h>
#include <libswscale/swscale.h>
#include <nanogui/widget.h>
#include <nanogui/imageview.h>
#include <functional>
#include <thread>

NAMESPACE_BEGIN(nanogui)

using namespace std;

enum VideoViewStatus {
    R_VIDEO_UNINITLED,
    R_VIDEO_RUNNING,
    R_VIDEO_INITLED,
};

#define LED3000_MPP_H265
#define SRCURL_MAX    128
/**
 * \class VideoView imageview.h sdl_gui/imageview.h
 *
 * \brief Widget used to display images.
 */
class NANOGUI_EXPORT VideoView : public ImageView
{
public:
    VideoView(Widget* parent);
    VideoView(Widget* parent, const char *srcurl): VideoView(parent)
    {
        memcpy(mSrcUrl, srcurl, strlen(srcurl) + 1);
    };
    ~VideoView();

    void draw(NVGcontext *ctx);

    virtual bool mouse_button_event(const Vector2i &p, int button, bool down, int modifiers) override;
    static int video_draw_handler(void *object);

    int setSrcUrl(const char *srcurl) {memcpy(mSrcUrl, srcurl, strlen(srcurl) + 1); return 0;}
    Texture * get_texture() {return m_texture;};

protected:
    char mSrcUrl[SRCURL_MAX];
    std::thread *m_thread;
    VideoViewStatus mStatus;
    uint8_t * m_pixels;
#ifdef LED3000_MPP_H265
    uint8_t * m_crop4h265;
#endif
    Texture *m_texture;
    int m_no_frame_counts;
};

NAMESPACE_END(nanogui)
