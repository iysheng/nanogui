/*
    nanogui/messagedialog.h -- Simple "OK" or "Yes/No"-style modal dialogs

    NanoGUI was developed by Wenzel Jakob <wenzel.jakob@epfl.ch>.
    The widget drawing code is based on the NanoVG demo application
    by Mikko Mononen.

    All rights reserved. Use of this source code is governed by a
    BSD-style license that can be found in the LICENSE.txt file.
*/
/** \file */

#pragma once

#include <nanogui/window.h>

NAMESPACE_BEGIN(nanogui)

/**
 * \class MessageDialog messagedialog.h nanogui/messagedialog.h
 *
 * \brief Simple "OK" or "Yes/No"-style modal dialogs.
 */
class NANOGUI_EXPORT MessageDialog : public Window {
public:
    /// Classification of the type of message this MessageDialog represents.
    enum class Type {
        Information,
        Question,
        Warning,
        Choose
    };

    /*
     * 临时修改，保证函数重载异常，需要进一步分析 2022-09-09
     * new MessageDialog(this, MessageDialog::Type::Question, "绿灯控制", "确认要打开绿光么?", "确认", "取消", do_with_green_light_normal, false); });
     * 不再尾部加一个参数，上述函数不会执行下属构造函数，奇怪 ！！！
     * */
    MessageDialog(Widget *parent, Type type, const std::string &title,
                  const std::string &message,
                  const std::string &confirmButtonText = "确认",
                  const std::string &cancleButtonText = "取消",
                  const std::string &set_text = "配置",
                  const std::function<void(Widget *, int)> &callback = nullptr,
                  const std::function<void (Widget *)> &paint = nullptr);

    Label *message_label() { return m_message_label; }
    const Label *message_label() const { return m_message_label; }

    std::function<void(int)> callback() const { return m_callback; }
    void set_callback(const std::function<void(int)> &callback) { m_callback = callback; }
    void set_widget_callback(const std::function<void(Widget *, int)> &callback) { m_widget_callback = callback; }
protected:
    std::function<void(int)> m_callback;
    std::function<void(Widget *, int)> m_widget_callback;
    Label *m_message_label;
    Button *m_cancel_button;
    Button *m_set_button;
    Button *m_confirm_button;
    Label *m_label_icon;
};

NAMESPACE_END(nanogui)
