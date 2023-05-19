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

#include <termios.h>
#include <unistd.h>

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
#include <boost/crc.hpp>
#include <boost/cstdint.hpp>
#include <algorithm>
#include <arpa/inet.h>

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

class RedBurntool : public Screen {
public:
    RedBurntool() : Screen(Vector2i(1024, 768), "CheckCRC@Red")
    {
        inc_ref();
        Widget *tools;
        Window *window = new Window(this, "CRC 计算");
        window->set_position(Vector2i(15, 15));
        window->set_layout(new GroupLayout());

        Widget *tty_dev_widget = new Widget(window);
        tty_dev_widget->set_layout(new BoxLayout(Orientation::Horizontal,
                                       Alignment::Fill, 0, 6));
        Button *b = new Button(window, "CRC-16/MODBUS");
        b->set_tooltip("16 8005 FFFF 0000 true true");
        b->set_callback([this] {
            std::string ans = this->raw_data_src();
            unsigned char data[32] = {0X31, 0X32, 0X33, 0X34, 0X35, 0X36, 0X37, 0X38, 0X39};
            int data_len;
            char crc_ans_buffer[32] = {0};

            boost::crc_optimal<16, 0X8005, 0XFFFF, 0, true, true> crc16_modebus;
            data_len = extract_with_delimiter(ans, ' ', data);
            if (!data_len)
                return;
            crc16_modebus = std::for_each( data, data + data_len, crc16_modebus );
            printf("%s 的结果是 %#X\n", ans.c_str(), crc16_modebus());
            snprintf(crc_ans_buffer, sizeof(crc_ans_buffer), "%02X %02X", htons(crc16_modebus()) >> 8 & 0XFF, htons(crc16_modebus()) & 0XFF);
            this->append_log_msg(data, data_len);
            /* 需要添加一个额外的空格,不然不能正常显示换行 */
            this->append_log_msg(crc_ans_buffer);
            this->append_log_msg(" \n");
        });

        b = new Button(window, "CRC-8");
        b->set_tooltip("8 07 00 00 false false");
        b->set_callback([this] {
            std::string ans = this->raw_data_src();
            unsigned char data[32] = {0X31, 0X32, 0X33, 0X34, 0X35, 0X36, 0X37, 0X38, 0X39};
            int data_len;
            char crc_ans_buffer[32] = {0};

            boost::crc_optimal<8, 0X07, 0X00, 0X00, false, false> crc8;
            data_len = extract_with_delimiter(ans, ' ', data);
            if (!data_len)
                return;
            crc8 = std::for_each( data, data + data_len, crc8 );
            printf("%s 的结果是 %#X\n", ans.c_str(), crc8());
            snprintf(crc_ans_buffer, sizeof(crc_ans_buffer), "%02X", crc8());
            this->append_log_msg(data, data_len);
            /* 需要添加一个额外的空格,不然不能正常显示换行 */
            this->append_log_msg(crc_ans_buffer);
            this->append_log_msg(" \n");
        });

        /* Alternative construction notation using variadic template */
        window = new Window(this, "数据输入窗口");
        m_progress = new ProgressBar(window);
        window->set_position(Vector2i(300, 15));
        window->set_layout(new GroupLayout());

        Widget *widget_bts = new Widget(window);
        widget_bts->set_layout(new BoxLayout(Orientation::Horizontal,
            Alignment::Fill, 0, 6));
        Button *clear_bt = new Button(widget_bts, "清除");
        clear_bt->set_callback([this]{
             if (this->m_log_text_area)
                this->m_log_text_area->clear();
        });

        VScrollPanel *vscroll = new VScrollPanel(window);
        m_log_text_area = new TextArea(vscroll);
        /* 和字体大小保持一致 */
        m_log_text_area->set_max_size(500);
        m_log_text_area->set_padding(window->font_size());
        m_log_text_area->set_background_color(Color(0,0X85,0XAD,100));
        m_log_text_area->set_selectable(true);

        new Label(window, "原始数据输入框", "sans-bold");
        m_text_box_src = new TextBox(window, "");
        m_text_box_src->set_editable(true);
        m_text_box_src->set_alignment(TextBox::Alignment::Left);

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

    int extract_with_delimiter(std::string src, char delimiter, uint8_t *dst)
    {
        int counts = 0;
        std::size_t index = 0;

        while(src.size())
        {
            index = src.find(delimiter);
            if (!index)
            {
                src = src.substr(1 + index, src.size());
                continue;
            }
            else if (index == std::string::npos)
            {
                index = src.size();
                dst[counts++] = std::stoi(src, &index, 16);
                src.clear();
            }
            else
            {
                dst[counts++] = std::stoi(src, &index, 16);
                src = src.substr(1 + index, src.size());
            }
            //printf("dst[%d]=%#X\n", counts - 1, dst[counts - 1]);
        }

        return counts;
    }

    void append_log_msg(std::string msg){
        m_log_text_area->append(msg);
    }

    void append_log_msg(unsigned char const *msg, int len){
        char * buffer = (char *)malloc(len * sizeof(char) * 4);
        int i = 0;

        if (!buffer)
        {
            printf("no enough memory\n");
            return;
        }

        for (; i < len; i++)
        {
            snprintf(&buffer[i * 3], 4, "%02X ", msg[i]);
        }

        printf("format buffer:%s [END] i=%d\n", buffer, i);
        m_log_text_area->append(buffer);
        free(buffer);
    }

    const std::string &raw_data_src(void)
    {
        return m_text_box_src->value();
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
    TextBox  *m_text_box_src;
    ComboBox *m_tty_dev_widget_cobo;
    ref<Shader> m_shader;
    ref<RenderPass> m_render_pass;

    using ImageHolder = std::unique_ptr<uint8_t[], void(*)(void*)>;
    std::vector<std::pair<ref<Texture>, ImageHolder>> m_images;
    int m_current_image;
    int m_tty_fd;
};
