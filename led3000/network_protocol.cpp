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
#include <nanogui/common.h>
#include <led3000gui.h>
#include <PolyM/include/polym/Msg.hpp>
#include <PolyM/include/polym/Queue.hpp>
#include <stdint.h>

using namespace nanogui;
using namespace std;

static NetworkUdp gs_network_udp[NETWORK_PROTOCOL_TYPE_COUNTS];
static Led3000Window *gs_screen = nullptr;
static TurntableAttitude gs_turntable_attitude[2];

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
        RedDebug::log("Invalid payload_len4recv_attitude_info");
        return -1;
    }

    info_valid_flags = net_package.payload()[0] << 8 | net_package.payload()[1];
    if (info_valid_flags & 0x01)
    {
        RedDebug::log("invalid attitude info");
    }
    direction_info = net_package.payload()[2] << 8 | net_package.payload()[3] << 16 |
        net_package.payload()[4] << 8 | net_package.payload()[5];
    vertical_info = net_package.payload()[6] << 8 | net_package.payload()[7] << 16 |
        net_package.payload()[8] << 8 | net_package.payload()[9];
    horizon_info = net_package.payload()[10] << 8 | net_package.payload()[11] << 16 |
        net_package.payload()[12] << 8 | net_package.payload()[13];

    /* Update ship attitude info */
    gs_turntable_attitude[0].update_attitude_info(direction_info, vertical_info, horizon_info);
    gs_turntable_attitude[1].update_attitude_info(direction_info, vertical_info, horizon_info);

    RedDebug::log("flag:%hx direction:%d vertical:%d horizon:%d", info_valid_flags,
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
        RedDebug::log("Invalid payload_len4recv_guide");
        return -1;
    }
    command_word = net_package.payload()[0] << 8 | net_package.payload()[1];
    dev_num = command_word >> 3 & 0x01;
    /* 1: 允许射击 2：禁止射击 */
    force_control = command_word & 0x07;
    RedDebug::log("dev_num:%u force_control:%u", dev_num, force_control);
    /* 更新界面显示和授权状态更新 */
    gs_screen->getJsonValue()->devices[dev_num].green_led.auth = 2 - force_control;
    gs_screen->get_dev_auth_label(dev_num)->set_caption((2 - force_control) ? "允许射击" : "禁止射击");
}

/**
  * @brief 处理指控发送的检测设备开机指令
  * retval .
  */
static int do_with_network_recv_probe(NetworkPackage &net_package)
{
    if (net_package.payload_len() != 0)
    {
        RedDebug::log("Invalid payload_len4recv_probe");
        return -1;
    }
    return 0;
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
    char target_position_buffer[32] = {0};

    uint8_t dev_num, led_type, led_mode, guide_enable;
    uint8_t distance_valid, direction_valid, elevation_valid, batch_valid;

    if (net_package.payload_len() != 12)
    {
        RedDebug::log("Invalid payload_len4recv_guide");
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

    if (guide_enable)
    {
        RedDebug::log("no guide enable, just return.");
        return 0;
    }

    target_batch_number = net_package.payload()[2] << 8 | net_package.payload()[3];
    target_distance = net_package.payload()[4] << 24 | net_package.payload()[5] << 16 | net_package.payload()[6] << 8 | net_package.payload()[7];
    target_direction = net_package.payload()[8] << 8 | net_package.payload()[9];
    target_elevation = net_package.payload()[10] << 8 | net_package.payload()[11];
    /* TODO correct target info with attitude */
    gs_turntable_attitude[dev_num].correct_target_info(target_direction, target_elevation);

    /* 方向,俯仰 */
    snprintf(target_position_buffer, sizeof(target_position_buffer), "%hd,%hd", target_direction, target_elevation);

    RedDebug::log("control_word:%hx batch_number:%hx distance:%x direction:%hx elevation:%hx", control_word, target_batch_number,
            target_distance, target_direction, target_elevation);
    /* TODO 发送消息到对应的设备控制线程 */
    /* 发送消息控制转台转动到指定角度 */
    gs_screen->getDeviceQueue(dev_num).put(PolyM::DataMsg<std::string>(POLYM_TURNTABLE_POSITION_SETTING, string(target_position_buffer)));
    if (led_type == NETWORK_PROTOCOL_WHITE_LED_TYPE)
    {
        if (led_mode == NETWORK_PROTOCOL_NORMAL_LED_MODE)
        {
            //gs_screen->getJsonValue()->devices[gs_screen->getDeviceQueue(dev_num)].white_led.normal_status = 100;
            gs_screen->getDeviceQueue(dev_num).put(PolyM::DataMsg<std::string>(POLYM_WHITE_NORMAL_SETTING, to_string(100)));
        }
        else if (led_mode == NETWORK_PROTOCOL_BINK_LED_MODE)
        {
            /* 默认 5HZ 爆闪 */
            gs_screen->getDeviceQueue(dev_num).put(PolyM::DataMsg<std::string>(POLYM_WHITE_BLINK_SETTING, to_string(5)));
        }
    }
    else if (led_type == NETWORK_PROTOCOL_GREEN_LED_TYPE)
    {
        if (led_mode == NETWORK_PROTOCOL_NORMAL_LED_MODE)
        {
            //gs_screen->getJsonValue()->devices[gs_screen->getDeviceQueue(dev_num)].white_led.normal_status = 100;
            gs_screen->getDeviceQueue(dev_num).put(PolyM::DataMsg<std::string>(POLYM_GREEN_NORMAL_SETTING, to_string(100)));
        }
        else if (led_mode == NETWORK_PROTOCOL_BINK_LED_MODE)
        {
            /* 默认 5HZ 爆闪 */
            gs_screen->getDeviceQueue(dev_num).put(PolyM::DataMsg<std::string>(POLYM_GREEN_BLINK_SETTING, to_string(5)));
        }
    }
    return 0;
}

/* 上报信息到一体化网络 */
int _do_report_msg2net(NetworkUdp &net_fd, NetworkPackage &network_package)
{
    char buffer[NETWORK_PACKGE_LEN_MAX] = {0};
    /* Fix dstip == 0.0.0.0 */
    if (network_package.dst_ip_n() == 0X00000000)
    {
        network_package.set_dst_ip_n(ntohl(((sockaddr_in *)net_fd.addrinfo()->ai_addr)->sin_addr.s_addr));
    }

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
int do_report_dev_status(NetworkPackage &net_package, char dev1_status, char dev1_green_status, char dev1_white_status,
    char dev2_status, char dev2_green_status, char dev2_white_status)
{
    if (gs_network_udp[NETWORK_PROTOCOL_TYPE_SEND_GUIDE_BROADCAST].get_socket() <= 0)
    {
        RedDebug::log("Novalid socket for network udp");
        return -EINVAL;
    }
    char dev_status_buffer[NETWORK_PACKGE_LEN_MAX] = {0};

    dev_status_buffer[0] = dev1_status << 7 | dev1_green_status << 6 | dev1_white_status << 5 |
        dev2_status << 4 | dev2_green_status << 3 | dev2_white_status << 2;
    dev_status_buffer[1] = 0x00;
    dev_status_buffer[2] = 0x00;
    dev_status_buffer[3] = 0x00;

    NetworkPackage dev_status(net_package.src_ip_n(), net_package.dst_ip_n(),
        gs_network_udp[NETWORK_PROTOCOL_TYPE_SEND_GUIDE_BROADCAST].sn(),
        0X00,
        0X01,
        0X01,
        0X01, NETWORK_SEND_STATUS, MK_MSG_FULL_LEN(0X0C),
        gs_network_udp[NETWORK_PROTOCOL_TYPE_SEND_GUIDE_BROADCAST].stamp(),
        dev_status_buffer);

    return _do_report_msg2net(gs_network_udp[NETWORK_PROTOCOL_TYPE_SEND_GUIDE_BROADCAST], dev_status);
}

/**
  * @brief 上报设备关机事件
  * retval Linux/errno.
  */
int do_report_dev_off(NetworkPackage &net_package)
{
    if (gs_network_udp[NETWORK_PROTOCOL_TYPE_SEND_GUIDE].get_socket() <= 0)
    {
        RedDebug::log("Novalid socket for network udp");
        return -EINVAL;
    }
    char dev_off_buffer[NETWORK_PACKGE_LEN_MAX] = {0};
    
    NetworkPackage dev_off(net_package.src_ip_n(),
        ntohl(((sockaddr_in *)gs_network_udp[NETWORK_PROTOCOL_TYPE_SEND_GUIDE].addrinfo()->ai_addr)->sin_addr.s_addr),
        gs_network_udp[NETWORK_PROTOCOL_TYPE_SEND_GUIDE].sn(),
        0X00,
        0X01,
        0X01,
        0X01,
        NETWORK_SEND_OFF, MK_MSG_FULL_LEN(0X8),
        gs_network_udp[NETWORK_PROTOCOL_TYPE_SEND_GUIDE].stamp(), dev_off_buffer);

    return _do_report_msg2net(gs_network_udp[NETWORK_PROTOCOL_TYPE_SEND_GUIDE], dev_off);
}

#define DIMENSION_90_SHORT    (90.0 / (2 << 13))

static inline short _do_format_dev_value_short(float value, double dimension)
{
    RedDebug::log("%f = %hd\n", value, short((value + 0.5 * dimension)/dimension));
    if (value < 0)
        return htonl(short((value - 0.5 * dimension)/dimension));
    else
        return htons(short((value + 0.5 * dimension)/dimension));
}

/**
  * @brief 上报设备数据
  * @param short dev1_direction: 设备一指向角， 指向角，右边为正，左边为负，+-180
  * @param short dev1_elevatioon: 设备一仰角，仰角范围是 +-90， 采用二进制补码
  * @param short dev2_direction: 设备二指向角
  * @param short dev2_elevation: 设备二仰角
  * retval Linux/errno.
  */
int do_report_dev_info(NetworkPackage &net_package, short dev1_direction, short dev1_elevation, short dev2_direction, short dev2_elevation)
{
    if (gs_network_udp[NETWORK_PROTOCOL_TYPE_SEND_GUIDE_BROADCAST].get_socket() <= 0)
    {
        RedDebug::log("Novalid socket for network udp");
        return -EINVAL;
    }
    char dev_info_buffer[NETWORK_PACKGE_LEN_MAX] = {0};

    dev1_direction = _do_format_dev_value_short(dev1_direction, DIMENSION_90_SHORT);
    dev_info_buffer[6] = dev1_direction >> 8 & 0xff;
    dev_info_buffer[7] = dev1_direction & 0xff;
    dev1_elevation = _do_format_dev_value_short(dev1_elevation, DIMENSION_90_SHORT);
    dev_info_buffer[8] = dev1_elevation >> 8 & 0xff;
    dev_info_buffer[9] = dev1_elevation & 0xff;

    dev2_direction = _do_format_dev_value_short(dev2_direction, DIMENSION_90_SHORT);
    dev_info_buffer[14] = dev2_direction >> 8 & 0xff;
    dev_info_buffer[15] = dev2_direction & 0xff;
    dev2_elevation = _do_format_dev_value_short(dev2_elevation, DIMENSION_90_SHORT);
    dev_info_buffer[16] = dev2_elevation >> 8 & 0xff;
    dev_info_buffer[17] = dev2_elevation & 0xff;

    NetworkPackage dev_info(net_package.src_ip_n(), net_package.dst_ip_n(),
        gs_network_udp[NETWORK_PROTOCOL_TYPE_SEND_GUIDE_BROADCAST].sn(),
        0X00,
        0X01,
        0X01,
        0X01,
        NETWORK_SEND_INFO, MK_MSG_FULL_LEN(0X1A), gs_network_udp[NETWORK_PROTOCOL_TYPE_SEND_GUIDE_BROADCAST].stamp(), dev_info_buffer);

    return _do_report_msg2net(gs_network_udp[NETWORK_PROTOCOL_TYPE_SEND_GUIDE_BROADCAST], dev_info);
}

/**
  * @brief respon 
  * retval Linux/errno.
  */
int do_force_respon(NetworkPackage &net_package)
{
    if (gs_network_udp[NETWORK_PROTOCOL_TYPE_SEND_GUIDE_BROADCAST].get_socket() <= 0)
    {
        RedDebug::log("Novalid socket for network udp");
        return -EINVAL;
    }
    char force_respon_buffer[NETWORK_PACKGE_LEN_MAX] = {0};
    NetworkPackage force_respon(net_package.src_ip_n(), 
        net_package.dst_ip_n(),
        gs_network_udp[NETWORK_PROTOCOL_TYPE_SEND_GUIDE_BROADCAST].sn(),
        net_package.ack(),
        0X03,
        0X00,
        gs_network_udp[NETWORK_PROTOCOL_TYPE_SEND_GUIDE_BROADCAST].index(), 0X0, MK_MSG_FULL_LEN(0X0), 0X0, nullptr);

    RedDebug::log("force respon 2 network\n");
    return _do_report_msg2net(gs_network_udp[NETWORK_PROTOCOL_TYPE_SEND_GUIDE_BROADCAST], force_respon);
}

/**
  * @brief 针对探测报文的应答
  * retval Linux/errno.
  */
int do_probe_respon(NetworkPackage &network_package)
{
    char dev_status[2] = {0}; char dev_green_status[2] = {0, 0}; char dev_white_status[2] = {0, 0};
    short int direction[2], elevation[2];
    std::string dev_info_str, direction_str, elevation_str;
    std::size_t split_index = 0;
    const led3000_config_t *json_value_ptr = gs_screen->getJsonValue();
    int index = 0;

    for (; index < 2; index++)
    {
        if (gs_screen->get_dev_state_label(index)->caption() == std::string("故障"))
            dev_status[index] = 1;
        if (json_value_ptr->devices[index].white_led.mode == LED_NORMAL_MODE && 
            json_value_ptr->devices[index].white_led.normal_status == 0)
        {
            dev_white_status[index] = 1;
        }
        if ((gs_screen->get_dev_auth_label(index)->caption() == std::string("禁止射击")) || 
            (json_value_ptr->devices[index].green_led.mode == LED_NORMAL_MODE && 
            json_value_ptr->devices[index].green_led.normal_status == 0))
        {
            dev_green_status[index] = 1;
        }
        dev_info_str = gs_screen->get_dev_angle_label(index)->caption();
        split_index = dev_info_str.find("/");
        direction_str = dev_info_str.substr(0, split_index);
        elevation_str = dev_info_str.substr(1 + split_index);
        RedDebug::log("gre:%s dire:%s elevation:%s whole:%s\n", gs_screen->get_dev_auth_label(index)->caption().c_str(),
            direction_str.c_str(), elevation_str.c_str(), dev_info_str.c_str());
        direction[index] = std::atoi(direction_str.c_str());
        elevation[index] = std::atoi(elevation_str.c_str());
    }
    do_report_dev_status(network_package, dev_status[0], dev_green_status[0], dev_white_status[0],
        dev_status[1], dev_green_status[1], dev_white_status[1]);
    do_report_dev_info(network_package, direction[0], elevation[0], direction[1], elevation[1]);
    RedDebug::log("respon 2 network\n");
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
        RedDebug::log("invalid fd type:%hd", fd_type);
        return -EINVAL;
    }

    if (gs_network_udp[fd_type].get_socket() > 0)
    {
        RedDebug::log("the fd type:%hd has registered before", fd_type);
        ret = 1;
    }

    gs_network_udp[fd_type] = net_fd;

    return ret;
}

/**
  * @brief 注册 Led3000Window 对象指针
  * @param Led3000Window *window: 
  * retval Linux/errno.
  */
int screen_window_register(void *window)
{
    int ret = 0;

    if (window && !gs_screen)
    {
        gs_screen = (Led3000Window *)window;
    }

    return ret;
}

/**
  * @brief 
  * retval .
  */
int handle_with_network_buffer(char *buffer, int size)
{
    int ret;
    NetworkPackage net_package;

    /* TODO check gs_screen valid */

    ret = net_package.convert_from_buffer(buffer, size);
    if (ret < 0)
    {
        RedDebug::log("Failed convert buffer to NetworkPackage err=%d.", ret);
        return ret;
    }
    /* TODO debug buffer msg */

    switch (net_package.id())
    {
        case NETWORK_RECV_FORCE:
            do_with_network_recv_force(net_package);
            do_force_respon(net_package);
            RedDebug::log("TODO with FORCE");
            break;
        case NETWORK_RECV_ATTITUDE_INFO:
            RedDebug::log("TODO with ATTITUDE_INFO");
            do_with_network_attitude_info(net_package);
            break;
        case NETWORK_RECV_GUIDE:
            RedDebug::log("TODO with GUIDE");
            do_with_network_recv_guide(net_package);
            break;
        case NETWORK_RECV_PROBE:
            RedDebug::log("TODO with PROBE");
            do_with_network_recv_probe(net_package);
            do_probe_respon(net_package);
            break;
        case NETWORK_RECV_OFF:
            RedDebug::log("TODO with OFF");
            break;
        case NETWORK_PINPONG_TEST:
            RedDebug::log("TODO with pingpong test");
            /* 返回 99 表示测试 pingong */
            ret = 99;
            break;
        default:
            break;
    }

    return ret;
}

/******************** export function ****************************/
int update_sysinfo2network(void)
{
    NetworkPackage network_package;
    do_probe_respon(network_package);
}

int update_offinfo2network(void)
{
    NetworkPackage network_package;
    do_report_dev_off(network_package);
}

