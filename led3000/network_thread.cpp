/******************************************************************************
* File:             network_thread.cpp
*
* Author:           yangyongsheng@jari.cn  
* Created:          09/05/22 
* Description:      指控通信的网络线程
*****************************************************************************/

#include <sys/prctl.h>
#include "PolyM/include/polym/Msg.hpp"
#include <nanogui/common.h>
#include <led3000gui.h>
#include <thread>
#include <unistd.h>
#include <PolyM/include/polym/Queue.hpp>
#include <network_udp.h>
#include <network_protocol.h>

using namespace nanogui;
using namespace std;

typedef struct {
    char name[32];
    NetworkUdp udp;
    int fd_type;
    Led3000Window *screen;
} network_fd_t;

static network_fd_t gs_network_fd[2] = {
    {
        {.name = "guide_client"},
        .fd_type = NETWORK_PROTOCOL_TYPE_SEND_GUIDE,
    },
    {
        {.name = "guide_broadcast"},
        .fd_type = NETWORK_PROTOCOL_TYPE_SEND_GUIDE_BROADCAST,
    }
};

void *network_entry(void *arg)
{
    network_fd_t * network_devp = (network_fd_t *)arg;
    char buffer_recv[256] = {0};
    int len;

    if (strlen(network_devp->name))
        prctl(PR_SET_NAME, network_devp->name);

    network_devp->udp.send2server("Hello World", strlen("Hello World"));
    printf("send hello world to test\n");
    while(1)
    {
        len = network_devp->udp.recv_from_server(buffer_recv, sizeof(buffer_recv));
        if (len > 0)
            handle_with_network_buffer(buffer_recv, len);
    }
}

void *network_thread(void *arg)
{
    Led3000Window *screen = (Led3000Window *)arg;
    red_debug_lite("Hello Network Thread json file path:%s", screen->getFileName().c_str());
    red_debug_lite("connect:%s@%u", screen->getJsonValue()->server.ip, screen->getJsonValue()->server.port);

    /* 创建和指控广播组通信的句柄 */
    /* TODO just for test */
    gs_network_fd[0].screen = screen;
    gs_network_fd[1].screen = screen;
    NetworkUdp udp_client = NetworkUdp("192.168.1.111", screen->getJsonValue()->server.port, screen->getJsonValue()->server.port);
    gs_network_fd[0].udp = udp_client;
    /* 创建和指控通信的句柄 */
    NetworkUdp udp_broadcast_client = NetworkUdp("192.168.2.111", screen->getJsonValue()->server.port, screen->getJsonValue()->server.port);
    gs_network_fd[1].udp = udp_broadcast_client;
    screen_window_register(screen);

    if (gs_network_fd[0].udp.get_socket() > 0)
    {
        if (network_protocol_registe(NETWORK_PROTOCOL_TYPE_SEND_GUIDE, gs_network_fd[0].udp) < 0)
        {
            red_debug_lite("Failed register network protocol.");
        }
        else
        {
            red_debug_lite("Register guide socket success.");
        }
    }

    /* TODO just for test */
    if (gs_network_fd[1].udp.get_socket() > 0)
    {
        if (network_protocol_registe(NETWORK_PROTOCOL_TYPE_SEND_GUIDE_BROADCAST, gs_network_fd[1].udp) < 0)
        {
            red_debug_lite("Failed register network fd broadcast.");
        }
        else
        {
            red_debug_lite("Register guide broadcast socket success.");
        }
    }

    std::thread gsNetwork0Thread(network_entry, &gs_network_fd[0]);
    std::thread gsNetwork1Thread(network_entry, &gs_network_fd[1]);

    gsNetwork0Thread.detach();
    gsNetwork1Thread.detach();
    
    while(1)
    {
        /* 保活临时构造的 NetworkUdp 对象 */
        sleep(100000);
    }
}
