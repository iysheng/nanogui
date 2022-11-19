/******************************************************************************
* File:             network_tcp.cpp
*
* Author:           Yangtyongsheng@jari.cn  
* Created:          10/26/22 
* Description:      tcp class source file
*****************************************************************************/

#include "network_tcp.h"
#include <debug.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>

using namespace std;

/* 默认设置 socket 为无阻塞模式 */
#define SOCKET_NO_BLOCK

NetworkTcp& NetworkTcp::operator=(NetworkTcp& ref) noexcept
{
    m_socket = ref.get_socket();
    m_addrinfo = ref.addrinfo();
    m_source_sin = ref.source_sin();
    return *this;
}

int NetworkTcp::stamp()
{
    static int stamp = 0;

    stamp += 10;

    return stamp;
}

NetworkTcp::NetworkTcp(string dstip, uint16_t dst_port):m_index(0)
{
    char decimal_port[16];
    snprintf(decimal_port, sizeof(decimal_port), "%u", dst_port);
    decimal_port[sizeof(decimal_port) / sizeof(decimal_port[0]) - 1] = '\0';
    struct addrinfo hints;
    memset(&hints, 0, sizeof(hints));
    hints.ai_family = AF_INET;
    hints.ai_socktype = SOCK_STREAM;
    hints.ai_protocol = IPPROTO_TCP;
    int r(getaddrinfo(dstip.c_str(), decimal_port, &hints, &m_addrinfo));
    if(r != 0 || m_addrinfo == NULL)
    {
        printf("invalid address or port: %s:%u\n", dstip.c_str(), dst_port);
        return;
    }
    else
    {
        red_debug_lite("tcp socket create to:%s success. -------------------------------", inet_ntoa(((sockaddr_in *)m_addrinfo->ai_addr)->sin_addr));
    }
    m_socket = socket(m_addrinfo->ai_family, SOCK_STREAM | SOCK_CLOEXEC, IPPROTO_TCP);
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

    r = connect(m_socket, m_addrinfo->ai_addr, m_addrinfo->ai_addrlen);
    if(r != 0)
    {
        freeaddrinfo(m_addrinfo);
        close(m_socket);
        printf("could not bind TCP socket with port:%u\n", dst_port);
    }
    printf("Create socket success.\n");
}

NetworkTcp::NetworkTcp(string dstip, uint16_t source_port, uint16_t dst_port):m_index(0)
{
    char decimal_port[16];
    snprintf(decimal_port, sizeof(decimal_port), "%u", source_port);
    decimal_port[sizeof(decimal_port) / sizeof(decimal_port[0]) - 1] = '\0';
    struct addrinfo hints;
    memset(&hints, 0, sizeof(hints));
    hints.ai_family = AF_INET;
    hints.ai_socktype = SOCK_STREAM;
    hints.ai_protocol = IPPROTO_TCP;
    int r(getaddrinfo(dstip.c_str(), decimal_port, &hints, &m_addrinfo));
    if(r != 0 || m_addrinfo == NULL)
    {
        printf("invalid address or port: %s:%u\n", dstip.c_str(), source_port);
        return;
    }
    else
    {
        red_debug_lite("tcp socket create to:%s success. -------------------------------", inet_ntoa(((sockaddr_in *)m_addrinfo->ai_addr)->sin_addr));
    }
    m_socket = socket(m_addrinfo->ai_family, SOCK_STREAM | SOCK_CLOEXEC, IPPROTO_TCP);
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

    m_source_sin.sin_addr.s_addr = htonl(INADDR_ANY);
    m_source_sin.sin_family = AF_INET;
    m_source_sin.sin_port = htons(source_port);
    r = connect(m_socket, m_addrinfo->ai_addr, m_addrinfo->ai_addrlen);
    if(r != 0)
    {
        freeaddrinfo(m_addrinfo);
        close(m_socket);
        printf("could not bind UDP socket with port:%u\n", source_port);
    }
    printf("Create socket success.\n");
}

int NetworkTcp::send2server(char *buffer, uint16_t len, int flags)
{
    int ret;
    ret = send(m_socket, buffer, len, flags);

    if (-1 == ret)
    {
        printf("Failed send msg to server :%d\n", errno);
    }
    m_index++;

    return ret;
}

int NetworkTcp::recv_from_server(char *buffer, uint16_t len, int flags)
{
    int ret;
    ret = recvfrom(m_socket, buffer, len, flags, m_addrinfo->ai_addr, &(m_addrinfo->ai_addrlen));
    if (-1 == ret)
    {
        /* 为了测试暂时屏蔽该错误打印 */
        //printf("Failed recvfrom server :%d\n", errno);
    }
    else
    {
        red_debug_lite("%s", inet_ntoa(((sockaddr_in *)m_addrinfo->ai_addr)->sin_addr));
        RedDebug::hexdump("TCP RECV_FROM_SEREVR", buffer, ret);
    }

    return ret;
}

NetworkTcp::~NetworkTcp()
{
    freeaddrinfo(m_addrinfo);
    close(m_socket);
}
