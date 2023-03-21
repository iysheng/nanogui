/******************************************************************************
* File:             network_package.h
*
* Author:           yangyongsheng@jari.cn
* Created:          10/27/22
* Description:      网络数据包类头文件
*****************************************************************************/

#include <string.h>
#include <sys/types.h>
#include <stdint.h>

#pragma once

using namespace std;

/* A full cssmxp message demonstrate
 *
 * msg
 * package
 * package
 * package
 * ...
 * */

/* network cssmxp msg prefix len */
#define CSSMXP_MSG_PREFIX    16

#define MK_MSG_FULL_LEN(x)   (x + CSSMXP_MSG_PREFIX)

/* network cssmxp package prefix len */
#define CSSMXP_PACKAGE_PREFIX    8

/* 获取时戳总长度 */
#define MK_PAYLOAD_LEN(x)    (x - CSSMXP_PACKAGE_PREFIX)

/* 获取软件包总长度 */
#define MK_PACKAGE_LEN(x)    (x + CSSMXP_PACKAGE_PREFIX)

#define NETWORK_PACKAGE_PAYLOAD_LEN    128
#define NETWORK_PACKGE_LEN_MAX         MK_PACKAGE_LEN(NETWORK_PACKAGE_PAYLOAD_LEN)

class NetworkPackage
{
public:
    NetworkPackage(): m_payload_len(0), m_dst_ip_n(0), m_src_ip_n(0) {memset(m_payload, 0, sizeof(m_payload));};
    NetworkPackage(int payload_len): m_payload_len(payload_len) {memset(m_payload, 0, sizeof(m_payload));};
    NetworkPackage(char index, char id, char len, int stamp, char * payload);
    NetworkPackage(uint32_t src_ip, uint32_t dst_ip, uint8_t sn, uint8_t ack,
                   uint8_t flag, uint8_t count,
                   char index, char id, char len, int stamp, char * payload);
    ~NetworkPackage();

    int convert_from_buffer(char *buffer, short int len);
    int convert_to_buffer(char *buffer, short int len);

    char id() {return m_id;};
    char* payload() {return m_payload;};
    short payload_len() {return m_payload_len;};
    short len() {return m_len;};

    uint32_t src_ip_n() {return m_src_ip_n;};
    uint32_t dst_ip_n() {return m_dst_ip_n;};
    void set_dst_ip_n(uint32_t dstip_n) {m_dst_ip_n = dstip_n;};
    uint8_t sn() {return m_sn;};
    uint8_t ack() {return m_ack;};
    uint8_t flag() {return m_flag;};
    uint8_t count() {return m_count;};
    uint32_t stamp() {return m_stamp;};
    /* 全局的参考时戳 */
    static time_t s_stamp_stand;
    static void init_stamp_stand(void);

private:
    uint32_t m_src_ip_n;
    uint32_t m_dst_ip_n;
    uint8_t m_sn;
    uint8_t m_ack;
    uint8_t m_flag;
    uint8_t m_count;

    /* 序号 */
    char m_index;
    /* 标识 */
    char m_id;
    /* 帧总长度 */
    short m_len;
    /* 帧负载的长度， 信息单元长度 - 单元序号(1) - 单元标识(1) - 单元长度(2) - 时戳(4) */
    short m_payload_len;
    /* 时戳 */
    uint32_t m_stamp;
    /* 时戳参考,参考当天 0 点的时戳 */
    uint32_t m_stamp_stand;
    /* 根据协议，目前 payload 区最长的是眩目拒止协议，长度为 18 字节，这里留足空间 */
    char m_payload[NETWORK_PACKAGE_PAYLOAD_LEN];
};

