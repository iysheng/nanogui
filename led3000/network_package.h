/******************************************************************************
* File:             network_package.h
*
* Author:           yangyongsheng@jari.cn  
* Created:          10/27/22 
* Description:      网络数据包类头文件
*****************************************************************************/

#include <string.h>

#pragma once

using namespace std;

#define NETWORK_PACKAGE_PAYLOAD_LEN    32
class NetworkPackage {
public:
    NetworkPackage():m_payload_len(0){memset(m_payload, 0, sizeof(m_payload));};
    NetworkPackage(int payload_len):m_payload_len(payload_len){memset(m_payload, 0, sizeof(m_payload));};
    NetworkPackage(char *buffer, int len);
    ~NetworkPackage();

    int convert_from_buffer(char *buffer, short int len);
    char id(){return m_id;};
private:
    /* 序号 */
    char m_index;
    /* 标识 */
    char m_id;
    /* 帧总长度 */
    short m_len;
    /* 帧负载的长度 */
    short m_payload_len;
    /* 时戳 */
    int m_stamp;
    /* 根据协议，目前 payload 区最长的是眩目拒止协议，长度为 18 字节，这里留足空间 */
    char m_payload[NETWORK_PACKAGE_PAYLOAD_LEN];
};

