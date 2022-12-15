/******************************************************************************
* File:             network_protocol.h
*
* Author:           yangyongsheng@jari.cn  
* Created:          10/27/22 
*                   网络协议处理头文件
*****************************************************************************/

#pragma once

/********************  指控主动发送的命令集合  ******************************/
/* 指控问询开机指令 */
#define NETWORK_RECV_PROBE    0XE3
/* 指控引导指令 */
#define NETWORK_RECV_GUIDE    0X81
/* 指控作战干预 */
#define NETWORK_RECV_FORCE    0X05
/* 指控关机发送 */
#define NETWORK_RECV_OFF      0XE4
/* 指控测试发送应答延时 */
#define NETWORK_PINPONG_TEST  0X59
/* 指控舰艇姿态信息发送 */
#define NETWORK_RECV_ATTITUDE_INFO      0XB0


/********************   发送给主控的命令集合   ******************************/
/* 上报眩目系统工作状态指令 */
#define NETWORK_SEND_STATUS   0XC9
/* 上报转台方位信息指令 */
#define NETWORK_SEND_INFO     0XA1
/* 上报系统关机信息 */
#define NETWORK_SEND_OFF      0XE4

typedef enum {
    NETWORK_PROTOCOL_WHITE_LED_TYPE,
    NETWORK_PROTOCOL_GREEN_LED_TYPE,
    NETWORK_PROTOCOL_LED_TYPE_COUNTS,
} network_protocol_led_type_E;

typedef enum {
    NETWORK_PROTOCOL_NORMAL_LED_MODE,
    NETWORK_PROTOCOL_BINK_LED_MODE,
    NETWORK_PROTOCOL_LED_MODE_COUNTS,
} network_protocol_led_mode_E;

/*
 *       |    send          |  recv  |
 * probe | guide_controler | group 1|
 * guide | guide_controler | own |
 * force | guide_controler | own |
 * guide_controler_off | guide_controler | group 1 |
 * probe_respon_status | own | group 2 |
 * probe_respon_info | own | group 2 |
 * owm_off | own | group 1 |
 * */

typedef enum {
    NETWORK_PROTOCOL_TYPE_SEND_GUIDE, /* 组 1 综合指挥网络 224.100.100.101  */
    NETWORK_PROTOCOL_TYPE_SEND_GUIDE_BROADCAST, /* 组 2 指控网络广播组 224.100.100.102 */
    NETWORK_PROTOCOL_TYPE_COUNTS,
} network_protocol_type_E;

class TurntableAttitude {
#define VERCTICAL_INFO_ANGLE_MAX    30
public:
    TurntableAttitude():m_direction_info(0), m_vertical_info(0), m_horizon_info(0), m_delta_info(0){};
    TurntableAttitude(int delta_info):m_direction_info(0), m_vertical_info(0), m_horizon_info(0), m_delta_info(delta_info){};
    void set_delta_info(int delta_info){m_delta_info = delta_info;};
    void update_attitude_info(int direction_info, int vertical_info, int horizon_info)
    {
#if 1
        m_direction_info = direction_info;
#else
        if (direction_info <= 180)
        {
            m_direction_info = direction_info;
        }
        else if (direction_info <= 360)
        {
            m_direction_info = direction_info - 360;
        }
#endif
        m_vertical_info = vertical_info;
        m_horizon_info = horizon_info;
    }
    void correct_target_info(short int &direction_info, short int &vertical_info)
    {
        /* TODO use shipinfo correct target info */
        direction_info -= m_direction_info;
        if (direction_info > 180)
        {
            /* correct direction info with -180 ~ 0 */
            direction_info -= 360;
        }

        vertical_info -= m_vertical_info;
        /* correct with cirtical value */
        if (vertical_info > VERCTICAL_INFO_ANGLE_MAX)
        {
            vertical_info = VERCTICAL_INFO_ANGLE_MAX;
        }
        else if (vertical_info < -VERCTICAL_INFO_ANGLE_MAX)
        {
            vertical_info = -VERCTICAL_INFO_ANGLE_MAX;
        }
    }


private:
    /* 航向角 */
    int m_direction_info;
    /* 横摇角 */
    int m_vertical_info;
    /* 纵摇角 */
    int m_horizon_info;
    int m_delta_info;
};

/**
  * @brief 注册指定类型的 NetworkUdp 句柄
  * @param char fd_type: 
  * @param NetworkUdp &net_fd: 
  * retval Linux/errno.
  */
extern int network_protocol_registe(char fd_type, NetworkUdp &net_fd);

/**
  * @brief 注册 Led3000Window 对象指针
  * @param void *window: 
  * retval Linux/errno.
  */
extern int screen_window_register(void *window);

extern int handle_with_network_buffer(char *buffer, int size);
