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

using namespace nanogui;
using namespace std;

void *network_thread(void *arg)
{
    Led3000Window *screen = (Led3000Window *)arg;
	red_debug_lite("Hello Network Thread json file path:%s", screen->getFileName().c_str());

    prctl(PR_SET_NAME, "network");
    while(1)
    {
        red_debug_lite("Hello Network UDP");
        sleep(1);
    }
}
