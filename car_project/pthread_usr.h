/**
 ******************************************************************************
 * @file           : Pthread_usr.h
 * @author         : Dylan
 * @brief          : 线程实现
 * @attention      : 包含了串口文件符
 * @date           :  2023.3.23 2023.3.22
 ******************************************************************************
 */
#ifndef OH_CAR_CHASSIS_CONTROL_PTHREAD_USR_H
#define OH_CAR_CHASSIS_CONTROL_PTHREAD_USR_H

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
#include "serial_protocol.h"
#include "serial.h"
#include "REG.h"
#include "wit_c_sdk.h"
#include <stdint.h>

/**
 * @type            :串口标识符
 * @brief           ：数传 串口标识符
 */
extern int fd_lora;

/**
 * @type            :串口标识符
 * @brief           :IMU 串口标识符号
 */
extern int fd_imu;

/**
 * @type            :串口标识符
 * @brief           :底盘串口表示符
 */
extern int fd_chassis;

/**
 * @type            :串口标识符
 * @brief           :蓝牙串口标识符
 */
extern int fd_BLE;

/**
 * @type            :线程同步FLAG
 * @brief           :线程同步FLAG
 */
extern int FLAG;

/**/

extern volatile char s_cDataUpdate;

/**
 * @brief           :串口接受线程
 * @return          :none
 * @state           :未启用
 */
void *_serial_BLE_input_task(void);
/**
 * @brief           :串口输出线程
 * @return          :none
 */
void _serial_output_task(void);

/**
 * @brief           :串口接受线程
 * @return
 */
void _serial_IMU_input_task(void);

/**
 * @brief           :数传传输线程
 * @return
 */
void _serial_LORA_OUTPUT_task(void);
#endif // OH_CAR_CHASSIS_CONTROL_PTHREAD_USR_H
