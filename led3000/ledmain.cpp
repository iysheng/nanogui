/*
    nanogui/example1.cpp -- C++ version of an example application that shows
    how to use the various widget classes.

    Based on NanoGUI by Wenzel Jakob <wenzel@inf.ethz.ch>.
    Adaptation for SDL by Dalerank <dalerankn8@gmail.com>

    All rights reserved. Use of this source code is governed by a
    BSD-style license that can be found in the LICENSE.txt file.
*/

#include <atomic>
#include <led3000gui.h>

#include <thread>
#include <unistd.h>

using std::cout;
using std::cerr;
using std::endl;

#undef main

extern void *devices_thread(void *arg);
extern void *json_thread(void *arg);

using namespace nanogui;
using namespace rapidjson;

int g_test_btn_len;
int main(int /* argc */, char ** /* argv */)
{
    char rendername[256] = {0};

    /* 创建了测试窗口类 */
    Led3000Window *screen = new Led3000Window();

    std::thread sJsonThread(json_thread, screen);
    sJsonThread.detach();

    std::thread sDevicesThread(devices_thread, screen);
    sDevicesThread.detach();

    try
    {
        red_debug_lite("test demo");
    }
    catch (const std::runtime_error &e)
    {
        std::string error_msg = std::string("Caught a fatal error: ") + std::string(e.what());
        #if defined(_WIN32)
            MessageBoxA(nullptr, error_msg.c_str(), NULL, MB_ICONERROR | MB_OK);
        #else
            std::cerr << error_msg << endl;
        #endif
        return -1;
    }
    return 0;
}
