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

/**
 * \class Keyboard keyboard.h sdl_gui/keyboard.h
 *
 * \brief Keyboard window for combo boxes, keyboard buttons, nested dialogs etc.
 *
 * Usually the Keyboard instance is constructed by another widget (e.g. \ref KeyboardButton)
 * and does not need to be created by hand.
 */
class NANOGUI_EXPORT Keyboard : public Window {
public:
    /// Create a new keyboard parented to a screen (first argument) and a parent window
    Keyboard(Widget *parent, Window *parentWindow, KeyboardType type = KeyboardType::Number);
    TextBox *get_textbox(){return mTextBox;}

protected:
    std::string mKeyboardValue;
    TextBox *mTextBox;
    KeyboardType mKeyboardType;
};

NAMESPACE_END(nanogui)
