/**
  ******************************************************************************
  * @file           : Pthread_usr.h
  * @author         : Dylan
  * @brief          : 线程实现
  * @attention      : 包含了串口文件符
  * @date           : 2023.3.22
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
#include "serial_protocal.h"


/**
 * @type            :串口标识符
 * @brief           :底盘串口表示符
 */
extern int fd_chassis;


/**
 * @brief           :串口接受线程
 * @return          :none
 * @state           :未启用
 */
void *_serial_input_task(void);
/**
 * @brief           :串口输出线程
 * @return          :none
 */
void _serial_output_task(void);

#endif //OH_CAR_CHASSIS_CONTROL_PTHREAD_USR_H
