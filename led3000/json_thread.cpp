/******************************************************************************
* File:             json_thread.cpp
*
* Author:           Yangyongsheng@jari.cn
* Created:          08/09/22
* Description:      rapidjson 后台线程
*****************************************************************************/

#include <sys/prctl.h>
#include "PolyM/include/polym/Msg.hpp"
#include <nanogui/common.h>
#include <led3000gui.h>

#include <rapidjson/pointer.h>
#include <rapidjson/filereadstream.h>
#include <rapidjson/filewritestream.h>
#include <rapidjson/document.h>     // rapidjson's DOM-style API
#include <rapidjson/istreamwrapper.h>
#include <rapidjson/ostreamwrapper.h>
#include <fstream>
#include <rapidjson/prettywriter.h> // for stringify JSON
#include <rapidjson/writer.h>
#include <cstdio>
#include <iostream>
#include <unistd.h>
#include <thread>

#include <PolyM/include/polym/Queue.hpp>

/* 最大频率 15 */
#define BLINK_MAX_FREQ       15
/* 亮度等级最大为 10 */
#define NORMAL_MAX_LEVEL     10

using std::cout;
using std::cerr;
using std::endl;

using namespace nanogui;
using namespace std;

void just_show_sysinfo(led3000_config_t *config)
{
    int i;
    assert(config);

    printf("version:%u id:%u\n", config->sys_config.version, config->sys_config.id);
    printf("%-16s %-16s %-16s %-16s\n", "name", "ip", "netmask", "gateway");
    for (i = 0; i < 2; i++) {
        printf("%-16s %-16s %-16s %-16s\n", config->eths[i].name, config->eths[i].ip, config->eths[i].netmask, config->eths[i].gateway);
    }
    printf("server:%s port:%hu\n", config->server.ip, config->server.port);

    printf("%-16s %-16s %-16s %-s\n", "white:mode", "normal_status", "blink_freq", "mocode");
    for (i = 0; i < 2; i++) {
        printf("%-16u %-16u %-16u %-s\n", config->devices[i].white_led.mode,
               config->devices[i].white_led.normal_status,
               config->devices[i].white_led.blink_freq,
               config->devices[i].white_led.mocode);
    }
    printf("%-16s %-16s %-16s %-16s %-s\n", "green:auth", "mode", "normal_status", "blink_freq", "mocode");
    for (i = 0; i < 2; i++) {
        printf("%-16u %-16u %-16u %-16u %-s\n", config->devices[i].green_led.auth,
               config->devices[i].green_led.mode,
               config->devices[i].green_led.normal_status,
               config->devices[i].green_led.blink_freq,
               config->devices[i].green_led.mocode);
    }
    printf("%-16s %-16s %-16s\n", "turntable:mode", "target_pos_x", "target_pos_y");
    for (i = 0; i < 2; i++) {
        printf("%-16u %-16u %-16u\n", config->devices[i].turntable.mode,
               config->devices[i].turntable.target_pos_x,
               config->devices[i].turntable.target_pos_y);
    }
}

static led3000_config_t gs_json_value_backend;

static void do_with_cancel(std::string message, Led3000Window * window)
{
    red_debug_lite("cancel msg:%s", message.c_str());
}

static void _sync_json(Led3000Window *window)
{
    Document *d = window->getDocument();
    ofstream ofs(window->getFileName().c_str());
    OStreamWrapper osw(ofs);


    Pointer("/eths/0/name").Set(*d, window->getJsonValue()->eths[0].name);
    Pointer("/eths/0/ip").Set(*d, window->getJsonValue()->eths[0].ip);
    Pointer("/eths/0/netmask").Set(*d, window->getJsonValue()->eths[0].netmask);
    Pointer("/eths/0/gateway").Set(*d, window->getJsonValue()->eths[0].gateway);

    Pointer("/eths/1/name").Set(*d, window->getJsonValue()->eths[1].name);
    Pointer("/eths/1/ip").Set(*d, window->getJsonValue()->eths[1].ip);
    Pointer("/eths/1/netmask").Set(*d, window->getJsonValue()->eths[1].netmask);
    Pointer("/eths/1/gateway").Set(*d, window->getJsonValue()->eths[1].gateway);

    Pointer("/server/ip").Set(*d, window->getJsonValue()->server.ip);
    Pointer("/server/port").Set(*d, window->getJsonValue()->server.port);

    Pointer("/devices/0/camera_url").Set(*d, window->getJsonValue()->devices[0].camera_url);
    Pointer("/devices/0/white_led/mode").Set(*d, window->getJsonValue()->devices[0].white_led.mode);

    if (window->getJsonValue()->devices[0].white_led.normal_status > NORMAL_MAX_LEVEL)
        window->getJsonValue()->devices[0].white_led.normal_status = NORMAL_MAX_LEVEL;

    if (window->getJsonValue()->devices[0].white_led.blink_freq > BLINK_MAX_FREQ)
        window->getJsonValue()->devices[0].white_led.blink_freq = BLINK_MAX_FREQ;
    else if (window->getJsonValue()->devices[0].white_led.blink_freq < 1)
        window->getJsonValue()->devices[0].white_led.blink_freq = 1;
    Pointer("/devices/0/white_led/blink_freq").Set(*d, window->getJsonValue()->devices[0].white_led.blink_freq);
    Pointer("/devices/0/white_led/mocode").Set(*d, window->getJsonValue()->devices[0].white_led.mocode);
    Pointer("/devices/0/green_led/mode").Set(*d, window->getJsonValue()->devices[0].green_led.mode);
    if (window->getJsonValue()->devices[0].green_led.normal_status > NORMAL_MAX_LEVEL)
        window->getJsonValue()->devices[0].green_led.normal_status = NORMAL_MAX_LEVEL;

    if (window->getJsonValue()->devices[0].green_led.blink_freq > BLINK_MAX_FREQ)
        window->getJsonValue()->devices[0].green_led.blink_freq = BLINK_MAX_FREQ;
    else if (window->getJsonValue()->devices[0].green_led.blink_freq < 1)
        window->getJsonValue()->devices[0].green_led.blink_freq = 1;
    Pointer("/devices/0/green_led/blink_freq").Set(*d, window->getJsonValue()->devices[0].green_led.blink_freq);
    Pointer("/devices/0/green_led/mocode").Set(*d, window->getJsonValue()->devices[0].green_led.mocode);
    Pointer("/devices/0/turntable/mode").Set(*d, window->getJsonValue()->devices[0].turntable.mode);
    Pointer("/devices/0/turntable/target_pos_x").Set(*d, window->getJsonValue()->devices[0].turntable.target_pos_x);
    Pointer("/devices/0/turntable/target_pos_y").Set(*d, window->getJsonValue()->devices[0].turntable.target_pos_y);
    Pointer("/devices/0/turntable/scan_stay_time").Set(*d, window->getJsonValue()->devices[0].turntable.scan_stay_time);
    Pointer("/devices/0/turntable/scan_speed_level").Set(*d, window->getJsonValue()->devices[0].turntable.scan_speed_level);

    Pointer("/devices/1/camera_url").Set(*d, window->getJsonValue()->devices[1].camera_url);
    Pointer("/devices/1/white_led/mode").Set(*d, window->getJsonValue()->devices[1].white_led.mode);

    if (window->getJsonValue()->devices[1].white_led.normal_status > NORMAL_MAX_LEVEL)
        window->getJsonValue()->devices[1].white_led.normal_status = NORMAL_MAX_LEVEL;

    if (window->getJsonValue()->devices[1].white_led.blink_freq > BLINK_MAX_FREQ)
        window->getJsonValue()->devices[1].white_led.blink_freq = BLINK_MAX_FREQ;
    else if (window->getJsonValue()->devices[1].white_led.blink_freq < 1)
        window->getJsonValue()->devices[1].white_led.blink_freq = 1;
    Pointer("/devices/1/white_led/blink_freq").Set(*d, window->getJsonValue()->devices[1].white_led.blink_freq);
    Pointer("/devices/1/white_led/mocode").Set(*d, window->getJsonValue()->devices[1].white_led.mocode);
    Pointer("/devices/1/green_led/mode").Set(*d, window->getJsonValue()->devices[1].green_led.mode);

    if (window->getJsonValue()->devices[1].green_led.normal_status > NORMAL_MAX_LEVEL)
        window->getJsonValue()->devices[1].green_led.normal_status = NORMAL_MAX_LEVEL;

    if (window->getJsonValue()->devices[1].green_led.blink_freq > BLINK_MAX_FREQ)
        window->getJsonValue()->devices[1].green_led.blink_freq = BLINK_MAX_FREQ;
    else if (window->getJsonValue()->devices[1].green_led.blink_freq < 1)
        window->getJsonValue()->devices[1].green_led.blink_freq = 1;
    Pointer("/devices/1/green_led/blink_freq").Set(*d, window->getJsonValue()->devices[1].green_led.blink_freq);
    Pointer("/devices/1/green_led/mocode").Set(*d, window->getJsonValue()->devices[1].green_led.mocode);
    Pointer("/devices/1/turntable/mode").Set(*d, window->getJsonValue()->devices[1].turntable.mode);
    Pointer("/devices/1/turntable/target_pos_x").Set(*d, window->getJsonValue()->devices[1].turntable.target_pos_x);
    Pointer("/devices/1/turntable/target_pos_y").Set(*d, window->getJsonValue()->devices[1].turntable.target_pos_y);
    Pointer("/devices/1/turntable/scan_stay_time").Set(*d, window->getJsonValue()->devices[1].turntable.scan_stay_time);
    Pointer("/devices/1/turntable/scan_speed_level").Set(*d, window->getJsonValue()->devices[1].turntable.scan_speed_level);

    Writer<OStreamWrapper> writer(osw);
    d->Accept(writer);
}

static void do_with_confirm(std::string message, Led3000Window * window)
{
    if (message == std::string("config")) {
        red_debug_lite("sync config");
    } else if (message == std::string("json")) {
        red_debug_lite("sync json");
        _sync_json(window);
    } else {
        red_debug_lite("wow invalid confirm info");
    }
}

void *json_thread(void *arg)
{
    Led3000Window *screen = (Led3000Window *)arg;

    prctl(PR_SET_NAME, "json");
    red_debug_lite("Hello Json Thread json file path:%s", screen->getFileName().c_str());

    memcpy(&gs_json_value_backend, screen->getJsonValue(), sizeof(gs_json_value_backend));
    just_show_sysinfo(&gs_json_value_backend);
    while (1) {
        auto m = screen->getJsonQueue().get();
        auto& dm = dynamic_cast<PolyM::DataMsg<std::string>&>(*m);
        red_debug_lite("Json thread heart:%u@%s", dm.getMsgId(), dm.getPayload().c_str());
        switch (dm.getMsgId()) {
        case POLYM_BUTTON_CONFIRM:
            do_with_confirm(dm.getPayload(), screen);
            break;
        case POLYM_BUTTON_CANCEL:
            do_with_cancel(dm.getPayload(), screen);
            break;
        default:
            red_debug_lite("invalid MsgId:%u", dm.getMsgId());
            break;
        }
    }
}
