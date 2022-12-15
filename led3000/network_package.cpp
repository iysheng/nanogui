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
#include <sys/types.h>

NetworkPackage::~NetworkPackage()
{
}

NetworkPackage::NetworkPackage(uint32_t src_ip, uint32_t dst_ip, uint8_t sn, uint8_t ack,
  uint8_t flag, uint8_t count,
  char index, char id, char len, int stamp, char * payload):
  m_src_ip_n(src_ip), m_dst_ip_n(dst_ip), m_sn(sn), m_ack(ack), m_flag(flag), m_count(count), 
  m_index(index), m_id(id), m_len(len), m_stamp(stamp)
{
    m_payload_len = MK_PAYLOAD_LEN(len - CSSMXP_MSG_PREFIX);
    memcpy(m_payload, payload, MK_PAYLOAD_LEN(len));
}

NetworkPackage::NetworkPackage(char index, char id, char len, int stamp, char * payload):m_index(index),
  m_id(id), m_len(len), m_stamp(stamp)
{
    //m_src_ip_n = ;
    m_payload_len = MK_PAYLOAD_LEN(len - CSSMXP_MSG_PREFIX);
    memcpy(m_payload, payload, MK_PAYLOAD_LEN(len));
}

int NetworkPackage::convert_from_buffer(char *buffer, short int len)
{
    if (!buffer || len < 16)
    {
        return -1;
    }

    if (len - CSSMXP_MSG_PREFIX != (buffer[2 + CSSMXP_MSG_PREFIX] << 8 | buffer[3 + CSSMXP_MSG_PREFIX]))
    {
        return -2;
    }

    m_len = buffer[0] << 8 | buffer[1];
    m_src_ip_n = buffer[4] << 24 | buffer[5] << 16 | buffer[6] << 8 | buffer[7];
    m_dst_ip_n = buffer[8] << 24 | buffer[9] << 16 | buffer[10] << 8 | buffer[11];
    m_sn = buffer[12];
    m_ack = buffer[13];
    m_flag = buffer[14];
    m_id = buffer[15];

    m_index = buffer[CSSMXP_MSG_PREFIX + 0];
    m_id = buffer[CSSMXP_MSG_PREFIX + 1];
    m_stamp = buffer[CSSMXP_MSG_PREFIX + 4] << 24 | buffer[CSSMXP_MSG_PREFIX + 5] << 16 | buffer[CSSMXP_MSG_PREFIX + 6] << 8 | buffer[CSSMXP_MSG_PREFIX + 7];
    m_payload_len = MK_PAYLOAD_LEN(len - CSSMXP_MSG_PREFIX);
    memcpy(m_payload, buffer + CSSMXP_MSG_PREFIX + 8, m_payload_len);
    return 0;
}

int NetworkPackage::convert_to_buffer(char *buffer, short int len)
{
  if (len < m_len)
  {
    red_debug_lite("buffer too small");
    return -ENOMEM;
  }

  buffer[0] = m_len >> 8 & 0xff;
  buffer[1] = m_len & 0xff;
  buffer[2] = 0x00;
  buffer[3] = 0x00;
  buffer[4] = m_src_ip_n >> 24;
  buffer[5] = m_src_ip_n >> 16;
  buffer[6] = m_src_ip_n >> 8;
  buffer[7] = m_src_ip_n;
  buffer[8] = m_dst_ip_n >> 24;
  buffer[9] = m_dst_ip_n >> 16;
  buffer[10] = m_dst_ip_n >> 8;
  buffer[11] = m_dst_ip_n;
  buffer[12] = m_sn;
  buffer[13] = m_ack;
  buffer[14] = m_flag;
  buffer[15] = m_count;

  buffer[CSSMXP_MSG_PREFIX + 0] = m_index;
  buffer[CSSMXP_MSG_PREFIX + 1] = m_id;
  buffer[CSSMXP_MSG_PREFIX + 2] = m_len - CSSMXP_MSG_PREFIX >> 8 & 0xff;
  buffer[CSSMXP_MSG_PREFIX + 3] = m_len - CSSMXP_MSG_PREFIX & 0xff;
  buffer[CSSMXP_MSG_PREFIX + 4] = m_stamp >> 24 & 0xff;
  buffer[CSSMXP_MSG_PREFIX + 5] = m_stamp >> 16 & 0xff;
  buffer[CSSMXP_MSG_PREFIX + 6] = m_stamp >> 8 & 0xff;
  buffer[CSSMXP_MSG_PREFIX + 7] = m_stamp & 0xff;
  memcpy(&buffer[CSSMXP_MSG_PREFIX + 8], m_payload, m_payload_len);

  return 0;
}
