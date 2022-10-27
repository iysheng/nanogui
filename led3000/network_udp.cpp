/******************************************************************************
* File:             network_udp.cpp
*
* Author:           Yangtyongsheng@jari.cn  
* Created:          10/26/22 
* Description:      udp class source file
*****************************************************************************/

#include "network_udp.h"

using namespace std;
NetworkUdp::NetworkUdp(string dstip, uint16_t source_port, uint16_t dst_port)
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
    m_socket = socket(m_addrinfo->ai_family, SOCK_DGRAM | SOCK_CLOEXEC, IPPROTO_UDP);
    if(m_socket == -1)
    {
        freeaddrinfo(m_addrinfo);
        printf("could not create socket address or port: %s:%u\n", dstip.c_str(), dst_port);
        return;
    }

    m_source_sin.sin_addr.s_addr = htonl(INADDR_ANY);
    m_source_sin.sin_family = AF_INET;
    m_source_sin.sin_port = htons(source_port);
    r = bind(m_socket, (struct sockaddr *) &m_source_sin, sizeof m_source_sin);
    if(r != 0)
    {
        freeaddrinfo(m_addrinfo);
        close(m_socket);
        printf("could not bind UDP socket with port:%u\n", source_port);
    }
    printf("Create socket success.\n");
}

void NetworkUdp::send2server(char *buffer, uint16_t len, int flags)
{
    int ret;
    ret = sendto(m_socket, buffer, len, flags, m_addrinfo->ai_addr, m_addrinfo->ai_addrlen);
    if (-1 == ret)
    {
        printf("Failed send msg to server :%d\n", errno);
    }
}

NetworkUdp::~NetworkUdp()
{
    freeaddrinfo(m_addrinfo);
    close(m_socket);
}
