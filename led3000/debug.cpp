/******************************************************************************
* File:             debug.cpp
*
* Author:           yangyongsheng@jari.cn
* Created:          11/02/22
*                   debug 源文件
*****************************************************************************/

#include <stdarg.h>
#include <iostream>
#include <stdlib.h>
#include <signal.h>
#include "debug.h"
#include "version.h"

using namespace spdlog;
using namespace std;

std::shared_ptr<logger> RedDebug::m_logger;
std::shared_ptr<logger> RedDebug::m_console_logger;

#define LOGGER_BUFFER_TMP_LEN    256
static void flush_sigint_handler(int sigarg)
{
    RedDebug::get_logger()->flush();
    exit(EXIT_SUCCESS);
}

int RedDebug::init(char *filename)
{
    signal(SIGINT, flush_sigint_handler);
    try {
        RedDebug::m_logger = spdlog::basic_logger_mt("SPD", filename);
        RedDebug::m_console_logger = spdlog::stdout_color_mt("console");
        std::cout << "Log init ok at: " << filename << std::endl;
        /* 设置 console 的打印级别为 warn */
        RedDebug::m_console_logger->set_level(spdlog::level::warn);
    } catch (const spdlog::spdlog_ex &ex) {
        std::cout << "Log init failed: " << ex.what() << std::endl;
        return -1;
    }
    return 0;
}

void RedDebug::hexdump(char *title, char *buffer, uint16_t len)
{
    RedDebug::m_logger->info("{0:s}[{1:d}]{2:n}", title, len, spdlog::to_hex(buffer, buffer + len));
    RedDebug::m_console_logger->info("{0:s}[{1:d}]{2:n}", title, len, spdlog::to_hex(buffer, buffer + len));
}

void RedDebug::print_soft_info()
{
    RedDebug::m_logger->info(RED_DEBUG_PREFIX"Version:{0:x} CommitID:{1:s}\n", LED3000_VERSION, LED3000_COMMIT_ID);
    RedDebug::m_logger->info(RED_DEBUG_PREFIX"CompileTime:{0:s} {1:s}\n", __DATE__, __TIME__);

    RedDebug::m_console_logger->info(RED_DEBUG_PREFIX"Version:{0:x} CommitID:{1:s}\n", LED3000_VERSION, LED3000_COMMIT_ID);
    RedDebug::m_console_logger->info(RED_DEBUG_PREFIX"CompileTime:{0:s} {1:s}\n", __DATE__, __TIME__);
}

void RedDebug::log(char *fmt, ...)
{
    char buf[256] = {0};
    va_list args;
    va_start(args, fmt);
    vsnprintf(buf, sizeof(buf) - 1, fmt, args);
    va_end(args);

    RedDebug::m_logger->info(buf);
    RedDebug::m_console_logger->info(buf);
}

void RedDebug::warn(char *fmt, ...)
{
    char buf[256] = {0};
    va_list args;
    va_start(args, fmt);
    vsnprintf(buf, sizeof(buf) - 1, fmt, args);
    va_end(args);

    RedDebug::m_logger->warn(buf);
    RedDebug::m_console_logger->warn(buf);
}

void RedDebug::err(char *fmt, ...)
{
    char buf[LOGGER_BUFFER_TMP_LEN] = {0};
    va_list args;
    va_start(args, fmt);
    vsnprintf(buf, sizeof(buf) - 1, fmt, args);
    va_end(args);

    RedDebug::m_logger->error(buf);
    RedDebug::m_console_logger->error(buf);
}
