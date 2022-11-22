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

#define red_debug_lite(format, ...) {fprintf(stderr, "[%s@%d] " format "\n", __FUNCTION__, __LINE__, ##__VA_ARGS__);}
#define RED_DEBUG_PREFIX    "[Red]"

class RedDebug {
    public:
      static void hexdump(char *title, char *buffer, uint16_t len);
      static void print_soft_info(void);
};
