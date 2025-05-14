/**
  ******************************************************************************
  * @file           : pthread_usr.c
  * @author         : Dylan
  * @brief          : 串口底层驱动实现
  * @attention      : none
  * @date           :  2023.3.23 2023.3.22
  ******************************************************************************
  */

#include "serial_base.h"

speed_t conver_baudrate(int baudrate)
{
    switch (baudrate)
    {
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

void set_baud(int fd_chassis, int baud)
{
    int ret = ERR;
    struct termios opt;

    tcgetattr(fd_chassis, &opt);    // tcgetattr用来获取终端参数，将从终端获得的信息fd_imu，保存到opt结构体中
    tcflush(fd_chassis, TCIOFLUSH); // 刷清缓冲区
    cfsetispeed(&opt, baud);
    cfsetospeed(&opt, baud);

    ret = tcsetattr(fd_chassis, TCSANOW, &opt); // 设置终端参数到opt中，使之立即生效
    if (ret == ERR)
    {
        perror("tcsetattr fd_chassis");
        exit(0);
    }

    tcflush(fd_chassis, TCIOFLUSH); // 刷清缓冲区
}



int setup_data_bits(int setup_databits, struct termios *options_databits)
{
    if (options_databits == NULL)
    {
        perror("setup_data_bits error");
        return ERR;
    }

    switch (setup_databits)
    {
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



int set_params_parity(int setup_parity, struct termios *options_parity)
{
    switch (setup_parity)
    {
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



int set_params(int fd_chassis, int databits, int stopbits, int parity)
{
    struct termios options;
    int ret = ERR;

    if (tcgetattr(fd_chassis, &options) != 0)
    {
        perror("tcgetattr fail\n");
        return ERR;
    }

    options.c_iflag = 0;
    options.c_oflag = 0;

    // setup data bits
    options.c_cflag &= ~CSIZE;
    ret = setup_data_bits(databits, &options);
    if (ret == ERR)
    {
        return ERR;
    }

    // parity
    ret = set_params_parity(parity, &options);
    if (ret == ERR)
    {
        return ERR;
    }

    // stop bits/
    switch (stopbits)
    {
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

    tcflush(fd_chassis, TCIFLUSH);
    if (tcsetattr(fd_chassis, TCSANOW, &options) != 0)
    {
        return ERR;
    }

    return OK;
}


// 设置波特率
int uart_init(int fd_chassis, int uartBaud)
{
    set_baud(fd_chassis, conver_baudrate(uartBaud));
    // uart param /
    if (set_params(fd_chassis, 8L, 1, 'n'))
    {
        perror("set uart parameters fail\n");
        return ERR;
    }
    return OK;
}




