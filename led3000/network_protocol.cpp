/******************************************************************************
* File:             network_protocol.c
*
* Author:           yangyongsheng@jari.cn  
* Created:          10/27/22 
*                   网络协议处理源文件
*****************************************************************************/

#include <network_udp.h>
#include <network_protocol.h>
#include <network_package.h>
#include <debug.h>
#include <stdint.h>


static NetworkUdp gs_network_udp[NETWORK_PROTOCOL_TYPE_COUNTS];


/**
  * @brief 处理指控发送的舰艇姿态信息
  * retval .
  */
static int do_with_network_attitude_info(NetworkPackage &net_package)
{
    short info_valid_flags /* 状态及数据有效标志 */;
    int direction_info /* 航向角 */, vertical_info /* 纵摇角 */, horizon_info /* 横摇角 */;

    if (net_package.len() != 0X16)
    {
        red_debug_lite("Invalid payload_len4recv_attitude_info");
        return -1;
    }

    info_valid_flags = net_package.payload()[0] << 8 | net_package.payload()[1];
    if (info_valid_flags & 0x01)
    {
        red_debug_lite("invalid attitude info");
    }
    direction_info = net_package.payload()[2] << 8 | net_package.payload()[3] << 16 |
        net_package.payload()[4] << 8 | net_package.payload()[5];
    vertical_info = net_package.payload()[6] << 8 | net_package.payload()[7] << 16 |
        net_package.payload()[8] << 8 | net_package.payload()[9];
    horizon_info = net_package.payload()[10] << 8 | net_package.payload()[11] << 16 |
        net_package.payload()[12] << 8 | net_package.payload()[13];

    red_debug_lite("flag:%hx direction:%x vertical:%x horizon:%x", info_valid_flags,
        direction_info, vertical_info, horizon_info);
    return 0;
}

/**
  * @brief 处理指控发送的作战干预指令
  * retval .
  */
static int do_with_network_recv_force(NetworkPackage &net_package)
{
    short command_word /* 命令字 */;
    uint8_t dev_num, force_control;

    if (net_package.payload_len() != 4)
    {
        red_debug_lite("Invalid payload_len4recv_guide");
        return -1;
    }
    command_word = net_package.payload()[0] << 8 | net_package.payload()[1];
    dev_num = command_word >> 3 & 0x01;
    /* 1: 允许射击 2：禁止射击 */
    force_control = command_word & 0x07;
    red_debug_lite("dev_num:%u force_control:%u", dev_num, force_control);
    /* TODO 发送消息到对应的设备控制线程 */
}

/**
  * @brief 处理指控发送的引导指令
  * @param NetworkPackage &net_package: 
  * retval Linux/errno.
  */
static int do_with_network_recv_guide(NetworkPackage &net_package)
{
    short control_word, target_batch_number /* 目标批号 */;
    int target_distance /* 目标距离 */;
    short target_direction /* 方位角 */, target_elevation /* 仰角 */;

    uint8_t dev_num, led_type, led_mode, guide_enable;
    uint8_t distance_valid, direction_valid, elevation_valid, batch_valid;

    if (net_package.payload_len() != 12)
    {
        red_debug_lite("Invalid payload_len4recv_guide");
        return -1;
    }

    control_word = net_package.payload()[0] << 8 | net_package.payload()[1];
    /* 0 标识设备 1， 1 标识设备 2 */
    dev_num = control_word >> 9 & 0x01;
    led_type = control_word >> 7 & 0x03;
    led_mode = control_word >> 5 & 0x03;
    guide_enable = control_word >> 4 & 0x01;

    distance_valid = control_word >> 3 & 0x01;
    direction_valid = control_word >> 2 & 0x01;
    elevation_valid = control_word >> 1 & 0x01;
    batch_valid = control_word & 0x01;

    if (!guide_enable)
    {
        red_debug_lite("no guide enable, just return.");
        return 0;
    }

    target_batch_number = net_package.payload()[2] << 8 | net_package.payload()[3];
    target_distance = net_package.payload()[4] << 24 | net_package.payload()[5] << 16 | net_package.payload()[6] << 8 | net_package.payload()[7];
    target_direction = net_package.payload()[8] << 8 | net_package.payload()[9];
    target_elevation = net_package.payload()[10] << 8 | net_package.payload()[11];

    red_debug_lite("control_word:%hx batch_number:%hx distance:%x direction:%hx elevation:%hx", control_word, target_batch_number,
            target_distance, target_direction, target_elevation);
    /* TODO 发送消息到对应的设备控制线程 */
}

/**
  * @brief 
  * retval .
  */
int handle_with_network_buffer(char *buffer, int size)
{
    int ret;
    NetworkPackage net_package;

    ret = net_package.convert_from_buffer(buffer, size);
    if (ret < 0)
    {
        red_debug_lite("Failed convert buffer to NetworkPackage err=%d.", ret);
        return ret;
    }
    /* TODO debug buffer msg */

    switch (net_package.id())
    {
        case NETWORK_RECV_FORCE:
            do_with_network_recv_force(net_package);
            red_debug_lite("TODO with FORCE");
            break;
        case NETWORK_RECV_ATTITUDE_INFO:
            red_debug_lite("TODO with ATTITUDE_INFO");
            do_with_network_attitude_info(net_package);
            break;
        case NETWORK_RECV_GUIDE:
            red_debug_lite("TODO with GUIDE");
            do_with_network_recv_guide(net_package);
            break;
        case NETWORK_RECV_PROBE:
            red_debug_lite("TODO with PROBE");
            break;
        case NETWORK_RECV_OFF:
            red_debug_lite("TODO with OFF");
            break;
        default:
            break;
    }

    return ret;
}

/* 上报信息到一体化网络 */
int _do_report_msg2net(NetworkUdp &net_fd, NetworkPackage &network_package)
{
    char buffer[NETWORK_PACKGE_LEN_MAX] = {0};
    if (network_package.convert_to_buffer(buffer, NETWORK_PACKGE_LEN_MAX))
    {
        return -ENOMEM;
    }
    net_fd.send2server(buffer, network_package.len());
    return 0;
}

/**
  * @brief 上报设备状态到指控
  * retval Linux/errno.
  */
int do_report_dev_status(char dev1_status, char dev1_green_status, char dev1_white_status,
    char dev2_status, char dev2_green_status, char dev2_white_status)
{
    if (gs_network_udp[NETWORK_PROTOCOL_TYPE_SEND_GUIDE].get_socket() <= 0)
    {
        red_debug_lite("Novalid socket for network udp");
        return -EINVAL;
    }
    char dev_status_buffer[NETWORK_PACKGE_LEN_MAX] = {0};

    dev_status_buffer[0] = dev1_status << 7 | dev1_green_status << 6 | dev1_white_status << 5 |
        dev2_status << 4 | dev2_green_status << 3 | dev2_white_status << 2;
    dev_status_buffer[1] = 0x00;
    dev_status_buffer[2] = 0x00;
    dev_status_buffer[3] = 0x00;

    NetworkPackage dev_status(gs_network_udp[NETWORK_PROTOCOL_TYPE_SEND_GUIDE].index(), NETWORK_SEND_STATUS, 0X0C, gs_network_udp[NETWORK_PROTOCOL_TYPE_SEND_GUIDE].stamp(), dev_status_buffer);

    return _do_report_msg2net(gs_network_udp[NETWORK_PROTOCOL_TYPE_SEND_GUIDE], dev_status);
}

/**
  * @brief 上报设备关机事件
  * retval Linux/errno.
  */
int do_report_dev_off()
{
    if (gs_network_udp[NETWORK_PROTOCOL_TYPE_SEND_GUIDE_BROADCAST].get_socket() <= 0)
    {
        red_debug_lite("Novalid socket for network udp");
        return -EINVAL;
    }
    char dev_off_buffer[NETWORK_PACKGE_LEN_MAX] = {0};
    NetworkPackage dev_off(gs_network_udp[NETWORK_PROTOCOL_TYPE_SEND_GUIDE_BROADCAST].index(), NETWORK_SEND_OFF, 0X08, gs_network_udp[NETWORK_PROTOCOL_TYPE_SEND_GUIDE].stamp(), dev_off_buffer);

    return _do_report_msg2net(gs_network_udp[NETWORK_PROTOCOL_TYPE_SEND_GUIDE_BROADCAST], dev_off);
}

/**
  * @brief 上报设备数据
  * @param short dev1_direction: 设备一指向角， 指向角，右边为正，左边为负，+-180
  * @param short dev1_elevatioon: 设备一仰角，仰角范围是 +-90， 采用二进制补码
  * @param short dev2_direction: 设备二指向角
  * @param short dev2_elevation: 设备二仰角
  * retval Linux/errno.
  */
int do_report_dev_info(short dev1_direction, short dev1_elevation, short dev2_direction, short dev2_elevation)
{
    if (gs_network_udp[NETWORK_PROTOCOL_TYPE_SEND_GUIDE].get_socket() <= 0)
    {
        red_debug_lite("Novalid socket for network udp");
        return -EINVAL;
    }
    char dev_info_buffer[NETWORK_PACKGE_LEN_MAX] = {0};

    dev_info_buffer[6] = dev1_direction >> 8 & 0xff;
    dev_info_buffer[7] = dev1_direction & 0xff;
    dev_info_buffer[8] = dev1_elevation >> 8 & 0xff;
    dev_info_buffer[9] = dev1_elevation & 0xff;

    dev_info_buffer[14] = dev2_direction >> 8 & 0xff;
    dev_info_buffer[15] = dev2_direction & 0xff;
    dev_info_buffer[16] = dev2_elevation >> 8 & 0xff;
    dev_info_buffer[17] = dev2_elevation & 0xff;

    NetworkPackage dev_info(gs_network_udp[NETWORK_PROTOCOL_TYPE_SEND_GUIDE].index(), NETWORK_SEND_INFO, 0X1A, gs_network_udp[NETWORK_PROTOCOL_TYPE_SEND_GUIDE].stamp(), dev_info_buffer);

    return _do_report_msg2net(gs_network_udp[NETWORK_PROTOCOL_TYPE_SEND_GUIDE], dev_info);
}

/**
  * @brief 注册指定类型的 NetworkUdp 句柄
  * @param char fd_type: 
  * @param NetworkUdp &net_fd: 
  * retval Linux/errno.
  */
int network_protocol_registe(char fd_type, NetworkUdp &net_fd)
{
    int ret = 0;

    if (fd_type < 0 || fd_type >= NETWORK_PROTOCOL_TYPE_COUNTS)
    {
        red_debug_lite("invalid fd type:%hd", fd_type);
        return -EINVAL;
    }

    if (gs_network_udp[fd_type].get_socket() > 0)
    {
        red_debug_lite("the fd type:%hd has registered before", fd_type);
        ret = 1;
    }

    gs_network_udp[fd_type] = net_fd;

    return ret;
}
