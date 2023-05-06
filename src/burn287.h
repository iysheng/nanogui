/*
    src/example1.cpp -- C++ version of an example application that shows
    how to use the various widget classes. For a Python implementation, see
    '../python/example1.py'.

    NanoGUI was developed by Wenzel Jakob <wenzel.jakob@epfl.ch>.
    The widget drawing code is based on the NanoVG demo application
    by Mikko Mononen.

    All rights reserved. Use of this source code is governed by a
    BSD-style license that can be found in the LICENSE.txt file.
*/

#pragma once

#include <nanogui/opengl.h>
#include <nanogui/screen.h>
#include <nanogui/window.h>
#include <nanogui/layout.h>
#include <nanogui/label.h>
#include <nanogui/checkbox.h>
#include <nanogui/button.h>
#include <nanogui/toolbutton.h>
#include <nanogui/popupbutton.h>
#include <nanogui/combobox.h>
#include <nanogui/progressbar.h>
#include <nanogui/icons.h>
#include <nanogui/messagedialog.h>
#include <nanogui/textbox.h>
#include <nanogui/slider.h>
#include <nanogui/imagepanel.h>
#include <nanogui/imageview.h>
#include <nanogui/vscrollpanel.h>
#include <nanogui/colorwheel.h>
#include <nanogui/colorpicker.h>
#include <nanogui/graph.h>
#include <nanogui/tabwidget.h>
#include <nanogui/texture.h>
#include <nanogui/textarea.h>
#include <nanogui/shader.h>
#include <nanogui/renderpass.h>
#include <iostream>
#include <memory>

#define STB_IMAGE_STATIC
#define STB_IMAGE_IMPLEMENTATION
#if defined(_MSC_VER)
#  pragma warning (disable: 4505) // don't warn about dead code in stb_image.h
#elif defined(__GNUC__)
#   pragma GCC diagnostic ignored "-Wunused-function"
#endif
#include <stb_image.h>

using namespace nanogui;

#define ARRAY_SIZE(x)    (sizeof(x) / sizeof(x[0]))

extern int open_uart_dev(const char *dev);

class RedBurntool : public Screen {
public:
    RedBurntool() : Screen(Vector2i(1024, 768), "RedBurntool") {
        inc_ref();
        Widget *tools;
        Window *window = new Window(this, "程序烧录窗口");
        window->set_position(Vector2i(15, 15));
        window->set_layout(new GroupLayout());

        Widget *tty_dev_widget = new Widget(window);
        tty_dev_widget->set_layout(new BoxLayout(Orientation::Horizontal,
                                       Alignment::Fill, 0, 6));
        m_tty_dev_widget_cobo =
            new ComboBox(tty_dev_widget, {"/dev/ttyUSB0"});

        Button *tty_btn;
        tty_btn = new Button(tty_dev_widget, "打开");
        tty_btn->set_callback([&] {
            open_uart_dev(m_tty_dev_widget_cobo->items().at(m_tty_dev_widget_cobo->selected_index()).c_str());
            printf("open device:%s\n", m_tty_dev_widget_cobo->items().at(m_tty_dev_widget_cobo->selected_index()).c_str());
        });
        tty_btn = new Button(tty_dev_widget, "关闭");
        tty_btn->set_callback([&] {
            printf("close device:%s\n", m_tty_dev_widget_cobo->items().at(m_tty_dev_widget_cobo->selected_index()).c_str());
        });

        /* No need to store a pointer, the data structure will be automatically
           freed when the parent window is deleted */
        Button *b = new Button(window, "zlg");
        b->set_callback([] { std::cout << "pushed!" << std::endl; });
        b->set_tooltip("zlg 暂停 U-Boot");

        /* Alternative construction notation using variadic template */
        b = window->add<Button>("U-Boot修改IP", FA_ETHERNET);
        b->set_background_color(Color(0, 0, 255, 25));
        b->set_callback([] { std::cout << "pushed!" << std::endl; });
        b->set_tooltip("在U-Boot下修改IP,为更新内核做准备");

        window = new Window(this, "监控窗口");
        window->set_position(Vector2i(200, 15));
        window->set_layout(new GroupLayout());

        new Label(window, "File dialog", "sans-bold");
        tools = new Widget(window);
        tools->set_layout(new BoxLayout(Orientation::Horizontal,
                                       Alignment::Fill, 0, 6));
        b = new Button(tools, "Open");
        b->set_callback([&] {
            std::cout << "File dialog result: " << file_dialog(
                    { {"png", "Portable Network Graphics"}, {"txt", "Text file"} }, false) << std::endl;
        });
        b = new Button(tools, "Save");
        b->set_callback([&] {
            std::cout << "File dialog result: " << file_dialog(
                    { {"png", "Portable Network Graphics"}, {"txt", "Text file"} }, true) << std::endl;
        });

        new Label(window, "Progress bar", "sans-bold");
        m_progress = new ProgressBar(window);

        new Label(window, "设备日志信息", "sans-bold");

        VScrollPanel *vscroll = new VScrollPanel(window);
        //vscroll->set_size(Vector2i(200, 200));
        //vscroll->set_scroll(1.0);
        m_log_text_area = new TextArea(vscroll);
        /* 和字体大小保持一致 */
        m_log_text_area->set_max_size(500);
        m_log_text_area->set_padding(window->font_size());
        m_log_text_area->set_background_color(Color(0,0X85,0XAD,100));
        m_log_text_area->set_selectable(true);
        m_log_text_area->append("RED SAY HELLO \n");
        m_log_text_area->append("RED SAY HELLO \n");

        new Label(window, "设备日志信息结束行", "sans-bold");

        perform_layout();

        /* All NanoGUI widgets are initialized at this point. Now
           create shaders to draw the main window contents.

           NanoGUI comes with a simple wrapper around OpenGL 3, which
           eliminates most of the tedious and error-prone shader and buffer
           object management.
        */

        m_render_pass = new RenderPass({ this });
        m_render_pass->set_clear_color(0, Color(0.3f, 0.3f, 0.32f, 1.f));

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

    void append_log_msg(std::string msg){
        m_log_text_area->append(msg);
    }

    virtual bool keyboard_event(int key, int scancode, int action, int modifiers) {
        if (Screen::keyboard_event(key, scancode, action, modifiers))
            return true;
        if (key == GLFW_KEY_ESCAPE && action == GLFW_PRESS) {
            set_visible(false);
            return true;
        }
        return false;
    }

    virtual void draw(NVGcontext *ctx) {
        /* Animate the scrollbar */
        m_progress->set_value(std::fmod((float) glfwGetTime() / 10, 1.0f));

        /* Draw the user interface */
        Screen::draw(ctx);
    }

    virtual void draw_contents() {
        Matrix4f mvp = Matrix4f::scale(Vector3f(
                           (float) m_size.y() / (float) m_size.x() * 0.25f, 0.25f, 0.25f)) *
                       Matrix4f::rotate(Vector3f(0, 0, 1), (float) glfwGetTime());

        m_shader->set_uniform("mvp", mvp);

        m_render_pass->resize(framebuffer_size());
        m_render_pass->begin();

        m_shader->begin();
        m_shader->draw_array(Shader::PrimitiveType::Triangle, 0, 6, true);
        m_shader->end();

        m_render_pass->end();
    }
private:
    ProgressBar *m_progress;
    TextArea *m_log_text_area;
    ComboBox *m_tty_dev_widget_cobo;
    ref<Shader> m_shader;
    ref<RenderPass> m_render_pass;

    using ImageHolder = std::unique_ptr<uint8_t[], void(*)(void*)>;
    std::vector<std::pair<ref<Texture>, ImageHolder>> m_images;
    int m_current_image;
};
