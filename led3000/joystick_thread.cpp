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

static Led3000Window *gs_screen = nullptr;

/* X 方向的点位信息 */
#define X_LEFT_POINT     479
#define X_RIGHT_POINT    521

/* Y 方向的点位信息 */
#define Y_LEFT_POINT     479
#define Y_RIGHT_POINT    521

/**
  * @brief 左转运动
  * @param int bias: 
  * retval Linux/errno.
  */
static int do_x_left_bias(short int bias)
{
    red_debug_lite("X LEFT:%d", bias);

    gs_screen->getCurrentDeviceQueue().put(PolyM::DataMsg<std::string>(POLYM_TURNTABLE_LEFT_SETTING, to_string(bias)));
    return 0;
}

/**
  * @brief 右转运动
  * @param short int bias: 
  * retval Linux/errno.
  */
static int do_x_right_bias(short int bias)
{
    red_debug_lite("X RIGHT:%d", bias);
    gs_screen->getCurrentDeviceQueue().put(PolyM::DataMsg<std::string>(POLYM_TURNTABLE_RIGHT_SETTING, to_string(bias)));
    return 0;
}

/**
  * @brief 向下运动
  * @param short int bias: 
  * retval Linux/errno.
  */
static int do_y_down_bias(short int bias)
{
    red_debug_lite("Y DOWN:%d", bias);
    gs_screen->getCurrentDeviceQueue().put(PolyM::DataMsg<std::string>(POLYM_TURNTABLE_DOWN_SETTING, to_string(bias)));
    return 0;
}

/**
  * @brief 向上运动
  * @param short int bias: 
  * retval Linux/errno.
  */
static int do_y_up_bias(short int bias)
{
    red_debug_lite("Y UP:%d", bias);
    gs_screen->getCurrentDeviceQueue().put(PolyM::DataMsg<std::string>(POLYM_TURNTABLE_UP_SETTING, to_string(bias)));
    return 0;
}

/**
  * @brief 向上运动
  * @param int bias: 
  * retval Linux/errno.
  */
static int do_xy_stop(void)
{
    red_debug_lite("XY STOP");
    gs_screen->getCurrentDeviceQueue().put(PolyM::DataMsg<std::string>(POLYM_TURNTABLE_STOP_SETTING, to_string(0)));
    return 0;
}

/**
  * @brief 向上运动
  * @param int bias: 
  * retval Linux/errno.
  */
static int do_with_xy_bias(int x_bias, int y_bias)
{
    if (x_bias < 0)
    {
        x_bias *= -1;
        do_x_left_bias(x_bias & 0xffff);
    }
    else if (x_bias > 0)
    {
        do_x_right_bias(x_bias & 0xffff);
    }

    if (y_bias < 0)
    {
        y_bias *= -1;
        do_y_down_bias(y_bias & 0xffff);
    }
    else if (y_bias > 0)
    {
        do_y_up_bias(y_bias & 0xffff);
    }

    return 0;
}




/**
  * @brief 对坐标信息进处理
  * retval .
  */
static int do_with_handle_axis(float x_axis, float y_axis)
{
    int x_turn_bais = 0, y_turn_bais = 0;

    if (x_axis < X_LEFT_POINT)
    {
        /* TODO 左转 */
        x_turn_bais = x_axis - X_LEFT_POINT;
    }
    else if (x_axis > X_RIGHT_POINT)
    {
        /* TODO 右转 */
        x_turn_bais = x_axis - X_RIGHT_POINT;
    }
    else
    {
        x_turn_bais = 0;
    }

    if (y_axis < Y_LEFT_POINT)
    {
        /* TODO 向上 */
        y_turn_bais = y_axis - Y_LEFT_POINT;
    }
    else if (y_axis > Y_RIGHT_POINT)
    {
        /* TODO 向下 */
        y_turn_bais = y_axis - Y_RIGHT_POINT;
    }
    else
    {
        y_turn_bais = 0;
    }

    if (!x_turn_bais && !y_turn_bais)
    {
        /* TODO 停止转台 */
        do_xy_stop();
    }
    else
    {
        do_with_xy_bias(x_turn_bais, y_turn_bais);
    }

    return 0;
}

void *joystick_thread(void *arg)
{
    gs_screen = (Led3000Window *)arg;
    std::string msg_payload;
    int msg_id;
    int fd, ret;
    int present, axis_count;
    const float* axes;
    char thread_name[16] = {0};
    /* 修改线程名 */
    snprintf(thread_name, sizeof(thread_name), "joystick");

    prctl(PR_SET_NAME, thread_name);

    red_debug_lite("Hello joystick: screen: %p", gs_screen);
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
        /* TODO 检查工作模式，仅仅在手动模式时受操纵杆控制 */
        if (gs_screen->getJsonValue()->devices[gs_screen->getCurrentDevice()].turntable.mode == TURNTABLE_MANUAL_MODE)
            do_with_handle_axis(axes[0], axes[1]);
#if 0
        /* 目前测试
         * X 轴向中心数据是 479 ~ 521
         * Y 轴向中心数据是 479 ~ 521
         * */
        for (i = 0; i < axis_count; i++)
        {
            red_debug_lite("%f@%d", axes[i], i);
        }
#endif
    }
}
