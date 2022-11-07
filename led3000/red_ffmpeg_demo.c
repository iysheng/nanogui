
#include <stdio.h>
#include <unistd.h>

#include <libavcodec/avcodec.h>
#include <libavformat/avformat.h>
#include <libavutil/dict.h>
#include <libavutil/imgutils.h>
#include <libavutil/pixfmt.h>
#include <libswscale/swscale.h>
#include <SDL2/SDL.h>

#include "MppDecode.h"
#include "im2d_api/im2d.hpp"
#include "RgaUtils.h"
#include "rga.h"

SDL_Renderer* sdlRenderer;
SDL_Texture* sdlTexture;
AVCodecContext *p_avcodec_context;
SDL_Rect sdl_rect;
struct SwsContext* sws_clx;
    uint8_t* pixels[4] = {0};
    int pitch[4] = {0};

#define ERROR               -1

#define DST_WIDTH  520
#define DST_HEIGHT 286
#define SRC_FORMAT RK_FORMAT_YCbCr_420_SP
#define DST_FORMAT RK_FORMAT_RGBA_8888

float get_bpp_from_format(int format) {
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
size_t mpp_buffer_group_usage(MppBufferGroup group)
{
    if (NULL == group)
    {
        printf("input invalid group %p\n", group);
        return MPP_BUFFER_MODE_BUTT;
    }

    MppBufferGroupImpl *p = (MppBufferGroupImpl *)group;
    return p->usage;
}

void deInit(MppPacket *packet, MppFrame *frame, MppCtx ctx, char *buf, MpiDecLoopData data )
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


    if (data.pkt_grp) {
        mpp_buffer_group_put(data.pkt_grp);
        data.pkt_grp = NULL;
    }

    if (data.frm_grp) {
        mpp_buffer_group_put(data.frm_grp);
        data.frm_grp = NULL;
    }

    if (data.fp_output) {
        fclose(data.fp_output);
        data.fp_output = NULL;
    }

    if (data.fp_input) {
        fclose(data.fp_input);
        data.fp_input = NULL;
    }
}

char * display_quick;
int  decode_simple(MpiDecLoopData *data, AVPacket *av_packet )
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
    if (!display_quick)
        display_quick = malloc(10 * DST_WIDTH * DST_HEIGHT * get_bpp_from_format(DST_FORMAT));
    if (!display_quick)
    {
        printf("invalid memory\n");
        return 0;
    }


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
                       //cv::Mat rgbImg;
                       //YUV420SP2Mat(frame, rgbImg);
                    //    cv::imwrite("./"+std::to_string(count++)+".jpg", rgbImg);
                    //    dump_mpp_frame_to_file(frame, data->fp_output);
                    /* 获取这个帧的 buffer 信息 */
                    buffer = mpp_frame_get_buffer(frame);
                    buf_size = mpp_frame_get_buf_size(frame);
                    h_stride = mpp_frame_get_hor_stride(frame);
                    v_stride = mpp_frame_get_ver_stride(frame);
                    RK_U8 * base = NULL;
                    /* 获取内存首地址 */
                    base = mpp_buffer_get_ptr(buffer);
                    printf("base=%p fmt=%d hstride=%d vstride=%d size=%d\n", base, mpp_frame_get_fmt(frame), h_stride, v_stride, buf_size);
#if 0
                    if (abc < 10)
                    {
                        snprintf(file_name, 32, "test_yuv420sp%d.yuv size=%d", abc++, buf_size);
                        yuv_file_fd = fopen(file_name, "w+b");
                        fwrite(base[0], 1, buf_size, yuv_file_fd);
                        fclose(yuv_file_fd);
                        printf("write to %s file ok\n", file_name);
                    }
#endif
                    SDL_RenderClear(sdlRenderer);
                    ///////////////////////////////
                    memset(&src_rect, 0, sizeof(src_rect));
                    memset(&dst_rect, 0, sizeof(dst_rect));
                    memset(&src, 0, sizeof(src));
                    memset(&dst, 0, sizeof(dst));
                    /* RGA 模块封装源信息 */
                src = wrapbuffer_virtualaddr(base, h_stride, v_stride, SRC_FORMAT);
#if 0
                if (!display_quick)
                   display_quick= (char*)malloc(DST_WIDTH*DST_HEIGHT*get_bpp_from_format(DST_FORMAT));

                    memset(display_quick,0x00,DST_WIDTH*DST_HEIGHT*get_bpp_from_format(DST_FORMAT));
#endif

                    /* RGA 模块封装目的信息 */
                    dst = wrapbuffer_virtualaddr(display_quick, DST_WIDTH, DST_HEIGHT, DST_FORMAT);
                    if(dst.width == 0) {
                        printf("%s, %s\n", __FUNCTION__, imStrError());
                        return ERROR;
                    }

                    /* 处理源和目的 rect 信息 */
            ret = imcheck(src, dst, src_rect, dst_rect);
            if (IM_STATUS_NOERROR != ret) {
                printf("%d, check error! %s", __LINE__, imStrError((IM_STATUS)ret));
                return -1;
            }

            /* 缩放之前标记编码格式 */
            src.format = RK_FORMAT_YCbCr_420_SP;
            dst.format = RK_FORMAT_RGBA_8888;
            ret = imresize(src, dst);
            printf("resizing .... %s\n", imStrError(ret));
            ////////////////////// 颜色变换
            //ret = imcvtcolor(src, dst, src.format, dst.format);
            //printf("cvtcolor .... cost time , %s\n", imStrError(ret));
            ////////////////
                    ///////////////////////////////////
                    SDL_UpdateTexture(sdlTexture, &sdl_rect, display_quick, DST_WIDTH * 4);
                    SDL_RenderCopy(sdlRenderer, sdlTexture, NULL, NULL);
                    SDL_RenderPresent(sdlRenderer);
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
int main (int argc, char **argv)
{
    AVFormatContext *p_avformat_context = NULL;
    AVCodecParameters *p_avcodec_parameter;
    const AVCodec *p_avcodec;
    AVDictionary* options = NULL;
    SDL_Event event;

    enum AVPixelFormat src_fix_fmt;
    enum AVPixelFormat dst_fix_fmt;

    int value = 0;
    int video_stream_index = -1;

    if (argc > 1)
    {
        printf("hello red ffmpeg demo:%s\n", argv[1]);
    }
    else
    {
        printf("Please input source file\n");
        return 0;
    }

    p_avformat_context = avformat_alloc_context();
    if (!p_avformat_context)
    {
        printf("Failed avformat_alloc_context\n");
        return -1;
    }

    value = av_dict_set(&options, "buffer_size", "10240000", 0);
    value = av_dict_set(&options, "rtsp_transport", "udp", 0);
    value = av_dict_set(&options, "stimeout", "10000000", 0);
    //value = av_dict_set(&options, "framerate", "30", 0);
    //value = av_dict_set(&options, "preset", "medium", 0);
    value = av_dict_set(&options, "gpu_copy", "on", 0);
    if (value < 0)
    {
        printf("Failed av_dict_set %d\n", value);
        return -2;
    }

    value = avformat_open_input(&p_avformat_context, argv[1], NULL, &options);
    if (value)
    {
        printf("Failed open av input\n");
        return -3;
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
    av_dump_format(p_avformat_context, 0, argv[1], 0);

    AVFrame* p_frame = av_frame_alloc();

    if (!p_frame)
    {
        printf("Failed alloc AVFrame\n");
        return -7;
    }

    /* test code 单个像素对应的字节长度这里暂定为 2, 应该有函数可以根据类型识别出来 */
    AVPacket av_packet;

    sdl_rect.x = 0;
    sdl_rect.y = 0;
    sdl_rect.w = 
        p_avcodec_context->width;
    sdl_rect.h = 
        p_avcodec_context->height;

    if (SDL_Init(SDL_INIT_VIDEO | SDL_INIT_AUDIO | SDL_INIT_TIMER))
    {
        printf("Failed sdl init\n");
        return -10;
    }

    SDL_Window* sdl_window = SDL_CreateWindow("rtsp", SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, sdl_rect.w, sdl_rect.h, SDL_WINDOW_SHOWN);

	if (!sdl_window)
	{
        printf("Failed sdl window create\n");
        return -11;
    }
    sdlRenderer = SDL_CreateRenderer(sdl_window, -1, SDL_RENDERER_ACCELERATED | SDL_RENDERER_PRESENTVSYNC | SDL_RENDERER_TARGETTEXTURE);

    sdlTexture = SDL_CreateTexture(sdlRenderer, SDL_PIXELFORMAT_ABGR8888/*SDL_PIXELFORMAT_NV12 */, SDL_TEXTUREACCESS_STREAMING, sdl_rect.w, sdl_rect.h);
    SDL_SetTextureBlendMode(sdlTexture, SDL_BLENDMODE_BLEND);

    //printf ("size=%d\n", av_image_get_buffer_size(dst_fix_fmt, p_avcodec_context->width, p_avcodec_context->height, 1));
    value = av_image_alloc(pixels, pitch, sdl_rect.w, sdl_rect.h , dst_fix_fmt, 1);
    if (value < 0)
    {
        printf("Failed create av image\n");
        return -12;
    }
    else
    {
        printf("dst memory size=%d\n", value);
    }

    ////////////////
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


    MpiDecLoopData data;

    printf("mpi_dec_test start\n");
    memset(&data, 0, sizeof(data));

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

    data.ctx            = ctx;
    data.mpi            = mpi;
    data.eos            = 0;
    data.packet_size    = packet_size;
    data.frame          = frame;
    data.frame_count    = 0;
    ///////////////

    int result, i = 0;
    while(1)
    {
        result = av_read_frame(p_avformat_context, &av_packet);
        if (result >= 0 && av_packet.stream_index != video_stream_index)
        {
            av_packet_unref(&av_packet);
            continue;
        }

        if (result < 0)
            result = avcodec_send_packet(p_avcodec_context, NULL);
        else
        {
#if 0
            if (av_packet.pts == AV_NOPTS_VALUE)
                av_packet.pts = av_packet.dts = i;
            printf("1111111111111111\n");
            /* 这个函数比较耗时 */
            result = avcodec_send_packet(p_avcodec_context, &av_packet);
            printf("222222222222222222:%d\n", result);
#else
            decode_simple(&data, &av_packet);
#endif
        }
        av_packet_unref(&av_packet);
        continue;

#if 0
        if (result < 0)
        {
            printf("error while submitting a av_packet to decoding:%d\n", result);
            return result;
        }

        while (result >= 0)
        {
            result = avcodec_receive_frame(p_avcodec_context, p_frame);
            printf("33333333333333 result=%d\n", result);
            if (result == AVERROR_EOF)
                goto finish;
            else if (result == AVERROR(EAGAIN))
            {
                result = 0;
                break;
            }
            else if (result < 0)
            {
                printf("error decoding frame :%d\n", result);
                return result;
            }
#endif
#if 0
        }
        i++;



        if (packet.stream_index == video_stream_index)
        {
            int response = avcodec_send_packet(p_avcodec_context, &packet);

            if (response < 0) {
              printf("Error while sending a packet to the decoder: %s", av_err2str(response));
              return response;
            }

            int frame_finished;
            frame_finished = avcodec_receive_frame(p_avcodec_context, p_frame);

            if (!frame_finished)
            {
#if 0
                printf(
                  "Frame %d (type=%c, size=%d bytes, format=%d) pts %d key_frame %d [DTS %d]\n",
                  p_avcodec_context->frame_number,
                  av_get_picture_type_char(p_frame->pict_type),
                  p_frame->pkt_size,
                  p_frame->format,
                  p_frame->pts,
                  p_frame->key_frame,
                  p_frame->coded_picture_number
                );
                printf("%p\n", p_frame->data);
                sws_scale(sws_clx, (uint8_t const* const *)p_frame->data, p_frame->linesize, 0, p_avcodec_context->height / 2, pixels, pitch);
                //SDL_UpdateTexture(sdlTexture, &sdl_rect, pixels, pitch);
//#else
                SDL_UpdateTexture(sdlTexture, &sdl_rect, p_frame->data[0], p_frame->linesize[0]);
#endif
                SDL_RenderClear(sdlRenderer);
                if (!SDL_LockTexture(sdlTexture, &sdl_rect, pixels, pitch)) {
                    sws_scale(sws_clx, (const uint8_t * const *)p_frame->data, p_frame->linesize, 0, p_avcodec_context->height, pixels, pitch);
//                    for (int j = 0; j < 4; j++)
//                    {
//                        printf("over scale@%d raw linesize=%d linesize=%d\n", j, p_frame->linesize[j], pitch[j]);
//                    }
                    //SDL_UpdateTexture(sdlTexture, &sdl_rect, pixels[0], pitch[0]);
                    SDL_UnlockTexture(sdlTexture);
                }
                else
                {
                    printf("Failed lock sws scale:%s\n", SDL_GetError());
                }
                SDL_RenderCopy(sdlRenderer, sdlTexture, NULL, NULL);
                SDL_RenderPresent(sdlRenderer);
                av_packet_unref(&packet);
            }
            else
            {
                printf("Error frame-finished=%d\n", frame_finished);
                sleep(1);
            }
        }
#endif
        if (SDL_PollEvent(&event))
        {
            switch (event.type) {
            case SDL_QUIT:
                break;
            }
        }
    }

finish:
    //av_packet_free(&&packet);
    av_frame_free(&p_frame);
    avformat_close_input(&p_avformat_context);
    avcodec_free_context(&p_avcodec_context);
    SDL_DestroyWindow(sdl_window);
    SDL_DestroyRenderer(sdlRenderer);
    SDL_DestroyTexture(sdlTexture);
    SDL_Quit();

    return 0;
}
