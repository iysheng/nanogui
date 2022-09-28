/*
    src/button.cpp -- [Normal/Toggle/Radio/Popup] Button widget

    NanoGUI was developed by Wenzel Jakob <wenzel.jakob@epfl.ch>.
    The widget drawing code is based on the NanoVG demo application
    by Mikko Mononen.

    All rights reserved. Use of this source code is governed by a
    BSD-style license that can be found in the LICENSE.txt file.
*/

#include <nanogui/screen.h>
#include <nanogui/button.h>
#include <nanogui/popupbutton.h>
#include <nanogui/theme.h>
#include <nanogui/opengl.h>

NAMESPACE_BEGIN(nanogui)

Button::Button(Widget *parent, const std::string &caption, int icon)
    : Widget(parent), m_caption(caption), m_icon(icon), m_background_image(0),
      m_pushed_background_image(0), m_icon_position(IconPosition::LeftCentered), m_pushed(false),
      m_flags(NormalButton), m_background_color(Color(0x31, 0x57, 0x97, 255)),
      m_text_color(Color(0, 0)) { }

Button::Button(Widget *parent, const std::string &caption, const std::string &BackgroundImage, int icon)
    : Widget(parent), m_caption(caption), m_icon(icon), m_background_image(0),m_pushed_background_image(0),
      m_icon_position(IconPosition::LeftCentered), m_pushed(false),
      m_flags(NormalButton), m_background_color(Color(0, 0)),
      m_text_color(Color(0, 0)) {
    if (!BackgroundImage.empty())
    {
        NVGcontext *ctx = screen()->nvg_context();
        m_background_image = nvgCreateImage(ctx, BackgroundImage.c_str(), 0);
    }
}

Button::Button(Widget *parent, const std::string &caption, const std::string &BackgroundImage, const std::string &PushedBackgroundImage, int icon)
    : Widget(parent), m_caption(caption), m_icon(icon), m_background_image(0),
      m_icon_position(IconPosition::LeftCentered), m_pushed(false),
      m_flags(NormalButton), m_background_color(Color(0, 0)),
      m_text_color(Color(0, 0)) {
    NVGcontext *ctx = screen()->nvg_context();
    if (!BackgroundImage.empty())
    {
        m_background_image = nvgCreateImage(ctx, BackgroundImage.c_str(), 0);
    }
    if (!PushedBackgroundImage.empty())
    {
        m_pushed_background_image = nvgCreateImage(ctx, PushedBackgroundImage.c_str(), 0);
    }
}

Vector2i Button::preferred_size(NVGcontext *ctx) const {
    int font_size = m_font_size == -1 ? m_theme->m_button_font_size : m_font_size;
    nvgFontSize(ctx, font_size);
    nvgFontFace(ctx, "sans-bold");
    float tw = nvgTextBounds(ctx, 0,0, m_caption.c_str(), nullptr, nullptr);
    float iw = 0.0f, ih = font_size;

    if (m_background_image)
    {
        int w, h;
        nvgImageSize(ctx, m_background_image, &w, &h);
        return Vector2i(w, h);
    } else if (m_icon) {
        if (nvg_is_font_icon(m_icon)) {
            ih *= icon_scale();
            nvgFontFace(ctx, "icons");
            nvgFontSize(ctx, ih);
            iw = nvgTextBounds(ctx, 0, 0, utf8(m_icon).data(), nullptr, nullptr)
                + m_size.y() * 0.15f;
        } else {
            int w, h;
            ih *= 0.9f;
            nvgImageSize(ctx, m_icon, &w, &h);
            iw = w * ih / h;
        }
    }
    return Vector2i((int)(tw + iw) + 20, font_size + 10);
}

bool Button::mouse_enter_event(const Vector2i &p, bool enter) {
    Widget::mouse_enter_event(p, enter);
    return true;
}

/*
 * modifiers : 这个参数表示什么意思呢？？？, 修饰？？？是类似 shift， 这类模式按键？？？
 * */
bool Button::mouse_button_event(const Vector2i &p, int button, bool down, int modifiers) {
    Widget::mouse_button_event(p, button, down, modifiers);
    /* Temporarily increase the reference count of the button in case the
       button causes the parent window to be destructed */
    /* 临时增加这个 button 的引用计数，防止这个 button 导致 parent window 析构 */
    ref<Button> self = this;

    if (m_enabled == 1 &&
        ((button == GLFW_MOUSE_BUTTON_1 && !(m_flags & MenuButton)) ||
         (button == GLFW_MOUSE_BUTTON_2 &&  (m_flags & MenuButton)))) {
        /* 备份初始的 m_pushed 的状态 */
        bool pushed_backup = m_pushed;
        /* 如果这个 button 是按下状态 */
        if (down) {
            /* 如果是单选按钮 */
            if (m_flags & RadioButton) {
                /* 如果没有定义 button_group，那么就更新 parent() 所有的单选按钮 */
                if (m_button_group.empty()) {
                    for (auto widget : parent()->children()) {
                        Button *b = dynamic_cast<Button *>(widget);
                        if (b != this && b && (b->flags() & RadioButton) && b->m_pushed) {
                            b->m_pushed = false;
                            if (b->m_change_callback)
                                b->m_change_callback(false);
                        }
                    }
                } else {
                    /* 如果这个 button 的 m_button_group 不为空，那么更新这个 button_group 中所有的单选 button 的状态 */
                    for (auto b : m_button_group) {
                        if (b != this && (b->flags() & RadioButton) && b->m_pushed) {
                            b->m_pushed = false;
                            if (b->m_change_callback)
                                b->m_change_callback(false);
                        }
                    }
                }
            }
            /* 如果是弹出 button */
            if (m_flags & PopupButton) {
                for (auto widget : parent()->children()) {
                    Button *b = dynamic_cast<Button *>(widget);
                    if (b != this && b && (b->flags() & PopupButton) && b->m_pushed) {
                        b->m_pushed = false;
                        if (b->m_change_callback)
                            b->m_change_callback(false);
                    }
                }
                dynamic_cast<nanogui::PopupButton*>(this)->popup()->request_focus();
            }
            /* 如果是开关按钮,即是自锁按钮 */
            if (m_flags & ToggleButton)
                m_pushed = !m_pushed;
            else
                /* 否则是自复位的按钮 */
                m_pushed = true;
        } else if (m_pushed || (m_flags & MenuButton)) {
            if (contains(p) && m_callback)
                m_callback();
            /* 如果是常规的按钮，执行完回调函数后就标记按键弹出，即鼠标松开 */
            if (m_flags & NormalButton)
                m_pushed = false;
        }
        /* 如果状态发生了改变，那么回调这个 button 的 m_change_callback 函数 */
        if (pushed_backup != m_pushed && m_change_callback)
            m_change_callback(m_pushed);

        return true;
    }
    return false;
}

void Button::draw(NVGcontext *ctx) {
    /* 执行 child 的 widgets 的 draw() */
    Widget::draw(ctx);
    int background_image = m_background_image;

    NVGcolor grad_top = m_theme->m_button_gradient_top_unfocused;
    NVGcolor grad_bot = m_theme->m_button_gradient_bot_unfocused;

    if (m_pushed || (m_mouse_focus && (m_flags & MenuButton))) {
        grad_top = m_theme->m_button_gradient_top_pushed;
        grad_bot = m_theme->m_button_gradient_bot_pushed;
        if(m_pushed_background_image)
        {
            background_image = m_pushed_background_image;
        }
    } else if (m_mouse_focus && m_enabled) {
        grad_top = m_theme->m_button_gradient_top_focused;
        grad_bot = m_theme->m_button_gradient_bot_focused;
    }

    nvgBeginPath(ctx);
    
    nvgRoundedRect(ctx, m_pos.x() + 1, m_pos.y() + 1.0f, m_size.x() - 2,
                       m_size.y() - 2, m_theme->m_button_corner_radius - 1);
    if (!background_image)
    {
        /* 初始化的时候 m_background_color 是为 0 的 */
        if (m_background_color.w() != 0) {
            nvgFillColor(ctx, Color(m_background_color[0], m_background_color[1],
                                    m_background_color[2], 1.f));
            nvgFill(ctx);
            if (m_pushed) {
                grad_top.a = grad_bot.a = 0.8f;
            } else {
                double v = 1 - m_background_color.w();
                grad_top.a = grad_bot.a = m_enabled ? v : v * .5f + .5f;
            }
        }
    
        NVGpaint bg = nvgLinearGradient(ctx, m_pos.x(), m_pos.y(), m_pos.x(),
                                        m_pos.y() + m_size.y(), grad_top, grad_bot);
    
        nvgFillPaint(ctx, bg);
        nvgFill(ctx);
    }
    else
    {
        /* 绘制按键的背景图片 */
        NVGpaint img_paint = nvgImagePattern(ctx, m_pos.x() + 1, m_pos.y() + 1.0f, m_size.x() - 2,
                   m_size.y() - 2, 0, background_image, m_enabled ? 0.5f : 0.25f);
        nvgFillPaint(ctx, img_paint);
        nvgFill(ctx);
    }
    /* 描边 */
    nvgBeginPath(ctx);
    nvgStrokeWidth(ctx, 1.0f);
    nvgRoundedRect(ctx, m_pos.x() + 0.5f, m_pos.y() + (m_pushed ? 0.5f : 1.5f), m_size.x() - 1,
                   m_size.y() - 1 - (m_pushed ? 0.0f : 1.0f), m_theme->m_button_corner_radius);
    nvgStrokeColor(ctx, m_theme->m_border_light);
    nvgStroke(ctx);

    /* 继续描边 */
    nvgBeginPath(ctx);
    nvgRoundedRect(ctx, m_pos.x() + 0.5f, m_pos.y() + 0.5f, m_size.x() - 1,
                   m_size.y() - 2, m_theme->m_button_corner_radius);
    nvgStrokeColor(ctx, m_theme->m_border_dark);
    nvgStroke(ctx);

    int font_size = m_font_size == -1 ? m_theme->m_button_font_size : m_font_size;
    nvgFontSize(ctx, font_size);
    nvgFontFace(ctx, "sans-bold");
    /* 返回显示字体的下一个位置，用来显示 icon 图标 */
    float tw = nvgTextBounds(ctx, 0,0, m_caption.c_str(), nullptr, nullptr);

    Vector2f center = Vector2f(m_pos) + Vector2f(m_size) * 0.5f;
    Vector2f text_pos(center.x() - tw * 0.5f, center.y() - 1);
    NVGcolor text_color =
        m_text_color.w() == 0 ? m_theme->m_text_color : m_text_color;
    if (!m_enabled)
        text_color = m_theme->m_disabled_text_color;

    /* 如果有图标 icon */
    if (m_icon) {
        auto icon = utf8(m_icon);

        float iw, ih = font_size;
        if (nvg_is_font_icon(m_icon)) {
            ih *= icon_scale();
            nvgFontSize(ctx, ih);
            nvgFontFace(ctx, "icons");
            iw = nvgTextBounds(ctx, 0, 0, icon.data(), nullptr, nullptr);
        } else {
            int w, h;
            ih *= 0.9f;
            nvgImageSize(ctx, m_icon, &w, &h);
            iw = w * ih / h;
        }
        if (m_caption != "")
            iw += m_size.y() * 0.15f;
        nvgFillColor(ctx, text_color);
        nvgTextAlign(ctx, NVG_ALIGN_LEFT | NVG_ALIGN_MIDDLE);
        Vector2f icon_pos = center;
        icon_pos.y() -= 1;

        if (m_icon_position == IconPosition::LeftCentered) {
            icon_pos.x() -= (tw + iw) * 0.5f;
            text_pos.x() += iw * 0.5f;
        } else if (m_icon_position == IconPosition::RightCentered) {
            text_pos.x() -= iw * 0.5f;
            icon_pos.x() += tw * 0.5f;
        } else if (m_icon_position == IconPosition::Left) {
            icon_pos.x() = m_pos.x() + 8;
        } else if (m_icon_position == IconPosition::Right) {
            icon_pos.x() = m_pos.x() + m_size.x() - iw - 8;
        }

        /* 如果是一个字体图标 */
        if (nvg_is_font_icon(m_icon)) {
            nvgText(ctx, icon_pos.x(), icon_pos.y()+1, icon.data(), nullptr);
        } else {
            /* 如果是一副图片 */
            NVGpaint img_paint = nvgImagePattern(ctx,
                    icon_pos.x(), icon_pos.y() - ih/2, iw, ih, 0, m_icon, m_enabled ? 0.5f : 0.25f);

            nvgFillPaint(ctx, img_paint);
            nvgFill(ctx);
        }
    }

    nvgFontSize(ctx, font_size);
    nvgFontFace(ctx, "sans-bold");
    nvgTextAlign(ctx, NVG_ALIGN_LEFT | NVG_ALIGN_MIDDLE);
    /* 显示按键的字体,阴影效果 */
    nvgFillColor(ctx, m_theme->m_text_color_shadow);
    nvgText(ctx, text_pos.x(), text_pos.y(), m_caption.c_str(), nullptr);
    /* 显示按键的字体 */
    nvgFillColor(ctx, text_color);
    nvgText(ctx, text_pos.x(), text_pos.y() + 1, m_caption.c_str(), nullptr);
}

NAMESPACE_END(nanogui)
