/**
  ******************************************************************************
  * @file           : Serial_protocal.h
  * @author         : Dylan
  * @brief          : 串口协议
  * @attention      : 已经实现底盘控制协议，通过设定 vel ，yaw ，angle  实现
  * @date           : 2023.3.22
  ******************************************************************************
  */

/**
 * TODO：
 * 1. IMU 串口协议接入
 * 2. GPS 串口协议接入
 * 3. 毫米波雷达串口协议接入
 * 4. 蓝牙串口协议接入
 */

#ifndef OH_CAR_CHASSIS_CONTROL_SERIAL_PROTOCAL_H
#define OH_CAR_CHASSIS_CONTROL_SERIAL_PROTOCAL_H

/**
 * @brief           :公共基础库
 */
#include <termios.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/types.h>
#include <fcntl.h>
#include <unistd.h>
#include <pthread.h>

/**
 * @brief           :自定义库
 */
#include "serial_base.h"
/**
 * @type            :struct
 * @brief           :底盘串口协议结构体
 */
typedef struct
{
    char header0;
    char header1;
    short data_length;
    char response_id;
    unsigned char data[32];
    unsigned char xor_num;
    unsigned char send_data[40];
} PC_SendType;



/**
 * @brief 发送数据初始化  设定vel：速度，yaw：偏航角度，ang:自旋模式的旋转角速度
 * @param vel
 * @param yaw
 * @param ang
 * @return PC_SendType 格式的结构体，返回后调用输出函数进行串口输出
 */
PC_SendType ChangeSendType(int vel, int yaw, int ang);
/**
 * @brief 把参数转换为16进制编码
 * @param vel
 * @param yaw
 * @param ang
 * @param send_data
 */
void dec_to_hex(int vel, int yaw, int ang, unsigned char *send_data);
/**
 * @brief 计算校验位
 * @param pc_send
 */
void xor_cal(PC_SendType *pc_send);
/**
 * @brief 上电指令
 * @param state 1上电  0下电
 * @return
 */
PC_SendType Power(int state);
#endif //OH_CAR_CHASSIS_CONTROL_SERIAL_PROTOCAL_H
