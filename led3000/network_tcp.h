/******************************************************************************
* File:             network_tcp.h
*
* Author:           yangyongsheng@jari.cn  
* Created:          10/26/22 
*                   network tcp class head file
*****************************************************************************/

#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>
#include <unistd.h>
#include <string.h>
#include <iostream>

#pragma once

using namespace std;

class NetworkTcp {
  public:
    ~NetworkTcp();
    NetworkTcp():m_index(0){};
    NetworkTcp(string dstip, uint16_t dst_port);
    NetworkTcp(string dstip, uint16_t source_port, uint16_t dst_port);
    /* 表示该函数不会抛出异常 */
    NetworkTcp& operator=(NetworkTcp& r) noexcept;

    int send2server(char *buffer, uint16_t len, int flags = 0);
    int recv_from_server(char *buffer, uint16_t len, int flags = 0);

    int get_socket(){return m_socket;};
    struct addrinfo * addrinfo(){return m_addrinfo;};
    struct sockaddr_in source_sin(){return m_source_sin;};
    int stamp();
    char index() {return m_index;};

  private:
    int m_index;
    int m_socket;
    /* 目的地址信息 */
    struct addrinfo *m_addrinfo;
    /* 为了绑定源端口号 */
    struct sockaddr_in m_source_sin;
};
