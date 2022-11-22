/******************************************************************************
* File:             debug.cpp
*
* Author:           yangyongsheng@jari.cn  
* Created:          11/02/22 
*                   debug 源文件
*****************************************************************************/

#include <stdarg.h>
#include "debug.h"
#include "version.h"

using namespace std;

void RedDebug::hexdump(char *title, char *buffer, uint16_t len)
{
    int i;

    printf("[%s(%d)]", title, len);
    if (buffer)
    {
        for (i = 0; i < len; i++)
        {
            printf("%02hhX ", buffer[i]);
        }
        printf("\n");
    }
}

void RedDebug::print_soft_info()
{
    fprintf(stdout, RED_DEBUG_PREFIX"Version:%#x CommitID:%s\n", LED3000_VERSION, LED3000_COMMIT_ID);
    fprintf(stdout, RED_DEBUG_PREFIX"CompileTime:%s %s\n", __DATE__, __TIME__);
}

void RedDebug::log(char *fmt, ...)
{
	char buf[4096] = {0};
	va_list args;
	va_start(args, fmt);
	vsnprintf(buf, sizeof(buf) - 1, fmt, args);
	va_end(args);

	//std::string time_now = FormatDateTime(std::chrono::system_clock::now());
	//fprintf(stdout, "%s\n", time_now.c_str(), buf);
	fprintf(stdout, RED_DEBUG_PREFIX"%s\n", buf);
}
