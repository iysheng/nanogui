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

class NetworkUdp {
  public:
    ~NetworkUdp();
    NetworkUdp(){};
    NetworkUdp(string dstip, uint16_t source_port, uint16_t dst_port);
    void send2server(char *buffer, uint16_t len, int flags = 0);
    void recv_from_server(char *buffer, uint16_t len, int flags = 0);
    static void hexdump(char * title, char *buffer, uint16_t len);

  private:
    int m_socket;
    /* 目的地址信息 */
    struct addrinfo *m_addrinfo;
    /* 为了绑定源端口号 */
    struct sockaddr_in m_source_sin;
};
