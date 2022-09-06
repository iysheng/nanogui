/******************************************************************************
* File:             led3000gui.cpp
*
* Author:           yangyongsheng@jari.cn
* Created:          08/10/22
* Description:      led3000 gui source file
*****************************************************************************/

#include <led3000gui.h>
#include <stdlib.h>
#include <string>

using namespace nanogui;
using std::cout;
using std::endl;

#define LED3000_VERSION    9u
#define LED3000_ID         11000001u

void hexdump(unsigned char * addr, int len)
{
    int i = 0, g = len / 8;

    for (i = 0; i < g ; i++)
        printf("%x %x %x %x %x %x %x %x\n", addr[i<<3], addr[i<<3 + 1], addr[i<<3 + 2], addr[i<<3 + 3], addr[i<<3 + 4], addr[i<<3 + 5], addr[i<<3 + 6], addr[i<<3 + 7]);
    for (i = 0; i < len % 8; i++)
        printf("%x ", addr[i<<3]);
    printf("\n");
}

void do_with_sysconfig(Widget *widget, int choose)
{
  std::cout << "do with sysconfig :" << choose << std::endl;
  if (choose != 2)
  {
      Led3000Window * led3000Window = dynamic_cast<Led3000Window *>(widget->window()->parent());
      /* "json"  消息表示更新配置文件 */
      led3000Window->getJsonQueue().put(PolyM::DataMsg<std::string>(choose == 1 ? POLYM_BUTTON_CONFIRM : POLYM_BUTTON_CANCEL, "json"));
  }
  else
  {
      Led3000Window * led3000Window = dynamic_cast<Led3000Window *>(widget->window()->parent());
      Window * setWindow = new Window(widget->window()->parent(), "系统参数配置");
      Window * sysWindow = widget->window();
      /* 标记 sysconfig winow 为 no Modal, 并修改 setWindow 为 Modal,setWindow 会提前到最前面图层 */
      sysWindow->set_modal(false);
      setWindow->set_modal(true);
      setWindow->set_layout(new BoxLayout(Orientation::Vertical,
                              Alignment::Middle, 0, 15));

      Widget * ethsWidget = setWindow->add<Widget>();
      ethsWidget->set_layout(new BoxLayout(Orientation::Horizontal,
                              Alignment::Middle, 0, 15));

      Widget * configWidget = ethsWidget->add<Widget>();
      GridLayout * layout = new GridLayout(Orientation::Horizontal, 2,
                                     Alignment::Middle, 15, 5);
      layout->set_col_alignment({ Alignment::Maximum, Alignment::Fill });
      layout->set_spacing(0, 10);
      /* 定义了这个窗口的布局 */
      configWidget->set_layout(layout);
      configWidget->add<Label>("网卡0IP:", "sans-bold");
      /* 创建 textBox */
      auto* textBox = configWidget->add<TextBox>("",KeyboardType::NumberIP);
      textBox->set_editable(true);
      /* 设置控件大小 */
      textBox->set_fixed_size(Vector2i(150, 20));
      textBox->set_value(led3000Window->getJsonValue()->eths[0].ip);
      textBox->setSyncCharsValue(led3000Window->getJsonValue()->eths[0].ip);
      /* 设置字体大小 */
      textBox->set_font_size(16);
      //textBox->setFormat("[-]?[0-9]*\\.?[0-9]+");
      textBox->set_alignment(TextBox::Alignment::Left);
      configWidget->add<Label>("网卡0子网掩码:", "sans-bold");
      textBox = configWidget->add<TextBox>("",KeyboardType::NumberIP);
      textBox->set_editable(true);
      textBox->set_fixed_size(Vector2i(150, 20));
      textBox->set_value(led3000Window->getJsonValue()->eths[0].netmask);
      textBox->setSyncCharsValue(led3000Window->getJsonValue()->eths[0].netmask);
      textBox->set_font_size(16);
      textBox->set_alignment(TextBox::Alignment::Left);
      configWidget->add<Label>("网卡0网关:", "sans-bold");
      textBox = configWidget->add<TextBox>("",KeyboardType::NumberIP);
      textBox->set_editable(true);
      textBox->set_fixed_size(Vector2i(150, 20));
      textBox->set_value(led3000Window->getJsonValue()->eths[0].gateway);
      textBox->setSyncCharsValue(led3000Window->getJsonValue()->eths[0].gateway);
      textBox->set_font_size(16);
      textBox->set_alignment(TextBox::Alignment::Left);

      Widget * eth1Widget = ethsWidget->add<Widget>();
      layout = new GridLayout(Orientation::Horizontal, 2,
                                     Alignment::Middle, 15, 5);
      layout->set_col_alignment({Alignment::Maximum, Alignment::Fill });
      layout->set_spacing(0, 10);
      /* 定义了这个窗口的布局 */
      eth1Widget->set_layout(layout);
      eth1Widget->add<Label>("网卡1IP:", "sans-bold");
      textBox = eth1Widget->add<TextBox>("",KeyboardType::NumberIP);
      textBox->set_editable(true);
      textBox->set_fixed_size(Vector2i(150, 20));
      textBox->set_value(led3000Window->getJsonValue()->eths[1].ip);
      textBox->setSyncCharsValue(led3000Window->getJsonValue()->eths[1].ip);
      textBox->set_font_size(16);
      textBox->set_alignment(TextBox::Alignment::Left);
      eth1Widget->add<Label>("网卡1子网掩码:", "sans-bold");
      textBox = eth1Widget->add<TextBox>("",KeyboardType::NumberIP);
      textBox->set_editable(true);
      textBox->set_fixed_size(Vector2i(150, 20));
      textBox->set_value(led3000Window->getJsonValue()->eths[1].netmask);
      textBox->setSyncCharsValue(led3000Window->getJsonValue()->eths[1].netmask);
      textBox->set_font_size(16);
      textBox->set_alignment(TextBox::Alignment::Left);
      eth1Widget->add<Label>("网卡1网关:", "sans-bold");
      textBox = eth1Widget->add<TextBox>("",KeyboardType::NumberIP);
      textBox->set_editable(true);
      textBox->set_fixed_size(Vector2i(150, 20));
      textBox->set_value(led3000Window->getJsonValue()->eths[1].gateway);
      textBox->setSyncCharsValue(led3000Window->getJsonValue()->eths[1].gateway);
      textBox->set_font_size(16);
      textBox->set_alignment(TextBox::Alignment::Left);

      Widget *serverWidget = setWindow->add<Widget>();
      serverWidget->set_layout(new BoxLayout(Orientation::Horizontal,
                                      Alignment::Middle, 0, 15));
      Widget *serveripWidget = serverWidget->add<Widget>();
      serveripWidget->set_layout(layout);
      serveripWidget->add<Label>("服务器IP:", "sans-bold");
      textBox = serveripWidget->add<TextBox>("", KeyboardType::NumberIP);
      textBox->set_editable(true);
      textBox->set_fixed_size(Vector2i(150, 20));
      textBox->set_value(led3000Window->getJsonValue()->server.ip);
      textBox->setSyncCharsValue(led3000Window->getJsonValue()->server.ip);
      textBox->set_font_size(16);
      textBox->set_alignment(TextBox::Alignment::Left);

      Widget *serverPortWidget = serverWidget->add<Widget>();
      serverPortWidget->set_layout(layout);
      serverPortWidget->add<Label>("服务器端口:", "sans-bold");
      textBox = serverPortWidget->add<TextBox>();
      textBox->set_editable(true);
      textBox->set_fixed_size(Vector2i(150, 20));
      textBox->set_value(std::to_string(led3000Window->getJsonValue()->server.port));
      textBox->setSyncUshortValue(&(led3000Window->getJsonValue()->server.port));
      textBox->set_font_size(16);
      textBox->set_alignment(TextBox::Alignment::Left);

      Widget *cameraWidget = setWindow->add<Widget>();
      cameraWidget->set_layout(new BoxLayout(Orientation::Horizontal,
                                      Alignment::Middle, 0, 15));
      cameraWidget->add<Label>("摄像头1路径:", "sans-bold");
      textBox = cameraWidget->add<TextBox>("", KeyboardType::Full);
      textBox->set_editable(true);
      textBox->set_fixed_size(Vector2i(350, 20));
      textBox->set_value(led3000Window->getJsonValue()->devices[0].camera_url);
      textBox->setSyncCharsValue(&(led3000Window->getJsonValue()->devices[0].camera_url[0]));
      textBox->set_font_size(16);
      textBox->set_alignment(TextBox::Alignment::Left);

      cameraWidget = setWindow->add<Widget>();
      cameraWidget->set_layout(new BoxLayout(Orientation::Horizontal,
                                      Alignment::Middle, 0, 15));
      cameraWidget->add<Label>("摄像头2路径:", "sans-bold");
      textBox = cameraWidget->add<TextBox>("", KeyboardType::Full);
      textBox->set_editable(true);
      textBox->set_fixed_size(Vector2i(350, 20));
      textBox->set_value(led3000Window->getJsonValue()->devices[1].camera_url);
      textBox->setSyncCharsValue(&(led3000Window->getJsonValue()->devices[1].camera_url[0]));
      textBox->set_font_size(16);
      textBox->set_alignment(TextBox::Alignment::Left);

      Widget *btn_widget = setWindow->add<Widget>();
      btn_widget->set_layout(new BoxLayout(Orientation::Horizontal,
                                      Alignment::Middle, 0, 150));
      btn_widget->add<Button>("返回")->set_callback([btn_widget](){
          Led3000Window * led3000Window = dynamic_cast<Led3000Window *>(btn_widget->window()->parent());
          btn_widget->window()->dispose();
          led3000Window->getJsonQueue().put(PolyM::DataMsg<std::string>(POLYM_BUTTON_CANCEL, "config"));
          std::cout << "返回" << std::endl;
      });
      btn_widget->add<Button>("确认")->set_callback([btn_widget](){
          Led3000Window * led3000Window = dynamic_cast<Led3000Window *>(btn_widget->window()->parent());
          /* syncto json config */
          btn_widget->window()->dispose();
          led3000Window->getJsonQueue().put(PolyM::DataMsg<std::string>(POLYM_BUTTON_CONFIRM, "config"));
          std::cout << "确认" << std::endl;
      });
      setWindow->center();
      setWindow->request_focus();
  }
}

void do_with_power_off(Widget *widget, int choose)
{
  std::cout << "do with power off :" << choose << std::endl;
  if (choose == 1)
  {
      /* TODO 发送关机消息到一体化网络 */

      /* do power off */
      system("poweroff");
  }
}

void do_with_green_light_normal(Widget *widget, int choose)
{
  std::cout << "green light normal:" << choose << std::endl;
  if (choose != 2)
  {
    /* TODO change green light status */
  }
}

void do_with_green_light_mocode(Widget *widget, int choose)
{
  std::cout << "mocode light normal:" << choose << std::endl;
  if (choose == 1)
  {
    Led3000Window * led3000Window = dynamic_cast<Led3000Window *>(widget->window()->parent());
    led3000Window->getCurrentDeviceQueue().put(PolyM::DataMsg<std::string>(POLYM_GREEN_MOCODE_SETTING, led3000Window->getJsonValue()->devices[led3000Window->getCurrentDevice()].green_led.mocode));
    return;
  }
  else if (choose == 2)
  {
      Led3000Window * led3000Window = dynamic_cast<Led3000Window *>(widget->window()->parent());
      Window * setWindow = new Window(widget->window()->parent(), "莫码参数配置");
      setWindow->set_background_image("/tmp/abc/red0.jpg");
      Window * mocodeWindow = widget->window();
      /* 标记为 modal winow, 该 window 会提前到最前面图层 */
      setWindow->set_modal(true);
      setWindow->set_layout(new BoxLayout(Orientation::Vertical,
                              Alignment::Middle, 0, 15));

      Widget * configWidget = setWindow->add<Widget>();
      GridLayout * layout = new GridLayout(Orientation::Horizontal, 2,
                                     Alignment::Middle, 15, 5);
      layout->set_col_alignment({ Alignment::Maximum, Alignment::Fill });
      layout->set_spacing(0, 10);
      /* 定义了这个窗口的布局 */
      configWidget->set_layout(layout);
      /* 创建 textBox */
      configWidget->add<Label>("莫码信息:", "sans-bold");
      auto* textBox = configWidget->add<TextBox>("", KeyboardType::Full);
      textBox->set_editable(true);
      textBox->set_fixed_size(Vector2i(150, 20));
      textBox->set_value(led3000Window->getJsonValue()->devices[led3000Window->getCurrentDevice()].green_led.mocode);
      textBox->setSyncCharsValue(led3000Window->getJsonValue()->devices[led3000Window->getCurrentDevice()].green_led.mocode);
      textBox->set_font_size(16);
      textBox->set_alignment(TextBox::Alignment::Left);

      Widget *btWidget = setWindow->add<Widget>();
      btWidget->set_layout(new BoxLayout(Orientation::Horizontal,
                                      Alignment::Middle, 0, 15));
      btWidget->add<Button>("返回")->set_callback([textBox, setWindow, mocodeWindow](){
          textBox->keyboard_window()->dispose();
          setWindow->dispose();
          //mocodeWindow->set_modal(true);
          std::cout << "返回" << std::endl;
      });
      btWidget->add<Button>("确认")->set_callback([textBox, setWindow, mocodeWindow](){
          Led3000Window * led3000Window = dynamic_cast<Led3000Window *>(mocodeWindow->screen());
          textBox->keyboard_window()->dispose();
          setWindow->dispose();
          //mocodeWindow->set_modal(true);
          led3000Window->getJsonQueue().put(PolyM::DataMsg<std::string>(POLYM_BUTTON_CONFIRM, "json"));
          led3000Window->get_dev_morse_code_label(led3000Window->getCurrentDevice())->set_caption_merge(led3000Window->getJsonValue()->devices[led3000Window->getCurrentDevice()].green_led.mocode, led3000Window->getJsonValue()->devices[led3000Window->getCurrentDevice()].white_led.mocode, '/');
          std::cout << "确认" << std::endl;
      });
      setWindow->center();
      setWindow->request_focus();
  }
}

Led3000Window::Led3000Window():Screen(Vector2i(1280, 800), "NanoGUI Test", false, true),
        mFileName("/opt/led3000.json"), mFp(nullptr)
      {
          /* 系统配置参数初始化 */
        {
            memcpy(mjsonBuffer, "{}", strlen("{}") + 1);
            mFp = fopen(mFileName.c_str(), "r");
            if (mFp)
            {
              fread(mjsonBuffer, sizeof(char), sizeof(mjsonBuffer), mFp);
              cout << "mjsonBuffer:" << mjsonBuffer << "@" << mFileName << endl;
              if (mDocument.ParseInsitu(mjsonBuffer).HasParseError())
              {
                std::cout << "Invalid content of " << mFileName << std::endl;
                return;
              }
            }
            else if (!mFp)
            {
                mFp = fopen(mFileName.c_str(), "w+");
                assert(mFp);
                //fwrite(mjsonBuffer, sizeof(char), strlen(mjsonBuffer) + 1, mFp);
                assert(false == mDocument.ParseInsitu(mjsonBuffer).HasParseError());
            }

            Document::AllocatorType& allocator = mDocument.GetAllocator();

            if (mDocument.FindMember("sys_config") == mDocument.MemberEnd())
            {
                Value sys_config(kObjectType);
                sys_config.AddMember("version", LED3000_VERSION, allocator);
                sys_config.AddMember("id", LED3000_ID, allocator);
                mDocument.AddMember("sys_config", sys_config, allocator);
                cout << "Add sys_config object" << endl;
            }
            else
            {
                mJsonValue.sys_config.version = Pointer("/sys_config/version").Get(mDocument)->GetUint();
                mJsonValue.sys_config.id = Pointer("/sys_config/id").Get(mDocument)->GetUint();
            }

            if (mDocument.MemberEnd() == mDocument.FindMember("eths"))
            {
                Value eth_array(kArrayType);
                Value eth0_config(kObjectType);
                Value eth1_config(kObjectType);

                eth0_config.AddMember("name", "引导", allocator);
                eth0_config.AddMember("ip", "10.20.52.110", allocator);
                eth0_config.AddMember("netmask", "255.255.255.0", allocator);
                eth0_config.AddMember("gateway", "10.20.52.1", allocator);

                eth1_config.AddMember("name", "摄像头", allocator);
                eth1_config.AddMember("ip", "192.168.100.110", allocator);
                eth1_config.AddMember("netmask", "255.255.255.0", allocator);
                eth1_config.AddMember("gateway", "192.168.100.1", allocator);

                eth_array.PushBack(eth0_config, allocator);
                eth_array.PushBack(eth1_config, allocator);

                mDocument.AddMember("eths", eth_array, allocator);
                cout << "Add eths array" << endl;
            }
            else
            {
              const Value& eths_info = mDocument["eths"];
              for (SizeType i = 0; i < eths_info.Size(); i++)
              {
                memcpy(mJsonValue.eths[i].name, eths_info[i]["name"].GetString(), strlen(eths_info[i]["name"].GetString()) + 1);
                memcpy(mJsonValue.eths[i].ip, eths_info[i]["ip"].GetString(), strlen(eths_info[i]["ip"].GetString()) + 1);
                memcpy(mJsonValue.eths[i].netmask, eths_info[i]["netmask"].GetString(), strlen(eths_info[i]["netmask"].GetString()) + 1);
                memcpy(mJsonValue.eths[i].gateway, eths_info[i]["gateway"].GetString(), strlen(eths_info[i]["gateway"].GetString()) + 1);
              }
            }

            if (mDocument.MemberEnd() == mDocument.FindMember("server"))
            {
                Value server_config(kObjectType);

                server_config.AddMember("ip", "10.20.52.110", allocator);
                server_config.AddMember("port", 5000, allocator);

                mDocument.AddMember("server", server_config, allocator);
                cout << "Add sever info" << endl;
            }
            else
            {
              const Value& server = mDocument["server"];
              memcpy(mJsonValue.server.ip, server["ip"].GetString(), strlen(server["ip"].GetString()) + 1);
              mJsonValue.server.port = server["port"].GetUint();
            }

            if (mDocument.MemberEnd() == mDocument.FindMember("devices"))
            {
                Value device_array(kArrayType);
                Value device_config_template(kObjectType);
                Value white_led_config(kObjectType);
                Value green_led_config(kObjectType);
                Value turntable_config(kObjectType);

                device_config_template.AddMember("camera_url", "192.168.100.64", allocator);
                white_led_config.AddMember("mode", 1, allocator);
                white_led_config.AddMember("normal_status", 0, allocator);
                white_led_config.AddMember("blink_freq", 1, allocator);
                white_led_config.AddMember("mocode", "", allocator);

                green_led_config.AddMember("auth", 1, allocator);
                green_led_config.AddMember("mode", 1, allocator);
                green_led_config.AddMember("normal_status", 0, allocator);
                green_led_config.AddMember("blink_freq", 1, allocator);
                green_led_config.AddMember("mocode", "", allocator);

                turntable_config.AddMember("mode", 1, allocator);
                turntable_config.AddMember("target_pos_x", 0, allocator);
                turntable_config.AddMember("target_pos_y", 0, allocator);

                device_config_template.AddMember("white_led", white_led_config, allocator);
                device_config_template.AddMember("green_led", green_led_config, allocator);
                device_config_template.AddMember("turntable", turntable_config, allocator);
                Value device_config;

                for (int i = 0; i < 2; i++)
                {
                    /* deep copy */
                    device_config.CopyFrom(device_config_template, allocator);
                    device_array.PushBack(device_config, allocator);
                }

                mDocument.AddMember("devices", device_array, allocator);
                cout << "Add devices array" << endl;
            }
            else
            {
              const Value& devices = mDocument["devices"];
              for (SizeType i = 0; i < devices.Size(); i++)
              {
                memcpy(mJsonValue.devices[i].camera_url, devices[i]["camera_url"].GetString(), strlen(devices[i]["camera_url"].GetString()) + 1);
                mJsonValue.devices[i].white_led.mode = (unsigned char)devices[i]["white_led"]["mode"].GetUint();
                mJsonValue.devices[i].white_led.normal_status = (unsigned char)devices[i]["white_led"]["normal_status"].GetUint();
                mJsonValue.devices[i].white_led.blink_freq = (unsigned char)devices[i]["white_led"]["blink_freq"].GetUint();
                memcpy(mJsonValue.devices[i].white_led.mocode, devices[i]["white_led"]["mocode"].GetString(), strlen(devices[i]["white_led"]["mocode"].GetString()) + 1);

                mJsonValue.devices[i].green_led.auth = (unsigned char)devices[i]["green_led"]["auth"].GetUint();
                mJsonValue.devices[i].green_led.mode = (unsigned char)devices[i]["green_led"]["mode"].GetUint();
                mJsonValue.devices[i].green_led.normal_status = (unsigned char)devices[i]["green_led"]["normal_status"].GetUint();
                mJsonValue.devices[i].green_led.blink_freq = (unsigned char)devices[i]["green_led"]["blink_freq"].GetUint();
                memcpy(mJsonValue.devices[i].green_led.mocode, devices[i]["green_led"]["mocode"].GetString(), strlen(devices[i]["green_led"]["mocode"].GetString()) + 1);

                mJsonValue.devices[i].turntable.mode = (unsigned char)devices[i]["turntable"]["mode"].GetUint();
                mJsonValue.devices[i].turntable.target_pos_x = (unsigned short)devices[i]["turntable"]["target_pos_x"].GetUint();
                mJsonValue.devices[i].turntable.target_pos_y = (unsigned short)devices[i]["turntable"]["target_pos_y"].GetUint();
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
          auto* swindow = new Window(this, "设备状态");
          /////swindow->set_id("sWindow");
          printf("swindow addr=%p\n", &swindow);
          swindow->set_background_image("/tmp/abc/red0.jpg");
          /* 确定了 swindow 的位置 */
          swindow->set_position({0, 550});
          /* 创建一个新的布局 */
          auto* layout = new GridLayout(Orientation::Horizontal, 6,
                                         Alignment::Minimum, 15, 5);
          layout->set_col_alignment({ Alignment::Maximum, Alignment::Fill });
          layout->set_spacing(0, 10);
          /* 定义了这个窗口的布局 */
          swindow->set_layout(layout);

          /* 调用 add 函数模板
           * 将新创建的 label 控件关联到 swindow 作为其 parent
           * */
          new Label(swindow, "        ", "sans-bold");
          new Label(swindow, "设备状态", "sans-bold");
          new Label(swindow, "水平/垂直角度(swindow, 糖)", "sans-bold");
          new Label(swindow, "水平/垂直速度(swindow, 糖/s)", "sans-bold");
          new Label(swindow, "莫码信息", "sans-bold");
          new Label(swindow, "绿灯状态", "sans-bold");

          new Label(swindow, "灯光装置终端一", "sans-bold");
          m_dev_state[0] = new Label(swindow, "在线", "sans");
          m_dev_angle[0] = new Label(swindow, "120/10", "sans");
          m_dev_angular_speed[0] = new Label(swindow, "20/10", "sans");
          m_dev_morse_code[0] = new Label(swindow, "", "sans");
          m_dev_morse_code[0]->set_caption_merge(mJsonValue.devices[0].green_led.mocode, mJsonValue.devices[0].white_led.mocode, '/');
          m_dev_auth[0] = new Label(swindow, "", "sans");
          m_dev_auth[0]->set_caption_merge((mJsonValue.devices[0].green_led.auth ? "已" : "未"), "授权", '\0');

          new Label(swindow, "灯光装置终端二", "sans-bold");
          m_dev_state[1] = new Label(swindow, "离线", "sans");
          m_dev_angle[1] = new Label(swindow, "-/-", "sans");
          m_dev_angular_speed[1] = new Label(swindow, "-/-", "sans");
          m_dev_morse_code[1] = new Label(swindow, "", "sans");
          m_dev_morse_code[1]->set_caption_merge(mJsonValue.devices[1].green_led.mocode, mJsonValue.devices[1].white_led.mocode, '/');
          m_dev_auth[1] = new Label(swindow, "", "sans");
          m_dev_auth[1]->set_caption_merge((mJsonValue.devices[1].green_led.auth ? "已" : "未"), "授权", '\0');
        }

        /* 设备控制窗口 */
        {
          auto* cwindow = new Window(this, "灯光功能 ࿒ ࿓ ࿔");
          cwindow->set_background_image("/tmp/abc/red.png");

          /* 确定了 cwindow 的位置 */
          cwindow->set_position({0, 670});
          /* 创建一个新的布局 */
          GridLayout * layout = new GridLayout(Orientation::Horizontal, 4,
                                         Alignment::Middle, 15, 5);
          layout->set_col_alignment({ Alignment::Maximum, Alignment::Fill });
          layout->set_spacing(0, 10);
          /* 定义了这个窗口的布局 */
          cwindow->set_layout(layout);
          m_green_dev = new Label(cwindow, "绿灯一", "sans-bold");
          Button *test_btn = new Button(cwindow, "关闭");
          test_btn->set_callback([&] {
              new MessageDialog(this, MessageDialog::Type::Question, "绿灯控制", "确认要打开绿光么?", "确认", "取消", do_with_green_light_normal); });
          new Button(cwindow, "绿闪");
          /* 这里会弹出来新的 MessageDialog, 新的 MessageDialog 支持弹出新的 Window
           * 测试发现这个 MessageDialog Widget 的 parent 竟然是 Led3000Window * ？？？
           * */
          test_btn = new Button(cwindow, "莫码");
          test_btn->set_callback([&] {
              MessageDialog *dlg = new MessageDialog(this, MessageDialog::Type::Choose, "莫码发送设置", "准备发送莫码?", "确认", "配置", "取消", true);
              dlg->set_widget_callback(do_with_green_light_mocode); });

          m_white_dev = new Label(cwindow, "白灯一", "sans-bold");
          new Button(cwindow, "常亮");
          new Button(cwindow, "白闪");
          new Button(cwindow, "莫码");
        }

        /* 系统窗口 */
        {
          Button * sysconfig_btn;
          auto* swindow = new Window(this, "系统功能");

          /* 确定了 swindow 的位置 */
          swindow->set_position({980, 0});
          /* 创建一个新的布局 */
          GridLayout * layout = new GridLayout(Orientation::Horizontal, 1,
                                         Alignment::Middle, 5, 5);
          layout->set_col_alignment({ Alignment::Fill, Alignment::Fill });
          //layout->set_spacing(0, 5);
          /* 定义了这个窗口的布局 */
          swindow->set_layout(layout);
          sysconfig_btn = swindow->add<Button>("关机⏻ ");
          sysconfig_btn->set_callback([&] {
              new MessageDialog(this, MessageDialog::Type::Question, "关机", "确认要关机么?", "确认", "取消", do_with_power_off); });
          sysconfig_btn = swindow->add<Button>("系统设置");
          sysconfig_btn->set_callback([&] {
              new MessageDialog(this, MessageDialog::Type::Choose, "系统参数设置", "准备配置参数", "参数配置", do_with_sysconfig);});
        }

        /* 设备选择 */
        {
          auto* chooseWindow = new Window(this, "设备选择");

          chooseWindow->set_background_image("/tmp/abc/dev.png");
          /* 确定了chooseWindow 的位置 */
          chooseWindow->set_position({980, 150});
          /* 创建一个新的布局 */
          GridLayout * layout = new GridLayout(Orientation::Horizontal, 1,
                                         Alignment::Middle, 5, 5);
          layout->set_col_alignment({ Alignment::Fill, Alignment::Fill });
          /* 定义了这个窗口的布局 */
          chooseWindow->set_layout(layout);
          Button *devBtn = chooseWindow->add<Button>("灯光装置终端一");
          devBtn->set_callback([this]() {
            cout << "choose device 1" << endl;
            this->get_green_dev_label()->set_caption("绿灯一");
            this->get_white_dev_label()->set_caption("白灯一");
            this->setCurrentDevice(0);
          });
          /* 根据实际系统功能中按键大小，为了保持大小一致，修改设备选择按键大小保持一致 */
          ///devBtn->set_fixed_size(Vector2i(165, 30));
          devBtn = chooseWindow->add<Button>("灯光装置终端二");
          devBtn->set_callback([this]() {
            cout << "choose device 2" << endl;
            this->get_green_dev_label()->set_caption("绿灯二");
            this->get_white_dev_label()->set_caption("白灯二");
            this->setCurrentDevice(1);
          });
          ///devBtn->set_fixed_size(Vector2i(165, 30));
        }

        /* 转台功能 */
        {
          auto* turntableWindow = new Window(this, "转台功能");

          /* 确定了 turntableWindow 的位置 */
          turntableWindow->set_position({800, 600});
          /* 创建一个新的布局 */
          GridLayout * layout = new GridLayout(Orientation::Horizontal, 1,
                                         Alignment::Middle, 5, 5);
          layout->set_col_alignment({ Alignment::Fill, Alignment::Fill });
          /* 定义了这个窗口的布局 */
          turntableWindow->set_layout(layout);
          turntableWindow->add<Button>("目标检测")->set_flags(Button::RadioButton);
          turntableWindow->add<Button>("手动")->set_flags(Button::RadioButton);
          turntableWindow->add<Button>("复位")->set_flags(Button::RadioButton);
        }

        /* 摄像头功能 */
        {
          auto* cameraWindow = new Window(this, "摄像头功能");

          /* 确定了 cameraWindow 的位置 */
          cameraWindow->set_position({1000, 300});
          /* 创建一个新的布局 */
          GridLayout * layout = new GridLayout(Orientation::Horizontal, 1,
                                         Alignment::Middle, 5, 5);
          layout->set_col_alignment({Alignment::Middle, Alignment::Minimum});
          /* 定义了这个窗口的布局 */
          cameraWindow->set_layout(layout);
          cameraWindow->add<Label>("摄像头一焦距");
          Widget *wdg = cameraWindow->add<Widget>();
          Button *btn;
          BoxLayout *box_layout = new BoxLayout(Orientation::Horizontal, Alignment::Middle, 15, 30);
          wdg->set_layout(box_layout);
          btn = wdg->add<Button>("ﯪ");
          btn->set_callback([this]() {
                  cout << "decrease camera 1 focal len" << endl;
                  this->getDeviceQueue(0).put(PolyM::DataMsg<std::string>(POLYM_FOCAL_SETTING, "-"));
           });
          btn->set_fixed_size(Vector2i(50, 30));
          btn->set_font_size(30);
          btn = wdg->add<Button>("ﯫ");
          btn->set_callback([this]() {
                  this->getDeviceQueue(0).put(PolyM::DataMsg<std::string>(POLYM_FOCAL_SETTING, "+"));
                  cout << "increase camera 1 focal len" << endl; });
          btn->set_fixed_size(Vector2i(50, 30));
          btn->set_font_size(30);
          cameraWindow->add<Label>("摄像头二焦距");
          wdg = cameraWindow->add<Widget>();
          wdg->set_layout(box_layout);
          btn = wdg->add<Button>("ﯪ");
          btn->set_callback([this]() {
                  this->getDeviceQueue(1).put(PolyM::DataMsg<std::string>(POLYM_FOCAL_SETTING, "-"));
                  cout << "decrease camera 2 focal len" << endl;
              });
          btn->set_fixed_size(Vector2i(50, 30));
          btn->set_font_size(30);
          btn = wdg->add<Button>("ﯫ");
          btn->set_callback([this]() {
                  this->getDeviceQueue(1).put(PolyM::DataMsg<std::string>(POLYM_FOCAL_SETTING, "+"));
                  cout << "increase camera 2 focal len" << endl; });
          btn->set_fixed_size(Vector2i(50, 30));
          btn->set_font_size(30);
        }

        {
            /* 创建一个新的 window 对象用来显示图片 */
            auto* img_window = new Window(this, "摄像头一视频");
            /* 设置各个方向的 margin 为 0 */
            img_window->set_layout(new GroupLayout(0,0,0,0));
            img_window->set_size(Vector2i(400, 300));
            img_window->set_position(Vector2i(0, 0));

            /* 在这个 window 上创建一个 img_window 控件 */
            img_window->add<VideoView>(mJsonValue.devices[0].camera_url);

            auto* img2_window = new Window(this, "摄像头二视频");
            /* 设置各个方向的 margin 为 0 */
            img2_window->set_layout(new GroupLayout(0,0,0,0));
            img2_window->set_position(Vector2i(400, 0));

            /* 在这个 window 上创建一个 img2_window 控件 */
            auto * video_image = img2_window->add<VideoView>(mJsonValue.devices[1].camera_url);
            video_image->set_fixed_size(Vector2i(400, 300));
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

        uint32_t indices[3*2] = {
            0, 1, 2,
            2, 3, 0
        };

        float positions[3*4] = {
            -1.f, -1.f, 0.f,
            1.f, -1.f, 0.f,
            1.f, 1.f, 0.f,
            -1.f, 1.f, 0.f
        };

        m_shader->set_buffer("indices", VariableType::UInt32, {3*2}, indices);
        m_shader->set_buffer("position", VariableType::Float32, {4, 3}, positions);
        m_shader->set_uniform("intensity", 0.5f);
    }

