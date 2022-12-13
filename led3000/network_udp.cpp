/******************************************************************************
* File:             network_udp.cpp
*
* Author:           Yangtyongsheng@jari.cn  
* Created:          10/26/22 
* Description:      udp class source file
*****************************************************************************/

#include "network_udp.h"
#include <debug.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>

using namespace std;

/* 默认设置 socket 为无阻塞模式 */
#define SOCKET_NO_BLOCK

NetworkUdp& NetworkUdp::operator=(NetworkUdp& ref) noexcept
{
    m_socket = ref.get_socket();
    m_addrinfo = ref.addrinfo();
    m_source_sin = ref.source_sin();
    return *this;
}

int NetworkUdp::stamp()
{
    static int stamp = 0;

    stamp += 10;

    return stamp;
}

int NetworkUdp::try_to_connect(void)
{
    if (!m_addrinfo)
    {
        printf("invalid m_addrinfo for retry connect");
        return -1;
    }

    m_socket = socket(m_addrinfo->ai_family, SOCK_DGRAM | SOCK_CLOEXEC, IPPROTO_UDP);
    if(m_socket == -1)
    {
        //RedDebug::log("could not retry create udp socket\n");
        return -1;
    }
#ifdef SOCKET_NO_BLOCK
    struct timeval tv;
    tv.tv_sec = 3;
    tv.tv_usec = 0;
    /* 设置接收超时 */
    setsockopt(m_socket, SOL_SOCKET, SO_RCVTIMEO, &tv, sizeof(tv));
#endif

    int r;
    r = bind(m_socket, (struct sockaddr *) &m_source_sin, sizeof m_source_sin);
    if(r != 0)
    {
        close(m_socket);
        m_socket = -1;
        printf("could not try to bind UDP socket with port\n");
    }
    else
    {
        printf("Try to create socket success.\n");
    }

    return m_socket;
}
NetworkUdp::NetworkUdp(string dstip, uint16_t source_port, uint16_t dst_port):m_index(0), m_socket(-1)
{
    char decimal_port[16];
    snprintf(decimal_port, sizeof(decimal_port), "%u", source_port);
    decimal_port[sizeof(decimal_port) / sizeof(decimal_port[0]) - 1] = '\0';
    struct addrinfo hints;
    memset(&hints, 0, sizeof(hints));
    hints.ai_family = AF_INET;
    hints.ai_socktype = SOCK_DGRAM;
    hints.ai_protocol = IPPROTO_UDP;
    int r(getaddrinfo(dstip.c_str(), decimal_port, &hints, &m_addrinfo));
    if(r != 0 || m_addrinfo == NULL)
    {
        printf("invalid address or port: %s:%u\n", dstip.c_str(), source_port);
        return;
    }
    else
    {
        red_debug_lite("udp socket create to:%p %ssuccess. -------------------------------", m_addrinfo, inet_ntoa(((sockaddr_in *)m_addrinfo->ai_addr)->sin_addr));
    }
    m_socket = socket(m_addrinfo->ai_family, SOCK_DGRAM | SOCK_CLOEXEC, IPPROTO_UDP);
    if(m_socket == -1)
    {
        freeaddrinfo(m_addrinfo);
        printf("could not create socket address or port: %s:%u\n", dstip.c_str(), dst_port);
        return;
    }
#ifdef SOCKET_NO_BLOCK
    struct timeval tv;
    tv.tv_sec = 3;
    tv.tv_usec = 0;
    /* 设置接收超时 */
    setsockopt(m_socket, SOL_SOCKET, SO_RCVTIMEO, &tv, sizeof(tv));
#endif
    int ret;
    struct ip_mreq req = {0};//结构体对象
	req.imr_multiaddr.s_addr = inet_addr(dstip.c_str());//设置组播地址
	req.imr_interface.s_addr = INADDR_ANY;//本地地址
	ret  = setsockopt(m_socket, IPPROTO_IP, IP_ADD_MEMBERSHIP, &(req), sizeof(req));//IPPROTO_IP通过IP组播，IP_ADD_MEMBERSHIP -> 加入组播组
	if(ret < 0)
    {
        RedDebug::log("add multi error -------------------------\n");
	}
    else
    {
        RedDebug::log("add multi to %s success ------------------------\n", dstip);
    }

    m_source_sin.sin_addr.s_addr = htonl(INADDR_ANY);
    m_source_sin.sin_family = AF_INET;
    m_source_sin.sin_port = htons(source_port);
    r = bind(m_socket, (struct sockaddr *) &m_source_sin, sizeof m_source_sin);
    if(r != 0)
    {
        freeaddrinfo(m_addrinfo);
        close(m_socket);
        m_socket = -1;
        printf("could not bind UDP socket with port:%u\n", source_port);
    }
    else
    {
        printf("Create socket success.\n");
    }
}

int NetworkUdp::send2server(char *buffer, uint16_t len, int flags)
{
    int ret;
    if ((m_socket <= 0) && (try_to_connect() <= 0))
    {
        //RedDebug::log("invalid udp socket and try to connect server failed");
        return -1;
    }
    ret = sendto(m_socket, buffer, len, flags, m_addrinfo->ai_addr, m_addrinfo->ai_addrlen);
    if (-1 == ret)
    {
        printf("Failed send msg to server :%d\n", errno);
    }
    m_index++;

    return ret;
}

int NetworkUdp::recv_from_server(char *buffer, uint16_t len, int flags)
{
    int ret;
    if ((m_socket <= 0) && (try_to_connect() <= 0))
    {
        RedDebug::log("invalid udp socket and try to connect server failed");
        return -1;
    }
    ret = recvfrom(m_socket, buffer, len, flags, m_addrinfo->ai_addr, &(m_addrinfo->ai_addrlen));
    if (-1 == ret)
    {
        /* 为了测试暂时屏蔽该错误打印 */
        printf("Failed recvfrom server :%d\n", errno);
        RedDebug::log("%s %p", inet_ntoa(((sockaddr_in *)m_addrinfo->ai_addr)->sin_addr), m_addrinfo);
    }
    else if(ret > 0)
    {
        red_debug_lite("%s %p", inet_ntoa(((sockaddr_in *)m_addrinfo->ai_addr)->sin_addr), m_addrinfo);
        RedDebug::hexdump("RECV_FROM_SEREVR", buffer, ret);
    }

    return ret;
}

NetworkUdp::~NetworkUdp()
{
    freeaddrinfo(m_addrinfo);
    close(m_socket);
}
