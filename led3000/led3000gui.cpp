/******************************************************************************
* File:             led3000gui.cpp
*
* Author:           yangyongsheng@jari.cn
* Created:          08/10/22
* Description:      led3000 gui source file
*****************************************************************************/

#include <cstdint>
#include <led3000gui.h>
#include <stdlib.h>
#include <sys/time.h>
#include <string>
#include <version.h>

using namespace nanogui;
using std::endl;

#define LED3000_ID         11000001u

void do_paint_sysconfig(Widget *widget)
{
    {
        MessageDialog * msg_dlg = dynamic_cast<MessageDialog *>(widget);
        msg_dlg->message_label()->set_position(Vector2i(30, 15));
        msg_dlg->confirm_button()->set_position(Vector2i(10, 677));
        msg_dlg->confirm_button()->set_fixed_size(Vector2i(156, 60));
        msg_dlg->cancel_button()->set_position(Vector2i(176, 677));
        msg_dlg->cancel_button()->set_fixed_size(Vector2i(156, 60));
        msg_dlg->label_icon()->set_icon("");
        widget->window()->set_fixed_size(Vector2i(342, 747));
        widget->window()->set_background_image(RED_LED3000_ASSETS_DIR"/set_msgdlg3.png");
        Led3000Window * led3000Window = dynamic_cast<Led3000Window *>(widget->window()->parent());

        auto *label = widget->add<Label>("网卡0IP:", "sans-bold");
        label->set_position(Vector2i(51, 84));
        label->set_font_size(20);
        /* 创建 textBox */
        auto* textBox = widget->add<TextBox>("", KeyboardType::NumberIP);
        textBox->set_editable(true);
        textBox->set_position(Vector2i(146, 71));
        /* 设置控件大小 */
        textBox->set_fixed_size(Vector2i(176, 46));
        textBox->set_value(led3000Window->getJsonValue()->eths[0].ip);
        textBox->setSyncCharsValue(led3000Window->getJsonValue()->eths[0].ip);
        /* 设置字体大小 */
        textBox->set_font_size(16);

        //textBox->setFormat("[-]?[0-9]*\\.?[0-9]+");
        textBox->set_alignment(TextBox::Alignment::Left);
        label = widget->add<Label>("网卡0子网掩码:", "sans-bold");
        label->set_position(Vector2i(29, 140));
        label->set_font_size(20);
        textBox = widget->add<TextBox>("", KeyboardType::NumberIP);
        textBox->set_position(Vector2i(146, 127));
        textBox->set_editable(true);
        textBox->set_fixed_size(Vector2i(176, 46));
        textBox->set_value(led3000Window->getJsonValue()->eths[0].netmask);
        textBox->setSyncCharsValue(led3000Window->getJsonValue()->eths[0].netmask);
        textBox->set_font_size(16);
        textBox->set_alignment(TextBox::Alignment::Left);
        label = widget->add<Label>("网卡0网关:", "sans-bold");
        label->set_position(Vector2i(44, 196));
        label->set_font_size(20);
        textBox = widget->add<TextBox>("", KeyboardType::NumberIP);
        textBox->set_position(Vector2i(146, 183));
        textBox->set_editable(true);
        textBox->set_fixed_size(Vector2i(176, 46));
        textBox->set_value(led3000Window->getJsonValue()->eths[0].gateway);
        textBox->setSyncCharsValue(led3000Window->getJsonValue()->eths[0].gateway);
        textBox->set_font_size(16);
        textBox->set_alignment(TextBox::Alignment::Left);

        /* 定义了这个窗口的布局 */
        label = widget->add<Label>("网卡1IP:", "sans-bold");
        label->set_position(Vector2i(49, 252));
        label->set_font_size(20);
        textBox = widget->add<TextBox>("", KeyboardType::NumberIP);
        textBox->set_editable(true);
        textBox->set_position(Vector2i(146, 239));
        textBox->set_fixed_size(Vector2i(176, 46));
        textBox->set_value(led3000Window->getJsonValue()->eths[1].ip);
        textBox->setSyncCharsValue(led3000Window->getJsonValue()->eths[1].ip);
        textBox->set_font_size(16);
        textBox->set_alignment(TextBox::Alignment::Left);

        label = widget->add<Label>("网卡1子网掩码:", "sans-bold");
        label->set_position(Vector2i(29, 308));
        label->set_font_size(20);
        textBox = widget->add<TextBox>("", KeyboardType::NumberIP);
        textBox->set_position(Vector2i(146, 295));
        textBox->set_editable(true);
        textBox->set_fixed_size(Vector2i(176, 46));
        textBox->set_value(led3000Window->getJsonValue()->eths[1].netmask);
        textBox->setSyncCharsValue(led3000Window->getJsonValue()->eths[1].netmask);
        textBox->set_font_size(16);
        textBox->set_alignment(TextBox::Alignment::Left);

        label = widget->add<Label>("网卡1网关:", "sans-bold");
        label->set_position(Vector2i(44, 364));
        label->set_font_size(20);
        textBox = widget->add<TextBox>("", KeyboardType::NumberIP);
        textBox->set_position(Vector2i(146, 351));
        textBox->set_editable(true);
        textBox->set_fixed_size(Vector2i(176, 46));
        textBox->set_value(led3000Window->getJsonValue()->eths[1].gateway);
        textBox->setSyncCharsValue(led3000Window->getJsonValue()->eths[1].gateway);
        textBox->set_font_size(16);
        textBox->set_alignment(TextBox::Alignment::Left);

        label = widget->add<Label>("服务器IP:", "sans-bold");
        label->set_position(Vector2i(44, 420));
        label->set_font_size(20);
        textBox = widget->add<TextBox>("", KeyboardType::NumberIP);
        textBox->set_position(Vector2i(146, 407));
        textBox->set_editable(true);
        textBox->set_fixed_size(Vector2i(176, 46));
        textBox->set_value(led3000Window->getJsonValue()->server.ip);
        textBox->setSyncCharsValue(led3000Window->getJsonValue()->server.ip);
        textBox->set_font_size(16);
        textBox->set_alignment(TextBox::Alignment::Left);

        label = widget->add<Label>("服务器端口:", "sans-bold");
        label->set_position(Vector2i(42, 476));
        label->set_font_size(20);
        textBox = widget->add<TextBox>(std::to_string(led3000Window->getJsonValue()->server.port), KeyboardType::NumberIP);
        textBox->set_editable(true);
        textBox->set_position(Vector2i(146, 463));
        textBox->set_fixed_size(Vector2i(176, 46));
        textBox->setSyncUshortValue(&(led3000Window->getJsonValue()->server.port));
        textBox->set_font_size(16);
        textBox->set_alignment(TextBox::Alignment::Left);

        label = widget->add<Label>("摄像头1路径:", "sans-bold");
        label->set_position(Vector2i(37, 541));
        label->set_font_size(20);
        textBox = widget->add<TextBox>("", KeyboardType::Full);
        textBox->set_editable(true);
        textBox->set_position(Vector2i(146, 519));
        textBox->set_fixed_size(Vector2i(176, 64));
        textBox->set_value(led3000Window->getJsonValue()->devices[0].camera_url);
        textBox->setSyncCharsValue(&(led3000Window->getJsonValue()->devices[0].camera_url[0]));
        textBox->set_font_size(16);
        textBox->set_alignment(TextBox::Alignment::Left);

        label = widget->add<Label>("摄像头2路径:", "sans-bold");
        label->set_position(Vector2i(37, 615));
        label->set_font_size(20);
        textBox = widget->add<TextBox>("", KeyboardType::Full);
        textBox->set_position(Vector2i(146, 593));
        textBox->set_fixed_size(Vector2i(176, 64));
        textBox->set_editable(true);
        textBox->set_value(led3000Window->getJsonValue()->devices[1].camera_url);
        textBox->setSyncCharsValue(&(led3000Window->getJsonValue()->devices[1].camera_url[0]));
        textBox->set_font_size(16);
        textBox->set_alignment(TextBox::Alignment::Left);
    }
}

void do_with_sysconfig(Widget *widget, int choose)
{
    if (choose == 1) {
        Led3000Window * led3000Window = dynamic_cast<Led3000Window *>(widget->window()->parent());
        /* "json"  消息表示更新配置文件 */
        led3000Window->getJsonQueue().put(PolyM::DataMsg<std::string>(choose == 1 ? POLYM_BUTTON_CONFIRM : POLYM_BUTTON_CANCEL, "json"));
    }
}

void do_with_power_off(Widget *widget, int choose)
{
    if (choose == 1) {
        /* TODO 发送关机消息到一体化网络 */
        extern int update_offinfo2network(void);
        update_offinfo2network();
        /* do power off */
        system("poweroff");
    }
}

/*
 * 绿灯未授权时的弹窗显示
 * */
static void _do_paint_green_light_no_auth(Widget *widget)
{
    MessageDialog * msg_dlg = dynamic_cast<MessageDialog *>(widget);

    widget->window()->set_fixed_size(Vector2i(480, 324));
    widget->window()->set_background_image(RED_LED3000_ASSETS_DIR"/set_msgdlg2.png");
    msg_dlg->message_label()->set_caption("绿灯已锁定,限制使用,需手动解锁");
    msg_dlg->message_label()->set_font_size(20);
    msg_dlg->message_label()->set_position(Vector2i(113, 186));
    msg_dlg->label_icon()->set_position(Vector2i(217, 91));
    msg_dlg->label_icon()->set_icon(RED_LED3000_ASSETS_DIR"/led_alarm.png");
    msg_dlg->cancel_button()->set_position(Vector2i(126, 254));
    msg_dlg->cancel_button()->set_fixed_size(Vector2i(225, 60));
    msg_dlg->confirm_button()->set_visible(false);
}

void do_with_white_light_normal(Widget *widget, int choose)
{
    Led3000Window * led3000Window = dynamic_cast<Led3000Window *>(widget->screen());
    const std::vector<Button *> * white_dev_btns = led3000Window->get_white_dev_control_btns();
    if (choose == 1)
    {
        /* 发送消息控制开灯 */
        led3000Window->getJsonValue()->devices[led3000Window->getCurrentDevice()].white_led.mode = LED_NORMAL_MODE;
        led3000Window->getJsonValue()->devices[led3000Window->getCurrentDevice()].white_led.normal_status = 100;
        led3000Window->getCurrentDeviceQueue().put(PolyM::DataMsg<std::string>(POLYM_WHITE_NORMAL_SETTING, to_string(led3000Window->getJsonValue()->devices[led3000Window->getCurrentDevice()].white_led.normal_status)));
        led3000Window->getJsonQueue().put(PolyM::DataMsg<std::string>(POLYM_BUTTON_CONFIRM, "json"));
        white_dev_btns->at(0)->set_pushed(true);
        white_dev_btns->at(1)->set_pushed(false);
        white_dev_btns->at(2)->set_pushed(false);
    }
    else if (led3000Window->getJsonValue()->devices[led3000Window->getCurrentDevice()].white_led.mode == LED_NORMAL_MODE)
    {
        led3000Window->getJsonValue()->devices[led3000Window->getCurrentDevice()].white_led.mode = LED_NORMAL_MODE_OFF;
        led3000Window->getJsonValue()->devices[led3000Window->getCurrentDevice()].white_led.normal_status = 0;
        led3000Window->getJsonQueue().put(PolyM::DataMsg<std::string>(POLYM_BUTTON_CONFIRM, "json"));
        /* 发送消息控制关灯 */
        led3000Window->getCurrentDeviceQueue().put(PolyM::DataMsg<std::string>(POLYM_WHITE_NORMAL_SETTING, to_string(led3000Window->getJsonValue()->devices[led3000Window->getCurrentDevice()].white_led.normal_status)));
    }
}

void do_paint_white_light_blink(Widget *widget)
{
    MessageDialog * msg_dlg = dynamic_cast<MessageDialog *>(widget);
    Led3000Window * led3000Window = dynamic_cast<Led3000Window *>(widget->window()->parent());
    auto * blink_title = widget->add<Label>("爆闪频率：", "sans-bold");
    blink_title->set_font_size(20);
    blink_title->set_position(Vector2i(48, 180));
    widget->window()->set_fixed_size(Vector2i(342, 313));
    widget->window()->set_background_image(RED_LED3000_ASSETS_DIR"/set_dlg_blink.png");
    blink_title = widget->add<Label>("(2~15)HZ", "sans-bold");
    blink_title->set_font_size(20);
    blink_title->set_position(Vector2i(233, 180));

    auto *textBox = widget->add<TextBox>("", KeyboardType::NumberIP);
    textBox->set_position(Vector2i(134, 167));
    textBox->set_fixed_size(Vector2i(90, 46));
    textBox->set_editable(true);
    textBox->set_value(std::to_string(led3000Window->getJsonValue()->devices[led3000Window->getCurrentDevice()].white_led.blink_freq));
    textBox->setSyncUcharValue(&(led3000Window->getJsonValue()->devices[led3000Window->getCurrentDevice()].white_led.blink_freq));
    textBox->set_alignment(TextBox::Alignment::Left);

    msg_dlg->label_icon()->set_position(Vector2i(148, 91));
    msg_dlg->label_icon()->set_icon(RED_LED3000_ASSETS_DIR"/sys_icon.png");
    msg_dlg->confirm_button()->set_position(Vector2i(10, 243));
    msg_dlg->confirm_button()->set_fixed_size(Vector2i(156, 60));
    msg_dlg->cancel_button()->set_position(Vector2i(176, 243));
    msg_dlg->cancel_button()->set_fixed_size(Vector2i(156, 60));
}

void do_with_white_light_blink(Widget *widget, int choose)
{
    Led3000Window * led3000Window = dynamic_cast<Led3000Window *>(widget->screen());
    const std::vector<Button *> * white_dev_btns = led3000Window->get_white_dev_control_btns();
    if (choose == 1) {
        /* 发送消息控制频闪 */
        led3000Window->getJsonValue()->devices[led3000Window->getCurrentDevice()].white_led.mode = LED_BLINK_MODE;
        led3000Window->m4PolyM[POLYM_WHITE_BLINK_SETTING].assign(to_string(led3000Window->getJsonValue()->devices[led3000Window->getCurrentDevice()].white_led.blink_freq));
        led3000Window->getCurrentDeviceQueue().put(PolyM::DataMsg<std::string>(POLYM_WHITE_BLINK_SETTING, led3000Window->m4PolyM[POLYM_WHITE_BLINK_SETTING]));
        /* 同步消息内容到 json 文件 */
        led3000Window->getJsonQueue().put(PolyM::DataMsg<std::string>(POLYM_BUTTON_CONFIRM, "json"));
        white_dev_btns->at(0)->set_pushed(false);
        white_dev_btns->at(1)->set_pushed(true);
        white_dev_btns->at(2)->set_pushed(false);
    }
    else if (led3000Window->getJsonValue()->devices[led3000Window->getCurrentDevice()].white_led.mode == LED_BLINK_MODE)
    {
        /* 发送消息关灯 */
        led3000Window->getCurrentDeviceQueue().put(PolyM::DataMsg<std::string>(POLYM_WHITE_NORMAL_SETTING, to_string(0X00)));
        led3000Window->getJsonValue()->devices[led3000Window->getCurrentDevice()].white_led.mode = LED_NORMAL_MODE_OFF;
        /* 同步消息内容到 json 文件 */
        led3000Window->getJsonQueue().put(PolyM::DataMsg<std::string>(POLYM_BUTTON_CONFIRM, "json"));
        white_dev_btns->at(1)->set_pushed(false);
    }
}

void do_paint_white_light_mocode(Widget *widget)
{
    MessageDialog * msg_dlg = dynamic_cast<MessageDialog *>(widget);
    Led3000Window * led3000Window = dynamic_cast<Led3000Window *>(widget->window()->parent());

    auto * mocode_value_title = widget->add<Label>("莫码参数为：", "sans-bold");
    mocode_value_title->set_font_size(20);
    widget->window()->set_background_image(RED_LED3000_ASSETS_DIR"/set_msgdlg1.png");
    auto *textBox = widget->add<TextBox>("", KeyboardType::Full);
    textBox->set_position(Vector2i(134, 167));
    textBox->set_fixed_size(Vector2i(178, 46));
    textBox->set_editable(true);
    textBox->set_value(led3000Window->getJsonValue()->devices[led3000Window->getCurrentDevice()].white_led.mocode);
    textBox->setSyncCharsValue(&(led3000Window->getJsonValue()->devices[led3000Window->getCurrentDevice()].white_led.mocode[0]));
    textBox->set_font_size(16);
    textBox->set_alignment(TextBox::Alignment::Left);

    mocode_value_title->set_position(Vector2i(41, 180));

    msg_dlg->label_icon()->set_position(Vector2i(148, 91));
    msg_dlg->message_label()->set_position(Vector2i(104, 282));
    msg_dlg->confirm_button()->set_position(Vector2i(10, 354));
    msg_dlg->confirm_button()->set_fixed_size(Vector2i(156, 60));
    msg_dlg->cancel_button()->set_position(Vector2i(176, 354));
    msg_dlg->cancel_button()->set_fixed_size(Vector2i(156, 60));
}

void do_with_white_light_mocode(Widget *widget, int choose)
{
    Led3000Window * window = dynamic_cast<Led3000Window *>(widget->screen());
    const std::vector<Button *> * white_dev_btns = window->get_white_dev_control_btns();
    Led3000Window * led3000Window = dynamic_cast<Led3000Window *>(widget->window()->parent());
    if (choose == 1) {
        led3000Window->getJsonValue()->devices[led3000Window->getCurrentDevice()].white_led.mode = LED_MOCODE_MODE;
        /* 发送消息控制莫码 */
        led3000Window->getCurrentDeviceQueue().put(PolyM::DataMsg<std::string>(POLYM_WHITE_MOCODE_SETTING, led3000Window->getJsonValue()->devices[led3000Window->getCurrentDevice()].white_led.mocode));
        /* 更新界面莫码显示 */
        led3000Window->get_dev_morse_code_label(led3000Window->getCurrentDevice())->set_caption_merge(led3000Window->getJsonValue()->devices[led3000Window->getCurrentDevice()].green_led.mocode, led3000Window->getJsonValue()->devices[led3000Window->getCurrentDevice()].white_led.mocode, '/');
        /* 同步消息内容到 json 文件 */
        led3000Window->getJsonQueue().put(PolyM::DataMsg<std::string>(POLYM_BUTTON_CONFIRM, "json"));

        white_dev_btns->at(0)->set_pushed(false);
        white_dev_btns->at(1)->set_pushed(false);
        white_dev_btns->at(2)->set_pushed(true);
        return;
    }
    else if (led3000Window->getJsonValue()->devices[led3000Window->getCurrentDevice()].white_led.mode == LED_MOCODE_MODE)
    {
        led3000Window->getCurrentDeviceQueue().put(PolyM::DataMsg<std::string>(POLYM_WHITE_NORMAL_SETTING, to_string(0X00)));
        led3000Window->getJsonValue()->devices[led3000Window->getCurrentDevice()].white_led.mode = LED_NORMAL_MODE_OFF;
        led3000Window->getJsonQueue().put(PolyM::DataMsg<std::string>(POLYM_BUTTON_CONFIRM, "json"));
        white_dev_btns->at(2)->set_pushed(false);
        return;
    }
}

void do_paint_green_light_normal(Widget *widget)
{
    Led3000Window * led3000Window = dynamic_cast<Led3000Window *>(widget->window()->parent());
    if (!led3000Window->getJsonValue()->devices[led3000Window->getCurrentDevice()].green_led.auth) {
        _do_paint_green_light_no_auth(widget);
    } else {
        MessageDialog * msg_dlg = dynamic_cast<MessageDialog *>(widget);
        widget->window()->set_fixed_size(Vector2i(480, 324));
        widget->window()->set_background_image(RED_LED3000_ASSETS_DIR"/set_msgdlg2.png");

        msg_dlg->label_icon()->set_position(Vector2i(217, 91));
        msg_dlg->label_icon()->set_icon(RED_LED3000_ASSETS_DIR"/sys_icon.png");
        msg_dlg->confirm_button()->set_position(Vector2i(10, 254));
        msg_dlg->confirm_button()->set_fixed_size(Vector2i(225, 60));
        msg_dlg->cancel_button()->set_position(Vector2i(245, 254));
        msg_dlg->cancel_button()->set_fixed_size(Vector2i(225, 60));
    }
}

void do_with_green_light_normal(Widget *widget, int choose)
{
    Led3000Window * led3000Window = dynamic_cast<Led3000Window *>(widget->screen());
    const std::vector<Button *> * green_dev_btns = led3000Window->get_green_dev_control_btns();

    if (!led3000Window->getJsonValue()->devices[led3000Window->getCurrentDevice()].green_led.auth) {
        return;
    }

    if (1 == choose)
    {
        /* 发送消息控制开灯 */
        led3000Window->getJsonValue()->devices[led3000Window->getCurrentDevice()].green_led.mode = LED_NORMAL_MODE;
        led3000Window->getJsonValue()->devices[led3000Window->getCurrentDevice()].green_led.normal_status = 100;
        led3000Window->m4PolyM[POLYM_GREEN_NORMAL_SETTING].assign(to_string(led3000Window->getJsonValue()->devices[led3000Window->getCurrentDevice()].green_led.normal_status));
        led3000Window->getCurrentDeviceQueue().put(PolyM::DataMsg<std::string>(POLYM_GREEN_NORMAL_SETTING, led3000Window->m4PolyM[POLYM_GREEN_NORMAL_SETTING]));
        led3000Window->getJsonQueue().put(PolyM::DataMsg<std::string>(POLYM_BUTTON_CONFIRM, "json"));
        green_dev_btns->at(0)->set_pushed(true);
        green_dev_btns->at(1)->set_pushed(false);
        green_dev_btns->at(2)->set_pushed(false);
    }
    else if (led3000Window->getJsonValue()->devices[led3000Window->getCurrentDevice()].green_led.mode == LED_NORMAL_MODE)
    {
        led3000Window->getJsonValue()->devices[led3000Window->getCurrentDevice()].green_led.mode = LED_NORMAL_MODE_OFF;
        led3000Window->getJsonValue()->devices[led3000Window->getCurrentDevice()].green_led.normal_status = 0;
        led3000Window->m4PolyM[POLYM_GREEN_NORMAL_SETTING].assign(to_string(0));
        /* 发送消息控制关灯 */
        led3000Window->getCurrentDeviceQueue().put(PolyM::DataMsg<std::string>(POLYM_GREEN_NORMAL_SETTING, led3000Window->m4PolyM[POLYM_GREEN_NORMAL_SETTING]));
        led3000Window->getJsonQueue().put(PolyM::DataMsg<std::string>(POLYM_BUTTON_CONFIRM, "json"));
    }
}

void do_paint_green_light_blink(Widget *widget)
{
    Led3000Window * led3000Window = dynamic_cast<Led3000Window *>(widget->window()->parent());
    if (!led3000Window->getJsonValue()->devices[led3000Window->getCurrentDevice()].green_led.auth) {
        _do_paint_green_light_no_auth(widget);
    } else {
        MessageDialog * msg_dlg = dynamic_cast<MessageDialog *>(widget);
        auto * blink_title = widget->add<Label>("爆闪频率：", "sans-bold");
        blink_title->set_font_size(20);
        blink_title->set_position(Vector2i(48, 180));
        widget->window()->set_fixed_size(Vector2i(342, 313));
        widget->window()->set_background_image(RED_LED3000_ASSETS_DIR"/set_dlg_blink.png");
        blink_title = widget->add<Label>("(1~15)HZ", "sans-bold");
        blink_title->set_font_size(20);
        blink_title->set_position(Vector2i(233, 180));

        auto *textBox = widget->add<TextBox>("", KeyboardType::NumberIP);
        textBox->set_position(Vector2i(134, 167));
        textBox->set_fixed_size(Vector2i(90, 46));
        textBox->set_editable(true);
        textBox->set_value(std::to_string(led3000Window->getJsonValue()->devices[led3000Window->getCurrentDevice()].green_led.blink_freq));
        textBox->setSyncUcharValue(&(led3000Window->getJsonValue()->devices[led3000Window->getCurrentDevice()].green_led.blink_freq));
        textBox->set_alignment(TextBox::Alignment::Left);

        msg_dlg->label_icon()->set_position(Vector2i(148, 91));
        msg_dlg->label_icon()->set_icon(RED_LED3000_ASSETS_DIR"/sys_icon.png");
        msg_dlg->confirm_button()->set_position(Vector2i(10, 243));
        msg_dlg->confirm_button()->set_fixed_size(Vector2i(156, 60));
        msg_dlg->cancel_button()->set_position(Vector2i(176, 243));
        msg_dlg->cancel_button()->set_fixed_size(Vector2i(156, 60));
    }
}

void do_with_green_light_blink(Widget *widget, int choose)
{
    Led3000Window * led3000Window = dynamic_cast<Led3000Window *>(widget->screen());
    const std::vector<Button *> * green_dev_btns = led3000Window->get_green_dev_control_btns();

    if (!led3000Window->getJsonValue()->devices[led3000Window->getCurrentDevice()].green_led.auth) {
        return;
    }

    if (choose == 1) {
        led3000Window->m4PolyM[POLYM_GREEN_BLINK_SETTING].assign(to_string(led3000Window->getJsonValue()->devices[led3000Window->getCurrentDevice()].green_led.blink_freq));
        /* 发送消息控制频闪 */
        led3000Window->getCurrentDeviceQueue().put(PolyM::DataMsg<std::string>(POLYM_GREEN_BLINK_SETTING, led3000Window->m4PolyM[POLYM_GREEN_BLINK_SETTING]));
        led3000Window->getJsonValue()->devices[led3000Window->getCurrentDevice()].green_led.mode = LED_BLINK_MODE;
        /* 同步消息内容到 json 文件 */
        led3000Window->getJsonQueue().put(PolyM::DataMsg<std::string>(POLYM_BUTTON_CONFIRM, "json"));
        green_dev_btns->at(0)->set_pushed(false);
        green_dev_btns->at(1)->set_pushed(true);
        green_dev_btns->at(2)->set_pushed(false);
    }
    else if (led3000Window->getJsonValue()->devices[led3000Window->getCurrentDevice()].green_led.mode = LED_BLINK_MODE)
    {
        led3000Window->m4PolyM[POLYM_GREEN_NORMAL_SETTING].assign(to_string(0));
        led3000Window->getJsonValue()->devices[led3000Window->getCurrentDevice()].green_led.mode = LED_NORMAL_MODE_OFF;
        /* 发送消息关灯 */
        led3000Window->getCurrentDeviceQueue().put(PolyM::DataMsg<std::string>(POLYM_GREEN_NORMAL_SETTING, led3000Window->m4PolyM[POLYM_GREEN_NORMAL_SETTING]));
        /* 同步消息内容到 json 文件 */
        led3000Window->getJsonQueue().put(PolyM::DataMsg<std::string>(POLYM_BUTTON_CONFIRM, "json"));
        green_dev_btns->at(1)->set_pushed(false);
    }
}

void do_paint_green_light_mocode(Widget *widget)
{
    Led3000Window * led3000Window = dynamic_cast<Led3000Window *>(widget->window()->parent());
    if (!led3000Window->getJsonValue()->devices[led3000Window->getCurrentDevice()].green_led.auth) {
        _do_paint_green_light_no_auth(widget);
    } else {
        MessageDialog * msg_dlg = dynamic_cast<MessageDialog *>(widget);
        auto * mocode_value_title = widget->add<Label>("莫码参数为：", "sans-bold");
        mocode_value_title->set_font_size(20);
        widget->window()->set_background_image(RED_LED3000_ASSETS_DIR"/set_msgdlg1.png");
        auto *textBox = widget->add<TextBox>("", KeyboardType::Full);
        textBox->set_position(Vector2i(134, 167));
        textBox->set_fixed_size(Vector2i(178, 46));
        textBox->set_editable(true);
        textBox->set_value(led3000Window->getJsonValue()->devices[led3000Window->getCurrentDevice()].green_led.mocode);
        textBox->setSyncCharsValue(&(led3000Window->getJsonValue()->devices[led3000Window->getCurrentDevice()].green_led.mocode[0]));
        textBox->set_font_size(16);
        textBox->set_alignment(TextBox::Alignment::Left);

        mocode_value_title->set_position(Vector2i(41, 180));

        msg_dlg->label_icon()->set_position(Vector2i(148, 91));
        msg_dlg->message_label()->set_position(Vector2i(104, 282));
        msg_dlg->confirm_button()->set_position(Vector2i(10, 354));
        msg_dlg->confirm_button()->set_fixed_size(Vector2i(156, 60));
        msg_dlg->cancel_button()->set_position(Vector2i(176, 354));
        msg_dlg->cancel_button()->set_fixed_size(Vector2i(156, 60));
    }
}

void do_with_green_light_mocode(Widget *widget, int choose)
{
    Led3000Window * window = dynamic_cast<Led3000Window *>(widget->screen());
    const std::vector<Button *> * green_dev_btns = window->get_green_dev_control_btns();
    Led3000Window * led3000Window = dynamic_cast<Led3000Window *>(widget->window()->parent());

    if (!led3000Window->getJsonValue()->devices[led3000Window->getCurrentDevice()].green_led.auth) {
        return;
    }

    if (choose == 1) {
        led3000Window->getJsonValue()->devices[led3000Window->getCurrentDevice()].green_led.mode = LED_MOCODE_MODE;
        /* 发送消息控制莫码 */
        led3000Window->getCurrentDeviceQueue().put(PolyM::DataMsg<std::string>(POLYM_GREEN_MOCODE_SETTING, led3000Window->getJsonValue()->devices[led3000Window->getCurrentDevice()].green_led.mocode));
        /* 更新界面莫码显示 */
        led3000Window->get_dev_morse_code_label(led3000Window->getCurrentDevice())->set_caption_merge(led3000Window->getJsonValue()->devices[led3000Window->getCurrentDevice()].green_led.mocode, led3000Window->getJsonValue()->devices[led3000Window->getCurrentDevice()].white_led.mocode, '/');
        /* 同步消息内容到 json 文件 */
        led3000Window->getJsonQueue().put(PolyM::DataMsg<std::string>(POLYM_BUTTON_CONFIRM, "json"));

        green_dev_btns->at(0)->set_pushed(false);
        green_dev_btns->at(1)->set_pushed(false);
        green_dev_btns->at(2)->set_pushed(true);
        return;
    }
    else if (led3000Window->getJsonValue()->devices[led3000Window->getCurrentDevice()].green_led.mode = LED_MOCODE_MODE)
    {
        led3000Window->getJsonValue()->devices[led3000Window->getCurrentDevice()].green_led.mode = LED_NORMAL_MODE_OFF;
        led3000Window->m4PolyM[POLYM_GREEN_NORMAL_SETTING].assign(to_string(0));
        /* 发送消息关灯 */
        led3000Window->getCurrentDeviceQueue().put(PolyM::DataMsg<std::string>(POLYM_GREEN_NORMAL_SETTING, led3000Window->m4PolyM[POLYM_GREEN_NORMAL_SETTING]));
        /* 同步消息内容到 json 文件 */
        led3000Window->getJsonQueue().put(PolyM::DataMsg<std::string>(POLYM_BUTTON_CONFIRM, "json"));
        green_dev_btns->at(2)->set_pushed(false);
        return;
    }
}

void do_paint_scan_setting(Widget *widget)
{
    MessageDialog * msg_dlg = dynamic_cast<MessageDialog *>(widget);
    Led3000Window * led3000Window = dynamic_cast<Led3000Window *>(widget->window()->parent());

    widget->window()->set_fixed_size(Vector2i(480, 542));
    widget->window()->set_background_image(RED_LED3000_ASSETS_DIR"/set_msgdlg4.png");

    auto * stay_time_value_title = widget->add<Label>("扫海间隔(秒)：", "sans-bold");
    stay_time_value_title->set_font_size(20);

    auto *textBox = widget->add<TextBox>("", KeyboardType::NumberIP);
    textBox->set_position(Vector2i(174, 167));
    textBox->set_fixed_size(Vector2i(276, 46));
    textBox->set_editable(true);
    textBox->set_value(to_string(led3000Window->getJsonValue()->devices[led3000Window->getCurrentDevice()].turntable.scan_stay_time));
    textBox->setSyncShortValue(&(led3000Window->getJsonValue()->devices[led3000Window->getCurrentDevice()].turntable.scan_stay_time));
    textBox->set_font_size(16);
    textBox->set_alignment(TextBox::Alignment::Left);

    auto * speed_level_title = widget->add<Label>("速度等级：", "sans-bold");
    speed_level_title->set_font_size(20);
    textBox = widget->add<TextBox>("", KeyboardType::NumberIP);
    textBox->set_position(Vector2i(174, 223));
    textBox->set_fixed_size(Vector2i(276, 46));
    textBox->set_editable(true);
    textBox->set_value(to_string(led3000Window->getJsonValue()->devices[led3000Window->getCurrentDevice()].turntable.scan_speed_level));
    textBox->setSyncShortValue(&(led3000Window->getJsonValue()->devices[led3000Window->getCurrentDevice()].turntable.scan_speed_level));
    textBox->set_font_size(16);
    textBox->set_alignment(TextBox::Alignment::Left);

    Button *btn_boundary = widget->add<Button>("左边界");
    btn_boundary->set_fixed_size({200, 46});
    btn_boundary->set_position({30, 289});
    btn_boundary->set_callback([ = ] {
        red_debug_lite("Set left boundary");
        led3000Window->getCurrentDeviceQueue().put(PolyM::DataMsg<std::string>(POLYM_TURNTABLE_SCAN_MODE_CONFIG_LEFT_BOUNDARY, to_string(0)));
    });

    btn_boundary = widget->add<Button>("右边界");
    btn_boundary->set_fixed_size({200, 46});
    btn_boundary->set_position({250, 289});
    btn_boundary->set_callback([ = ] {
        led3000Window->getCurrentDeviceQueue().put(PolyM::DataMsg<std::string>(POLYM_TURNTABLE_SCAN_MODE_CONFIG_RIGHT_BOUNDARY, to_string(0)));
        red_debug_lite("Set right boundary");
    });

    stay_time_value_title->set_position(Vector2i(55, 180));
    speed_level_title->set_position(Vector2i(55, 236));

    msg_dlg->label_icon()->set_position(Vector2i(217, 91));
    msg_dlg->message_label()->set_position(Vector2i(188, 404));

    msg_dlg->confirm_button()->set_position(Vector2i(10, 472));
    msg_dlg->confirm_button()->set_fixed_size(Vector2i(225, 60));
    msg_dlg->cancel_button()->set_position(Vector2i(245, 472));
    msg_dlg->cancel_button()->set_fixed_size(Vector2i(225, 60));
}

void do_with_scan_setting(Widget *widget, int choose)
{
    //Led3000Window * window = dynamic_cast<Led3000Window *>(widget->screen());
    Led3000Window * led3000Window = dynamic_cast<Led3000Window *>(widget->window()->parent());
    /* 需要对该数据在发送端转换为大端发送出去 */
    short value_nethost = 0;
    if (choose == 1) {
        /* 发送扫海参数配置 */
        value_nethost = led3000Window->getJsonValue()->devices[led3000Window->getCurrentDevice()].turntable.scan_stay_time;
        led3000Window->m4PolyM[POLYM_TURNTABLE_SCAN_MODE_CONFIG_STAY_TIME] = to_string(value_nethost);
        led3000Window->getCurrentDeviceQueue().put(PolyM::DataMsg<std::string>(POLYM_TURNTABLE_SCAN_MODE_CONFIG_STAY_TIME, led3000Window->m4PolyM[POLYM_TURNTABLE_SCAN_MODE_CONFIG_STAY_TIME]));

        usleep(100000);
        value_nethost = led3000Window->getJsonValue()->devices[led3000Window->getCurrentDevice()].turntable.scan_speed_level;
        led3000Window->m4PolyM[POLYM_TURNTABLE_SCAN_MODE_CONFIG_SPEED_LEVEL] = to_string(value_nethost);
        led3000Window->getCurrentDeviceQueue().put(PolyM::DataMsg<std::string>(POLYM_TURNTABLE_SCAN_MODE_CONFIG_SPEED_LEVEL, led3000Window->m4PolyM[POLYM_TURNTABLE_SCAN_MODE_CONFIG_SPEED_LEVEL]));
        /* 同步消息内容到 json 文件 */
        led3000Window->getJsonQueue().put(PolyM::DataMsg<std::string>(POLYM_BUTTON_CONFIRM, "json"));
        return;
    } else {
        led3000Window->getJsonQueue().put(PolyM::DataMsg<std::string>(POLYM_BUTTON_CANCEL, "json"));
        return;
    }
}

void do_with_guide_leave(Widget *widget, int choose)
{
    Led3000Window * led3000Window = dynamic_cast<Led3000Window *>(widget->window()->parent());
    if (choose == 1) {
        /* 脱离引导 */
        led3000Window->set_guide_leave(true, led3000Window->getCurrentDevice());
        return;
    }
    else {
        led3000Window->set_guide_leave(false, led3000Window->getCurrentDevice());
    }
}

void do_with_turntable_reset(Widget *widget, int choose)
{
    Led3000Window * led3000Window = dynamic_cast<Led3000Window *>(widget->window()->parent());
    /* 需要对该数据在发送端转换为大端发送出去 */
    if (choose == 1) {
        /* 发送复位转台消息 */
        led3000Window->getCurrentDeviceQueue().put(PolyM::DataMsg<std::string>(POLYM_TURNTABLE_RESET, ""));
        return;
    }
}

Led3000Window::Led3000Window(): Screen(Vector2i(1280, 800), "NanoGUI Test", false, true),
    mFileName("/opt/led3000.json"), mFp(nullptr), m_attitude_info(nullptr),
    m_time4dispaly(nullptr)
{
    m_guide_mode_icon[0] = nullptr;
    m_guide_mode_icon[1] = nullptr;
    m_guide_info_label[0] = nullptr;
    m_guide_info_label[1] = nullptr;
    m_guide_status[0] = false;
    m_guide_status[1] = false;
    /* 默认非屏蔽指挥 */
    m_guide_leave_status[0] = false;
    m_guide_leave_status[1] = false;
    m_guide_leave_icon[0] = nullptr;
    m_guide_leave_icon[1] = nullptr;
    /* 默认指控禁止射击 */
    m_guide_shoot_status[0] = false;
    m_guide_shoot_status[1] = false;

    /* 系统配置参数初始化 */
    {
        memcpy(mjsonBuffer, "{}", strlen("{}") + 1);
        mFp = fopen(mFileName.c_str(), "r");
        if (mFp) {
            fread(mjsonBuffer, sizeof(char), sizeof(mjsonBuffer), mFp);
            if (mDocument.ParseInsitu(mjsonBuffer).HasParseError()) {
                return;
            }
        } else if (!mFp) {
            mFp = fopen(mFileName.c_str(), "w+");
            assert(mFp);
            //fwrite(mjsonBuffer, sizeof(char), strlen(mjsonBuffer) + 1, mFp);
            assert(false == mDocument.ParseInsitu(mjsonBuffer).HasParseError());
        }

        Document::AllocatorType& allocator = mDocument.GetAllocator();

        if (mDocument.FindMember("sys_config") == mDocument.MemberEnd()) {
            Value sys_config(kObjectType);
            sys_config.AddMember("version", LED3000_VERSION, allocator);
            sys_config.AddMember("id", LED3000_ID, allocator);
            mDocument.AddMember("sys_config", sys_config, allocator);
        } else {
            mJsonValue.sys_config.version = Pointer("/sys_config/version").Get(mDocument)->GetUint();
            mJsonValue.sys_config.id = Pointer("/sys_config/id").Get(mDocument)->GetUint();
        }

        if (mDocument.MemberEnd() == mDocument.FindMember("eths")) {
            Value eth_array(kArrayType);
            Value eth0_config(kObjectType);
            Value eth1_config(kObjectType);

            eth0_config.AddMember("name", "引导", allocator);
            eth0_config.AddMember("ip", "168.9.0.1", allocator);
            eth0_config.AddMember("netmask", "255.255.0.0", allocator);
            eth0_config.AddMember("gateway", "168.9.255.254", allocator);

            eth1_config.AddMember("name", "摄像头", allocator);
            eth1_config.AddMember("ip", "192.168.100.110", allocator);
            eth1_config.AddMember("netmask", "255.255.255.0", allocator);
            eth1_config.AddMember("gateway", "192.168.100.1", allocator);

            eth_array.PushBack(eth0_config, allocator);
            eth_array.PushBack(eth1_config, allocator);

            mDocument.AddMember("eths", eth_array, allocator);
        } else {
            const Value& eths_info = mDocument["eths"];
            for (SizeType i = 0; i < eths_info.Size(); i++) {
                memcpy(mJsonValue.eths[i].name, eths_info[i]["name"].GetString(), strlen(eths_info[i]["name"].GetString()) + 1);
                memcpy(mJsonValue.eths[i].ip, eths_info[i]["ip"].GetString(), strlen(eths_info[i]["ip"].GetString()) + 1);
                memcpy(mJsonValue.eths[i].netmask, eths_info[i]["netmask"].GetString(), strlen(eths_info[i]["netmask"].GetString()) + 1);
                memcpy(mJsonValue.eths[i].gateway, eths_info[i]["gateway"].GetString(), strlen(eths_info[i]["gateway"].GetString()) + 1);
            }
        }

        if (mDocument.MemberEnd() == mDocument.FindMember("server")) {
            Value server_config(kObjectType);

            server_config.AddMember("ip", "168.6.0.4", allocator);
            server_config.AddMember("port", 20840, allocator);

            mDocument.AddMember("server", server_config, allocator);
        } else {
            const Value& server = mDocument["server"];
            memcpy(mJsonValue.server.ip, server["ip"].GetString(), strlen(server["ip"].GetString()) + 1);
            mJsonValue.server.port = server["port"].GetUint();
        }

        if (mDocument.MemberEnd() == mDocument.FindMember("devices")) {
            Value device_array(kArrayType);
            Value device_config_template(kObjectType);
            Value white_led_config(kObjectType);
            Value green_led_config(kObjectType);
            Value turntable_config(kObjectType);

            device_config_template.AddMember("camera_url", "rtsp://192.168.100.64", allocator);
            /* 默认为关灯模式 */
            white_led_config.AddMember("mode", LED_NORMAL_MODE_OFF, allocator);
            white_led_config.AddMember("normal_status", 0, allocator);
            white_led_config.AddMember("blink_freq", 1, allocator);
            white_led_config.AddMember("mocode", "", allocator);

            /* 默认为禁止射击并且关灯模式 */
            green_led_config.AddMember("auth", 0, allocator);
            green_led_config.AddMember("mode", LED_NORMAL_MODE_OFF, allocator);
            green_led_config.AddMember("normal_status", 0, allocator);
            green_led_config.AddMember("blink_freq", 1, allocator);
            green_led_config.AddMember("mocode", "", allocator);

            turntable_config.AddMember("mode", TURNTABLE_MANUAL_MODE, allocator);
            turntable_config.AddMember("target_pos_x", 0, allocator);
            turntable_config.AddMember("target_pos_y", 0, allocator);
            turntable_config.AddMember("scan_stay_time", 10, allocator);
            turntable_config.AddMember("scan_speed_level", 32, allocator);

            device_config_template.AddMember("white_led", white_led_config, allocator);
            device_config_template.AddMember("green_led", green_led_config, allocator);
            device_config_template.AddMember("turntable", turntable_config, allocator);
            Value device_config;

            for (int i = 0; i < 2; i++) {
                /* deep copy */
                device_config.CopyFrom(device_config_template, allocator);
                device_array.PushBack(device_config, allocator);
            }

            mDocument.AddMember("devices", device_array, allocator);
        } else {
            const Value& devices = mDocument["devices"];
            for (SizeType i = 0; i < devices.Size(); i++) {
                memcpy(mJsonValue.devices[i].camera_url, devices[i]["camera_url"].GetString(), strlen(devices[i]["camera_url"].GetString()) + 1);
                mJsonValue.devices[i].white_led.mode = (unsigned char)devices[i]["white_led"]["mode"].GetUint();
                mJsonValue.devices[i].white_led.normal_status = (unsigned char)devices[i]["white_led"]["normal_status"].GetUint();
                mJsonValue.devices[i].white_led.blink_freq = (unsigned char)devices[i]["white_led"]["blink_freq"].GetUint();
                memcpy(mJsonValue.devices[i].white_led.mocode, devices[i]["white_led"]["mocode"].GetString(), strlen(devices[i]["white_led"]["mocode"].GetString()) + 1);

                /* 更新 auth 状态 */
                mJsonValue.devices[i].green_led.auth = (unsigned char)devices[i]["green_led"]["auth"].GetUint();
                mJsonValue.devices[i].green_led.mode = (unsigned char)devices[i]["green_led"]["mode"].GetUint();
                mJsonValue.devices[i].green_led.normal_status = (unsigned char)devices[i]["green_led"]["normal_status"].GetUint();
                mJsonValue.devices[i].green_led.blink_freq = (unsigned char)devices[i]["green_led"]["blink_freq"].GetUint();
                memcpy(mJsonValue.devices[i].green_led.mocode, devices[i]["green_led"]["mocode"].GetString(), strlen(devices[i]["green_led"]["mocode"].GetString()) + 1);

                mJsonValue.devices[i].turntable.mode = (unsigned char)devices[i]["turntable"]["mode"].GetUint();
                mJsonValue.devices[i].turntable.target_pos_x = (unsigned short)devices[i]["turntable"]["target_pos_x"].GetUint();
                mJsonValue.devices[i].turntable.target_pos_y = (unsigned short)devices[i]["turntable"]["target_pos_y"].GetUint();
                mJsonValue.devices[i].turntable.scan_stay_time = (unsigned short)devices[i]["turntable"]["scan_stay_time"].GetUint();
                mJsonValue.devices[i].turntable.scan_speed_level = (unsigned short)devices[i]["turntable"]["scan_speed_level"].GetUint();
            }
        }

        fclose(mFp);

        mFp = fopen(mFileName.c_str(), "w");
        FileWriteStream os(mFp, mjsonBuffer, sizeof(mjsonBuffer));
        Writer<FileWriteStream> writer(os);
        mDocument.Accept(writer);
        fclose(mFp);
    }
    /* 设备状态窗口 */
    {
        auto* swindow = new Window(this, "");
        swindow->set_fixed_size({1240, 166});
        swindow->set_background_image(RED_LED3000_ASSETS_DIR"/status.png");
        /* 确定了 swindow 的位置 */
        swindow->set_position({20, 444});

        /* 调用 add 函数模板
         * 将新创建的 label 控件关联到 swindow 作为其 parent
         * */
        auto * label = swindow->add<Label>("设备名称", "sans-bold");
        label->set_position(Vector2i(60, 10));
        label = swindow->add<Label>("设备状态", "sans-bold");
        label->set_position(Vector2i(323, 10));
        label = swindow->add<Label>("方位/俯仰角度(°)", "sans-bold");
        label->set_position(Vector2i(516, 10));
        label = swindow->add<Label>("方位/俯仰角速度(°/s)", "sans-bold");
        label->set_position(Vector2i(701, 10));
        label = swindow->add<Label>("莫码信息", "sans-bold");
        label->set_position(Vector2i(930, 10));
        label = swindow->add<Label>("绿灯状态", "sans-bold");
        label->set_position(Vector2i(1135, 10));

        label = swindow->add<Label>("灯光装置终端二", "sans-bold");
        label->set_position(Vector2i(40, 62));
        m_dev_state[0] = new Label(swindow, "离线", "sans");
        m_dev_state[0]->set_position(Vector2i(323, 62));
        m_dev_angle[0] = new Label(swindow, "-/-", "sans");
        m_dev_angle[0]->set_position(Vector2i(516, 62));
        m_dev_angle[0]->set_fixed_size(Vector2i(200, 50));
        m_dev_angular_speed[0] = new Label(swindow, "-/-", "sans");
        m_dev_angular_speed[0]->set_position(Vector2i(701, 62));
        m_dev_angular_speed[0]->set_fixed_size(Vector2i(200, 50));
        m_dev_morse_code[0] = new Label(swindow, "", "sans");
        m_dev_morse_code[0]->set_caption_merge(mJsonValue.devices[0].green_led.mocode, mJsonValue.devices[0].white_led.mocode, '/');
        m_dev_morse_code[0]->set_position(Vector2i(930, 46));
        m_dev_morse_code[0]->set_fixed_size(Vector2i(200, 50));
        m_dev_auth[0] = new Label(swindow, "", "sans");
        m_dev_auth[0]->set_caption("禁止发射");
        m_dev_auth[0]->set_position(Vector2i(1135, 62));

        label = swindow->add<Label>("灯光装置终端一", "sans-bold");
        label->set_position(Vector2i(40, 125));
        m_dev_state[1] = new Label(swindow, "离线", "sans");
        m_dev_state[1]->set_position(Vector2i(323, 125));
        m_dev_angle[1] = new Label(swindow, "-/-", "sans");
        m_dev_angle[1]->set_position(Vector2i(516, 125));
        m_dev_angle[1]->set_fixed_size(Vector2i(200, 50));
        m_dev_angular_speed[1] = new Label(swindow, "-/-", "sans");
        m_dev_angular_speed[1]->set_position(Vector2i(701, 125));
        m_dev_angular_speed[1]->set_fixed_size(Vector2i(200, 50));
        m_dev_morse_code[1] = new Label(swindow, "", "sans");
        m_dev_morse_code[1]->set_caption_merge(mJsonValue.devices[1].green_led.mocode, mJsonValue.devices[1].white_led.mocode, '/');
        m_dev_morse_code[1]->set_position(Vector2i(930, 109));
        m_dev_morse_code[1]->set_fixed_size(Vector2i(200, 50));
        m_dev_auth[1] = new Label(swindow, "", "sans");
        m_dev_auth[1]->set_caption("禁止发射");
        m_dev_auth[1]->set_position(Vector2i(1135, 125));
    }

#if 0
    {
        m_dev_auth_light_fd[0] = open("/sys/devices/platform/leds/leds/auth0/brightness", O_RDWR);
        m_dev_auth_light_fd[1] = open("/sys/devices/platform/leds/leds/auth1/brightness", O_RDWR);
        /* 上电默认是未授权 */
        const char value_off = 0x30;
        if (m_dev_auth_light_fd[0])
            write(m_dev_auth_light_fd[0], &value_off, 1);
        if (m_dev_auth_light_fd[1])
            write(m_dev_auth_light_fd[1], &value_off, 1);
    }
#endif
    /* 设备控制窗口 */
    {
        auto* cwindow = new Window(this, "");
        cwindow->set_background_image(RED_LED3000_ASSETS_DIR"/green1.png");

        /* 确定了 cwindow 的位置 */
        cwindow->set_position({20, 630});
        cwindow->set_fixed_size({400, 150});
        /* 创建一个新的布局 */
        GridLayout * layout = new GridLayout(Orientation::Horizontal, 3,
                                             Alignment::Middle, 10, 10);
        layout->set_col_alignment({ Alignment::Maximum, Alignment::Fill });
        layout->set_spacing(0, 10);
        /* 定义了这个窗口的布局 */
        //cwindow->set_layout(layout);

        m_green_dev = new Label(cwindow, "灯光装置终端二 绿灯");
        m_green_dev->set_position({39, 9});
        Button *btn_green_led = new Button(cwindow, "常亮");
        btn_green_led->set_fixed_size({120, 92});
        btn_green_led->set_position({10, 48});
        btn_green_led->set_callback([&] {
            new MessageDialog(this, MessageDialog::Type::Warning, "", "是否开启绿灯常亮功能", "确认", "取消", "", do_with_green_light_normal, do_paint_green_light_normal);
        });

        Button *btn_green_blink = new Button(cwindow, "绿闪");
        btn_green_blink->set_fixed_size({120, 92});
        btn_green_blink->set_position({140, 48});
        btn_green_blink->set_callback([&] {
            new MessageDialog(this, MessageDialog::Type::Question, "", "", "确认", "取消", "", do_with_green_light_blink, do_paint_green_light_blink);
        });
        Button *btn_green_mocode = new Button(cwindow, "莫码");
        btn_green_mocode->set_fixed_size({120, 92});
        btn_green_mocode->set_position({270, 48});
        btn_green_mocode->set_callback([&] {
            new MessageDialog(this, MessageDialog::Type::Question, "", "开启绿光莫码模式？", "确认", "取消", "", do_with_green_light_mocode, do_paint_green_light_mocode);
        });

        btn_green_led->push_button_group(btn_green_led);
        btn_green_led->push_button_group(btn_green_blink);
        btn_green_led->push_button_group(btn_green_mocode);
        btn_green_blink->set_button_group(btn_green_led->button_group());
        btn_green_mocode->set_button_group(btn_green_led->button_group());
        set_green_dev_control_btns(&btn_green_led->button_group());
        set_green_dev_control_btns_status(getJsonValue()->devices[0].green_led.mode);

        cwindow = new Window(this, "");
        cwindow->set_background_image(RED_LED3000_ASSETS_DIR"/green1.png");
        cwindow->set_position({440, 630});
        cwindow->set_fixed_size({400, 150});
        m_white_dev = new Label(cwindow, "灯光装置终端二 白灯");
        m_white_dev->set_position({39, 9});
        auto * btn_white_led = new Button(cwindow, "常亮");
        btn_white_led->set_position({10, 48});
        btn_white_led->set_fixed_size({120, 92});
        btn_white_led->set_callback([&] {
            new MessageDialog(this, MessageDialog::Type::Warning, "", "确认要打开白光么?", "确认", "取消", "", do_with_white_light_normal);
        });
        auto * btn_white_blink = new Button(cwindow, "白闪");
        btn_white_blink->set_position({140, 48});
        btn_white_blink->set_fixed_size({120, 92});
        btn_white_blink->set_callback([&] {
            new MessageDialog(this, MessageDialog::Type::Question, "", "", "确认", "取消", "", do_with_white_light_blink, do_paint_white_light_blink);
        });
        auto * btn_white_mocode = new Button(cwindow, "莫码");
        btn_white_mocode->set_position({270, 48});
        btn_white_mocode->set_fixed_size({120, 92});
        btn_white_mocode->set_callback([&] {
            new MessageDialog(this, MessageDialog::Type::Question, "", "开启白光莫码模式？", "确认", "取消", "", do_with_white_light_mocode, do_paint_white_light_mocode);
        });

        btn_white_led->push_button_group(btn_white_led);
        btn_white_led->push_button_group(btn_white_blink);
        btn_white_led->push_button_group(btn_white_mocode);
        btn_white_blink->set_button_group(btn_white_led->button_group());
        btn_white_mocode->set_button_group(btn_white_led->button_group());
        set_white_dev_control_btns(&btn_white_led->button_group());
        set_white_dev_control_btns_status(getJsonValue()->devices[0].white_led.mode);
    }

    /* 系统窗口 */
    {
        auto* swindow = new Window(this, "");
        Button * sysconfig_btn;

        /* 确定了 swindow 的位置 */
        swindow->set_position({0, 0});
        swindow->set_fixed_size({1280, 78});
        swindow->set_background_image(RED_LED3000_ASSETS_DIR"/head.png");

        /* 创建一个 label 显示软件版本号 */
        auto *soft_name_label = swindow->add<Label>("眩目拒止设备显控装置", "sans-bold");
        soft_name_label->set_position(Vector2i(81, 25));
        soft_name_label->set_font_size(15);

        auto *ver_label = swindow->add<Label>("V" + to_string(LED3000_VERSION >> 16 & 0XFF) +
                                              "." +
                                              to_string(LED3000_VERSION >> 8 & 0XFF) +
                                              + " 中国船舶第七一六研究所", "sans-bold");
        ver_label->set_position(Vector2i(81, 40));
        ver_label->set_font_size(15);

        m_guide_mode_icon[0] = swindow->add<Label>("", "sans-bold");
        m_guide_mode_icon[0]->set_icon(RED_LED3000_ASSETS_DIR"/guide.png");
        m_guide_mode_icon[0]->set_position(Vector2i(870, 15));
        m_guide_mode_icon[0]->set_visible(m_guide_status[0]);

        m_guide_mode_icon[1] = swindow->add<Label>("", "sans-bold");
        m_guide_mode_icon[1]->set_icon(RED_LED3000_ASSETS_DIR"/guide.png");
        m_guide_mode_icon[1]->set_position(Vector2i(870, 15));
        m_guide_mode_icon[1]->set_visible(m_guide_status[1]);

        m_guide_leave_icon[0] = swindow->add<Label>("", "sans-bold");
        m_guide_leave_icon[0]->set_icon(RED_LED3000_ASSETS_DIR"/guide_leave.png");
        m_guide_leave_icon[0]->set_position(Vector2i(360, 15));
        m_guide_leave_icon[0]->set_visible(m_guide_leave_status[0]);

        m_guide_leave_icon[1] = swindow->add<Label>("", "sans-bold");
        m_guide_leave_icon[1]->set_icon(RED_LED3000_ASSETS_DIR"/guide_leave.png");
        m_guide_leave_icon[1]->set_position(Vector2i(360, 15));
        m_guide_leave_icon[1]->set_visible(m_guide_leave_status[1]);

        m_guide_info_label[0] = swindow->add<Label>("[--------/------]", "sans-bold");
        m_guide_info_label[0]->set_position(Vector2i(920, 40));
        m_guide_info_label[0]->set_font_size(20);
        m_guide_info_label[0]->set_visible(m_guide_status[0]);

        m_guide_info_label[1] = swindow->add<Label>("[--------/------]", "sans-bold");
        m_guide_info_label[1]->set_position(Vector2i(920, 40));
        m_guide_info_label[1]->set_font_size(20);
        m_guide_info_label[1]->set_visible(m_guide_status[1]);

        m_time4dispaly= swindow->add<Label>("       ", "digital");
        m_time4dispaly->set_position(Vector2i(1120, 30));
        m_time4dispaly->set_font_size(20);

        sysconfig_btn = swindow->add<Button>("", RED_LED3000_ASSETS_DIR"/power.png", 0);
        sysconfig_btn->set_position({1219, 15});
        sysconfig_btn->set_fixed_size({46, 46});
        sysconfig_btn->set_callback([&] {
            new MessageDialog(this, MessageDialog::Type::Question, "", "确认要关机么?", "确认", "取消", "", do_with_power_off);
        });
        sysconfig_btn = swindow->add<Button>("", RED_LED3000_ASSETS_DIR"/sys.png", 0);
        sysconfig_btn->set_position({15, 15});
        sysconfig_btn->set_fixed_size({46, 46});
        sysconfig_btn->set_callback([&] {
            new MessageDialog(this, MessageDialog::Type::Question, "", "配置参数", "确认", "取消", "", do_with_sysconfig, do_paint_sysconfig);
        });

        Button *devBtn = swindow->add<Button>("    灯光装置终端二", RED_LED3000_ASSETS_DIR"/dev_unchoose.png", RED_LED3000_ASSETS_DIR"/dev_choose.png", 0);
        devBtn->set_flags(Button::RadioButton);
        devBtn->set_position({415, 13});
        devBtn->set_fixed_size({220, 50});
        devBtn->set_pushed(true);
        devBtn->set_callback([this]() {
            this->get_green_dev_label()->set_caption("灯光装置终端二 绿灯");
            this->get_white_dev_label()->set_caption("灯光装置终端二 白灯");
            this->get_turntable_label()->set_caption("灯光装置终端二 转台");
            this->setCurrentDevice(0);
            /* 更新灯光装置终端的状态 */
            this->set_white_dev_control_btns_status(this->getJsonValue()->devices[0].white_led.mode);
            this->set_green_dev_control_btns_status(this->getJsonValue()->devices[0].green_led.mode);
            this->set_turntable_dev_control_btns_status(this->getJsonValue()->devices[0].turntable.mode);
            this->sync_guide_relate_display(0);

        });
        /* 根据实际系统功能中按键大小，为了保持大小一致，修改设备选择按键大小保持一致 */
        ///devBtn->set_fixed_size(Vector2i(165, 30));
        Button *devBtn2 = swindow->add<Button>("    灯光装置终端一", RED_LED3000_ASSETS_DIR"/dev_unchoose.png", RED_LED3000_ASSETS_DIR"/dev_choose.png", 0);
        devBtn2->set_flags(Button::RadioButton);
        devBtn2->set_position({645, 13});
        devBtn2->set_fixed_size({220, 50});
        devBtn2->set_callback([this]() {
            this->get_green_dev_label()->set_caption("灯光装置终端一 绿灯");
            this->get_white_dev_label()->set_caption("灯光装置终端一 白灯");
            this->get_turntable_label()->set_caption("灯光装置终端一 转台");
            this->setCurrentDevice(1);
            /* 更新灯光装置终端的状态 */
            this->set_white_dev_control_btns_status(this->getJsonValue()->devices[1].white_led.mode);
            this->set_green_dev_control_btns_status(this->getJsonValue()->devices[1].green_led.mode);
            this->set_turntable_dev_control_btns_status(this->getJsonValue()->devices[1].turntable.mode);
            this->sync_guide_relate_display(1);
        });

        devBtn2->push_button_group(devBtn2);
        devBtn2->push_button_group(devBtn);
        devBtn->push_button_group(devBtn);
        devBtn->push_button_group(devBtn2);
    }

    /* 转台功能 */
    {
        auto* turntableWindow = new Window(this, "");
        turntableWindow->set_background_image(RED_LED3000_ASSETS_DIR"/green1.png");

        /* 确定了 turntableWindow 的位置 */
        turntableWindow->set_position({860, 630});

        m_turntable_dev = turntableWindow->add<Label>("灯光装置终端二 转台");
        m_turntable_dev->set_position({39, 9});

        /* 屏蔽引导信息 */
        Button *guide_leave_btn = turntableWindow->add<Button>("", RED_LED3000_ASSETS_DIR"/leave.png", 0);
        guide_leave_btn->set_flags(Button::ToggleButton);
        guide_leave_btn->set_position({230, 4});
        guide_leave_btn->set_fixed_size({32, 32});
#if 1
        guide_leave_btn->set_change_callback([guide_leave_btn](bool state) { std::cout << "Toggle button state: " << state << std::endl; do_with_guide_leave(guide_leave_btn, state);});
#else
        guide_leave_btn->set_callback([this]() {
            new MessageDialog(this, MessageDialog::Type::Warning, "", "确认要脱离引导么?", "确认", "取消", "", do_with_guide_leave);
        });
#endif

        /* 转台复位按键 */
        Button *turntable_reset_btn = turntableWindow->add<Button>("", RED_LED3000_ASSETS_DIR"/turntable_reset.png", 0);
        turntable_reset_btn->set_position({285, 4});
        turntable_reset_btn->set_fixed_size({32, 32});
        turntable_reset_btn->set_callback([this]() {
            new MessageDialog(this, MessageDialog::Type::Warning, "", "确认要复位当前转台么?", "确认", "取消", "", do_with_turntable_reset);
        });

        /* 扫海参数配置按键 */
        Button *scan_setting_btn = turntableWindow->add<Button>("", RED_LED3000_ASSETS_DIR"/setting_scan.png", 0);
        scan_setting_btn->set_position({345, 4});
        scan_setting_btn->set_fixed_size({32, 32});
        scan_setting_btn->set_callback([this]() {
            new MessageDialog(this, MessageDialog::Type::Question, "", "设置扫海参数", "确认", "取消", "", do_with_scan_setting, do_paint_scan_setting);
        });

        auto * btn_ai = turntableWindow->add<Button>("目标检测");
        btn_ai->set_callback([&] {
            this->getJsonValue()->devices[this->getCurrentDevice()].turntable.mode = TURNTABLE_TRACK_MODE;
            this->m4PolyM[POLYM_TURNTABLE_MODE_SETTING].assign(to_string(TURNTABLE_TRACK_MODE));
            this->getCurrentDeviceQueue().put(PolyM::DataMsg<std::string>(POLYM_TURNTABLE_MODE_SETTING, this->m4PolyM[POLYM_TURNTABLE_MODE_SETTING]));
            /* 同步消息内容到 json 文件 */
            this->getJsonQueue().put(PolyM::DataMsg<std::string>(POLYM_BUTTON_CONFIRM, "json"));
        });
        btn_ai->set_flags(Button::RadioButton);
        btn_ai->set_fixed_size({120, 92});
        btn_ai->set_position({10, 48});
        auto *btn_manual = turntableWindow->add<Button>("手动");
        btn_manual->set_callback([&] {
            this->getJsonValue()->devices[this->getCurrentDevice()].turntable.mode = TURNTABLE_MANUAL_MODE;
            this->m4PolyM[POLYM_TURNTABLE_MODE_SETTING].assign(to_string(TURNTABLE_MANUAL_MODE));
            this->getCurrentDeviceQueue().put(PolyM::DataMsg<std::string>(POLYM_TURNTABLE_MODE_SETTING, this->m4PolyM[POLYM_TURNTABLE_MODE_SETTING]));
            /* 同步消息内容到 json 文件 */
            this->getJsonQueue().put(PolyM::DataMsg<std::string>(POLYM_BUTTON_CONFIRM, "json"));
        });
        btn_manual->set_flags(Button::RadioButton);
        btn_manual->set_fixed_size({120, 92});
        btn_manual->set_position({140, 48});
        auto *btn_scan = turntableWindow->add<Button>("扫海");
        btn_scan->set_callback([&] {
            this->getJsonValue()->devices[this->getCurrentDevice()].turntable.mode = TURNTABLE_SCAN_MODE;
            this->m4PolyM[POLYM_TURNTABLE_MODE_SETTING].assign(to_string(TURNTABLE_SCAN_MODE));
            this->getCurrentDeviceQueue().put(PolyM::DataMsg<std::string>(POLYM_TURNTABLE_MODE_SETTING, this->m4PolyM[POLYM_TURNTABLE_MODE_SETTING]));
            /* 同步消息内容到 json 文件 */
            this->getJsonQueue().put(PolyM::DataMsg<std::string>(POLYM_BUTTON_CONFIRM, "json"));
        });
        btn_scan->set_flags(Button::RadioButton);
        btn_scan->set_fixed_size({120, 92});
        btn_scan->set_position({270, 48});
        set_turntable_mode_btns(btn_ai, btn_manual, btn_scan);
        /* 在这里更新 button 的状态,根据系统配置参数更新转台工作在哪种模式 */
        switch (mJsonValue.devices[0].turntable.mode) {
        case TURNTABLE_TRACK_MODE:
        case TURNTABLE_FUZZY_TRACK_MODE: btn_ai->set_pushed(true); break;
        case TURNTABLE_MANUAL_MODE: btn_manual->set_pushed(true); break;
        case TURNTABLE_SCAN_MODE: btn_scan->set_pushed(true); break;
        default:break;
        }
    }

    /* 显示船体姿态状态窗口 at 990,780 font size 14
     * 弦角:-000.00 横摇:-00.00 纵摇:-00.00
     * */
    {
        m_attitude_info = new Label(this, "艏向角(°):------- 横摇角(°):------- 纵摇角(°):------ ", "sans");

        m_attitude_info->set_position(Vector2i(950, 780));
        m_attitude_info->set_font_size(17);
    }

    {
        /* 创建一个新的 window 对象用来显示图片 */
        auto* img_window = new Window(this, "");
        img_window->set_fixed_size({610, 326});
        img_window->set_position(Vector2i(20, 98));
        img_window->set_background_image(RED_LED3000_ASSETS_DIR"/video.png");

        auto * track_video_btn = img_window->add<Button>("");
        track_video_btn->set_pseudo(true);
        track_video_btn->set_fixed_size(Vector2i(64, 60));
        track_video_btn->set_position(Vector2i(543, 4));
        track_video_btn->set_callback([&] {
            if (getJsonValue()->devices[0].turntable.mode <= TURNTABLE_FUZZY_TRACK_MODE)
            {
                if (this->getJsonValue()->devices[0].turntable.mode == TURNTABLE_TRACK_MODE) {

                    this->getJsonValue()->devices[0].turntable.mode = TURNTABLE_FUZZY_TRACK_MODE;
                    this->m4PolyM[POLYM_TURNTABLE_MODE_SETTING].assign(to_string(TURNTABLE_FUZZY_TRACK_MODE));
                    this->getDeviceQueue(0).put(PolyM::DataMsg<std::string>(POLYM_TURNTABLE_MODE_SETTING, this->m4PolyM[POLYM_TURNTABLE_MODE_SETTING]));
                    red_debug_lite("switch fuzzy track mode");
                } else {
                    this->getJsonValue()->devices[0].turntable.mode = TURNTABLE_TRACK_MODE;
                    this->m4PolyM[POLYM_TURNTABLE_MODE_SETTING].assign(to_string(TURNTABLE_TRACK_MODE));
                    this->getDeviceQueue(0).put(PolyM::DataMsg<std::string>(POLYM_TURNTABLE_MODE_SETTING, this->m4PolyM[POLYM_TURNTABLE_MODE_SETTING]));
                    red_debug_lite("switch target track mode");
                }
            }
        });

        /* 在这个 window 上创建一个 img_window 控件 */
        auto * video_image = img_window->add<VideoView>(mJsonValue.devices[0].camera_url, 0);
        video_image->set_fixed_size(Vector2i(520, 286));
        video_image->set_position(Vector2i(20, 20));

        auto *label = img_window->add<Label>("2");
        label->set_font("sans-bold");
        label->set_position(Vector2i(568, 34));

        auto *btn = img_window->add<Button>("", RED_LED3000_ASSETS_DIR"/focal_small.png");
        btn->set_fixed_size({64, 48});
        btn->set_position({544, 72});
        btn->set_callback([this]() {
            this->getDeviceQueue(0).put(PolyM::DataMsg<std::string>(POLYM_FOCAL_SETTING, "3"));
        });
        btn = img_window->add<Button>("", RED_LED3000_ASSETS_DIR"/focal_big.png");
        btn->set_fixed_size({64, 48});
        btn->set_position({544, 122});
        btn->set_callback([this]() {
            this->getDeviceQueue(0).put(PolyM::DataMsg<std::string>(POLYM_FOCAL_SETTING, "2"));
        });

        btn = img_window->add<Button>("", RED_LED3000_ASSETS_DIR"/focal_revert.png");
        btn->set_fixed_size({64, 48});
        btn->set_position({544, 172});
        btn->set_callback([this]() {
            this->getDeviceQueue(0).put(PolyM::DataMsg<std::string>(POLYM_FOCAL_SETTING, "4"));
        });

        btn = img_window->add<Button>("", RED_LED3000_ASSETS_DIR"/dec_focal.png");
#if 0
        btn->set_callback([this]() {
            this->getDeviceQueue(0).put(PolyM::DataMsg<std::string>(POLYM_FOCAL_SETTING, "-"));
        });
#endif
        btn->set_change_callback([this](bool push) {
            RedDebug::err("debug focal status=%d", push == true ? 1 : 0);

            if (push == true)
                this->getDeviceQueue(0).put(PolyM::DataMsg<std::string>(POLYM_FOCAL_SETTING, "D"));
            else
                this->getDeviceQueue(0).put(PolyM::DataMsg<std::string>(POLYM_FOCAL_SETTING, "S"));
        });
        btn->set_fixed_size({64, 48});
        btn->set_position({544, 222});

        btn = img_window->add<Button>("", RED_LED3000_ASSETS_DIR"/inc_focal.png");
        btn->set_fixed_size({64, 48});
        btn->set_position({544, 272});
#if 0
        btn->set_callback([this]() {
            this->getDeviceQueue(0).put(PolyM::DataMsg<std::string>(POLYM_FOCAL_SETTING, "+"));
        });
#else
        btn->set_change_callback([this](bool push) {
            RedDebug::err("debug focal status=%d", push == true ? 1 : 0);

            if (push == true)
                this->getDeviceQueue(0).put(PolyM::DataMsg<std::string>(POLYM_FOCAL_SETTING, "I"));
            else
                this->getDeviceQueue(0).put(PolyM::DataMsg<std::string>(POLYM_FOCAL_SETTING, "S"));
        });
#endif

        auto* img2_window = new Window(this, "");
        img2_window->set_fixed_size({610, 326});
        img2_window->set_position(Vector2i(650, 98));
        img2_window->set_background_image(RED_LED3000_ASSETS_DIR"/video.png");

        /* 在这个 window 上创建一个 img2_window 控件 */
        video_image = img2_window->add<VideoView>(mJsonValue.devices[1].camera_url, 1);
        /* 固定大小为 520 * 286 */
        video_image->set_fixed_size(Vector2i(520, 286));
        video_image->set_position(Vector2i(20, 20));

        auto * track_video_btn2 = img2_window->add<Button>("");
        track_video_btn2->set_pseudo(true);
        track_video_btn2->set_fixed_size(Vector2i(30, 72));
        track_video_btn2->set_position(Vector2i(560, 20));
        track_video_btn2->set_callback([&] {
            if (getJsonValue()->devices[1].turntable.mode <= TURNTABLE_FUZZY_TRACK_MODE)
            {
                if (this->getJsonValue()->devices[1].turntable.mode == TURNTABLE_TRACK_MODE) {

                    this->getJsonValue()->devices[1].turntable.mode = TURNTABLE_FUZZY_TRACK_MODE;
                    this->getDeviceQueue(1).put(PolyM::DataMsg<std::string>(POLYM_TURNTABLE_MODE_SETTING, to_string(TURNTABLE_FUZZY_TRACK_MODE)));
                    red_debug_lite("switch fuzzy track mode");
                } else {
                    this->getJsonValue()->devices[1].turntable.mode = TURNTABLE_TRACK_MODE;
                    this->getDeviceQueue(1).put(PolyM::DataMsg<std::string>(POLYM_TURNTABLE_MODE_SETTING, to_string(TURNTABLE_TRACK_MODE)));
                    red_debug_lite("switch target track mode");
                }
            }
        }
        );

        label = img2_window->add<Label>("1");
        label->set_font("sans-bold");
        label->set_position(Vector2i(568, 34));

        btn = img2_window->add<Button>("", RED_LED3000_ASSETS_DIR"/focal_small.png");
        btn->set_fixed_size({64, 48});
        btn->set_position({544, 72});
        btn->set_callback([this]() {
            this->getDeviceQueue(1).put(PolyM::DataMsg<std::string>(POLYM_FOCAL_SETTING, "3"));
        });

        btn = img2_window->add<Button>("", RED_LED3000_ASSETS_DIR"/focal_big.png");
        btn->set_fixed_size({64, 48});
        btn->set_position({544, 122});
        btn->set_callback([this]() {
            this->getDeviceQueue(1).put(PolyM::DataMsg<std::string>(POLYM_FOCAL_SETTING, "2"));
        });

        btn = img2_window->add<Button>("", RED_LED3000_ASSETS_DIR"/focal_revert.png");
        btn->set_fixed_size({64, 48});
        btn->set_position({544, 172});
        btn->set_callback([this]() {
            this->getDeviceQueue(1).put(PolyM::DataMsg<std::string>(POLYM_FOCAL_SETTING, "4"));
        });

        btn = img2_window->add<Button>("", RED_LED3000_ASSETS_DIR"/dec_focal.png", 0);
        btn->set_fixed_size({64, 48});
        btn->set_position({544, 222});
#if 0
        btn->set_callback([this]() {
            this->getDeviceQueue(1).put(PolyM::DataMsg<std::string>(POLYM_FOCAL_SETTING, "-"));
        });
#endif
        btn->set_change_callback([this](bool push) {
            RedDebug::err("debug focal status=%d", push == true ? 1 : 0);

            if (push == true)
                this->getDeviceQueue(1).put(PolyM::DataMsg<std::string>(POLYM_FOCAL_SETTING, "D"));
            else
                this->getDeviceQueue(1).put(PolyM::DataMsg<std::string>(POLYM_FOCAL_SETTING, "S"));
        });

        btn = img2_window->add<Button>("", RED_LED3000_ASSETS_DIR"/inc_focal.png", 0);
        btn->set_fixed_size({64, 48});
        btn->set_position({544, 272});
        btn->set_change_callback([this](bool push) {
            RedDebug::err("debug focal status=%d", push == true ? 1 : 0);

            if (push == true)
                this->getDeviceQueue(1).put(PolyM::DataMsg<std::string>(POLYM_FOCAL_SETTING, "I"));
            else
                this->getDeviceQueue(1).put(PolyM::DataMsg<std::string>(POLYM_FOCAL_SETTING, "S"));
        });

    }

    /* 确定每一个部件的大小
     * 这个函数使用 layout 布局来计算每一个控件的大小的位置, 一般地这个函数只会执行一次，所以新
     * 弹出来的窗口，需要主动执行 perform_layout() 重新计算新弹出来的窗口上的部件的位置和大小信息
     * */
    perform_layout();

    /* All NanoGUI widgets are initialized at this point. Now
       create shaders to draw the main window contents.

       NanoGUI comes with a simple wrapper around OpenGL 3, which
       eliminates most of the tedious and error-prone shader and buffer
       object management.
    */

    m_render_pass = new RenderPass({ this });
    m_render_pass->set_clear_color(0, Color(0.3f, 0.3f, 0.32f, 1.f));

    /* 创建一个 shader, 着色器 */
    m_shader = new Shader(
        m_render_pass,

        /* An identifying name */
        "a_simple_shader",

#if defined(NANOGUI_USE_OPENGL)
        R"(/* Vertex shader */
            #version 330
            uniform mat4 mvp;
            in vec3 position;
            void main() {
                gl_Position = mvp * vec4(position, 1.0);
            })",

        /* Fragment shader */
        R"(#version 330
            out vec4 color;
            uniform float intensity;
            void main() {
                color = vec4(vec3(intensity), 1.0);
            })"
#elif defined(NANOGUI_USE_GLES)
        R"(/* Vertex shader */
            precision highp float;
            uniform mat4 mvp;
            attribute vec3 position;
            void main() {
                gl_Position = mvp * vec4(position, 1.0);
            })",

        /* Fragment shader */
        R"(precision highp float;
            uniform float intensity;
            void main() {
                gl_FragColor = vec4(vec3(intensity), 1.0);
            })"
#elif defined(NANOGUI_USE_METAL)
        R"(using namespace metal;
            struct VertexOut {
                float4 position [[position]];
            };

            vertex VertexOut vertex_main(const device packed_float3 *position,
                                         constant float4x4 &mvp,
                                         uint id [[vertex_id]]) {
                VertexOut vert;
                vert.position = mvp * float4(position[id], 1.f);
                return vert;
            })",

        /* Fragment shader */
        R"(using namespace metal;
            fragment float4 fragment_main(const constant float &intensity) {
                return float4(intensity);
            })"
#endif
    );

    uint32_t indices[3 * 2] = {
        0, 1, 2,
        2, 3, 0
    };

    float positions[3 * 4] = {
        -1.f, -1.f, 0.f,
            1.f, -1.f, 0.f,
            1.f, 1.f, 0.f,
            -1.f, 1.f, 0.f
        };

    m_shader->set_buffer("indices", VariableType::UInt32, {3 * 2}, indices);
    m_shader->set_buffer("position", VariableType::Float32, {4, 3}, positions);
    m_shader->set_uniform("intensity", 0.5f);
}

void Led3000Window::update_time4display(void)
{
    struct timeval tv = {0};
    char current_time[64] = {0};
    if (0 == gettimeofday(&tv, NULL))
    {
        if (ctime_r(&tv.tv_sec, current_time))
            m_time4dispaly->set_caption(&current_time[11]);
    }
}

void Led3000Window::update_attitudeinfo4display(float direction_float, float vertical_float, float horizon_float, bool valid)
{
    if (!m_attitude_info)
        return;

    if (valid == true)
    {
        /* 为了保持两位有效小数位 */
        m_attitude_info->set_caption(
            "艏向角(°):"
            + std::to_string(FLOAT_KEEP_PRECISON2(direction_float)).erase(to_string(direction_float).find('.')+3, string::npos)
            + ' '
            + "横摇角(°):"
            + std::to_string(FLOAT_KEEP_PRECISON2(horizon_float)).erase(to_string(horizon_float).find('.')+3, string::npos)
            + ' '
            + "纵摇角(°):"
            + std::to_string(FLOAT_KEEP_PRECISON2(vertical_float)).erase(to_string(vertical_float).find('.')+3, string::npos)
            );
    }
    else
    {
        m_attitude_info->set_caption("艏向角(°):------- 横摇角(°):------- 纵摇角(°):------ ");
    }
}

