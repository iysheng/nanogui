/******************************************************************************
* File:             network_protocol.c
*
* Author:           yangyongsheng@jari.cn  
* Created:          10/27/22 
*                   网络协议处理源文件
*****************************************************************************/

#include <network_protocol.h>
#include <network_package.h>
#include <debug.h>

/**
  * @brief 
  * retval .
  */
int handle_with_network_buffer(char *buffer, int size)
{
    int ret;
    NetworkPackage net_package;

    ret = net_package.convert_from_buffer(buffer, size);
    if (ret < 0)
    {
        red_debug_lite("Failed convert buffer to NetworkPackage err=%d.", ret);
        return ret;
    }
    /* TODO debug buffer msg */

    switch (net_package.id())
    {
        case NETWORK_RECV_GUIDE:
            red_debug_lite("TODO with GUIDE");
            break;
        default:
            break;
    }

    return ret;
}
