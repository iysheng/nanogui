/******************************************************************************
* File:             led3000gui.h
*
* Author:           yangyongsheng@jair.cn  
* Created:          08/10/22 
* Description:      led3000 gui 类
*****************************************************************************/

#pragma once

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
//#include <nanogui/videoview.h>
#include <nanogui/vscrollpanel.h>
#include <nanogui/colorwheel.h>
#include <nanogui/graph.h>
#include <nanogui/tabwidget.h>
#include <nanogui/formhelper.h>
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
      } turntable;
    } devices[2];
} led3000_config_t;

/* 测试窗口类 */
class NANOGUI_EXPORT Led3000Window : public Screen
{
public:
    Led3000Window(){};

    ~Led3000Window() {
    }
    void init_json_file(void);
#if 0
    virtual bool keyboardEvent(int key, int scancode, int action, int modifiers)
    {
        if (Screen::keyboardEvent(key, scancode, action, modifiers))
            return true;

        //if (key == GLFW_KEY_ESCAPE && action == GLFW_PRESS)
        // {
        //    setVisible(false);
        //    return true;
        //}
        return false;
    }

    virtual void draw(NVGcontext *ctx) {
    {
      if (auto pbar = gfind<ProgressBar>("progressbar"))
      {
        /* 更新 progressbar 进度条更新 */
        pbar->setValue(pbar->value() + 0.001f);
        if (pbar->value() >= 1.f)
          pbar->setValue(0.f);
      }

      Screen::draw(renderer);
    }

    virtual void drawContents()
    {
    }

    FILE*& getMfp(void){return mFp;};
    Document* getDocument(void){return &mDocument;};
    std::string& getFileName(void){return mFileName;};
    led3000_config_t* getJsonValue(void){return &mJsonValue;};
    PolyM::Queue& getJsonQueue(void){return mJsonQueue;};
    PolyM::Queue& getDeviceQueue(int device){
      if (device < LED3000_DEVICES_COUNTS)
      {
        return mDeviceQueue[device];
      }
      else
        return mDeviceQueue[0]; /* 默认控制第一台设备 */
    };
    PolyM::Queue& getCurrentDeviceQueue(void) {
        return getDeviceQueue(mCurrentDevice);
    }
    void setCurrentDevice(int index){mCurrentDevice = index;};
    int getCurrentDevice(void){return mCurrentDevice;};
private:
    std::vector<SDL_Texture*> mImagesData;
    int mCurrentImage;
    int mCurrentDevice; /* 当前选定的设备编号 */
    char mjsonBuffer[4096];
    Document mDocument;
    led3000_config_t mJsonValue;
    PolyM::Queue mJsonQueue;
    PolyM::Queue mDeviceQueue[LED3000_DEVICES_COUNTS];
    FILE *mFp;
    std::string mFileName;
#endif
};

NAMESPACE_END(nanogui)
