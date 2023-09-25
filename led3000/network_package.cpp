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
#include <sys/socket.h>
#include <sys/time.h>
#include <netinet/in.h>
#include <arpa/inet.h>

time_t NetworkPackage::s_stamp_stand = 0;

extern bool check_time_sync_valid(void);
void NetworkPackage::init_stamp_stand(void)
{
    struct tm tm4sync = {0};
    time_t time4stand = 0;

    time4stand = time(NULL);
    if (-1 == time4stand)
    {
        red_debug_lite("failed do time function, err:%d", errno);
        return;
    }

    if (NULL == gmtime_r(&time4stand, &tm4sync))
    {
        RedDebug::err("Failed gmtime_r, err:%d", errno);
        return;
    }
    else
    {
        tm4sync.tm_sec = 0;
        tm4sync.tm_min = 0;
        tm4sync.tm_hour = 0;
        NetworkPackage::s_stamp_stand = mktime(&tm4sync);
        NetworkPackage::s_stamp_stand *= 1000;
        red_debug_lite("init stand_stamp raw:%ld ok -------------", time4stand);
        red_debug_lite("init stand_stamp:%ldok", NetworkPackage::s_stamp_stand);
    }
}

NetworkPackage::~NetworkPackage()
{
}

NetworkPackage::NetworkPackage(uint32_t src_ip, uint32_t dst_ip, uint8_t sn, uint8_t ack,
                               uint8_t flag, uint8_t count,
                               char index, char id, char len, int stamp, char * payload):
    m_src_ip_n(src_ip), m_dst_ip_n(dst_ip), m_sn(sn), m_ack(ack), m_flag(flag), m_count(count),
    m_index(index), m_id(id), m_len(len), m_stamp(stamp)
{
    struct timeval tv;
    time_t t_stamp_ms = 0;

    if (check_time_sync_valid() && 0 == gettimeofday(&tv, NULL)) {
        t_stamp_ms = tv.tv_sec * 1000 ;
        if (t_stamp_ms > NetworkPackage::s_stamp_stand)
        {
            if (t_stamp_ms - NetworkPackage::s_stamp_stand < 24 * 60 * 60 * 1000)
            {
                m_stamp = t_stamp_ms - NetworkPackage::s_stamp_stand;
                /* 但是是 0.1 ms */
                m_stamp *= 10;
                m_stamp += tv.tv_usec / 100;
            }
        }
        else
        {
            m_stamp = 0;
        }
    }
    else
    {
        m_stamp = 0XFFFFFFFF;
    }
    /* 强制源头地址 */
    m_src_ip_n = ntohl(inet_addr("168.9.0.1"));
    if (len - CSSMXP_MSG_PREFIX >= CSSMXP_PACKAGE_PREFIX) {
        m_payload_len = MK_PAYLOAD_LEN(len - CSSMXP_MSG_PREFIX);
        memcpy(m_payload, payload, m_payload_len);
    } else {
        m_payload_len = 0;
        m_count = 0;
    }
}

NetworkPackage::NetworkPackage(char index, char id, char len, int stamp, char * payload): m_index(index),
    m_id(id), m_len(len), m_stamp(stamp)
{
    struct timeval tv;
    time_t t_stamp_ms = 0;

    if (check_time_sync_valid() && 0 == gettimeofday(&tv, NULL)) {
        t_stamp_ms = tv.tv_sec * 1000 ;
        if (t_stamp_ms > NetworkPackage::s_stamp_stand)
        {
            if (t_stamp_ms - NetworkPackage::s_stamp_stand < 24 * 60 * 60 * 1000)
            {
                m_stamp = t_stamp_ms - NetworkPackage::s_stamp_stand;
                /* 但是是 0.1 ms */
                m_stamp *= 10;
                m_stamp += tv.tv_usec / 100;
            }
        }
        else
        {
            m_stamp = 0;
        }
    }
    else
    {
        m_stamp = 0XFFFFFFFF;
    }
    /* net to host */
    m_src_ip_n = ntohl(inet_addr("168.9.0.1"));
    m_dst_ip_n = 0X00; //ntohl(inet_addr("168.9.0.1"));
    if (len - CSSMXP_MSG_PREFIX > CSSMXP_PACKAGE_PREFIX) {
        m_payload_len = MK_PAYLOAD_LEN(len - CSSMXP_MSG_PREFIX);
        memcpy(m_payload, payload, MK_PAYLOAD_LEN(len));
    } else {
        m_payload_len = 0;
    }
}

int NetworkPackage::convert_from_buffer(char *buffer, short int len)
{
    if (!buffer || len < 16) {
        return -1;
    }

    if (len != 16 && (len - CSSMXP_MSG_PREFIX != (buffer[2 + CSSMXP_MSG_PREFIX] << 8 | buffer[3 + CSSMXP_MSG_PREFIX])) )
    {
        RedDebug::hexdump("errPackage", buffer, len);
        return -2;
    }

    m_len = buffer[0] << 8 | buffer[1];
    /* 这里可以拿到发送方的地址 */
    m_src_ip_n = buffer[4] << 24 | buffer[5] << 16 | buffer[6] << 8 | buffer[7];
    //m_dst_ip_n = buffer[8] << 24 | buffer[9] << 16 | buffer[10] << 8 | buffer[11];
    m_sn = buffer[12];
    m_ack = buffer[13];
    m_flag = buffer[14];
    m_count = buffer[15];

    m_index = buffer[CSSMXP_MSG_PREFIX + 0];
    m_id = buffer[CSSMXP_MSG_PREFIX + 1];
    m_stamp = buffer[CSSMXP_MSG_PREFIX + 4] << 24 | buffer[CSSMXP_MSG_PREFIX + 5] << 16 | buffer[CSSMXP_MSG_PREFIX + 6] << 8 | buffer[CSSMXP_MSG_PREFIX + 7];
    /* 时间同步信息,跳过 4 个字节,其中时间戳的 4 个字节保存的是
     * 秒,分,时,时区 4 个信息, 在解析的时候需要特殊处理 m_stamp 字段
     * */
    m_payload_len = MK_PAYLOAD_LEN(len - CSSMXP_MSG_PREFIX);

    /* correct payload length */
    if (m_payload_len > 0)
        memcpy(m_payload, buffer + CSSMXP_MSG_PREFIX + 8, m_payload_len);
    else
        m_payload_len = 0;
    return 0;
}

int NetworkPackage::convert_to_buffer(char *buffer, short int len)
{
    if (len < m_len) {
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

    if (m_len - CSSMXP_MSG_PREFIX > 0) {
        buffer[CSSMXP_MSG_PREFIX + 0] = m_index;
        buffer[CSSMXP_MSG_PREFIX + 1] = m_id;
        buffer[CSSMXP_MSG_PREFIX + 2] = m_len - CSSMXP_MSG_PREFIX >> 8 & 0xff;
        buffer[CSSMXP_MSG_PREFIX + 3] = m_len - CSSMXP_MSG_PREFIX & 0xff;
        buffer[CSSMXP_MSG_PREFIX + 4] = m_stamp >> 24 & 0xff;
        buffer[CSSMXP_MSG_PREFIX + 5] = m_stamp >> 16 & 0xff;
        buffer[CSSMXP_MSG_PREFIX + 6] = m_stamp >> 8 & 0xff;
        buffer[CSSMXP_MSG_PREFIX + 7] = m_stamp & 0xff;
        if (m_payload_len > 0)
            memcpy(&buffer[CSSMXP_MSG_PREFIX + 8], m_payload, m_payload_len);
    }

    return 0;
}
