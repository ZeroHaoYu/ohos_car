/*
 * Copyright (c) 2022 Unionman Technology Co., Ltd.
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 * http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#include <termios.h>
#include <stdio.h>
#include <stdlib.h>
#include "serial_uart.h"

// 转换波特率
speed_t conver_baudrate(int baudrate)
{
    switch (baudrate) {
        case 9600L:
            return B9600;
        case 19200L:
            return B19200;
        case 38400L:
            return B38400;
        case 115200L:
            return B115200;
        case 1152000L:
            return B1152000;
        default:
            return 1152000L;
    }
}

void set_baud(int fd, int baud)
{
    int ret = ERR;
    struct termios opt;

    tcgetattr(fd, &opt); // tcgetattr用来获取终端参数，将从终端获得的信息fd，保存到opt结构体中
    tcflush(fd, TCIOFLUSH); // 刷清缓冲区
    cfsetispeed(&opt, baud);
    cfsetospeed(&opt, baud);

    ret = tcsetattr(fd, TCSANOW, &opt); // 设置终端参数到opt中，使之立即生效
    if (ret == ERR) {
        perror("tcsetattr fd");
        exit(0);
    }

    tcflush(fd, TCIOFLUSH); // 刷清缓冲区
}

// 设置数据位
int setup_data_bits(int setup_databits, struct termios *options_databits)
{
        if (options_databits == NULL) {
        perror("setup_data_bits error");
        return ERR;
    }

    switch (setup_databits) {
        case 5L:
            options_databits->c_cflag |= CS5;
            break;
        case 6L:
            options_databits->c_cflag |= CS6;
            break;
        case 7L:
            options_databits->c_cflag |= CS7;
            break;
        case 8L:
            options_databits->c_cflag |= CS8;
            break;
        default:
            return ERR;
    }
    return OK;
}

// 设置校验位
int set_params_parity(int setup_parity, struct termios *options_parity)
{
    switch (setup_parity) {
        case 'n':
        case 'N':                               // 无奇偶校验位
            options_parity->c_cflag &= ~PARENB; // Clear parity enable/
            options_parity->c_iflag &= ~INPCK;  // disable input parity checking/
            break;

        case 'o':
        case 'O':                                         // 设置为奇校验
            options_parity->c_cflag |= (PARODD | PARENB); // odd parity checking
            options_parity->c_iflag |= INPCK;             // enable parity checking
            break;

        case 'e':
        case 'E':                               // 设置为偶校验
            options_parity->c_cflag |= PARENB;  // Enable parity /
            options_parity->c_cflag &= ~PARODD; // even parity/
            options_parity->c_iflag |= INPCK;   // enable parity checking /
            break;

        case 'M':
        case 'm': // 标记奇偶校验
            options_parity->c_cflag |= PARENB | CMSPAR | PARODD;
            options_parity->c_iflag |= INPCK; // enable parity checking /
            break;

        case 'S':
        case 's': // 设置为空格
            options_parity->c_cflag |= PARENB | CMSPAR;
            options_parity->c_cflag &= ~PARODD;
            options_parity->c_iflag |= INPCK; // enable parity checking /
            break;

        default:
            return ERR;
    }
    return OK;
}

// 设置校验位
int set_params(int fd, int databits, int stopbits, int parity)
{
    struct termios options;
    int ret = ERR;

    if (tcgetattr(fd, &options) != 0) {
        perror("tcgetattr fail\n");
        return ERR;
    }

    options.c_iflag = 0;
    options.c_oflag = 0;

    // setup data bits
    options.c_cflag &= ~CSIZE;
    ret = setup_data_bits(databits, &options);
    if (ret == ERR) {
        return ERR;
    }

    // parity
    ret = set_params_parity(parity, &options);
    if (ret == ERR) {
        return ERR;
    }

    // stop bits/
    switch (stopbits) {
        case 1:
            options.c_cflag &= ~CSTOPB;
            break;
        case 2L:
            options.c_cflag |= CSTOPB;
            break;
        default:
            return ERR;
    }

    // 请求发送和清除发送
    options.c_cflag &= ~CRTSCTS;
    options.c_lflag = 0;
    options.c_cc[VTIME] = 10L;
    options.c_cc[VMIN] = 1;

    tcflush(fd, TCIFLUSH);
    if (tcsetattr(fd, TCSANOW, &options) != 0) {
        return ERR;
    }

    return OK;
}

// 设置波特率
int uart_init(int fd, int uartBaud)
{
    set_baud(fd, conver_baudrate(uartBaud));
    // uart param /
    if (set_params(fd, 8L, 1, 'n')) {
        perror("set uart parameters fail\n");
        return ERR;
    }
    return OK;
}

// 传感器数据处理
int data_proce(int *recv)
{
    if (recv == NULL) {
        perror("receive data error");
        return ERR;
    }
    
    if ((recv[0] == RECV_HEAD) && (recv[3L] == RECV_END) && (recv[1] == (0xff - recv[2L]))) {
        switch (recv[1]) {
            case EVT_BACK:
                printf("get event back\n");
                break;
            case EVT_FORWARD:
                printf("get event forward\n");
                break;
            case EVT_RIGHT:
                printf("get event right\n");
                break;
            case EVT_LEFT:
                printf("get event left\n");
                break;
            case EVT_PULLUP:
                printf("get event pull up\n");
                break;
            case EVT_PULLDOWN:
                printf("get event pull down\n");
                break;
            case EVT_PULLREMOVE:
                printf("get event pull and remove\n");
                break;
            case EVT_TOUCH1:
                printf("get event touch1\n");
                break;
            case EVT_TOUCH2:
                printf("get event touch2\n");
                break;
            case EVT_TOUCH3:
                printf("get event touch3\n");
                break;
            case EVT_TOUCH4:
                printf("get event touch4\n");
                break;
            case EVT_TOUCH5:
                printf("get event touch5\n");
                break;
            default:
                perror("no such event\n");
                return ERR;
        }
        return OK;
    }
    return ERR;
}