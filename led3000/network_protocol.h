/******************************************************************************
* File:             network_protocol.h
*
* Author:           yangyongsheng@jari.cn  
* Created:          10/27/22 
*                   网络协议处理头文件
*****************************************************************************/

#pragma once

/********************  指控主动发送的命令集合  ******************************/
/* 指控问询开机指令 */
#define NETWORK_RECV_PROBE    0XE3
/* 指控引导指令 */
#define NETWORK_RECV_GUIDE    0X81
/* 指控作战干预 */
#define NETWORK_RECV_FORCE    0X05
/* 指控关机发送 */
#define NETWORK_RECV_OFF      0XE4


/********************   发送给主控的命令集合   ******************************/
/* 上报眩目系统工作状态指令 */
#define NETWORK_SEND_STATUS   0XC9
/* 上报转台方位信息指令 */
#define NETWORK_SEND_INFO     0XA1
/* 上报系统关机信息 */
#define NETWORK_SEND_OFF      0XE4
