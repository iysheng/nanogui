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

    MessageDialog(Widget *parent, Type type, const std::string &title,
                  const std::string &message,
                  const std::string &confirmButtonText,
                  const std::string &setButtonText,
                  const std::string &cancleButtonText, bool setButton);

#if 0
    MessageDialog(Widget *parent, Type type, const std::string &title = "Untitled",
                  const std::string &message = "Message",
                  const std::string &button_text = "OK",
                  const std::string &alt_button_text = "Cancel", bool alt_button = false);
#endif
    MessageDialog(Widget *parent, Type type, const std::string &title = "Untitled",
                  const std::string &message = "Message",
                  const std::string &button_text = "OK",
                  const std::string &alt_button_text = "Cancel", bool alt_button = false)
      : MessageDialog(parent, type, title, message, button_text, "", alt_button_text, false)
    {}

    MessageDialog(Widget *parent, Type type, const std::string &title,
                  const std::string &message,
                  const std::string &confirmButtonText,
                  const std::string &cancleButtonText,
                  const std::function<void(Widget *, int)> &callback )
      : MessageDialog(parent, type, title, message, confirmButtonText, "", cancleButtonText, false)
    { set_widget_callback(callback); }

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
};

NAMESPACE_END(nanogui)
