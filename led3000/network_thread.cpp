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
#include "network_udp.h"

using namespace nanogui;
using namespace std;

void *network_thread(void *arg)
{
    Led3000Window *screen = (Led3000Window *)arg;
    red_debug_lite("Hello Network Thread json file path:%s", screen->getFileName().c_str());
    NetworkUdp test_udp_client("10.20.52.39", 5168, 5168);

    prctl(PR_SET_NAME, "network");
    while(1)
    {
        //red_debug_lite("Hello Network UDP");
        sleep(10);
        test_udp_client.send2server("Hello World", strlen("Hello World"));
        printf("send hello world to test\n");
    }
}
