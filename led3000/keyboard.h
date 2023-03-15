/*
    sdl_gui/keyboard.h -- Simple keyboard widget which is attached to another given
    window (can be nested)

    Based on NanoGUI by Wenzel Jakob <wenzel@inf.ethz.ch>.
    Adaptation for SDL by Dalerank <dalerankn8@gmail.com>

    All rights reserved. Use of this source code is governed by a
    BSD-style license that can be found in the LICENSE.txt file.
*/
/** \file */

#pragma once

#include <nanogui/window.h>
#include <nanogui/button.h>
#include <nanogui/textbox.h>
#include <vector>

NAMESPACE_BEGIN(nanogui)

enum class KeyboardType {
    Number = 0,
    NumberIP = 1,
    Full = 2,
};

class NANOGUI_EXPORT Keyboard : public Window
{
public:
    enum Side { Left = 0, Right };

    Keyboard(Widget *parent, Window *parentWindow, KeyboardType type = KeyboardType::Number);
    TextBox *get_textbox() {return mTextBox;}
    void set_textbox(TextBox *textbox) {mTextBox = textbox;}
    void set_value(std::string value) {mKeyboardValue = value;}
    /// Create a new popup parented to a screen (first argument) and a parent window (if applicable)
    //Keyboard(Widget *parent, Window *parent_window = nullptr);

    /// Return the anchor position in the parent window; the placement of the popup is relative to it
    void set_anchor_pos(const Vector2i &anchor_pos) { m_anchor_pos = anchor_pos; }
    /// Set the anchor position in the parent window; the placement of the popup is relative to it
    const Vector2i &anchor_pos() const { return m_anchor_pos; }

    /// Set the anchor height; this determines the vertical shift relative to the anchor position
    void set_anchor_offset(int anchor_offset) { m_anchor_offset = anchor_offset; }
    /// Return the anchor height; this determines the vertical shift relative to the anchor position
    int anchor_offset() const { return m_anchor_offset; }

    /// Set the anchor width
    void set_anchor_size(int anchor_size) { m_anchor_size = anchor_size; }
    /// Return the anchor width
    int anchor_size() const { return m_anchor_size; }

    /// Set the side of the parent window at which popup will appear
    void set_side(Side popup_side) { m_side = popup_side; }
    /// Return the side of the parent window at which popup will appear
    Side side() const { return m_side; }

    /// Return the parent window of the popup
    Window *parent_window() { return m_parent_window; }
    /// Return the parent window of the popup
    const Window *parent_window() const { return m_parent_window; }

    /// Invoke the associated layout generator to properly place child widgets, if any
    virtual void perform_layout(NVGcontext *ctx) override;

    /// Draw the popup window
    virtual void draw(NVGcontext* ctx) override;
protected:
    /// Internal helper function to maintain nested window position values
    virtual void refresh_relative_placement() override;

protected:
    Window *m_parent_window;
    Vector2i m_anchor_pos;
    int m_anchor_offset, m_anchor_size;
    Side m_side;
    std::string mKeyboardValue;
    TextBox *mTextBox;
    KeyboardType mKeyboardType;
};
NAMESPACE_END(nanogui)
