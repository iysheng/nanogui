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

void *joystick_thread(void *arg)
{
    Led3000Window * screen = (Led3000Window *)arg;
    std::string msg_payload;
    int msg_id;
    int fd, ret;
    int present, axis_count;
    const float* axes;
    char thread_name[16] = {0};
    /* 修改线程名 */
    snprintf(thread_name, sizeof(thread_name), "joystick");

    prctl(PR_SET_NAME, thread_name);

    red_debug_lite("Hello joystick");
    for (present = GLFW_JOYSTICK_1; present < GLFW_JOYSTICK_LAST; present++)
    {
        if (glfwJoystickPresent(present))
        {
            string joyname(glfwGetJoystickName(present));
            std::cout << joyname << std::endl;
            /* FIXME 根据名称确定是否是真实的操纵杆 */
            if (string::npos != joyname.find("joystick"))
            {
                red_debug_lite("<<<<<<<<<<<<<<<< Find the joystick name:%s", glfwGetJoystickName(present) ? : "ohno");
                break;
            }
        }
    }
    red_debug_lite("Oh no Hello joystick");

    int i;
    while(1)
    {
        axes = glfwGetJoystickAxes(present, &axis_count);
        if (!axes)
            continue;

        for (i = 0; i < axis_count; i++)
        {
            red_debug_lite("%f@%d", axes[i], i);
        }
    }
}
