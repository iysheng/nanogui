/******************************************************************************
* File:             led3000gui.h
*
* Author:           yangyongsheng@jair.cn
* Created:          08/10/22
* Description:      led3000 gui 类
*****************************************************************************/

#pragma once

#include <nanogui/opengl.h>
#include <nanogui/screen.h>
#include <nanogui/window.h>
#include <nanogui/layout.h>
#include <nanogui/label.h>
#include <nanogui/checkbox.h>
#include <nanogui/button.h>
#include <nanogui/toolbutton.h>
#include <nanogui/popupbutton.h>
#include <nanogui/combobox.h>
#include <nanogui/progressbar.h>
#include <nanogui/messagedialog.h>
#include <nanogui/textbox.h>
#include <nanogui/slider.h>
#include <nanogui/imagepanel.h>
#include <nanogui/imageview.h>
#include <videoview.h>
#include <nanogui/vscrollpanel.h>
#include <nanogui/colorwheel.h>
#include <nanogui/graph.h>
#include <nanogui/tabwidget.h>
#include <nanogui/formhelper.h>
#include <nanogui/shader.h>
#include <nanogui/renderpass.h>
#include <memory>

#include <rapidjson/pointer.h>
#include <rapidjson/filereadstream.h>
#include <rapidjson/filewritestream.h>
#include <rapidjson/document.h>     // rapidjson's DOM-style API
#include <rapidjson/prettywriter.h> // for stringify JSON
#include <rapidjson/writer.h>
#include <cstdio>
#include <iostream>
#include <map>

#include <PolyM/include/polym/Queue.hpp>

#define LED3000_DEVICES_COUNTS    2
#define FLOAT_KEEP_PRECISON2(f)    ((f < 0) ? (f - 0.005) : (f + 0.005))


typedef enum {
    TURNTABLE_TRACK_MODE,
    /* 在漏识别目标情况下,开启的模糊跟踪模式 */
    TURNTABLE_FUZZY_TRACK_MODE,
    TURNTABLE_MANUAL_MODE,
    TURNTABLE_SCAN_MODE,
    TURNTABLE_RESET_MODE,
} turntable_mode_E;

typedef enum {
    LED_NORMAL_MODE, /* 常亮,开灯状态 */
    LED_BLINK_MODE,  /* 爆闪模式 */
    LED_MOCODE_MODE, /* 莫码模式 */
    LED_NORMAL_MODE_OFF, /* 常亮,关灯状态 */
} led_mode_E;

using namespace rapidjson;
NAMESPACE_BEGIN(nanogui)

typedef struct {
    struct {
        unsigned int version;
        unsigned int id;
    } sys_config;
    struct {
        char name[64]; /* 0: 摄像头 1: 引导信息 */
        char ip[16];
        char netmask[16];
        char gateway[16];
    } eths[2];

    struct {
        char ip[16];
        unsigned short port;
    } server;

    struct {
        char camera_url[128];
        struct {
            unsigned char mode;
            unsigned char normal_status;
            unsigned char blink_freq;
            char mocode[255];
        } white_led;
        struct {
            unsigned char auth; /* 上电默认是授权状态 */
            unsigned char mode;
            unsigned char normal_status;
            unsigned char blink_freq;
            char mocode[255];
        } green_led;
        struct {
            unsigned char mode;
            unsigned short target_pos_x;
            unsigned short target_pos_y;
            short scan_stay_time;
            short scan_speed_level;
        } turntable;
    } devices[2];
} led3000_config_t;

#define GET_LABEL_WITH_INDEX(x, i) \
    (i < LED3000_DEVICES_COUNTS) ? x[i] : NULL

/* 测试窗口类 */
class NANOGUI_EXPORT Led3000Window : public Screen
{
public:
    Led3000Window();

    ~Led3000Window()
    {
    }
    void update_time4display(void);
    void update_attitudeinfo4display(float direction, float horizon, float vertical, bool valid = true);
    void init_json_file(void);
    virtual bool keyboard_event(int key, int scancode, int action, int modifiers)
    {
        //RedDebug::err("key=%d action=%d", key, action);
#if 0
        const char value_off = 0x30;
        const char value_on = 0x31;
#endif

        switch (key) {
        case GLFW_KEY_F1: {
            /* 取消设备二激光授权,支持自锁按键 */
            if (action == GLFW_PRESS) {
                mJsonValue.devices[0].green_led.auth = 1;
                //m_dev_auth[1]->set_caption("允许发射");
#if 0
                if (m_dev_auth_light_fd[1])
                    write(m_dev_auth_light_fd[1], &value_off, 1);
#endif
                RedDebug::log("F1 catched");
                // 禁止档位
            } else if (GLFW_RELEASE == action && mJsonValue.devices[0].green_led.auth == 1) {
                mJsonValue.devices[0].green_led.auth = 0;
                //m_dev_auth[1]->set_caption("禁止发射");
#if 0
                if (m_dev_auth_light_fd[1])
                    write(m_dev_auth_light_fd[1], &value_on, 1);
#endif
                RedDebug::log("F1 release");
            }
        }
        break;
        case GLFW_KEY_F2: {
            /* 取消设备一激光授权,支持自锁按键 */
            if (action == GLFW_PRESS) {
                mJsonValue.devices[1].green_led.auth = 1;
                //m_dev_auth[0]->set_caption("允许发射");
#if 0
                if (m_dev_auth_light_fd[0])
                    write(m_dev_auth_light_fd[0], &value_off, 1);
#endif
                RedDebug::log("F2 catched");
            } else if ((GLFW_RELEASE == action) && (mJsonValue.devices[1].green_led.auth == 1)) {
                mJsonValue.devices[1].green_led.auth = 0;
                //m_dev_auth[0]->set_caption("禁止发射");
#if 0
                if (m_dev_auth_light_fd[0])
                    write(m_dev_auth_light_fd[0], &value_on, 1);
#endif
                RedDebug::log("F2 release");
            }
        }
        break;
        default:
            break;
        }

        if (Screen::keyboard_event(key, scancode, action, modifiers))
            return true;
        if (key == GLFW_KEY_ESCAPE && action == GLFW_PRESS) {
            set_visible(false);
            return true;
        }
        return false;
    }

    virtual void draw(NVGcontext *ctx)
    {
        /* Animate the scrollbar */
        //m_progress->set_value(std::fmod((float) glfwGetTime() / 10, 1.0f));

        /* Draw the user interface */
        /* 执行 Screen 的 draw 函数 */
        Screen::draw(ctx);
    }

    /* 函数重载,绘制内容,初始化 m_shader 和 m_render_pass 相关的内容 */
    virtual void draw_contents()
    {
        glClearColor(m_background[0], m_background[1], m_background[2], m_background[3]);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);
    }

    FILE*& getMfp(void) {return mFp;};
    Document* getDocument(void) {return &mDocument;};
    std::string& getFileName(void) {return mFileName;};
    led3000_config_t* getJsonValue(void) {return &mJsonValue;};
    PolyM::Queue& getJsonQueue(void) {return mJsonQueue;};
    PolyM::Queue& getDeviceQueue(int device)
    {
        if (device < LED3000_DEVICES_COUNTS) {
            return mDeviceQueue[device];
        } else
            return mDeviceQueue[0]; /* 默认控制第一台设备 */
    };
    PolyM::Queue& getCurrentDeviceQueue(void)
    {
        return getDeviceQueue(mCurrentDevice);
    }
    void setCurrentDevice(int index) {mCurrentDevice = index;};
    int getCurrentDevice(void) {return mCurrentDevice;};

    Label *get_dev_state_label(int index)
    {
        return GET_LABEL_WITH_INDEX(m_dev_state, index);
    }
    Label *get_dev_angle_label(int index)
    {
        return GET_LABEL_WITH_INDEX(m_dev_angle, index);
    }
    Label *get_dev_angular_speed_label(int index)
    {
        return GET_LABEL_WITH_INDEX(m_dev_angular_speed, index);
    }
    Label *get_dev_morse_code_label(int index)
    {
        return GET_LABEL_WITH_INDEX(m_dev_morse_code, index);
    }
    Label *get_dev_auth_label(int index)
    {
        return GET_LABEL_WITH_INDEX(m_dev_auth, index);
    }
    Label *get_green_dev_label()
    {
        return m_green_dev;
    }
    Label *get_white_dev_label()
    {
        return m_white_dev;
    }
    Label *get_turntable_label()
    {
        return m_turntable_dev;
    }
    bool get_dev_guide_leave_mode(uint8_t dev_num = 0)
    {
        return m_guide_leave_status[dev_num];
    }
    bool get_dev_guide_shoot_mode(uint8_t dev_num = 0)
    {
        return m_guide_shoot_status[dev_num];
    }
    void set_dev_guide_shoot_mode(uint8_t dev_num = 0, bool mode = false)
    {
        m_guide_shoot_status[dev_num] = mode;
    }

    void set_guide_info(bool info, float direction_float = 0.0,
        float elevation_float = 0.0, uint8_t dev_num = 0)
    {
        if (dev_num > LED3000_DEVICES_COUNTS - 1)
            return;
        else if (nullptr == m_guide_info_label[dev_num])
            return;

        m_guide_info_label[dev_num]->set_caption('['
            + std::to_string(FLOAT_KEEP_PRECISON2(direction_float)).erase(to_string(direction_float).find('.')+3, string::npos)
            + '/'
            + std::to_string(FLOAT_KEEP_PRECISON2(elevation_float)).erase(to_string(elevation_float).find('.')+3, string::npos)
            + ']');
        m_guide_status[dev_num] = info;
        if (mCurrentDevice == dev_num)
        {
            m_guide_info_label[dev_num]->set_visible(m_guide_status[dev_num]);
        }
    }

    void set_guide_mode(bool mode, uint8_t dev_num = 0)
    {
        if (dev_num > LED3000_DEVICES_COUNTS - 1)
            return;
        else if (nullptr == m_guide_mode_icon[dev_num])
            return;

        m_guide_status[dev_num] = mode;
        if (mCurrentDevice == dev_num)
        {
            m_guide_mode_icon[dev_num]->set_visible(m_guide_status[dev_num]);
        }
    }

    void set_guide_leave(bool mode, uint8_t dev_num = 0)
    {
        if (dev_num > LED3000_DEVICES_COUNTS - 1)
            return;
        else if (nullptr == m_guide_leave_icon[dev_num])
            return;

        m_guide_leave_status[dev_num] = mode;
        if (mCurrentDevice == dev_num)
        {
            m_guide_leave_icon[dev_num]->set_visible(m_guide_leave_status[dev_num]);
        }
    }

    void sync_guide_relate_display(uint8_t dev_num)
    {
        if (0 == dev_num)
        {
            m_guide_info_label[1]->set_visible(false);
            m_guide_mode_icon[1]->set_visible(false);
            m_guide_leave_icon[1]->set_visible(false);
            m_guide_info_label[0]->set_visible(m_guide_status[0]);
            m_guide_mode_icon[0]->set_visible(m_guide_status[0]);
            m_guide_leave_icon[0]->set_visible(m_guide_leave_status[0]);
        }
        else if (1 == dev_num)
        {
            m_guide_info_label[0]->set_visible(false);
            m_guide_mode_icon[0]->set_visible(false);
            m_guide_leave_icon[0]->set_visible(false);
            m_guide_info_label[1]->set_visible(m_guide_status[1]);
            m_guide_mode_icon[1]->set_visible(m_guide_status[1]);
            m_guide_leave_icon[1]->set_visible(m_guide_leave_status[1]);
        }
    }

    void set_green_dev_control_btns(const std::vector<Button *> *btns)
    {
        m_green_dev_control_btns = btns;
    }

    const std::vector<Button *> * get_green_dev_control_btns(void)
    {
        return m_green_dev_control_btns;
    }

    void set_white_dev_control_btns(const std::vector<Button *> *btns)
    {
        m_white_dev_control_btns = btns;
    }

    const std::vector<Button *> * get_white_dev_control_btns(void)
    {
        return m_white_dev_control_btns;
    }

    void set_white_dev_control_btns_status(int mode)
    {
        switch (mode) {
        case LED_NORMAL_MODE_OFF:
            m_white_dev_control_btns->at(0)->set_pushed(false);
            m_white_dev_control_btns->at(1)->set_pushed(false);
            m_white_dev_control_btns->at(2)->set_pushed(false);
            break;
        case LED_NORMAL_MODE:
            m_white_dev_control_btns->at(0)->set_pushed(true);
            m_white_dev_control_btns->at(1)->set_pushed(false);
            m_white_dev_control_btns->at(2)->set_pushed(false);
            break;
        case LED_BLINK_MODE:
            m_white_dev_control_btns->at(0)->set_pushed(false);
            m_white_dev_control_btns->at(1)->set_pushed(true);
            m_white_dev_control_btns->at(2)->set_pushed(false);
            break;
        case LED_MOCODE_MODE:
            m_white_dev_control_btns->at(0)->set_pushed(false);
            m_white_dev_control_btns->at(1)->set_pushed(false);
            m_white_dev_control_btns->at(2)->set_pushed(true);
            break;
        default:
            break;
        }
    }

    void set_green_dev_control_btns_status(int mode)
    {
        switch (mode) {
        case LED_NORMAL_MODE_OFF:
            m_green_dev_control_btns->at(0)->set_pushed(false);
            m_green_dev_control_btns->at(1)->set_pushed(false);
            m_green_dev_control_btns->at(2)->set_pushed(false);
            break;
        case LED_NORMAL_MODE:
            m_green_dev_control_btns->at(0)->set_pushed(true);
            m_green_dev_control_btns->at(1)->set_pushed(false);
            m_green_dev_control_btns->at(2)->set_pushed(false);
            break;
        case LED_BLINK_MODE:
            m_green_dev_control_btns->at(0)->set_pushed(false);
            m_green_dev_control_btns->at(1)->set_pushed(true);
            m_green_dev_control_btns->at(2)->set_pushed(false);
            break;
        case LED_MOCODE_MODE:
            m_green_dev_control_btns->at(0)->set_pushed(false);
            m_green_dev_control_btns->at(1)->set_pushed(false);
            m_green_dev_control_btns->at(2)->set_pushed(true);
            break;
        default:
            break;
        }
    }

    void set_turntable_dev_control_btns_status(int mode)
    {
        if (!m_track_btn || !m_manual_btn || !m_scan_btn)
        {
            return;
        }
        switch (mode) {
        case TURNTABLE_TRACK_MODE:
        case TURNTABLE_FUZZY_TRACK_MODE:
            m_track_btn->set_pushed(true);
            m_manual_btn->set_pushed(false);
            m_scan_btn->set_pushed(false);
            break;
        case TURNTABLE_MANUAL_MODE:
            m_track_btn->set_pushed(false);
            m_manual_btn->set_pushed(true);
            m_scan_btn->set_pushed(false);
            break;
        case TURNTABLE_SCAN_MODE:
            m_track_btn->set_pushed(false);
            m_manual_btn->set_pushed(false);
            m_scan_btn->set_pushed(true);
            break;
        default:
            break;
        }
    }

    void set_guide_mode_icon(Label *label, uint8_t dev_num = 0)
    {
        if (dev_num < LED3000_DEVICES_COUNTS)
            m_guide_mode_icon[dev_num] = label;
    }

    void set_turntable_mode_btns(Button *track, Button *manual, Button *scan)
    {
        m_track_btn = track;
        m_manual_btn = manual;
        m_scan_btn = scan;
    }
private:
    /* 设备状态窗口 label 控件 */
    Label *m_dev_state[LED3000_DEVICES_COUNTS];
    Label *m_dev_angle[LED3000_DEVICES_COUNTS];
    Label *m_dev_angular_speed[LED3000_DEVICES_COUNTS];
    Label *m_dev_morse_code[LED3000_DEVICES_COUNTS];
    Label *m_dev_auth[LED3000_DEVICES_COUNTS];
    Label *m_green_dev;
    Label *m_white_dev;
    Label *m_turntable_dev;
    Label *m_guide_mode_icon[LED3000_DEVICES_COUNTS];
    Label *m_guide_leave_icon[LED3000_DEVICES_COUNTS];
    Label *m_guide_info_label[LED3000_DEVICES_COUNTS];
    bool   m_guide_status[LED3000_DEVICES_COUNTS];
    bool   m_guide_leave_status[LED3000_DEVICES_COUNTS];
    bool   m_guide_shoot_status[LED3000_DEVICES_COUNTS]; /* 标记指控是否允许射击 */
    Label *m_time4dispaly;  /* 显示时统时间 */
    Label *m_attitude_info; /* 显示姿态信息 */
    int m_dev_auth_light_fd[LED3000_DEVICES_COUNTS];

    const std::vector<Button *> *m_green_dev_control_btns;
    const std::vector<Button *> *m_white_dev_control_btns;

    Button * m_track_btn;
    Button * m_manual_btn;
    Button * m_scan_btn;

    ref<Shader> m_shader;
    ref<RenderPass> m_render_pass;

    using ImageHolder = std::unique_ptr<uint8_t[], void(*)(void*)>;
    std::vector<std::pair<ref<Texture>, ImageHolder>> m_images;
    int m_current_image;
    int mCurrentDevice; /* 当前选定的设备编号 */
    char mjsonBuffer[4096];
    Document mDocument;
    led3000_config_t mJsonValue;
    PolyM::Queue mJsonQueue;
    PolyM::Queue mDeviceQueue[LED3000_DEVICES_COUNTS];
    FILE *mFp;
    std::string mFileName;
public:
    std::string m4PolyM[POLYM_TYPE_MAX];
};

NAMESPACE_END(nanogui)
