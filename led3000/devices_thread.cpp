/******************************************************************************
* File:             devices_thread.cpp
*
* Author:           yangyongsheng@jari.cn  
* Created:          08/16/22 
* Description:      灯光装置控制线程
*****************************************************************************/

#include <cstdint>
#include <sys/prctl.h>
#include <nanogui/common.h>
#include <led3000gui.h>
#include <debug.h>
#include <PolyM/include/polym/Msg.hpp>
#include <PolyM/include/polym/Queue.hpp>

#include <termios.h>
#include <unistd.h>
#include <cstdio>
#include <iostream>
#include <unistd.h>
#include <thread>
#include <fcntl.h>

using std::cout;
using std::cerr;
using std::endl;

using namespace nanogui;
using namespace std;

static Led3000Window *gs_screen;

typedef struct {
    char name[32];
    int fd;
    unsigned int baud;
    unsigned char data_bit;
    unsigned char stop_bit;
    unsigned char even;
    unsigned char index;
} uartport_t;

typedef struct {
    uartport_t uart;
    Led3000Window *screen;
} led_device_t;

static led_device_t gs_led_devices[2] = {
    {
        .uart = {
            {.name = "/dev/ttyS3"}, /* C++ 要这样初始化，否则会报编译错误 */
            .baud = B115200,
            .data_bit = CS8,
            .stop_bit = 1, /* 一个停止位 */
            .even = 0, /* 无校验 */
            .index = 0,
        },
    },
    {
        .uart = {
            {.name = "/dev/ttyS4"}, /* C++ 要这样初始化，否则会报编译错误 */
            .baud = B115200,
            .data_bit = CS8,
            .stop_bit = 1, /* 一个停止位 */
            .even = 0, /* 无校验 */
            .index = 1,
        },
    },
};

int init_uart_port(uartport_t *uart)
{
    int ret = 0, fd = -1;
    struct termios uart_setting;

    if (!uart)
    {
        ret = -EINVAL;
        goto end;
    }

    fd = open(uart->name, O_RDWR | O_NOCTTY | O_NDELAY | O_NONBLOCK | O_SYNC);
    if (fd <= 0)
    {
        red_debug_lite("Failed open %s err=%d.", uart->name, fd);
        return -1;
    }

    bzero(&uart_setting, sizeof(uart_setting));
    tcgetattr(fd, &uart_setting);

    cfsetispeed(&uart_setting, uart->baud);
    cfsetospeed(&uart_setting, uart->baud);

    /* 偶校验 */
    if (uart->even == 1)
    {
        uart_setting.c_cflag |= PARENB;
        uart_setting.c_cflag &= ~PARODD;
    }
    /* 奇校验 */
    else if (uart->even == 2)
    {
        uart_setting.c_cflag |= PARENB;
        uart_setting.c_cflag |= PARODD;
    }
    /* 无校验 */
    else
    {
        uart_setting.c_cflag &= ~PARENB;
    }

    if (uart->stop_bit)
    {
        uart_setting.c_cflag |= CSTOPB;
    }
    else
    {
        uart_setting.c_cflag &= ~CSTOPB;
    }
    uart_setting.c_cflag &= ~CSIZE;
    uart_setting.c_cflag |= uart->data_bit;

    /* 禁止将输出的 NL 转换为 CR-NL */
    uart_setting.c_oflag &= ~ONLCR;
    uart_setting.c_lflag &= ~(ICANON | ECHO | ECHOE | ISIG);
    if((tcsetattr(fd, TCSANOW, &uart_setting)) != 0)
    {
        printf("Failed in Setting attributes");
        goto end;
    }

    uart->fd = fd;
    ret = 0;

    //write(fd, "RED", sizeof("RED"));
end:
    return ret;
}

/**
  * @brief 计算异或的和
  * @param uint8_t *src: 
  * @param int len: 
  * retval 异或和.
  */
static uint8_t _get_xor(uint8_t *src, int len)
{
    int i = 0;
    uint8_t xor_ans = 0;

    for (; i < len; i++)
    {
        xor_ans ^= src[i];
    }

    return xor_ans;
}

static void _do_with_turntable_left(led_device_t* devp, std::string message)
{
    uint16_t level = (uint16_t)stoi(message);
    uint8_t buffer[12] = {0X7E, 0X08 /* 帧长 */, 0X80, 0X11, 1 + devp->uart.index, 0X11 /* 左边手动 */,
        0X00, level, 0X00, 0X00, 0X00 /* 校验和 */, 0XE7};

    buffer[10] = _get_xor(&buffer[2], 8);
    write(devp->uart.fd, buffer, sizeof(buffer));
    red_debug_lite("left:%u", level);
}

static void _do_with_turntable_right(led_device_t* devp, std::string message)
{
    uint16_t level = (uint16_t)stoi(message);
    uint8_t buffer[12] = {0X7E, 0X08 /* 帧长 */, 0X80, 0X11, 1 + devp->uart.index, 0X21 /* 右边手动 */,
        0X00, level, 0X00, 0X00, 0X00 /* 校验和 */, 0XE7};

    buffer[10] = _get_xor(&buffer[2], 8);
    write(devp->uart.fd, buffer, sizeof(buffer));
    red_debug_lite("right:%s", message.c_str());
}

static void _do_with_turntable_down(led_device_t* devp, std::string message)
{
    uint16_t level = (uint16_t)stoi(message);
    uint8_t buffer[12] = {0X7E, 0X08 /* 帧长 */, 0X80, 0X11, 1 + devp->uart.index, 0X41 /* 下边手动 */,
        0X00, 0X00, 0X00, level, 0X00 /* 校验和 */, 0XE7};

    buffer[10] = _get_xor(&buffer[2], 8);
    write(devp->uart.fd, buffer, sizeof(buffer));
    red_debug_lite("down:%s", message.c_str());
}

static void _do_with_turntable_up(led_device_t* devp, std::string message)
{
    uint16_t level = (uint16_t)stoi(message);
    uint8_t buffer[12] = {0X7E, 0X08 /* 帧长 */, 0X80, 0X11, 1 + devp->uart.index, 0X31 /* 上边手动 */,
        0X00, 0X00, 0X00, level, 0X00 /* 校验和 */, 0XE7};

    buffer[10] = _get_xor(&buffer[2], 8);
    write(devp->uart.fd, buffer, sizeof(buffer));
    red_debug_lite("up:%s", message.c_str());
}

static void _do_with_turntable_stop(led_device_t* devp, std::string message)
{
    uint16_t level = (uint16_t)stoi(message);
    uint8_t buffer[12] = {0X7E, 0X08 /* 帧长 */, 0X80, 0X11, 1 + devp->uart.index, 0X01 /* 停止手动 */,
        0XFF, 0XFF, 0XFF, 0XFF, 0X00 /* 校验和 */, 0XE7};

    buffer[10] = _get_xor(&buffer[2], 8);
    write(devp->uart.fd, buffer, sizeof(buffer));
    red_debug_lite("stop:%s", message.c_str());
}

static void _do_with_turntable_mode_track(led_device_t* devp, std::string message)
{
    uint8_t buffer[12] = {0X7E, 0X08 /* 帧长 */, 0X80, 0X11, 1 + devp->uart.index, 0X03 /* 追踪模式 */,
        0XFF, 0XFF, 0XFF, 0XFF, 0X00 /* 校验和 */, 0XE7};

    buffer[10] = _get_xor(&buffer[2], 8);
    write(devp->uart.fd, buffer, sizeof(buffer));
    red_debug_lite("track:%s", message.c_str());
}

static void _do_with_turntable_mode_scan(led_device_t* devp, std::string message)
{
    uint8_t buffer[12] = {0X7E, 0X08 /* 帧长 */, 0X80, 0X11, 1 + devp->uart.index, 0X32 /* 扫海 */,
        0XFF, 0XFF, 0XFF, 0XFF, 0X00 /* 校验和 */, 0XE7};

    buffer[10] = _get_xor(&buffer[2], 8);
    write(devp->uart.fd, buffer, sizeof(buffer));
    red_debug_lite("scan:%s", message.c_str());
}

static void _do_with_turntable_mode_setting(led_device_t* devp, std::string message)
{
    uint8_t mode = (uint8_t)stoi(message);

    switch (mode)
    {
        case TURNTABLE_TRACK_MODE:
            _do_with_turntable_mode_track(devp, message);
            break;
        case TURNTABLE_SCAN_MODE:
            _do_with_turntable_mode_scan(devp, message);
            break;
        case TURNTABLE_MANUAL_MODE:
            /* 切换手动模式时，直接停机 */
            _do_with_turntable_stop(devp, message);
            break;
        default:
            break;
    }
    red_debug_lite("mode_setting:%s", message.c_str());
}

static void _do_with_turntable_track_setting(led_device_t* devp, std::string message)
{
    int16_t x_pos, y_pos;
    uint16_t level = (uint16_t)stoi(message);
    sscanf(message.c_str(), "%hd,%hd", &x_pos, &y_pos);
    uint8_t buffer[14] = {0X7E, 0X0A /* 帧长 */, 0X82, 0X11, 1 + devp->uart.index, 0X01 /* 停止手动,目标有效 */,
        x_pos >> 8, x_pos, y_pos >> 8, y_pos, 0X00 /* 不调焦 */, 0X00/* 不调视场 */, 0X00 /* 校验和 */, 0XE7};

    buffer[12] = _get_xor(&buffer[2], 0X0A);
    write(devp->uart.fd, buffer, sizeof(buffer));

    red_debug_lite("track_setting:%s", message.c_str());
}

static void _do_with_turntable_position_setting(led_device_t* devp, std::string message)
{
    int16_t direction, elevation;
    uint16_t level = (uint16_t)stoi(message);
    sscanf(message.c_str(), "%hd,%hd", &direction, &elevation);
    uint8_t buffer[12] = {0X7E, 0X08 /* 帧长 */, 0X80, 0X11, 1 + devp->uart.index, 0X51 /* 手动,角度 */,
        direction >> 8, direction, elevation >> 8, elevation, 0X00 /* 校验和 */, 0XE7};

    buffer[12] = _get_xor(&buffer[2], 0X08);
    write(devp->uart.fd, buffer, sizeof(buffer));

    red_debug_lite("position_setting:%s", message.c_str());
}
static void _do_with_focal(led_device_t* devp, std::string message)
{
    uint8_t buffer[14] = {0X7E, 0X0A /* 帧长 */, 0X82, 0X11, 1 + devp->uart.index, 0X00 /* 坐标无效 */,
        0XFF, 0XFF, 0XFF, 0XFF, 0X00 /* 不调焦 */, 0X00/* 不调视场 */, 0X00 /* 校验和 */, 0XE7};

    if ('-' == message.c_str()[0])
    {
        buffer[10] = 2;
    }
    else if ('+' == message.c_str()[0])
    {
        buffer[10] = 1;
    }
    buffer[12] = _get_xor(&buffer[2], 0X0A);
    write(devp->uart.fd, buffer, sizeof(buffer));

    red_debug_lite("focal:%s", message.c_str());
}

static void _do_with_green_mocode(led_device_t* devp, std::string message)
{
    /* 莫码长度 */
    int len = message.length();
    /* TODO 检查莫码长度最大不超过 255 - 0X08 字节 */
    /* 莫码最大长度不超过 255 */
    uint8_t buffer[300] = {0X7E, 0X00 /* 帧长 */, 0X81, 0X11, 1 + devp->uart.index, 0XFF, 0X01, 0XFF,
        0X02, 0X00 /* 参数长度 */, 0XFF /* 莫码内容 */, 0X00};

    /* 帧长度 */
    buffer[1] = (uint8_t)(len + 0X08);
    /* 莫码长度 */
    buffer[9] = (uint8_t)len;
    memcpy(&buffer[10], message.c_str(), len);
    buffer[10 + len] = _get_xor(&buffer[2], len + 0X08);
    buffer[11 + len] = 0XE7;
    write(devp->uart.fd, buffer, 12 + len);
    red_debug_lite("mocode:%s", message.c_str());
}

static void _do_with_green_blink(led_device_t* devp, std::string message)
{
    uint8_t freq = (uint8_t)stoi(message);
    uint8_t buffer[13] = {0X7E, 0X09 /* 帧长 */, 0X81, 0X11, 1 + devp->uart.index, 0XFF,
        0X01, 0XFF, 0X01, 0X01, freq, 0X00 /* 校验和 */, 0XE7};

    buffer[11] = _get_xor(&buffer[2], 9);
    write(devp->uart.fd, buffer, sizeof(buffer));
    red_debug_lite("blink:%u", freq);
}

static void _do_with_green_normal(led_device_t *devp, std::string message)
{
    uint8_t level = (uint8_t)stoi(message);
    uint8_t buffer[13] = {0X7E, 0X09 /* 帧长 */, 0X81, 0X11, 1 + devp->uart.index, 0XFF,
        0X01, 0XFF, 0X00, 0X01, level, 0X00 /* 校验和 */, 0XE7};

    buffer[11] = _get_xor(&buffer[2], 9);
    red_debug_lite("normal:%u", level);
    write(devp->uart.fd, buffer, sizeof(buffer));
}

static void _do_with_white_blink(led_device_t* devp, std::string message)
{
    uint8_t freq = (uint8_t)stoi(message);
    uint8_t buffer[13] = {0X7E, 0X09 /* 帧长 */, 0X81, 0X11, 1 + devp->uart.index, 0X01,
        0X01, freq, 0XFF, 0X01, 0XFF, 0X00 /* 校验和 */, 0XE7};

    buffer[11] = _get_xor(&buffer[2], 9);
    write(devp->uart.fd, buffer, sizeof(buffer));
    red_debug_lite("blink:%u", freq);
}

static void _do_with_white_mocode(led_device_t* devp, std::string message)
{
    /* 莫码长度 */
    int len = message.length();
    /* TODO 检查莫码长度最大不超过 255 - 0X08 字节 */
    /* 莫码最大长度不超过 255 */
    uint8_t buffer[300] = {0X7E, 0X00 /* 帧长 */, 0X81, 0X11, 1 + devp->uart.index, 0X02 /* 发送莫码 */,
        0X00 /* 参数长度 */, 0XFF /* 莫码内容 */, 0X00};

    /* 帧长度 */
    buffer[1] = (uint8_t)(len + 0X08);
    /* 莫码长度 */
    buffer[6] = (uint8_t)len;
    memcpy(&buffer[7], message.c_str(), len);
    buffer[7 + len] = 0XFF;
    buffer[8 + len] = 0X01;
    buffer[9 + len] = 0XFF;
    buffer[10 + len] = _get_xor(&buffer[2], len + 0X08);
    buffer[11 + len] = 0XE7;
    write(devp->uart.fd, buffer, 12 + len);
    red_debug_lite("mocode:%s", message.c_str());
}

static void _do_with_white_normal(led_device_t *devp, std::string message)
{
    uint8_t level = (uint8_t)stoi(message);
    uint8_t buffer[13] = {0X7E, 0X09 /* 帧长 */, 0X81, 0X11, 1 + devp->uart.index, 0X00,
        0X01, level, 0XFF, 0X01, 0XFF, 0X00 /* 校验和 */, 0XE7};

    buffer[11] = _get_xor(&buffer[2], 9);
    red_debug_lite("normal:%u", level);
    write(devp->uart.fd, buffer, sizeof(buffer));
}

static int _do_analysis_hear_msg(int index, char * buffer, int len)
{
    uint8_t dev_status;
    uint8_t white_mode, white_mode_param;
    uint8_t green_mode, green_mode_param;
    uint8_t turntable_mode;
    int16_t turntable_horizon, turntable_vertical;
    uint16_t turntable_horizon_speed, turntable_vertical_speed;
    uint16_t camera_falcon; /* 摄像头焦距 */

    dev_status = buffer[2];
    white_mode = buffer[3];
    white_mode_param = buffer[4];
    green_mode = buffer[5];
    green_mode_param = buffer[6];
    turntable_mode = buffer[7];
    turntable_horizon = buffer[8] << 8 | buffer[9];
    turntable_horizon /= 100;
    turntable_vertical = buffer[10] << 8 | buffer[11];
    turntable_vertical /= 100;
    turntable_horizon_speed = buffer[12] << 8 | buffer[13];
    turntable_vertical_speed = buffer[14] << 8 | buffer[15];
    camera_falcon = buffer[16];

    gs_led_devices[index].screen->get_dev_state_label(index)->set_caption(dev_status ? "故障" : "正常");
    gs_led_devices[index].screen->get_dev_angle_label(index)->set_caption(to_string(turntable_horizon) + '/' + to_string(turntable_vertical));
    gs_led_devices[index].screen->get_dev_angular_speed_label(index)->set_caption(to_string(turntable_horizon_speed) + '/' + to_string(turntable_vertical_speed));

    red_debug_lite("valid heart msg:dev_status:%u %u|%u %u|%u", dev_status, turntable_horizon, turntable_vertical, turntable_horizon_speed, turntable_vertical_speed);
}

static int get_device_heart_msg(int index)
{
#define HEAR_MSG_LEN    22
    char buffer[64] = {0};
    int ret;

    /* TODO check index valid */
    if (index > 1)
    {
        red_debug_lite("invalid index:%d", index);
        return -E2BIG;
    }

    ret = read(gs_led_devices[index].uart.fd, buffer, sizeof buffer);

    if (ret < HEAR_MSG_LEN)
    {
        return -1;
    }
    /* 打印获取的心跳信息 */
    RedDebug::hexdump("HEART", buffer, ret);

    if (buffer[0] != 0X7E || buffer[ret - 1] != 0XE7)
    {
        return -2;
    }
    else if (buffer[1] != 0X12 || buffer[2] != 0XC0 || buffer[3] != index + 1)
    {
        /*
         * 协议编号、或者长度不匹配
         * */
        return -3;
    }
    /* TODO 处理心跳数据 */
    _do_analysis_hear_msg(index, &buffer[2], 0X12);

    return ret;
}

static void *heart_msg_entry(void *arg)
{
    led_device_t * led_devp = (led_device_t *)arg;

    while(1)
    {
        /* 尝试读取心跳信息 */
        get_device_heart_msg(led_devp->uart.index);
        sleep(1);
    }
}

void *devices_entry(void *arg)
{
    led_device_t * led_devp = (led_device_t *)arg;
    Led3000Window * screen = led_devp->screen;
    std::string msg_payload;
    int msg_id;
    char thread_name[16] = {0};

    /* 修改线程名 */
    snprintf(thread_name, sizeof(thread_name), "devices%d", led_devp->uart.index);

    prctl(PR_SET_NAME, thread_name);
    /* TODO init uart */
    if (init_uart_port(&led_devp->uart) != 0)
    {
        red_debug_lite("Failed init %s.", led_devp->uart.name);
        return NULL;
    }

    std::thread gsDeviceHeartThread(heart_msg_entry, led_devp);
    gsDeviceHeartThread.detach();

    while(1)
    {
        auto m = screen->getDeviceQueue(led_devp->uart.index).get();
        if (m)
        {
            auto& dm = dynamic_cast<PolyM::DataMsg<std::string>&>(*m);
            msg_id = dm.getMsgId();
            msg_payload = dm.getPayload();
            red_debug_lite("Device%d thread :%u@%s", led_devp->uart.index, msg_id, msg_payload.c_str());
        }

        switch (msg_id)
        {
            case POLYM_FOCAL_SETTING:
                _do_with_focal(led_devp, msg_payload);
                break;
            case POLYM_GREEN_MOCODE_SETTING:
                _do_with_green_mocode(led_devp, msg_payload);
                break;
            case POLYM_GREEN_BLINK_SETTING:
                _do_with_green_blink(led_devp, msg_payload);
                break;
            case POLYM_GREEN_NORMAL_SETTING:
                _do_with_green_normal(led_devp, msg_payload);
                break;
            case POLYM_WHITE_MOCODE_SETTING:
                _do_with_white_mocode(led_devp, msg_payload);
                break;
            case POLYM_WHITE_BLINK_SETTING:
                _do_with_white_blink(led_devp, msg_payload);
                break;
            case POLYM_WHITE_NORMAL_SETTING:
                _do_with_white_normal(led_devp, msg_payload);
                break;
            case POLYM_TURNTABLE_LEFT_SETTING:
                _do_with_turntable_left(led_devp, msg_payload);
                break;
            case POLYM_TURNTABLE_RIGHT_SETTING:
                _do_with_turntable_right(led_devp, msg_payload);
                break;
            case POLYM_TURNTABLE_DOWN_SETTING:
                _do_with_turntable_down(led_devp, msg_payload);
                break;
            case POLYM_TURNTABLE_UP_SETTING:
                _do_with_turntable_up(led_devp, msg_payload);
                break;
            case POLYM_TURNTABLE_STOP_SETTING:
                _do_with_turntable_stop(led_devp, msg_payload);
                break;
            case POLYM_TURNTABLE_MODE_SETTING:
                _do_with_turntable_mode_setting(led_devp, msg_payload);
                break;
            case POLYM_TURNTABLE_TRACK_SETTING:
                _do_with_turntable_track_setting(led_devp, msg_payload);
                break;
            case POLYM_TURNTABLE_POSITION_SETTING:
                _do_with_turntable_position_setting(led_devp, msg_payload);
                break;
            default:
                red_debug_lite("No support this id:%d", msg_id);
                break;
        }
    }
}

void *devices_thread(void *arg)
{
    gs_led_devices[0].screen = (Led3000Window *)arg;
    gs_led_devices[1].screen = (Led3000Window *)arg;
    red_debug_lite("arg=%p", arg);

    std::thread gsDevice0Thread(devices_entry, &gs_led_devices[0]);
    std::thread gsDevice1Thread(devices_entry, &gs_led_devices[1]);

    gsDevice0Thread.detach();
    gsDevice1Thread.detach();

    return NULL;
}
