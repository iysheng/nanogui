/******************************************************************************
* File:             network_package.cpp
*
* Author:           yangyongsheng@jari.cn  
* Created:          10/27/22 
* Description:      网络数据包类
*****************************************************************************/

#include "network_package.h"
#include <debug.h>
#include <errno.h>

NetworkPackage::~NetworkPackage()
{
}

NetworkPackage::NetworkPackage(char index, char id, char len, int stamp, char * payload):m_index(index),
  m_id(id), m_len(len), m_stamp(stamp)
{
    m_payload_len = MK_PAYLOAD_LEN(len);
    memcpy(m_payload, payload, MK_PAYLOAD_LEN(len));
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
    m_payload_len = MK_PAYLOAD_LEN(len);
    memcpy(m_payload, buffer + 8, m_payload_len);
    return 0;
}

int NetworkPackage::convert_to_buffer(char *buffer, short int len)
{
  if (len < m_len)
  {
    red_debug_lite("buffer too small");
    return -ENOMEM;
  }

  buffer[0] = m_index;
  buffer[1] = m_id;
  buffer[2] = m_len >> 8 & 0xff;
  buffer[3] = m_len & 0xff;
  buffer[4] = m_stamp >> 24 & 0xff;
  buffer[5] = m_stamp >> 16 & 0xff;
  buffer[6] = m_stamp >> 8 & 0xff;
  buffer[7] = m_stamp & 0xff;
  memcpy(&buffer[8], m_payload, m_payload_len);

  return 0;
}
