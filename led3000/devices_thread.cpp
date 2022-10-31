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

    fd = open(uart->name, O_RDWR | O_NOCTTY | O_NDELAY);
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
    if (uart->even)
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

    if((tcsetattr(fd, TCSANOW, &uart_setting)) != 0)
    {
        printf("Failed in Setting attributes");
        goto end;
    }

    uart->fd = fd;
    ret = 0;

    write(fd, "RED", sizeof("RED"));
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

static void _do_with_turntable_mode_setting(led_device_t* devp, std::string message)
{
    red_debug_lite("mode_setting:%s", message.c_str());
}

static void _do_with_turntable_track_setting(led_device_t* devp, std::string message)
{
    int16_t x_pos, y_pos;
    uint16_t level = (uint16_t)stoi(message);
    uint8_t buffer[14] = {0X7E, 0X0A /* 帧长 */, 0X82, 0X11, 1 + devp->uart.index, 0X01 /* 停止手动 */,
        x_pos >> 8, x_pos, y_pos >> 8, y_pos, 0X00 /* 不调焦 */, 0X00/* 不调视场 */, 0X00 /* 校验和 */, 0XE7};
    sscanf(message.c_str(), "%hd,%hd", &x_pos, &y_pos);

    buffer[12] = _get_xor(&buffer[2], 0X0B);
    write(devp->uart.fd, buffer, sizeof(buffer));

    red_debug_lite("track_setting:%s", message.c_str());
}

static void _do_with_focal(led_device_t* devp, std::string message)
{
    red_debug_lite("focal:%s", message.c_str());
}

static void _do_with_green_mocode(led_device_t* devp, std::string message)
{
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

static void _do_with_white_normal(led_device_t *devp, std::string message)
{
    uint8_t level = (uint8_t)stoi(message);
    uint8_t buffer[13] = {0X7E, 0X09 /* 帧长 */, 0X81, 0X11, 1 + devp->uart.index, 0X00,
        0X01, level, 0XFF, 0X01, 0XFF, 0X00 /* 校验和 */, 0XE7};

    buffer[11] = _get_xor(&buffer[2], 9);
    red_debug_lite("normal:%u", level);
    write(devp->uart.fd, buffer, sizeof(buffer));
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
