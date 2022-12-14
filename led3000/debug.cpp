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

static void flush_sigint_handler(int sigarg)
{
    RedDebug::get_logger()->flush();
    exit(EXIT_SUCCESS);
}

int RedDebug::init(char *filename)
{
    signal(SIGINT, flush_sigint_handler);
    try
    {
        RedDebug::m_logger = spdlog::basic_logger_mt("SPD", filename);
        std::cout << "Log init ok at: " << filename << std::endl;
    }
    catch (const spdlog::spdlog_ex &ex)
    {
        std::cout << "Log init failed: " << ex.what() << std::endl;
        return -1;
    }
    return 0;
}

void RedDebug::hexdump(char *title, char *buffer, uint16_t len)
{
    RedDebug::m_logger->info("{0:s} {1:n}", title, spdlog::to_hex(buffer, buffer + len));
}

void RedDebug::print_soft_info()
{
    RedDebug::m_logger->info(RED_DEBUG_PREFIX"Version:{0:x} CommitID:{1:s}\n", LED3000_VERSION, LED3000_COMMIT_ID);
    RedDebug::m_logger->info(RED_DEBUG_PREFIX"CompileTime:{0:s} {1:s}\n", __DATE__, __TIME__);
}

void RedDebug::log(char *fmt, ...)
{
	char buf[4096] = {0};
	va_list args;
	va_start(args, fmt);
	vsnprintf(buf, sizeof(buf) - 1, fmt, args);
	va_end(args);

    RedDebug::m_logger->info(buf);
}
