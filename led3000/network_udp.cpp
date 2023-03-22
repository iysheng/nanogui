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

int NetworkUdp::gs_socket_udp_alone_fd = -1;

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
    if (!m_addrinfo) {
        red_debug_lite("invalid m_addrinfo for retry connect");
        return -1;
    }

    m_socket = socket(m_addrinfo->ai_family, SOCK_DGRAM | SOCK_CLOEXEC, IPPROTO_UDP);
    if (m_socket == -1) {
        //RedDebug::log("could not retry create udp socket\n");
        return -2;
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
    if (r != 0 && errno != EADDRINUSE) {
        close(m_socket);
        m_socket = -1;
        RedDebug::err("Red could not try to bind UDP socket with port err=%d addinuse:%d", errno, EADDRINUSE);
        return -3;
    }
    else if (errno == EADDRINUSE && NetworkUdp::gs_socket_udp_alone_fd > 0)
    {
        close(m_socket);
        m_socket = NetworkUdp::gs_socket_udp_alone_fd;
        red_debug_lite("Use gs_socket_udp_alone_fd instead");
    }
    else if (0 == r)
    {
        /* 根据对方要求修改 TTL */
        char ttl = 8;
        if (setsockopt(m_socket, IPPROTO_IP, IP_MULTICAST_TTL, (char *)&ttl, sizeof(ttl)) != 0) {
            RedDebug::log("Failed set ttl to %d err:%d", ttl, errno);
            close(m_socket);
            m_socket = -1;
            return -4;
        }
    }

    int ret;
    struct in_addr dstip_in_addr;
    dstip_in_addr.s_addr = ((sockaddr_in *)(this->addrinfo()->ai_addr))->sin_addr.s_addr;
    struct ip_mreq req = {0};//结构体对象
    req.imr_multiaddr.s_addr = ((sockaddr_in *)(this->addrinfo()->ai_addr))->sin_addr.s_addr;//设置组播地址
    req.imr_interface.s_addr = INADDR_ANY;//本地地址
    ret  = setsockopt(m_socket, IPPROTO_IP, IP_ADD_MEMBERSHIP, &(req), sizeof(req));//IPPROTO_IP通过IP组播，IP_ADD_MEMBERSHIP -> 加入组播组
    if (ret < 0) {
        RedDebug::log("add multi to %s error:%d", inet_ntoa(dstip_in_addr), errno);
        close(m_socket);
        m_socket = -1;
        return -5;
    } else {
        NetworkUdp::gs_socket_udp_alone_fd = m_socket;
        RedDebug::log("add multi to %s success", inet_ntoa(dstip_in_addr));
    }

    return m_socket;
}

NetworkUdp::NetworkUdp(string dstip, uint16_t source_port, uint16_t dst_port, int socket_fd): m_sn(1), m_socket(socket_fd)
{
    char decimal_port[16];
    snprintf(decimal_port, sizeof(decimal_port), "%u", source_port);
    decimal_port[sizeof(decimal_port) / sizeof(decimal_port[0]) - 1] = '\0';
    struct addrinfo hints;
    memset(&hints, 0, sizeof(hints));
    hints.ai_family = AF_INET;
    hints.ai_socktype = SOCK_DGRAM;
    hints.ai_protocol = IPPROTO_UDP;
    /*
     * m_addrinfo : 保存目标 ip 和端口信息
     * */
    int r(getaddrinfo(dstip.c_str(), decimal_port, &hints, &m_addrinfo));
    if (r != 0 || m_addrinfo == NULL) {
        red_debug_lite("invalid address or port: %s:%u\n", dstip.c_str(), source_port);
        return;
    } else {
        RedDebug::log("udp socket will connect to:%s", inet_ntoa(((sockaddr_in *)m_addrinfo->ai_addr)->sin_addr));
    }

    if (m_socket > 0) {
        RedDebug::log("use origin socket fd:%d", m_socket);
    } else {
        m_socket = socket(m_addrinfo->ai_family, SOCK_DGRAM | SOCK_CLOEXEC, IPPROTO_UDP);
        if (m_socket == -1) {
            freeaddrinfo(m_addrinfo);
            red_debug_lite("could not create socket address or port: %s:%u\n", dstip.c_str(), dst_port);
            return;
        }
#ifdef SOCKET_NO_BLOCK
        struct timeval tv;
        tv.tv_sec = 3;
        tv.tv_usec = 0;
        /* 设置接收超时 */
        setsockopt(m_socket, SOL_SOCKET, SO_RCVTIMEO, &tv, sizeof(tv));
#endif
        m_source_sin.sin_addr.s_addr = htonl(INADDR_ANY);//inet_addr(dstip.c_str());
        m_source_sin.sin_family = AF_INET;
        m_source_sin.sin_port = htons(source_port);
        r = bind(m_socket, (struct sockaddr *) &m_source_sin, sizeof m_source_sin);
        if (r != 0) {
            freeaddrinfo(m_addrinfo);
            close(m_socket);
            m_socket = -1;
            red_debug_lite("could not bind UDP socket with port when contruct:%u\n", source_port);
            return;
        } else {
            red_debug_lite("Create socket success.\n");
        }
        /* 根据对方要求修改 TTL */
        char ttl = 8;
        if (setsockopt(m_socket, IPPROTO_IP, IP_MULTICAST_TTL, (char *)&ttl, sizeof(ttl)) != 0) {
            RedDebug::log("Failed set ttl to %d err:%d", ttl, errno);
            close(m_socket);
            m_socket = -1;
            return;
        } else {
            RedDebug::log("Success set ttl to %d", ttl);
        }
    }

    int ret;
    struct ip_mreq req = {0};//结构体对象
    req.imr_multiaddr.s_addr = inet_addr(dstip.c_str());//设置组播地址
    req.imr_interface.s_addr = INADDR_ANY;//本地地址
    ret  = setsockopt(m_socket, IPPROTO_IP, IP_ADD_MEMBERSHIP, &(req), sizeof(req));//IPPROTO_IP通过IP组播，IP_ADD_MEMBERSHIP -> 加入组播组
    if (ret < 0) {
        RedDebug::log("add multi to %s error:%d", dstip.c_str(), errno);
        close(m_socket);
        m_socket = -1;
        return;
    } else {
        NetworkUdp::gs_socket_udp_alone_fd = m_socket;
        RedDebug::log("add multi to %s success", dstip.c_str());
    }
}

int NetworkUdp::send2server(char *buffer, uint16_t len, int flags)
{
#define MAX_BUFFER_LEN    256
    int ret;
    if ((m_socket <= 0) && (try_to_connect() <= 0)) {
        //RedDebug::log("invalid udp socket and try to connect server failed");
        return -1;
    }
    ret = sendto(m_socket, buffer, len, flags, m_addrinfo->ai_addr, m_addrinfo->ai_addrlen);
    if (-1 == ret) {
        RedDebug::log("Send msg to server %s failed:%d\n",
                      inet_ntoa(((sockaddr_in *)m_addrinfo->ai_addr)->sin_addr), errno);
    } else {
        //RedDebug::log("Send msg to server %s success\n", inet_ntoa(((sockaddr_in *)m_addrinfo->ai_addr)->sin_addr));
        RedDebug::hexdump("UDP SEND", (char *)buffer, len);
    }
    if (0 == ++m_sn) {
        m_sn = 1;
    }

    return ret;
}

int NetworkUdp::send2server(char *buffer, uint16_t len, int flags, struct addrinfo *addr)
{
#define MAX_BUFFER_LEN    256
    int ret;
    if ((m_socket <= 0) && (try_to_connect() <= 0)) {
        //RedDebug::log("invalid udp socket and try to connect server failed");
        return -1;
    }
    ret = sendto(m_socket, buffer, len, flags, addr->ai_addr, addr->ai_addrlen);
    if (-1 == ret) {
        RedDebug::log("Send msg to server %s failed:%d\n",
                      inet_ntoa(((sockaddr_in *)addr->ai_addr)->sin_addr), errno);
    } else {
        //RedDebug::log("Send msg to server %s success\n", inet_ntoa(((sockaddr_in *)addr->ai_addr)->sin_addr));
        RedDebug::hexdump("UDP SEND", (char *)buffer, len);
    }
    if (0 == ++m_sn) {
        m_sn = 1;
    }

    return ret;
}

int NetworkUdp::recv_from_server(char *buffer, uint16_t len, int flags)
{
    int ret;
    if ((m_socket <= 0) && (try_to_connect() <= 0)) {
        //RedDebug::log("invalid udp socket and try to connect server failed");
        return -1;
    }
    //RedDebug::log("before udp socket recv:%s %p", inet_ntoa(((sockaddr_in *)m_addrinfo->ai_addr)->sin_addr), m_addrinfo);
    ret = recvfrom(m_socket, buffer, len, flags, NULL, NULL);
    if (-1 == ret) {
        RedDebug::err("Failed recvfrom server :%d %s %p", errno, inet_ntoa(((sockaddr_in *)m_addrinfo->ai_addr)->sin_addr), m_addrinfo);
    } else if (ret > 0) {
        RedDebug::hexdump("RECV_FROM_SEREVR", buffer, ret);
    }

    return ret;
}

NetworkUdp::~NetworkUdp()
{
    freeaddrinfo(m_addrinfo);
    close(m_socket);
}
