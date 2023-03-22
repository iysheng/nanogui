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
    if (strings == NULL) {
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

/* 安装指定信号量的信号处理函数 */
#define LED3000_INSTALL_SIGNAL(n) sigaction(n, &action4backtrace, NULL)

static int init_backtrace(signal_func4trace_t func4trace)
{
    struct sigaction action4backtrace = {0};

    action4backtrace.sa_sigaction = func4trace;
    /* 所有信号都不忽略 */
    sigemptyset(&action4backtrace.sa_mask);
    action4backtrace.sa_flags = SA_RESTART | SA_SIGINFO;

    LED3000_INSTALL_SIGNAL(SIGHUP);
    LED3000_INSTALL_SIGNAL(SIGINT);
    LED3000_INSTALL_SIGNAL(SIGQUIT);
    LED3000_INSTALL_SIGNAL(SIGILL);
    LED3000_INSTALL_SIGNAL(SIGTRAP);
    LED3000_INSTALL_SIGNAL(SIGABRT);
    LED3000_INSTALL_SIGNAL(SIGBUS);
    LED3000_INSTALL_SIGNAL(SIGFPE);
    LED3000_INSTALL_SIGNAL(SIGKILL);
    LED3000_INSTALL_SIGNAL(SIGUSR1);
    LED3000_INSTALL_SIGNAL(SIGSEGV);
    LED3000_INSTALL_SIGNAL(SIGUSR2);
    //LED3000_INSTALL_SIGNAL(SIGPIPE);
    signal(SIGPIPE, SIG_IGN);
    LED3000_INSTALL_SIGNAL(SIGALRM);
    LED3000_INSTALL_SIGNAL(SIGTERM);
    LED3000_INSTALL_SIGNAL(SIGSTKFLT);
    LED3000_INSTALL_SIGNAL(SIGCHLD);
    LED3000_INSTALL_SIGNAL(SIGCONT);
    LED3000_INSTALL_SIGNAL(SIGSTOP);
    LED3000_INSTALL_SIGNAL(SIGTSTP);
    LED3000_INSTALL_SIGNAL(SIGTTIN);
    LED3000_INSTALL_SIGNAL(SIGTTOU);
    LED3000_INSTALL_SIGNAL(SIGURG);
    LED3000_INSTALL_SIGNAL(SIGXCPU);
    LED3000_INSTALL_SIGNAL(SIGXFSZ);
    LED3000_INSTALL_SIGNAL(SIGVTALRM);
    LED3000_INSTALL_SIGNAL(SIGPROF);
    LED3000_INSTALL_SIGNAL(SIGWINCH);
    LED3000_INSTALL_SIGNAL(SIGIO);
    LED3000_INSTALL_SIGNAL(SIGPWR);
    LED3000_INSTALL_SIGNAL(SIGSYS);
    LED3000_INSTALL_SIGNAL(SIGRTMIN);
    LED3000_INSTALL_SIGNAL(SIGRTMIN + 1);
    LED3000_INSTALL_SIGNAL(SIGRTMIN + 2);
    LED3000_INSTALL_SIGNAL(SIGRTMIN + 3);
    LED3000_INSTALL_SIGNAL(SIGRTMIN + 4);
    LED3000_INSTALL_SIGNAL(SIGRTMIN + 5);
    LED3000_INSTALL_SIGNAL(SIGRTMIN + 6);
    LED3000_INSTALL_SIGNAL(SIGRTMIN + 7);
    LED3000_INSTALL_SIGNAL(SIGRTMIN + 8);
    LED3000_INSTALL_SIGNAL(SIGRTMIN + 9);
    LED3000_INSTALL_SIGNAL(SIGRTMIN + 10);
    LED3000_INSTALL_SIGNAL(SIGRTMIN + 11);
    LED3000_INSTALL_SIGNAL(SIGRTMIN + 12);
    LED3000_INSTALL_SIGNAL(SIGRTMIN + 13);
    LED3000_INSTALL_SIGNAL(SIGRTMIN + 14);
    LED3000_INSTALL_SIGNAL(SIGRTMIN + 15);
    LED3000_INSTALL_SIGNAL(SIGRTMAX - 14);
    LED3000_INSTALL_SIGNAL(SIGRTMAX - 13);
    LED3000_INSTALL_SIGNAL(SIGRTMAX - 12);
    LED3000_INSTALL_SIGNAL(SIGRTMAX - 11);
    LED3000_INSTALL_SIGNAL(SIGRTMAX - 10);
    LED3000_INSTALL_SIGNAL(SIGRTMAX - 9);
    LED3000_INSTALL_SIGNAL(SIGRTMAX - 8);
    LED3000_INSTALL_SIGNAL(SIGRTMAX - 7);
    LED3000_INSTALL_SIGNAL(SIGRTMAX - 6);
    LED3000_INSTALL_SIGNAL(SIGRTMAX - 5);
    LED3000_INSTALL_SIGNAL(SIGRTMAX - 4);
    LED3000_INSTALL_SIGNAL(SIGRTMAX - 3);
    LED3000_INSTALL_SIGNAL(SIGRTMAX - 2);
    LED3000_INSTALL_SIGNAL(SIGRTMAX - 1);
    LED3000_INSTALL_SIGNAL(SIGRTMAX);

    return 0;
}

#endif


int g_test_btn_len;
int main(int /* argc */, char ** /* argv */)
{
#ifdef BACKTRACE_DEBUG
    init_backtrace(&signal_func4trace);
#endif
    RedDebug::init("/userdata/ledlogs/ledmain_log.txt");
    RedDebug::print_soft_info();
    /* 创建了测试窗口类 */
    try {
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
    } catch (const std::runtime_error &e) {
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
