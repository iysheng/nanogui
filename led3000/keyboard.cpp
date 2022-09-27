/*
    nanogui/keyboard.cpp -- Simple keyboard widget which is attached to another given
    window (can be nested)

    Based on NanoGUI by Wenzel Jakob <wenzel@inf.ethz.ch>.
    Adaptation for SDL by Dalerank <dalerankn8@gmail.com>

    All rights reserved. Use of this source code is governed by a
    BSD-style license that can be found in the LICENSE.txt file.
*/

#include <nanogui/textbox.h>
#include <nanogui/screen.h>
#include <keyboard.h>
#include <nanogui/layout.h>
#include <nanogui/theme.h>
#include <nanogui/opengl.h>

#include "nanovg.h"
#define NANOVG_RT_IMPLEMENTATION
#define NANORT_IMPLEMENTATION

NAMESPACE_BEGIN(nanogui)

#define KEYBOARD_BUTTON(x) \
  do { \
        this->mKeyboardValue.push_back(x); \
        this->get_textbox()->set_value(this->mKeyboardValue); \
  } while(0);

Keyboard::Keyboard(Widget *parent, Window *parent_window, KeyboardType type)
    : Window(parent, ""), mKeyboardType(type), mTextBox(nullptr), m_parent_window(parent_window), m_anchor_pos(Vector2i(0)),
      m_anchor_offset(30), m_anchor_size(10), m_side(Side::Right)
{
  if (type == KeyboardType::NumberIP)
  {
    set_fixed_size(Vector2i(200, 325));
    set_background_image(RED_LED3000_ASSETS_DIR"/keyboard_numip.png");
    Button * btn = this->add<Button>("1");
    btn->set_callback([this]() {KEYBOARD_BUTTON('1')});
    btn->set_fixed_size(Vector2i(50, 50));
    btn->set_position(Vector2i(20, 20));
    btn = this->add<Button>("2");
    btn->set_callback([this]() {KEYBOARD_BUTTON('2')});
    btn->set_fixed_size(Vector2i(50, 50));
    btn->set_position(Vector2i(75, 20));
    btn = this->add<Button>("3");
    btn->set_callback([this]() {KEYBOARD_BUTTON('3')});
    btn->set_fixed_size(Vector2i(50, 50));
    btn->set_position(Vector2i(130, 20));

    btn = this->add<Button>("4");
    btn->set_fixed_size(Vector2i(50, 50));
    btn->set_position(Vector2i(20, 75));
    btn->set_callback([this]() {KEYBOARD_BUTTON('4')});
    btn = this->add<Button>("5");
    btn->set_fixed_size(Vector2i(50, 50));
    btn->set_position(Vector2i(75, 75));
    btn->set_callback([this]() {KEYBOARD_BUTTON('5')});
    btn = this->add<Button>("6");
    btn->set_fixed_size(Vector2i(50, 50));
    btn->set_position(Vector2i(130, 75));
    btn->set_callback([this]() {KEYBOARD_BUTTON('6')});

    btn = this->add<Button>("7");
    btn->set_fixed_size(Vector2i(50, 50));
    btn->set_position(Vector2i(20, 130));
    btn->set_callback([this]() {KEYBOARD_BUTTON('7')});
    btn = this->add<Button>("8");
    btn->set_fixed_size(Vector2i(50, 50));
    btn->set_position(Vector2i(75, 130));
    btn->set_callback([this]() {KEYBOARD_BUTTON('8')});
    btn = this->add<Button>("9");
    btn->set_fixed_size(Vector2i(50, 50));
    btn->set_position(Vector2i(130, 130));
    btn->set_callback([this]() {KEYBOARD_BUTTON('9')});

    btn = new Button(this, "ఀ"); /* 0xc00 -> del 按键 */
    btn->set_callback([this](){
      if (this->mKeyboardValue.length())
      {
        this->mKeyboardValue.pop_back();
        this->get_textbox()->set_value(this->mKeyboardValue);
      }
    });
    btn->set_fixed_size(Vector2i(50, 50));
    btn->set_position(Vector2i(20, 185));
    btn = this->add<Button>("0");
    btn->set_callback([this]() {KEYBOARD_BUTTON('0')});
    btn->set_fixed_size(Vector2i(50, 50));
    btn->set_position(Vector2i(75, 185));
    btn = this->add<Button>(".");
    btn->set_callback([this]() {
        this->mKeyboardValue.push_back('.');
        this->get_textbox()->set_value(this->mKeyboardValue);
        });
    btn->set_fixed_size(Vector2i(50, 50));
    btn->set_position(Vector2i(130, 185));

    Button *button_ok = new Button(this, "确认");
    button_ok->set_callback([this]() {
                red_debug_lite("num ok pushed:%s\n", this->mKeyboardValue.c_str());
                this->window()->set_visible(false);
                this->get_textbox()->window()->set_modal(true);
                this->get_textbox()->window()->request_focus();
                this->get_textbox()->set_value(this->mKeyboardValue);
        });
    button_ok->set_fixed_size(Vector2i(180, 60));
    button_ok->set_position(Vector2i(10, 255));
  }
  else if (type == KeyboardType::Full)
  {
    set_fixed_size(Vector2i(456, 594));
    set_background_image(RED_LED3000_ASSETS_DIR"/keyboard_full.png");
    Button * btn = this->add<Button>("1");
    btn->set_callback([this]() {KEYBOARD_BUTTON('1')});
    btn->set_fixed_size(Vector2i(60, 60));
    btn->set_position(Vector2i(22, 22));
    btn = this->add<Button>("2");
    btn->set_callback([this]() {KEYBOARD_BUTTON('2')});
    btn->set_fixed_size(Vector2i(60, 60));
    btn->set_position(Vector2i(92, 22));
    btn = this->add<Button>("3");
    btn->set_callback([this]() {KEYBOARD_BUTTON('3')});
    btn->set_fixed_size(Vector2i(60, 60));
    btn->set_position(Vector2i(162, 22));
    btn = this->add<Button>("4");
    btn->set_callback([this]() {KEYBOARD_BUTTON('4')});
    btn->set_fixed_size(Vector2i(60, 60));
    btn->set_position(Vector2i(232, 22));
    btn = this->add<Button>("5");
    btn->set_callback([this]() {KEYBOARD_BUTTON('5')});
    btn->set_fixed_size(Vector2i(60, 60));
    btn->set_position(Vector2i(302, 22));
    btn = this->add<Button>("6");
    btn->set_callback([this]() {KEYBOARD_BUTTON('6')});
    btn->set_fixed_size(Vector2i(60, 60));
    btn->set_position(Vector2i(372, 22));
    btn = this->add<Button>("7");
    btn->set_callback([this]() {KEYBOARD_BUTTON('7')});
    btn->set_fixed_size(Vector2i(60, 60));
    btn->set_position(Vector2i(22, 92));
    btn = this->add<Button>("8");
    btn->set_callback([this]() {KEYBOARD_BUTTON('8')});
    btn->set_fixed_size(Vector2i(60, 60));
    btn->set_position(Vector2i(92, 92));
    btn = this->add<Button>("9");
    btn->set_callback([this]() {KEYBOARD_BUTTON('9')});
    btn->set_fixed_size(Vector2i(60, 60));
    btn->set_position(Vector2i(162, 92));
    btn = this->add<Button>("0");
    btn->set_callback([this]() {KEYBOARD_BUTTON('0')});
    btn->set_fixed_size(Vector2i(60, 60));
    btn->set_position(Vector2i(232, 92));

    btn = this->add<Button>("q");
    btn->set_callback([this]() {KEYBOARD_BUTTON('q');});
    btn->set_fixed_size(Vector2i(60, 60));
    btn->set_position(Vector2i(302, 92));
    btn = this->add<Button>("w");
    btn->set_callback([this]() {KEYBOARD_BUTTON('w');});
    btn->set_fixed_size(Vector2i(60, 60));
    btn->set_position(Vector2i(372, 92));
    btn = this->add<Button>("e");
    btn->set_callback([this]() {KEYBOARD_BUTTON('e');});
    btn->set_fixed_size(Vector2i(60, 60));
    btn->set_position(Vector2i(22, 162));
    btn = this->add<Button>("r");
    btn->set_callback([this]() {KEYBOARD_BUTTON('r');});
    btn->set_fixed_size(Vector2i(60, 60));
    btn->set_position(Vector2i(92, 162));
    btn = this->add<Button>("t");
    btn->set_callback([this]() {KEYBOARD_BUTTON('t');});
    btn->set_fixed_size(Vector2i(60, 60));
    btn->set_position(Vector2i(162, 162));
    btn = this->add<Button>("y");
    btn->set_callback([this]() {KEYBOARD_BUTTON('y');});
    btn->set_fixed_size(Vector2i(60, 60));
    btn->set_position(Vector2i(232, 162));
    btn = this->add<Button>("u");
    btn->set_callback([this]() {KEYBOARD_BUTTON('u');});
    btn->set_fixed_size(Vector2i(60, 60));
    btn->set_position(Vector2i(302, 162));
    btn = this->add<Button>("i");
    btn->set_callback([this]() {KEYBOARD_BUTTON('i');});
    btn->set_fixed_size(Vector2i(60, 60));
    btn->set_position(Vector2i(372, 162));
    btn = this->add<Button>("o");
    btn->set_callback([this]() {KEYBOARD_BUTTON('o');});
    btn->set_fixed_size(Vector2i(60, 60));
    btn->set_position(Vector2i(22, 232));
    btn = this->add<Button>("p");
    btn->set_callback([this]() {KEYBOARD_BUTTON('p');});
    btn->set_fixed_size(Vector2i(60, 60));
    btn->set_position(Vector2i(92, 232));

    btn = this->add<Button>("a");
    btn->set_callback([this]() {KEYBOARD_BUTTON('a');});
    btn->set_position(Vector2i(162, 232));
    btn->set_fixed_size(Vector2i(60, 60));
    btn = this->add<Button>("s");
    btn->set_callback([this]() {KEYBOARD_BUTTON('s');});
    btn->set_position(Vector2i(232, 232));
    btn->set_fixed_size(Vector2i(60, 60));
    btn = this->add<Button>("d");
    btn->set_callback([this]() {KEYBOARD_BUTTON('d');});
    btn->set_position(Vector2i(302, 232));
    btn->set_fixed_size(Vector2i(60, 60));
    btn = this->add<Button>("f");
    btn->set_callback([this]() {KEYBOARD_BUTTON('f');});
    btn->set_position(Vector2i(372, 232));
    btn->set_fixed_size(Vector2i(60, 60));
    btn = this->add<Button>("g");
    btn->set_callback([this]() {KEYBOARD_BUTTON('g');});
    btn->set_position(Vector2i(22, 302));
    btn->set_fixed_size(Vector2i(60, 60));
    btn = this->add<Button>("h");
    btn->set_callback([this]() {KEYBOARD_BUTTON('h');});
    btn->set_position(Vector2i(92, 302));
    btn->set_fixed_size(Vector2i(60, 60));
    btn = this->add<Button>("j");
    btn->set_callback([this]() {KEYBOARD_BUTTON('j');});
    btn->set_position(Vector2i(162, 302));
    btn->set_fixed_size(Vector2i(60, 60));
    btn = this->add<Button>("k");
    btn->set_callback([this]() {KEYBOARD_BUTTON('k');});
    btn->set_position(Vector2i(232, 302));
    btn->set_fixed_size(Vector2i(60, 60));
    btn = this->add<Button>("l");
    btn->set_callback([this]() {KEYBOARD_BUTTON('l');});
    btn->set_position(Vector2i(302, 302));
    btn->set_fixed_size(Vector2i(60, 60));
    btn = this->add<Button>("z");
    btn->set_callback([this]() {KEYBOARD_BUTTON('z');});
    btn->set_position(Vector2i(372, 302));
    btn->set_fixed_size(Vector2i(60, 60));

    btn = this->add<Button>("x");
    btn->set_callback([this]() {KEYBOARD_BUTTON('x');});
    btn->set_fixed_size(Vector2i(60, 60));
    btn->set_position(Vector2i(22, 372));
    btn = this->add<Button>("c");
    btn->set_callback([this]() {KEYBOARD_BUTTON('c');});
    btn->set_fixed_size(Vector2i(60, 60));
    btn->set_position(Vector2i(92, 372));
    btn = this->add<Button>("v");
    btn->set_callback([this]() {KEYBOARD_BUTTON('v');});
    btn->set_fixed_size(Vector2i(60, 60));
    btn->set_position(Vector2i(162, 372));
    btn = this->add<Button>("b");
    btn->set_callback([this]() {KEYBOARD_BUTTON('b');});
    btn->set_fixed_size(Vector2i(60, 60));
    btn->set_position(Vector2i(232, 372));
    btn = this->add<Button>("n");
    btn->set_callback([this]() {KEYBOARD_BUTTON('n');});
    btn->set_fixed_size(Vector2i(60, 60));
    btn->set_position(Vector2i(302, 372));
    btn = this->add<Button>("m");
    btn->set_callback([this]() {KEYBOARD_BUTTON('m');});
    btn->set_fixed_size(Vector2i(60, 60));
    btn->set_position(Vector2i(372, 372));

    btn = this->add<Button>("(");
    btn->set_callback([this]() {KEYBOARD_BUTTON('(');});
    btn->set_fixed_size(Vector2i(60, 60));
    btn->set_position(Vector2i(22, 442));
    btn = this->add<Button>(")");
    btn->set_callback([this]() {KEYBOARD_BUTTON(')');});
    btn->set_fixed_size(Vector2i(60, 60));
    btn->set_position(Vector2i(92, 442));
    btn = this->add<Button>(".");
    btn->set_callback([this]() {KEYBOARD_BUTTON('.');});
    btn->set_fixed_size(Vector2i(60, 60));
    btn->set_position(Vector2i(162, 442));

    btn = this->add<Button>("/");
    btn->set_callback([this]() {KEYBOARD_BUTTON('/');});
    btn->set_position(Vector2i(232, 442));
    btn->set_fixed_size(Vector2i(60, 60));
    btn = this->add<Button>("-");
    btn->set_callback([this]() {KEYBOARD_BUTTON('-');});
    btn->set_position(Vector2i(302, 442));
    btn->set_fixed_size(Vector2i(60, 60));
    btn = this->add<Button>("?");
    btn->set_callback([this]() {KEYBOARD_BUTTON('?');});
    btn->set_position(Vector2i(372, 442));
    btn->set_fixed_size(Vector2i(60, 60));
    btn = this->add<Button>("@");
    btn->set_callback([this]() {KEYBOARD_BUTTON('@');});
    btn->set_fixed_size(Vector2i(60, 60));
    btn->set_position(Vector2i(22, 512));
    btn = this->add<Button>(":");
    btn->set_callback([this]() {KEYBOARD_BUTTON(':');});
    btn->set_fixed_size(Vector2i(60, 60));
    btn->set_position(Vector2i(92, 512));

    btn = this->add<Button>("ఀ"); /* 0xc00 表示删除 */
    btn->set_fixed_size(Vector2i(130, 60));
    btn->set_position(Vector2i(162, 512));
    btn->set_callback([this](){
      if (this->mKeyboardValue.length())
      {
        this->mKeyboardValue.pop_back();
        this->get_textbox()->set_value(this->mKeyboardValue);
      }
    });
    btn = this->add<Button>("确认");
    btn->set_fixed_size(Vector2i(130, 60));
    btn->set_position(Vector2i(302, 512));
    btn->set_callback([this]() {
        red_debug_lite("num ok pushed:%s\n", this->mKeyboardValue.c_str());
        this->window()->set_visible(false);
        this->get_textbox()->window()->set_modal(true);
        this->get_textbox()->window()->request_focus();
        this->get_textbox()->set_value(this->mKeyboardValue);
    });
  }
}

void Keyboard::perform_layout(NVGcontext *ctx) {
    if (m_layout || m_children.size() != 1) {
        /* 自动调整窗口大小 */
        set_size(preferred_size(ctx));
        Widget::perform_layout(ctx);
    } else {
        m_children[0]->set_position(Vector2i(0));
        m_children[0]->set_size(m_size);
        m_children[0]->perform_layout(ctx);
    }
    if (m_side == Side::Left)
        m_anchor_pos[0] -= size()[0];
}

void Keyboard::refresh_relative_placement() {
    if (!m_parent_window)
        return;
    m_parent_window->refresh_relative_placement();
    m_visible &= m_parent_window->visible_recursive();
    /* 全键盘时因为尺寸过大，使用指定高度的显示 */
    if (mKeyboardType == KeyboardType::Full)
    {
        m_pos = Vector2i(m_anchor_size + m_parent_window->position()[0] + m_parent_window->size()[0], 104);
    }
    else
    {
        m_pos = m_parent_window->position() + m_anchor_pos - Vector2i(0, m_anchor_offset);
    }
}

void Keyboard::draw(NVGcontext* ctx) {
    refresh_relative_placement();

    if (!m_visible)
        return;

    int ds = m_theme->m_window_drop_shadow_size,
        cr = m_theme->m_window_corner_radius;

    nvgSave(ctx);
    nvgResetScissor(ctx);

    if (ds)
    {
        /* Draw a drop shadow */
        NVGpaint shadow_paint = nvgBoxGradient(
            ctx, m_pos.x(), m_pos.y(), m_size.x(), m_size.y(), cr*2, ds*2,
            m_theme->m_drop_shadow, m_theme->m_transparent);
    
        nvgBeginPath(ctx);
        nvgRect(ctx, m_pos.x()-ds,m_pos.y()-ds, m_size.x()+2*ds, m_size.y()+2*ds);
        nvgRoundedRect(ctx, m_pos.x(), m_pos.y(), m_size.x(), m_size.y(), cr);
        nvgPathWinding(ctx, NVG_HOLE);
        nvgFillPaint(ctx, shadow_paint);
        nvgFill(ctx);
    }

    /* Draw window */
    nvgBeginPath(ctx);
    nvgRoundedRect(ctx, m_pos.x(), m_pos.y(), m_size.x(), m_size.y(), cr);

    Vector2i base = m_pos + Vector2i(0, m_anchor_offset);
    int sign = -1;
    if (m_side == Side::Left) {
        base.x() += m_size.x();
        sign = 1;
    }

    nvgMoveTo(ctx, base.x() + m_anchor_size*sign, base.y());
    nvgLineTo(ctx, base.x() - 1*sign, base.y() - m_anchor_size);
    nvgLineTo(ctx, base.x() - 1*sign, base.y() + m_anchor_size);

    nvgFillColor(ctx, m_theme->m_window_popup);

    if (m_background_image)
    {
        NVGpaint img_paint = nvgImagePattern(ctx, m_pos.x(), m_pos.y(), m_size.x(),
                   m_size.y(), 0, m_background_image, m_enabled ? 1.0f : 0.5f);
        nvgFillPaint(ctx, img_paint);
    }
    nvgFill(ctx);
    nvgRestore(ctx);

    Widget::draw(ctx);
}
NAMESPACE_END(nanogui)
