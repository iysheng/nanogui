/*
    src/messagedialog.cpp -- Simple "OK" or "Yes/No"-style modal dialogs

    NanoGUI was developed by Wenzel Jakob <wenzel.jakob@epfl.ch>.
    The widget drawing code is based on the NanoVG demo application
    by Mikko Mononen.

    All rights reserved. Use of this source code is governed by a
    BSD-style license that can be found in the LICENSE.txt file.
*/

#include <functional>
#include <nanogui/messagedialog.h>
#include <nanogui/layout.h>
#include <nanogui/button.h>
#include <nanogui/label.h>

NAMESPACE_BEGIN(nanogui)

MessageDialog::MessageDialog(Widget *parent, Type type, const std::string &title,
                const std::string &message,
                const std::string &confirmButtonText,
                const std::string &cancleButtonText,
                const std::string &setButtonText,
                const std::function<void(Widget *, int)> &callback, const std::function<void (Widget *)> &paint)
  : Window(parent, title, RED_LED3000_ASSETS_DIR"/set_msgdlg2.png"), m_widget_callback(callback)
{
    bool setButton = false;
    std::string icon(RED_LED3000_ASSETS_DIR"/led_icon.png");
    this->set_fixed_size(Vector2i(480, 324));
    switch (type)
    {
        case Type::Information: break;
        case Type::Question: break;
        case Type::Warning: icon = std::string(RED_LED3000_ASSETS_DIR"/led_alarm.png"); break;
        case Type::Choose: setButton = true; this->set_fixed_size(Vector2i(480, 532)); this->set_background_image(RED_LED3000_ASSETS_DIR"/set_msgdlg.png"); break;
        default: break;
    }

    set_modal(true);

    m_label_icon = this->add<Label>("");
    m_label_icon->set_icon(icon);
    m_label_icon->set_position(Vector2i(217, 91));
    m_cancel_button = new Button(this, cancleButtonText, m_theme->m_message_alt_button_icon);
    m_cancel_button->set_callback([&] { if (m_widget_callback) m_widget_callback(m_cancel_button, 0); else printf("it's null\n"); dispose();});
    if (setButton)
    {
        m_set_button = new Button(this, setButtonText);
        m_set_button->set_callback([&] { if (m_widget_callback) m_widget_callback(m_set_button, 2);
        });
    }
    m_confirm_button = new Button(this, confirmButtonText, m_theme->m_message_primary_button_icon);
    m_confirm_button->set_callback([&] { if (m_widget_callback) m_widget_callback(m_confirm_button, 1); dispose(); });
    m_message_label = this->add<Label>(message);
    if (setButton)
    {
        m_message_label->set_position(Vector2i(173, 394));
        m_confirm_button->set_fixed_size(Vector2i(146, 60));
        m_confirm_button->set_position(Vector2i(10, 462));
        m_cancel_button->set_position(Vector2i(167, 462));
        m_cancel_button->set_fixed_size(Vector2i(146, 60));
        m_set_button->set_position(Vector2i(324, 462));
        m_set_button->set_fixed_size(Vector2i(146, 60));
    }
    else
    {
        m_message_label->set_position(Vector2i(158, 186));
        m_confirm_button->set_fixed_size(Vector2i(225, 60));
        m_confirm_button->set_position(Vector2i(10, 254));
        m_cancel_button->set_fixed_size(Vector2i(225, 60));
        m_cancel_button->set_position(Vector2i(245, 254));
    }

    if (paint)
    {
        paint(this);
    }
    /* 使自己保持居中，在 center 执行过程中会主动执行 screen 的 perform layout 函数重新计算 screen 所有部件的大小和位置信息 */
    center();
    /* 使 screen 聚焦 focus 到自身 */
    request_focus();
}

NAMESPACE_END(nanogui)
