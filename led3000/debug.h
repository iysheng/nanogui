/******************************************************************************
* File:             debug.h
*
* Author:           yangyongsheng@jari.cn
* Created:          10/27/22 
* Description:      调试相关的头文件  
*****************************************************************************/

#pragma once

#include <stdio.h>
#include <stdint.h>
#include "spdlog/spdlog.h"
#include "spdlog/sinks/basic_file_sink.h"
#include "spdlog/sinks/rotating_file_sink.h"
#include "spdlog/fmt/bin_to_hex.h"

using namespace spdlog;

#define red_debug_lite(format, ...) {fprintf(stderr, "[%s@%d] " format "\n", __FUNCTION__, __LINE__, ##__VA_ARGS__);}
#define RED_DEBUG_PREFIX    "[Red]"

class RedDebug {
    public:
      static std::shared_ptr<logger> get_logger(void){return m_logger;}
      static int init(char *file);
      static void hexdump(char *title, char *buffer, uint16_t len);
      static void print_soft_info(void);
      static void log(char *fmt, ...);
    private:
      static std::shared_ptr<logger> m_logger;
};
