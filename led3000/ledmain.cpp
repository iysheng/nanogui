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
#include <debug.h>
#include <sys/prctl.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <fcntl.h>
#include <signal.h>
#include <errno.h>

using std::cout;
using std::cerr;
using std::endl;

#undef main

extern void *devices_thread(void *arg);
extern void *json_thread(void *arg);
extern void *network_thread(void *arg);
extern void *joystick_thread(void *arg);

using namespace nanogui;
using namespace rapidjson;

#define BACKTRACE_DEBUG
#ifdef BACKTRACE_DEBUG
#include <execinfo.h>

typedef void (*signal_func4trace_t)(int sig, siginfo_t *info, void *secret);

static void signal_func4trace(int sig, siginfo_t *info, void *secret)
{
#define BT_BUF_SIZE 100
    int j, nptrs;
    void *buffer[BT_BUF_SIZE];
    char pthread_name[16] = {0};
    char **strings;

    nptrs = backtrace(buffer, BT_BUF_SIZE);
    printf("backtrace() returned %d addresses sig=%d\n", nptrs, sig);

    /* The call backtrace_symbols_fd(buffer, nptrs, STDOUT_FILENO)
       would produce similar output to the following: */

    strings = backtrace_symbols(buffer, nptrs);
    if (strings == NULL)
    {
        perror("backtrace_symbols");
        exit(EXIT_FAILURE);
    }

    for (j = 0; j < nptrs; j++)
        printf("%s\n", strings[j]);

    free(strings);
    if (!prctl(PR_GET_NAME, pthread_name))
        printf("thread_name=%s\n", pthread_name);
    else
        perror("failed to get thread name\n");

    exit(0);
}

static int init_backtrace(signal_func4trace_t func4trace)
{
    struct sigaction action4backtrace = {0};

    action4backtrace.sa_sigaction = func4trace;
    sigemptyset(&action4backtrace.sa_mask);
    action4backtrace.sa_flags = SA_RESTART | SA_SIGINFO;
    sigaction(SIGINT, &action4backtrace, NULL);
    sigaction(SIGILL, &action4backtrace, NULL);
    sigaction(SIGABRT, &action4backtrace, NULL);
    sigaction(SIGFPE, &action4backtrace, NULL);
    sigaction(SIGSEGV, &action4backtrace, NULL);
    sigaction(SIGTERM, &action4backtrace, NULL);

    sigaction(SIGQUIT, &action4backtrace, NULL);
    sigaction(SIGTRAP, &action4backtrace, NULL);
    sigaction(SIGKILL, &action4backtrace, NULL);
    sigaction(SIGSYS, &action4backtrace, NULL);
    sigaction(SIGBUS, &action4backtrace, NULL);
    sigaction(SIGSYS, &action4backtrace, NULL);

    return 0;
}

#endif


int g_test_btn_len;
int main(int /* argc */, char ** /* argv */)
{
#ifdef BACKTRACE_DEBUG
    init_backtrace(&signal_func4trace);
#endif
    RedDebug::init("/tmp/ledmain_log.txt");
    RedDebug::print_soft_info();
    /* 创建了测试窗口类 */
    try
    {
        nanogui::init();

        /* scoped variables */ {
            /* 赋值的时候，会执行 ref 类模板的符号重载，然后会增加这个引用计数 */
            nanogui::ref<Led3000Window> app = new Led3000Window();

            std::thread s_json_thread(json_thread, app);
            s_json_thread.detach();

            std::thread s_device_thread(devices_thread, app);
            s_device_thread.detach();

            std::thread s_joystick_thread(joystick_thread, app);
            s_joystick_thread.detach();

            std::thread s_network_thread(network_thread, app);
            s_network_thread.detach();

            /* 减少引用计数 */
            app->dec_ref();
            app->set_background(Color(0x17, 0x30, 0x69, 0xff));
            /* 调用基类的 draw_all 函数 */
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
