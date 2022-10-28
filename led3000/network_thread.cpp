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

void *network_thread(void *arg)
{
    Led3000Window *screen = (Led3000Window *)arg;
    red_debug_lite("Hello Network Thread json file path:%s", screen->getFileName().c_str());
    red_debug_lite("connect:%s@%u", screen->getJsonValue()->server.ip, screen->getJsonValue()->server.port);

    /* 创建和指控广播组通信的句柄 */
    /* TODO just for test */
    NetworkUdp udp_guide_broadcast_client("10.20.52.35", screen->getJsonValue()->server.port, screen->getJsonValue()->server.port);
    /* 创建和指控通信的句柄 */
    NetworkUdp udp_guide_client(screen->getJsonValue()->server.ip, screen->getJsonValue()->server.port, screen->getJsonValue()->server.port);

    if (udp_guide_client.get_socket() > 0)
    {
        if (network_protocol_registe(NETWORK_PROTOCOL_TYPE_SEND_GUIDE, udp_guide_client) < 0)
        {
            red_debug_lite("Failed register network protocol.");
        }
        else
        {
            red_debug_lite("Register guide socket success.");
        }
    }

    /* TODO just for test */
    if (udp_guide_broadcast_client.get_socket() > 0)
    {
        if (network_protocol_registe(NETWORK_PROTOCOL_TYPE_SEND_GUIDE_BROADCAST, udp_guide_broadcast_client) < 0)
        {
            red_debug_lite("Failed register network fd broadcast.");
        }
        else
        {
            red_debug_lite("Register guide socket success.");
        }
    }
    char buffer_recv[256] = {0};

    prctl(PR_SET_NAME, "network");
    while(1)
    {
#if 0
        test_udp_client.send2server("Hello World", strlen("Hello World"));
        printf("send hello world to test\n");
#endif
        udp_guide_client.recv_from_server(buffer_recv, sizeof(buffer_recv));
    }
}
