/*
    sdl_gui/videoview.cpp -- Widget used to display images.

    The image view widget was contributed by Stefan Ivanov.

    Based on NanoGUI by Wenzel Jakob <wenzel@inf.ethz.ch>.
    Adaptation for SDL by Dalerank <dalerankn8@gmail.com>

    All rights reserved. Use of this source code is governed by a
    BSD-style license that can be found in the LICENSE.txt file.
*/

#include <sdlgui/videoview.h>
#include <sdlgui/window.h>
#include <sdlgui/screen.h>
#if defined(_WIN32)
#include <SDL.h>
#else
#include <SDL.h>
#endif
#include <sdlgui/theme.h>
#include <cmath>

#include <unistd.h>

#include <sys/ioctl.h>
#include <linux/sockios.h>
#include <net/route.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>

NAMESPACE_BEGIN(sdlgui)

namespace 
{
    std::vector<std::string> splitString(const std::string& text, const std::string& delimiter) 
    {
        using std::string; using std::vector;
        vector<string> strings;
        string::size_type current = 0;
        string::size_type previous = 0;
        while ((current = text.find(delimiter, previous)) != string::npos) {
            strings.push_back(text.substr(previous, current - previous));
            previous = current + 1;
        }
        strings.push_back(text.substr(previous));
        return strings;
    }
}

int VideoView::video_draw_handler(void *object)
{
    VideoView *p_video_obj = (VideoView *)object;
    AVFormatContext *p_avformat_context = NULL;
    AVCodecParameters *p_avcodec_parameter = NULL;
    AVCodecContext *p_avcodec_context = NULL;
    const AVCodec *p_avcodec = NULL;
    AVDictionary* options = NULL;
    SDL_Event event;

    enum AVPixelFormat src_fix_fmt;
    enum AVPixelFormat dst_fix_fmt;

    AVPacket packet;
    struct SwsContext* sws_clx = NULL;
    AVFrame* p_frame = NULL;
    int video_stream_index;
    int value;
    Vector2i sdl_rect;

    p_video_obj->mStatus = R_VIDEO_RUNNING;

    //rprint_fib(1, 0);
    if (!strlen(p_video_obj->mSrcUrl))
    {
        return -1;
    }

    p_avformat_context = avformat_alloc_context();
    if (!p_avformat_context)
    {
        printf("Failed avformat_alloc_context\n");
        return -1;
    }

    value = av_dict_set(&options, "rtsp_transport", "tcp", 0);
    /* 修改超时时间，单位是 ms */
    value = av_dict_set(&options, "timeout", "5000", 0);
    if (value < 0)
    {
        printf("Failed av_dict_set %d\n", value);
        return -2;
    }

    char errbuf[128];
    printf("src_file=%s\n", p_video_obj->mSrcUrl);
re_open:
    value = avformat_open_input(&p_avformat_context, p_video_obj->mSrcUrl, NULL, &options);

    if (value)
    {
        av_strerror(value, errbuf, sizeof(errbuf));
        printf("Failed open av input:%d  %s\n", value, errbuf);
        sleep(1);
        goto re_open;
        return -3;
    }
    else
    {
        printf("Open input success\n");
    }

    value = avformat_find_stream_info(p_avformat_context, NULL);
    if (value)
    {
        printf("Failed find stream info\n");
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
        printf("Failed get video stream\n");
        return -5;
    }

    p_avcodec_parameter = p_avformat_context->streams[video_stream_index]->codecpar;
    p_avcodec = avcodec_find_decoder(p_avcodec_parameter->codec_id);
    if (!p_avcodec)
    {
        printf("Failed get avcodec format\n");
        return -6;
    }

    p_avcodec_context = avcodec_alloc_context3(p_avcodec);
    if (!p_avcodec_context)
    {
        printf("Failed create avcodec context\n");
        return -7;
    }
    value = avcodec_parameters_to_context(p_avcodec_context, p_avcodec_parameter);
    if (value < 0)
    {
        printf("Failed init avcodec context\n");
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
        printf("Failed alloc AVFrame\n");
        return -7;
    }

    sws_clx = sws_getContext(
        p_avcodec_context->width,
        p_avcodec_context->height,
        p_avcodec_context->pix_fmt,
        p_video_obj->imageSize().x,
        p_video_obj->imageSize().y,
        dst_fix_fmt,
        SWS_BILINEAR,
        NULL,
        NULL,
        NULL);


    sdl_rect = p_video_obj->imageSize();
    red_debug_lite("w=%d h=%d", sdl_rect.x, sdl_rect.y);

    value = av_image_alloc(p_video_obj->m_pixels, p_video_obj->m_pitch, sdl_rect.x, sdl_rect.y, dst_fix_fmt, 1);
    if (value < 0)
    {
        printf("Failed create av image\n");
        return -12;
    }
    else
    {
        printf("dst memory size=%d\n", value);
    }

    p_video_obj->mStatus = R_VIDEO_INITLED;
just_draw:
    while (1)
    {
        if (av_read_frame(p_avformat_context, &packet) < 0)
        {
            printf("Failed get frame 00000000000\n");
            //video_rebind_route("eth1", "eth3", "192.168.100.64");
            break;
        }

        if (packet.stream_index == video_stream_index)
        {
            int response = avcodec_send_packet(p_avcodec_context, &packet);
    
            if (response < 0) {
              printf("Error while sending a packet to the decoder\n");
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
                printf("Error frame-finished=%d\n", frame_finished);
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
        printf("Failed open av input:%d  %s\n", value, errbuf);
    }
    avformat_close_input(&p_avformat_context);
    avcodec_free_context(&p_avcodec_context);
    avformat_free_context(p_avformat_context);

    return 0;
}

VideoView::VideoView(Widget* parent, SDL_Texture* texture)
    : Widget(parent), mTexture(texture), mScale(1.0f), mOffset(Vector2f::Zero()),
    mFixedScale(false), mFixedOffset(false), mPixelInfoCallback(nullptr), m_thread(nullptr),
    mSrcUrl("rtsp://admin:jariled123@192.168.100.64")
{
    Window * wnd = parent->window();
    int hh = wnd->theme()->mWindowHeaderHeight;
    Screen* screen = dynamic_cast<Screen*>(wnd->parent());
    assert(screen);
    /* TODO create a thread to get image data */
    if (!mTexture)
    {
        /* 默认创建一个 window size - 20 大小的窗口 */
        mTexture = SDL_CreateTexture(screen->sdlRenderer(), SDL_PIXELFORMAT_ARGB8888, SDL_TEXTUREACCESS_STREAMING, wnd->size().x, wnd->size().y - hh);
        red_debug_lite("w=%d h=%d", wnd->size().x - 10, wnd->size().y - hh);
    }
    updateImageParameters();

    if (m_thread)
    {
        red_debug_lite("wait previous thread done");
        SDL_WaitThread(m_thread, NULL);
        red_debug_lite("new thread start");
    }
    m_thread = SDL_CreateThread(VideoView::video_draw_handler, mSrcUrl, this);
}

VideoView::~VideoView() {}

Vector2f VideoView::imageCoordinateAt(const Vector2f& position) const
{
    auto imagePosition = position - mOffset;
    return imagePosition / mScale;
}

Vector2f VideoView::clampedImageCoordinateAt(const Vector2f& position) const 
{
    Vector2f imageCoordinate = imageCoordinateAt(position);
    return imageCoordinate.cmax({ 0,0 }).cmin(imageSizeF());
}

Vector2f VideoView::positionForCoordinate(const Vector2f& imageCoordinate) const 
{
    return imageCoordinate*mScale + mOffset;
}

void VideoView::setImageCoordinateAt(const Vector2f& position, const Vector2f& imageCoordinate) 
{
    // Calculate where the new offset must be in order to satisfy the image position equation.
    // Round the floating point values to balance out the floating point to integer conversions.
    mOffset = position - (imageCoordinate * mScale); // .unaryExpr([](float x) { return std::round(x); });
    // Clamp offset so that the image remains near the screen.
    mOffset = mOffset.cmin(sizeF()).cmax(-scaledImageSizeF());
}

void VideoView::center() 
{
    mOffset = (sizeF() - scaledImageSizeF()) / 2;
}

void VideoView::fit() 
{
    // Calculate the appropriate scaling factor.
    mScale = (sizeF().cquotient(imageSizeF())).minCoeff();
    center();
}

void VideoView::setScaleCentered(float scale) {
    auto centerPosition = sizeF() / 2;
    auto p = imageCoordinateAt(centerPosition);
    mScale = scale;
    setImageCoordinateAt(centerPosition, p);
}

void VideoView::moveOffset(const Vector2f& delta) {
    // Apply the delta to the offset.
    mOffset += delta;

    // Prevent the image from going out of bounds.
    auto scaledSize = scaledImageSizeF();
    if (mOffset.x + scaledSize.x < 0)
        mOffset.x = -scaledSize.x;
    if (mOffset.x > sizeF().x)
        mOffset.x = sizeF().x;
    if (mOffset.y + scaledSize.y < 0)
        mOffset.y = -scaledSize.y;
    if (mOffset.y > sizeF().y)
        mOffset.y = sizeF().y;
}

void VideoView::zoom(int amount, const Vector2f& focusPosition) 
{
    auto focusedCoordinate = imageCoordinateAt(focusPosition);
    float scaleFactor = std::pow(mZoomSensitivity, amount);
    mScale = std::max(0.01f, scaleFactor * mScale);
    setImageCoordinateAt(focusPosition, focusedCoordinate);
}

bool VideoView::mouseDragEvent(const Vector2i& p, const Vector2i& rel, int button, int /*modifiers*/)
{
    return true;
}

bool VideoView::gridVisible() const 
{
    return (mGridThreshold != -1) && (mScale > mGridThreshold);
}

bool VideoView::pixelInfoVisible() const 
{
    return mPixelInfoCallback && (mPixelInfoThreshold != -1) && (mScale > mPixelInfoThreshold);
}

bool VideoView::helpersVisible() const 
{
    return gridVisible() || pixelInfoVisible();
}

bool VideoView::scrollEvent(const Vector2i& p, const Vector2f& rel)
{
    return true;
}

bool VideoView::keyboardEvent(int key, int /*scancode*/, int action, int modifiers) 
{
    return true;
}

bool VideoView::keyboardCharacterEvent(unsigned int codepoint) {
    return true;
}

Vector2i VideoView::preferredSize(SDL_Renderer* /*ctx*/) const 
{
    return mImageSize;
}

void VideoView::performLayout(SDL_Renderer* ctx) {
    Widget::performLayout(ctx);
    center();
}

/* 图像绘制函数 */
void VideoView::draw(SDL_Renderer* renderer) 
{
    Widget::draw(renderer);

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
}


void VideoView::updateImageParameters() 
{
  int w, h;
  SDL_QueryTexture(mTexture, nullptr, nullptr, &w, &h);
  mImageSize = Vector2i(w, h);
}

void VideoView::drawWidgetBorder(SDL_Renderer* renderer, const SDL_Point& ap) const 
{
  SDL_Color lc = mTheme->mBorderLight.toSdlColor();

  SDL_Rect lr{ ap.x - 1, ap.y - 1, mSize.x + 2, mSize.y + 2 };

  SDL_SetRenderDrawColor(renderer, lc.r, lc.g, lc.b, lc.a);
  SDL_RenderDrawRect(renderer, &lr);

  SDL_Color dc = mTheme->mBorderDark.toSdlColor();
  SDL_Rect dr{ ap.x - 1, ap.y - 1, mSize.x + 2, mSize.y + 2 };

  SDL_SetRenderDrawColor(renderer, dc.r, dc.g, dc.b, dc.a);
  SDL_RenderDrawRect(renderer, &dr);
}

void VideoView::drawImageBorder(SDL_Renderer* renderer, const SDL_Point& ap) const
{
  Vector2i borderPosition = Vector2i{ ap.x, ap.y } + mOffset.toint();
  Vector2i borderSize = scaledImageSizeF().toint();
  
  SDL_Rect br{ borderPosition.x + 1, borderPosition.y + 1,
                borderSize.x - 2, borderSize.y - 2 };

  PntRect r = srect2pntrect(br);
  PntRect wr = { ap.x, ap.y, ap.x + width(), ap.y + height() };

  if (r.x1 <= wr.x1) r.x1 = wr.x1;
  if (r.x2 >= wr.x2) r.x2 = wr.x2;
  if (r.y1 <= wr.y1) r.y1 = wr.y1;
  if (r.y2 >= wr.y2) r.y2 = wr.y2;
  
  SDL_SetRenderDrawColor(renderer, 255, 255, 255, 255);
  if (r.x1 > wr.x1) SDL_RenderDrawLine(renderer, r.x1, r.y1, r.x1, r.y2 - 1 );
  if (r.y1 > wr.y1) SDL_RenderDrawLine(renderer, r.x1, r.y1, r.x2-1, r.y1 );
  if (r.x2 < wr.x2) SDL_RenderDrawLine(renderer, r.x2, r.y1, r.x2, r.y2 - 1);
  if (r.y2 < wr.y2) SDL_RenderDrawLine(renderer, r.x1, r.y2, r.x2-1, r.y2);
}

void VideoView::drawHelpers(SDL_Renderer* renderer) const 
{
  Vector2f upperLeftCorner = positionForCoordinate(Vector2f{ 0, 0 }) + positionF();
  Vector2f lowerRightCorner = positionForCoordinate(imageSizeF()) + positionF();
    // Use the scissor method in NanoVG to display only the correct part of the grid.
  Vector2f scissorPosition = upperLeftCorner.cmax(positionF());
  Vector2f sizeOffsetDifference = sizeF() - mOffset;
  Vector2f scissorSize = sizeOffsetDifference.cmin(sizeF());

  SDL_Rect r{
      (int)std::round(scissorPosition.x),
      (int)std::round(scissorPosition.y),
      (int)std::round(scissorSize.x),
      (int)std::round(scissorSize.y)
  };
  if (gridVisible())
    drawPixelGrid(renderer, upperLeftCorner, lowerRightCorner, mScale);
  if (pixelInfoVisible())
    drawPixelInfo(renderer, mScale);
}

void VideoView::drawPixelGrid(SDL_Renderer* renderer, const Vector2f& upperLeftCorner,
                              const Vector2f& lowerRightCorner, const float stride) 
{
  SDL_SetRenderDrawColor(renderer, 255, 255, 255, 255);

    // Draw the vertical lines for the grid
    float currentX = std::floor(upperLeftCorner.x);
    while (currentX <= lowerRightCorner.x) 
    {
      SDL_RenderDrawLine(renderer, std::floor(currentX), std::floor(upperLeftCorner.y),
                          std::floor(currentX), std::floor(lowerRightCorner.y));
      currentX += stride;
    }
    // Draw the horizontal lines for the grid.
    float currentY = std::floor(upperLeftCorner.y);
    while (currentY <= lowerRightCorner.y) 
    {
      SDL_RenderDrawLine(renderer, std::floor(upperLeftCorner.x), std::floor(currentY),
                                    std::floor(lowerRightCorner.x), std::floor(currentY));
      currentY += stride;
    }
}

void VideoView::drawPixelInfo(SDL_Renderer* renderer, const float stride) const 
{
    // Extract the image coordinates at the two corners of the widget.
  Vector2f currentPixelF = clampedImageCoordinateAt({ 0,0 });
  Vector2f lastPixelF = clampedImageCoordinateAt(sizeF());
    // Round the top left coordinates down and bottom down coordinates up.
    // This is done so that the edge information does not pop up suddenly when it gets in range.
    currentPixelF = currentPixelF.floor();
    lastPixelF = lastPixelF.ceil();
    Vector2i currentPixel = currentPixelF.cast<int>();
    Vector2i lastPixel = lastPixelF.cast<int>();

    // Extract the positions for where to draw the text.
    Vector2f currentCellPosition = (positionF() + positionForCoordinate(currentPixelF));
    float xInitialPosition = currentCellPosition.x;
    int xInitialIndex = currentPixel.x;

    // Properly scale the pixel information for the given stride.
    auto fontSize = stride * mFontScaleFactor;
    static constexpr float maxFontSize = 30.0f;
    fontSize = fontSize > maxFontSize ? maxFontSize : fontSize;

   /* nvgSave(ctx);
    nvgBeginPath(ctx);
    nvgFontSize(ctx, fontSize);
    nvgTextAlign(ctx, NVG_ALIGN_CENTER | NVG_ALIGN_TOP);
    nvgFontFace(ctx, "sans");
    while (currentPixel.y() != lastPixel.y()) 
    {
        while (currentPixel.x() != lastPixel.x()) 
        {
            writePixelInfo(ctx, currentCellPosition, currentPixel, stride);
            currentCellPosition.x() += stride;
            ++currentPixel.x();
        }
        currentCellPosition.x() = xInitialPosition;
        currentCellPosition.y() += stride;
        ++currentPixel.y();
        currentPixel.x() = xInitialIndex;
    }
    nvgRestore(ctx);*/
}

void VideoView::writePixelInfo(SDL_Renderer* renderer, const Vector2f& cellPosition,
                               const Vector2i& pixel, const float stride) const
{
 /*   auto pixelData = mPixelInfoCallback(pixel);
    auto pixelDataRows = splitString(pixelData.first, "\n");

    // If no data is provided for this pixel then simply return.
    if (pixelDataRows.empty())
        return;

    nvgFillColor(ctx, pixelData.second);
    auto padding = stride / 10;
    auto maxSize = stride - 2 * padding;

    // Measure the size of a single line of text.
    float bounds[4];
    nvgTextBoxBounds(ctx, 0.0f, 0.0f, maxSize, pixelDataRows.front().data(), nullptr, bounds);
    auto rowHeight = bounds[3] - bounds[1];
    auto totalRowsHeight = rowHeight * pixelDataRows.size();

    // Choose the initial y offset and the index for the past the last visible row.
    auto yOffset = 0.0f;
    auto lastIndex = 0;

    if (totalRowsHeight > maxSize) {
        yOffset = padding;
        lastIndex = (int) (maxSize / rowHeight);
    } else {
        yOffset = (stride - totalRowsHeight) / 2;
        lastIndex = (int) pixelDataRows.size();
    }

    for (int i = 0; i != lastIndex; ++i) {
        nvgText(ctx, cellPosition.x() + stride / 2, cellPosition.y() + yOffset,
                pixelDataRows[i].data(), nullptr);
        yOffset += rowHeight;
    }
*/
}

NAMESPACE_END(sdlgui)
