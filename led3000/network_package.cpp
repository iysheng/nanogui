/******************************************************************************
* File:             network_package.cpp
*
* Author:           yangyongsheng@jari.cn  
* Created:          10/27/22 
* Description:      网络数据包类
*****************************************************************************/

#include "network_package.h"

NetworkPackage::~NetworkPackage()
{
}

int NetworkPackage::convert_from_buffer(char *buffer, short int len)
{
    if (!buffer || len < 8)
    {
        return -1;
    }

    if (len != (buffer[2] << 8 | buffer[3]))
    {
        return -2;
    }

    m_index = buffer[0];
    m_id = buffer[1];
    m_len = buffer[2] << 8 | buffer[3];
    m_stamp = buffer[4] << 24 | buffer[5] << 16 | buffer[6] << 8 | buffer[7];
    m_payload_len = len - 8;
    memcpy(m_payload, buffer + 8, m_payload_len);
    return 0;
}
