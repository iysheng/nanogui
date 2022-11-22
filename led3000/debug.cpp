/******************************************************************************
* File:             debug.cpp
*
* Author:           yangyongsheng@jari.cn  
* Created:          11/02/22 
*                   debug 源文件
*****************************************************************************/

#include "debug.h"

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
    fprintf(stdout, RED_DEBUG_PREFIX"CompileTime:%s %s", __DATE__, __TIME__);
}
