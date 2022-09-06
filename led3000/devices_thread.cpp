/******************************************************************************
* File:             devices_thread.cpp
*
* Author:           yangyongsheng@jari.cn  
* Created:          08/16/22 
* Description:      灯光装置控制线程
*****************************************************************************/

#include <sys/prctl.h>
#include <nanogui/common.h>
#include <led3000gui.h>
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
            {.name = "/dev/ttyS4"}, /* C++ 要这样初始化，否则会报编译错误 */
            .baud = B115200,
            .data_bit = CS8,
            .stop_bit = 1, /* 一个停止位 */
            .even = 1, /* 偶校验 */
            .index = 0,
        },
    },
    {
        .uart = {
            {.name = "/dev/ttyS5"}, /* C++ 要这样初始化，否则会报编译错误 */
            .baud = B115200,
            .data_bit = CS8,
            .stop_bit = 1, /* 一个停止位 */
            .even = 1, /* 偶校验 */
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

    if (uart->even)
    {
        uart_setting.c_cflag |= PARENB;
    }
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

static void _do_with_focal(std::string message, Led3000Window * window)
{
    red_debug_lite("focal:%s", message.c_str());
}

static void _do_with_green_mocode(std::string message, Led3000Window * window)
{
    red_debug_lite("mocode:%s", message.c_str());
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
            msg_payload = dm.getPayload();
            msg_id = dm.getMsgId();
            red_debug_lite("Device%d thread :%u@%s", led_devp->uart.index, msg_id, msg_payload.c_str());
        }

        switch (msg_id)
        {
            case POLYM_FOCAL_SETTING:
                _do_with_focal(msg_payload, screen);
                break;
            case POLYM_GREEN_MOCODE_SETTING:
                _do_with_green_mocode(msg_payload, screen);
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
