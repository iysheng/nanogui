/******************************************************************************
* File:             debug.h
*
* Author:           yangyongsheng@jari.cn
* Created:          10/27/22 
* Description:      调试相关的头文件  
*****************************************************************************/

#pragma once

#include <stdio.h>

#define red_debug_lite(format, ...) {fprintf(stderr, "[%s@%d] " format "\n", __FUNCTION__, __LINE__, ##__VA_ARGS__);}
