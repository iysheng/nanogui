/*
    src/messagedialog.cpp -- Simple "OK" or "Yes/No"-style modal dialogs

    NanoGUI was developed by Wenzel Jakob <wenzel.jakob@epfl.ch>.
    The widget drawing code is based on the NanoVG demo application
    by Mikko Mononen.

    All rights reserved. Use of this source code is governed by a
    BSD-style license that can be found in the LICENSE.txt file.
*/

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
                const std::function<void(Widget *, int)> &callback)
  : Window(parent, title), m_widget_callback(callback)
{
    bool setButton = false;
    printf("show sys config window\n");
    set_layout(new BoxLayout(Orientation::Vertical,
                            Alignment::Middle, 10, 10));
    set_modal(true);

    Widget *panel1 = new Widget(this);
    panel1->set_layout(new BoxLayout(Orientation::Horizontal,
                                    Alignment::Middle, 10, 15));
    int icon = 0;
    switch (type)
    {
        case Type::Information: icon = m_theme->m_message_information_icon; break;
        case Type::Question: icon = m_theme->m_message_question_icon; break;
        case Type::Warning: icon = m_theme->m_message_warning_icon; break;
        case Type::Choose: icon = 0; setButton = true; break;
    }
    Label *iconLabel = new Label(panel1, std::string(utf8(icon).data()), "icons");
    iconLabel->set_font_size(50);
    m_message_label = new Label(panel1, message);
    Widget *panel2 = new Widget(this);
    panel2->set_layout(new BoxLayout(Orientation::Horizontal,
                                    Alignment::Middle, 0, 15));

    m_cancel_button = new Button(panel2, cancleButtonText, m_theme->m_message_alt_button_icon);
    m_cancel_button->set_callback([&] { if (m_widget_callback) m_widget_callback(m_cancel_button, 0); else printf("it's null\n"); dispose();});
    if (setButton)
    {
        m_set_button = new Button(panel2, setButtonText);
        m_set_button->set_callback([&] { if (m_widget_callback) m_widget_callback(m_set_button, 2);
        });
    }
    m_confirm_button = new Button(panel2, confirmButtonText, m_theme->m_message_primary_button_icon);
    m_confirm_button->set_callback([&] { if (m_widget_callback) m_widget_callback(m_confirm_button, 1); dispose(); });
    /* 使自己保持居中，在 center 执行过程中会主动执行 screen 的 perform layout 函数重新计算 screen 所有部件的大小和位置信息 */
    center();
    /* 使 screen 聚焦 focus 到自身 */
    request_focus();
}

NAMESPACE_END(nanogui)
