/******************************************************************************
* File:             burn287_devices.cpp
*
* Author:           yangyongsheng819@163.com  
* Created:          05/05/23 
* Description:      设备访问线程
*****************************************************************************/

#include "burn287.h"
#include <termios.h>
#include <unistd.h>
#include <cstdio>
#include <iostream>
#include <unistd.h>
#include <thread>
#include <fcntl.h>

using namespace std;

typedef struct {
    char name[32];
    int fd;
    unsigned int baud;
    unsigned char data_bit;
    unsigned char stop_bit;
    unsigned char even;
    unsigned char index;
    int offline_counts;
} uartport_t;

typedef struct {
    int uart_fd;
    uartport_t uart;
    RedBurntool *screen;
    pthread_mutex_t uart_mtx;
} led_device_t;

typedef struct {
    char key_words[32];
    char respon_words[32];
    int (*append_hander_func)(int arg);
} msg_handler_map;

static led_device_t gs_led_devices = {
    .uart_fd = -1,
    .uart = {
        {.name = "/dev/ttyUSB0"}, /* C++ 要这样初始化，否则会报编译错误 */
        .baud = B115200,
        .data_bit = CS8,
        .stop_bit = 1, /* 一个停止位 */
        .even = 0, /* 无校验 */
        .index = 0,
        .offline_counts = 0,
    },
    .uart_mtx = PTHREAD_MUTEX_INITIALIZER,
};

int init_uart_port(uartport_t *uart)
{
    int ret = 0, fd = -1;
    struct termios uart_setting;

    if (!uart) {
        ret = -EINVAL;
        goto end;
    }

    ////// 参考周立功提供的串口回环测试修改
    /// 添加 O_NONBLOCK 判断离线
    fd = open(uart->name, O_RDWR | O_NOCTTY);
    if (fd <= 0) {
        //RedDebug::log("Failed open %s err=%d.", uart->name, fd);
        return -1;
    }

    bzero(&uart_setting, sizeof(uart_setting));
    tcgetattr(fd, &uart_setting);

    cfsetispeed(&uart_setting, uart->baud);
    cfsetospeed(&uart_setting, uart->baud);

    /* 偶校验 */
    if (uart->even == 1) {
        uart_setting.c_cflag |= PARENB;
        uart_setting.c_cflag &= ~PARODD;
    }
    /* 奇校验 */
    else if (uart->even == 2) {
        uart_setting.c_cflag |= PARENB;
        uart_setting.c_cflag |= PARODD;
    }
    /* 无校验 */
    else {
        uart_setting.c_cflag &= ~PARENB;
    }

    if (uart->stop_bit) {
        uart_setting.c_cflag |= CSTOPB;
    } else {
        uart_setting.c_cflag &= ~CSTOPB;
    }

    uart_setting.c_cflag &= ~CRTSCTS;
    uart_setting.c_cflag &= ~CSIZE;
    uart_setting.c_cflag |= uart->data_bit;

    /* 禁止将输出的 NL 转换为 CR-NL */
    uart_setting.c_oflag &= ~ONLCR;
    uart_setting.c_lflag &= ~(ICANON | ECHO | ECHOE | ISIG);
    ////// 参考周立功提供的串口回环测试修改
    uart_setting.c_lflag   &=   ~(ECHO   |   ICANON   |   IEXTEN   |   ISIG);
    uart_setting.c_iflag   &=   ~(BRKINT   |   ICRNL   |   INPCK   |   ISTRIP   |   IXON);
    uart_setting.c_oflag   &=   ~(OPOST);
    uart_setting.c_cflag   &=   ~(CSIZE   |   PARENB);
    uart_setting.c_cflag   |=   CS8;
    ///////
    if ((tcsetattr(fd, TCSANOW, &uart_setting)) != 0) {
        printf("Failed in Setting attributes");
        goto end;
    }

    uart->fd = fd;

end:
    return fd;
}

static int do_uboot_upkernel(int uart)
{
#define SET_UBOOT_ENV         "setenv ipaddr 192.168.100.100;setenv serverip 192.168.100.200\n"
#define RUN_UBOOT_UPKERNEL    "run upkernel"
    if (uart < 0)
    {
        printf("invalid uart device\n");
        return -1;
    }

    write(uart, SET_UBOOT_ENV, strlen(SET_UBOOT_ENV));
    tcdrain(uart);
    sleep(1);
    write(uart, RUN_UBOOT_UPKERNEL, strlen(RUN_UBOOT_UPKERNEL));
    tcdrain(uart);

    return 0;
}

static msg_handler_map gs_uart_handler_maps[] = {
    {"zlg/ZLG", "zlg", nullptr},
    {"U-Boot", "", do_uboot_upkernel},
    {"login:", "root\n", nullptr},
    {"password:", "root\n", nullptr},

};

static int do_with_recv_msg(std::string msg, int fd)
{
    if (fd < 0)
        return -EINVAL;

    for (int i = 0; i < ARRAY_SIZE(gs_uart_handler_maps); i++)
    {
        /* 如果没有找到对应的关键词 */
        if (msg.find(gs_uart_handler_maps[i].key_words) != string::npos)
        {
            if (strlen(gs_uart_handler_maps[i].respon_words))
            {
                write(fd, gs_uart_handler_maps[i].respon_words, strlen(gs_uart_handler_maps[i].respon_words));
                sleep(1);
            }
            if (nullptr != gs_uart_handler_maps[i].append_hander_func)
            {
                gs_uart_handler_maps[i].append_hander_func(fd);
            }
        }
    }

    return 0;
}

int open_uart_dev(const char *dev)
{
    if (0 != pthread_mutex_lock(&gs_led_devices.uart_mtx))
        return -1;
    if (gs_led_devices.uart_fd > 0)
    {
        close(gs_led_devices.uart_fd);
        gs_led_devices.uart_fd = -1;
    }

    memcpy(gs_led_devices.uart.name, dev, strlen(dev) + 1);
    gs_led_devices.uart_fd = init_uart_port(&gs_led_devices.uart);
    pthread_mutex_unlock(&gs_led_devices.uart_mtx);
    if (gs_led_devices.uart_fd > 0)
        printf("open uart dev:%s success\n", gs_led_devices.uart.name);
    else
        printf("open uart dev:%s failed err:%d\n", gs_led_devices.uart.name, errno);

    return gs_led_devices.uart_fd;
}

void devices_thread(RedBurntool *app)
{
    int len = 0;
    char buffer[1024] = {0};

    while(1)
    {
        if (0 != pthread_mutex_lock(&gs_led_devices.uart_mtx))
        {
            usleep(10000);
            continue;
        }

        if (gs_led_devices.uart_fd > 0)
        {
            len = read(gs_led_devices.uart_fd, buffer, 1024);
            if (len > 0)
            {
                do_with_recv_msg(std::string(buffer, len), gs_led_devices.uart_fd);
                app->append_log_msg(std::string());
            }
        }
        pthread_mutex_unlock(&gs_led_devices.uart_mtx);
        sleep(1);
    }

    close(gs_led_devices.uart_fd);
    gs_led_devices.uart_fd = -1;
}
