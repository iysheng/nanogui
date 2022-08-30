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
    /* 创建了测试窗口类 */
    try
    {
        nanogui::init();

        /* scoped variables */ {
            /* 赋值的时候，会执行 ref 类模板的符号重载，然后会增加这个引用计数 */
            ref<Led3000Window> app = new Led3000Window();

            std::thread sJsonThread(json_thread, app);
            sJsonThread.detach();
        
            std::thread sDevicesThread(devices_thread, app);
            sDevicesThread.detach();

            /* 减少引用计数 */
            app->dec_ref();
            app->draw_all();
            app->set_visible(true);
            nanogui::mainloop(1 / 60.f * 1000);
        }

        nanogui::shutdown();
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