/******************************************************************************
* File:             network_udp.h
*
* Author:           yangyongsheng@jari.cn
* Created:          10/26/22
*                   network udp class head file
*****************************************************************************/

#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>
#include <unistd.h>
#include <string.h>
#include <iostream>

#pragma once

using namespace std;

class NetworkUdp
{
public:
    ~NetworkUdp();
    NetworkUdp(): m_index(0) {};
    NetworkUdp(string dstip, uint16_t source_port, uint16_t dst_port, int socket_fd = -1);
    /* 表示该函数不会抛出异常 */
    NetworkUdp& operator=(NetworkUdp& r) noexcept;

    int try_to_connect(void);
    int send2server(char *buffer, uint16_t len, int flags = 0);
    int send2server(char *buffer, uint16_t len, int flags, struct addrinfo *addr);
    int recv_from_server(char *buffer, uint16_t len, int flags = 0);

    int get_socket() {return m_socket;};
    struct addrinfo * addrinfo() {return m_addrinfo;};
    struct sockaddr_in source_sin() {return m_source_sin;};
    int stamp();
    char index() {return m_index;};
    char sn() {return m_sn;};

private:
    uint32_t m_src_ip_n; /* big endian */
    uint32_t m_dst_ip_n; /* big endian */
    uint8_t m_sn;
    uint8_t m_ack;
    uint8_t m_flag;
    uint8_t m_count; /* msg unit counts */

    int m_index;
    int m_socket;
    /* 目的地址信息 */
    struct addrinfo *m_addrinfo;
    /* 为了绑定源端口号 */
    struct sockaddr_in m_source_sin;
};
