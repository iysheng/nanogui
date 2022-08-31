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
        ((Keyboard *)widget)->mKeyboardValue.push_back(x); \
        ((TextBox *)(((Keyboard *)widget)->get_textbox()))->set_value(((Keyboard *)widget)->mKeyboardValue); \
  } while(0);

Keyboard::Keyboard(Widget *parent, Window *parent_window, KeyboardType type)
    : Window(parent, ""), mKeyboardType(type), mTextBox(nullptr), m_parent_window(parent_window), m_anchor_pos(Vector2i(0)),
      m_anchor_offset(30), m_anchor_size(15), m_side(Side::Right)
{
#if 0
  if (type == KeyboardType::Number)
  {
    set_layout(new GridLayout(Orientation::Horizontal, 3, Alignment::Middle, 5, 5));
    wdg<Button>("1").set_widget_callback([](Widget *widget) {
        Keyboard *keyboard = dynamic_cast<Keyboard*>(widget);
        keyboard->mKeyboardValue.push_back('1');
        keyboard->get_textbox()->set_value(keyboard->mKeyboardValue);
        keyboard->get_textbox()->focus_event(true);
        });
    this->wdg<Button>("2").set_widget_callback([](Widget *widget) {
        ((Keyboard *)widget)->mKeyboardValue.push_back('2');
        ((TextBox *)(((Keyboard *)widget)->get_textbox()))->set_value(((Keyboard *)widget)->mKeyboardValue);
        ((TextBox *)(((Keyboard *)widget)->get_textbox()))->focus_event(true);
        });
    this->wdg<Button>("3").set_widget_callback([](Widget *widget) {
        ((Keyboard *)widget)->mKeyboardValue.push_back('3');
        ((TextBox *)(((Keyboard *)widget)->get_textbox()))->set_value(((Keyboard *)widget)->mKeyboardValue);
        ((TextBox *)(((Keyboard *)widget)->get_textbox()))->focus_event(true);
        });
    this->wdg<Button>("4").set_widget_callback([](Widget *widget) {
        ((Keyboard *)widget)->mKeyboardValue.push_back('4');
        ((TextBox *)(((Keyboard *)widget)->get_textbox()))->set_value(((Keyboard *)widget)->mKeyboardValue);
        ((TextBox *)(((Keyboard *)widget)->get_textbox()))->focus_event(true);
        });
    this->wdg<Button>("5").set_widget_callback([](Widget *widget) {
        ((Keyboard *)widget)->mKeyboardValue.push_back('5');
        ((TextBox *)(((Keyboard *)widget)->get_textbox()))->set_value(((Keyboard *)widget)->mKeyboardValue);
        ((TextBox *)(((Keyboard *)widget)->get_textbox()))->focus_event(true);
        });
    this->wdg<Button>("6").set_widget_callback([](Widget *widget) {
        ((Keyboard *)widget)->mKeyboardValue.push_back('6');
        ((TextBox *)(((Keyboard *)widget)->get_textbox()))->set_value(((Keyboard *)widget)->mKeyboardValue);
        ((TextBox *)(((Keyboard *)widget)->get_textbox()))->focus_event(true);
        });
    this->wdg<Button>("7").set_widget_callback([](Widget *widget) {
        ((Keyboard *)widget)->mKeyboardValue.push_back('7');
        ((TextBox *)(((Keyboard *)widget)->get_textbox()))->set_value(((Keyboard *)widget)->mKeyboardValue);
        ((TextBox *)(((Keyboard *)widget)->get_textbox()))->focus_event(true);
        });
    this->wdg<Button>("8").set_widget_callback([](Widget *widget) {
        ((Keyboard *)widget)->mKeyboardValue.push_back('8');
        ((TextBox *)(((Keyboard *)widget)->get_textbox()))->set_value(((Keyboard *)widget)->mKeyboardValue);
        ((TextBox *)(((Keyboard *)widget)->get_textbox()))->focus_event(true);
        });
    this->wdg<Button>("9").set_widget_callback([](Widget *widget) {
        ((Keyboard *)widget)->mKeyboardValue.push_back('9');
        ((TextBox *)(((Keyboard *)widget)->get_textbox()))->set_value(((Keyboard *)widget)->mKeyboardValue);
        ((TextBox *)(((Keyboard *)widget)->get_textbox()))->focus_event(true);
        });
    Button *button_del = new Button(this, "", ENTYPO_ICON_LEFT_THIN);
    button_del->set_widget_callback([](Widget *widget){
      Keyboard *keyboard = dynamic_cast<Keyboard*>(widget);
      if (keyboard->mKeyboardValue.length())
      {
        keyboard->mKeyboardValue.pop_back();
        keyboard->get_textbox()->set_value(keyboard->mKeyboardValue);
        keyboard->get_textbox()->focus_event(true);
      }
    });
    this->wdg<Button>("0").set_widget_callback([](Widget *widget) {
        ((Keyboard *)widget)->mKeyboardValue.push_back('0');
        ((TextBox *)(((Keyboard *)widget)->get_textbox()))->set_value(((Keyboard *)widget)->mKeyboardValue);
        ((TextBox *)(((Keyboard *)widget)->get_textbox()))->focus_event(true);
        });
    Button *button_ok = new Button(this, "↵");
    button_ok->set_widget_callback([](Widget *widget) {
                red_debug_lite("num ok pushed:%s\n", ((Keyboard *)widget)->mKeyboardValue.c_str());
                ((Keyboard *)widget)->set_visible(false);
                ((Keyboard *)widget)->parent()->request_focus();
                ((TextBox *)(((Keyboard *)widget)->get_textbox()))->set_value(((Keyboard *)widget)->mKeyboardValue);
        });
    /* 测试发现大小是 29，30 这里直接固定大小,但是随着字体大小的改变
     * 这个大小应该也要变化
     * */
    button_ok->set_fixed_size(Vector2i(29, 30));
    button_del->set_fixed_size(Vector2i(29, 30));
  }
  else if (type == KeyboardType::NumberIP)
  {
    AdvancedGridLayout *layout = new AdvancedGridLayout({34,34,34}, {34,34,34,34,34});
    layout->setMargin(5);
    //layout->setColStretch(2, 1);
    this->set_layout(layout);
    this->wdg<Button>("1").set_widget_callback([](Widget *widget) {
        Keyboard *keyboard = dynamic_cast<Keyboard*>(widget);
        keyboard->mKeyboardValue.push_back('1');
        keyboard->get_textbox()->set_value(keyboard->mKeyboardValue);
        keyboard->get_textbox()->focus_event(true);
        //((TextBox *)(((Keyboard *)widget)->get_textbox()))->focus_event(true);
        });
    this->wdg<Button>("2").set_widget_callback([](Widget *widget) {
        ((Keyboard *)widget)->mKeyboardValue.push_back('2');
        ((TextBox *)(((Keyboard *)widget)->get_textbox()))->set_value(((Keyboard *)widget)->mKeyboardValue);
        ((TextBox *)(((Keyboard *)widget)->get_textbox()))->focus_event(true);
        });
    this->wdg<Button>("3").set_widget_callback([](Widget *widget) {
        ((Keyboard *)widget)->mKeyboardValue.push_back('3');
        ((TextBox *)(((Keyboard *)widget)->get_textbox()))->set_value(((Keyboard *)widget)->mKeyboardValue);
        ((TextBox *)(((Keyboard *)widget)->get_textbox()))->focus_event(true);
        });
    this->wdg<Button>("4").set_widget_callback([](Widget *widget) {
        ((Keyboard *)widget)->mKeyboardValue.push_back('4');
        ((TextBox *)(((Keyboard *)widget)->get_textbox()))->set_value(((Keyboard *)widget)->mKeyboardValue);
        ((TextBox *)(((Keyboard *)widget)->get_textbox()))->focus_event(true);
        });
    this->wdg<Button>("5").set_widget_callback([](Widget *widget) {
        ((Keyboard *)widget)->mKeyboardValue.push_back('5');
        ((TextBox *)(((Keyboard *)widget)->get_textbox()))->set_value(((Keyboard *)widget)->mKeyboardValue);
        ((TextBox *)(((Keyboard *)widget)->get_textbox()))->focus_event(true);
        });
    this->wdg<Button>("6").set_widget_callback([](Widget *widget) {
        ((Keyboard *)widget)->mKeyboardValue.push_back('6');
        ((TextBox *)(((Keyboard *)widget)->get_textbox()))->set_value(((Keyboard *)widget)->mKeyboardValue);
        ((TextBox *)(((Keyboard *)widget)->get_textbox()))->focus_event(true);
        });
    this->wdg<Button>("7").set_widget_callback([](Widget *widget) {
        ((Keyboard *)widget)->mKeyboardValue.push_back('7');
        ((TextBox *)(((Keyboard *)widget)->get_textbox()))->set_value(((Keyboard *)widget)->mKeyboardValue);
        ((TextBox *)(((Keyboard *)widget)->get_textbox()))->focus_event(true);
        });
    this->wdg<Button>("8").set_widget_callback([](Widget *widget) {
        ((Keyboard *)widget)->mKeyboardValue.push_back('8');
        ((TextBox *)(((Keyboard *)widget)->get_textbox()))->set_value(((Keyboard *)widget)->mKeyboardValue);
        ((TextBox *)(((Keyboard *)widget)->get_textbox()))->focus_event(true);
        });
    this->wdg<Button>("9").set_widget_callback([](Widget *widget) {
        ((Keyboard *)widget)->mKeyboardValue.push_back('9');
        ((TextBox *)(((Keyboard *)widget)->get_textbox()))->set_value(((Keyboard *)widget)->mKeyboardValue);
        ((TextBox *)(((Keyboard *)widget)->get_textbox()))->focus_event(true);
        });
    Button *button_del = new Button(this, "", ENTYPO_ICON_LEFT_THIN);
    button_del->set_widget_callback([](Widget *widget){
      Keyboard *keyboard = dynamic_cast<Keyboard*>(widget);
      if (keyboard->mKeyboardValue.length())
      {
        keyboard->mKeyboardValue.pop_back();
        keyboard->get_textbox()->set_value(keyboard->mKeyboardValue);
        keyboard->get_textbox()->focus_event(true);
      }
    });
    this->wdg<Button>("0").set_widget_callback([](Widget *widget) {
        ((Keyboard *)widget)->mKeyboardValue.push_back('0');
        ((TextBox *)(((Keyboard *)widget)->get_textbox()))->set_value(((Keyboard *)widget)->mKeyboardValue);
        ((TextBox *)(((Keyboard *)widget)->get_textbox()))->focus_event(true);
        });
    this->wdg<Button>(".").set_widget_callback([](Widget *widget) {
        ((Keyboard *)widget)->mKeyboardValue.push_back('.');
        ((TextBox *)(((Keyboard *)widget)->get_textbox()))->set_value(((Keyboard *)widget)->mKeyboardValue);
        ((TextBox *)(((Keyboard *)widget)->get_textbox()))->focus_event(true);
        });
    Button *button_ok = new Button(this, "↵");
    button_ok->set_widget_callback([](Widget *widget) {
                red_debug_lite("num ok pushed:%s\n", ((Keyboard *)widget)->mKeyboardValue.c_str());
                ((Keyboard *)widget)->set_visible(false);
                ((Keyboard *)widget)->parent()->request_focus();
                ((TextBox *)(((Keyboard *)widget)->get_textbox()))->set_value(((Keyboard *)widget)->mKeyboardValue);
        });
    /* 测试发现大小是 29，30 这里直接固定大小,但是随着字体大小的改变
     * 这个大小应该也要变化
     * */
    button_ok->set_fixed_size(Vector2i(30 * 3 + 4, 34));
    button_del->set_fixed_size(Vector2i(30, 34));

    int i = 0 , j = 0;
    for (; i < 4; i++)
      for (j = 0; j < 3; j++)
      {
        dynamic_cast<Button *>(mChildren[i*3+j])->set_fixed_size(Vector2i(29, 30));
        layout->setAnchor(mChildren[i*3+j], AdvancedGridLayout::Anchor(j, i, 1, 1, Alignment::Middle));
      }
    mChildren[i*3]->set_fixed_size(Vector2i(34 + 29*2, 30));
    layout->setAnchor(mChildren[i*3], AdvancedGridLayout::Anchor(0, i, 3, 1, Alignment::Middle));
  }
#endif
  if (type == KeyboardType::Full)
  {
    set_layout(new GridLayout(Orientation::Horizontal, 10, Alignment::Middle, 1, 1));
    this->add<Button>("1")->set_widget_callback([](Widget *widget) {KEYBOARD_BUTTON('1');});
    this->add<Button>("2")->set_widget_callback([](Widget *widget) {KEYBOARD_BUTTON('2');});
    this->add<Button>("3")->set_widget_callback([](Widget *widget) {KEYBOARD_BUTTON('3');});
    this->add<Button>("4")->set_widget_callback([](Widget *widget) {KEYBOARD_BUTTON('4');});
    this->add<Button>("5")->set_widget_callback([](Widget *widget) {KEYBOARD_BUTTON('5');});
    this->add<Button>("6")->set_widget_callback([](Widget *widget) {KEYBOARD_BUTTON('6');});
    this->add<Button>("7")->set_widget_callback([](Widget *widget) {KEYBOARD_BUTTON('7');});
    this->add<Button>("8")->set_widget_callback([](Widget *widget) {KEYBOARD_BUTTON('8');});
    this->add<Button>("9")->set_widget_callback([](Widget *widget) {KEYBOARD_BUTTON('9');});
    this->add<Button>("0")->set_widget_callback([](Widget *widget) {KEYBOARD_BUTTON('0');});

    this->add<Button>("q")->set_widget_callback([](Widget *widget) {KEYBOARD_BUTTON('q');});
    this->add<Button>("w")->set_widget_callback([](Widget *widget) {KEYBOARD_BUTTON('w');});
    this->add<Button>("e")->set_widget_callback([](Widget *widget) {KEYBOARD_BUTTON('e');});
    this->add<Button>("r")->set_widget_callback([](Widget *widget) {KEYBOARD_BUTTON('r');});
    this->add<Button>("t")->set_widget_callback([](Widget *widget) {KEYBOARD_BUTTON('t');});
    this->add<Button>("y")->set_widget_callback([](Widget *widget) {KEYBOARD_BUTTON('y');});
    this->add<Button>("u")->set_widget_callback([](Widget *widget) {KEYBOARD_BUTTON('u');});
    this->add<Button>("i")->set_widget_callback([](Widget *widget) {KEYBOARD_BUTTON('i');});
    this->add<Button>("o")->set_widget_callback([](Widget *widget) {KEYBOARD_BUTTON('o');});
    this->add<Button>("p")->set_widget_callback([](Widget *widget) {KEYBOARD_BUTTON('p');});

    this->add<Button>("a")->set_widget_callback([](Widget *widget) {KEYBOARD_BUTTON('a');});
    this->add<Button>("s")->set_widget_callback([](Widget *widget) {KEYBOARD_BUTTON('s');});
    this->add<Button>("d")->set_widget_callback([](Widget *widget) {KEYBOARD_BUTTON('d');});
    this->add<Button>("f")->set_widget_callback([](Widget *widget) {KEYBOARD_BUTTON('f');});
    this->add<Button>("g")->set_widget_callback([](Widget *widget) {KEYBOARD_BUTTON('g');});
    this->add<Button>("h")->set_widget_callback([](Widget *widget) {KEYBOARD_BUTTON('h');});
    this->add<Button>("j")->set_widget_callback([](Widget *widget) {KEYBOARD_BUTTON('j');});
    this->add<Button>("k")->set_widget_callback([](Widget *widget) {KEYBOARD_BUTTON('k');});
    this->add<Button>("l")->set_widget_callback([](Widget *widget) {KEYBOARD_BUTTON('l');});
    this->add<Button>("/")->set_widget_callback([](Widget *widget) {KEYBOARD_BUTTON('/');});

    this->add<Button>("z")->set_widget_callback([](Widget *widget) {KEYBOARD_BUTTON('z');});
    this->add<Button>("x")->set_widget_callback([](Widget *widget) {KEYBOARD_BUTTON('x');});
    this->add<Button>("c")->set_widget_callback([](Widget *widget) {KEYBOARD_BUTTON('c');});
    this->add<Button>("v")->set_widget_callback([](Widget *widget) {KEYBOARD_BUTTON('v');});
    this->add<Button>("b")->set_widget_callback([](Widget *widget) {KEYBOARD_BUTTON('b');});
    this->add<Button>("n")->set_widget_callback([](Widget *widget) {KEYBOARD_BUTTON('n');});
    this->add<Button>("m")->set_widget_callback([](Widget *widget) {KEYBOARD_BUTTON('m');});
    this->add<Button>(".")->set_widget_callback([](Widget *widget) {KEYBOARD_BUTTON('.');});
    this->add<Button>(",")->set_widget_callback([](Widget *widget) {KEYBOARD_BUTTON(',');});
    this->add<Button>(" ")->set_widget_callback([](Widget *widget) {KEYBOARD_BUTTON(' ');});

    this->add<Button>("!")->set_widget_callback([](Widget *widget) {KEYBOARD_BUTTON('!');});
    this->add<Button>("@")->set_widget_callback([](Widget *widget) {KEYBOARD_BUTTON('@');});
    this->add<Button>("#")->set_widget_callback([](Widget *widget) {KEYBOARD_BUTTON('#');});
    this->add<Button>("$")->set_widget_callback([](Widget *widget) {KEYBOARD_BUTTON('$');});
    this->add<Button>("%")->set_widget_callback([](Widget *widget) {KEYBOARD_BUTTON('%');});
    this->add<Button>("^")->set_widget_callback([](Widget *widget) {KEYBOARD_BUTTON('^');});
    this->add<Button>("&")->set_widget_callback([](Widget *widget) {KEYBOARD_BUTTON('&');});
    this->add<Button>(":")->set_widget_callback([](Widget *widget) {KEYBOARD_BUTTON(':');});

    Button *button_del = new Button(this, "-");
    button_del->set_widget_callback([](Widget *widget){
      Keyboard *keyboard = dynamic_cast<Keyboard*>(widget);
      if (keyboard->mKeyboardValue.length())
      {
        keyboard->mKeyboardValue.pop_back();
        keyboard->get_textbox()->set_value(keyboard->mKeyboardValue);
        keyboard->get_textbox()->focus_event(true);
      }
    });
    Button *button_ok = new Button(this, "↵");
    button_ok->set_widget_callback([](Widget *widget) {
                red_debug_lite("num ok pushed:%s\n", ((Keyboard *)widget)->mKeyboardValue.c_str());
                ((Keyboard *)widget)->set_visible(false);
                ((Keyboard *)widget)->parent()->request_focus();
                ((TextBox *)(((Keyboard *)widget)->get_textbox()))->set_value(((Keyboard *)widget)->mKeyboardValue);
        });
    button_ok->set_fixed_size(Vector2i(31, 30));
    button_del->set_fixed_size(Vector2i(31, 30));
  }
}

void Keyboard::perform_layout(NVGcontext *ctx) {
    if (m_layout || m_children.size() != 1) {
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
    m_pos = m_parent_window->position() + m_anchor_pos - Vector2i(0, m_anchor_offset);
}

void Keyboard::draw(NVGcontext* ctx) {
    refresh_relative_placement();

    if (!m_visible)
        return;

    int ds = m_theme->m_window_drop_shadow_size,
        cr = m_theme->m_window_corner_radius;

    nvgSave(ctx);
    nvgResetScissor(ctx);

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
    nvgFill(ctx);
    nvgRestore(ctx);

    Widget::draw(ctx);
}
NAMESPACE_END(nanogui)
