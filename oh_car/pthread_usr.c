/**
 ******************************************************************************
 * @file           : pthread_usr.c
 * @author         : Dylan
 * @brief          : 线程函数实现
 * @attention      : 正在构建
 * @date           : 2023.3.22
 ******************************************************************************
 */

#include "pthread_usr.h"

pthread_mutex_t lock = PTHREAD_MUTEX_INITIALIZER;

PC_SendType send_data;
extern integral_acc integral_length;
float fAcc[3], fGyro[3], fAngle[3];
char imu_acc_buf[1000];
int mode_flag = 0;
int mode_select_FLAG=0;
int vel_temp = 20, yaw_temp = 0, ang_temp = 0;
// 存储累计的z轴旋转角度
float cumulativeZAngle = 0.0f;

// 存储速度和位移
float velocityY = 0.0f;
float distanceY = 0.0f;
float linearAccY = 0.0f;
float prevAccY = 0.0f;

// 重力加速度常数（假设为9.81 m/s²）
const float GRAVITY = 9.81f;

float gravityY = 0.0f;



#define ACC_UPDATE 0x01
#define GYRO_UPDATE 0x02
#define ANGLE_UPDATE 0x04
#define MAG_UPDATE 0x08
#define READ_UPDATE 0x80
/**
 * @attention 暂时未启用的串口读取函数
 * @return
 */


void *_serial_input_task(void)
{
    int i = 0;
    int ret = ERR;
    int buf = 0;
    char recv[FRAME_LEN] = {0};
    pthread_detach(pthread_self());
    printf("test _ready\n");
    while (1)
    {
        for (i = 0; i < FRAME_LEN; i++)
        {
            ret = read(fd_chassis, &buf, 1);
            if (ret == ERR)
            {
                perror("read error\n");
                exit(0);
            }
            recv[i] = buf;
        }
        ret = data_proce(recv);
        if (ret == ERR)
        {
            perror("data process error\n");
            exit(0);
        }
    }
}

void _serial_output_task(void)
{
    
    pthread_detach(pthread_self());
    send_data = Power(1);
    write(fd_chassis, (unsigned char *)send_data.send_data, 12);
    int forward_setAngle = -1000;
    
    while (1)
    {
        // pthread_mutex_lock(&lock);
        
        if (mode_flag == 1) {
            if (s_cDataUpdate & ANGLE_UPDATE)
            {
                // 打印累计Z轴角度、距离Y、线性加速度Y、加速度Y、重力Y
                printf("cumulativeZAngle=%f\r\n",cumulativeZAngle);
                printf("distanceY=%f\r\n",distanceY);
                printf("linearAccY=%f\r\n",linearAccY);
                printf("fAcc[1]=%f\r\n",fAcc[1]);
                printf("gravityY=%f\r\n",gravityY);
            }
            if (s_cDataUpdate & ANGLE_UPDATE)
            {
                printf("angle:%.3f %.3f %.3f\r\n", fAngle[0], fAngle[1], fAngle[2]);
            }            
            if (mode_select_FLAG == 0) {
                printf("setangle=%lf\r\n",pid.SetAngle);
                printf("ActualAngle=%lf\r\n",pid.ActualAngle);

                // 更新PID位置控制
                pid_pos_update(&pid);
                printf("PID out dead at %f\n", pid.out);
                
                // 设置发送数据
                send_data = ChangeSendType(0, 0, 10);
                write(fd_chassis, (unsigned char *) send_data.send_data, 12);
                if (pid.ActualAngle > pid.SetAngle - 5 && pid.ActualAngle < pid.SetAngle +5) {
                    mode_select_FLAG = 1;
                    double temp = pid.SetAngle;
                    velocityx[0] = 0;
                    positionX[0] = 0;
                    velocityx[1] = 0;
                    positionX[1] = 0;
                    pid.SetAngle = temp;
                    distanceY = 0;
                    
                }
            } else if (mode_select_FLAG == 1) {
                integral_length.Setlength=0.2;
                double temp = pid.SetAngle;
                ay_1=fAcc[1];
                integral_length.Actuallength=distanceY;
                ay_0=ay_1;
                vel_temp = 20;
                yaw_temp = 0;
                ang_temp = 0;
                pid.SetAngle = temp;
                // send_data = ChangeSendType(vel_temp, yaw_temp, ang_temp);
                send_data = ChangeSendType(vel_temp, 0, 0);
                write(fd_chassis, (unsigned char *)send_data.send_data, 12);

                printf("act_length=%f\n",integral_length.Actuallength);
                if (integral_length.Actuallength <integral_length.Setlength+0.05&&integral_length.Actuallength >integral_length.Setlength-0.05) {
                    mode_select_FLAG = 0;
                    pid.SetAngle+=90;
                    if(pid.SetAngle>180)pid.SetAngle=pid.SetAngle-360;
                    distanceY = 0; 
                    
                }

            }
            usleep(300000);
        } else if (mode_flag == 2) {
            if (forward_setAngle == -1000){
                forward_setAngle = cumulativeZAngle;
            }
            if (cumulativeZAngle >= (forward_setAngle-2) && cumulativeZAngle <= (forward_setAngle+2)){
                vel_temp = 20;
                yaw_temp = 0; 
                ang_temp = 0;
                send_data = ChangeSendType(vel_temp, yaw_temp, ang_temp);
                write(fd_chassis, (unsigned char *)send_data.send_data, 12);
                usleep(300000);
            } else if ((cumulativeZAngle - forward_setAngle) <= 180 && (cumulativeZAngle - forward_setAngle) >0) {
                vel_temp = 0;
                yaw_temp = 0; 
                ang_temp = -10;
                send_data = ChangeSendType(vel_temp, yaw_temp, ang_temp);
                write(fd_chassis, (unsigned char *)send_data.send_data, 12);
                usleep(300000);
            } else {
                vel_temp = 0;
                yaw_temp = 0; 
                ang_temp = 10;
                send_data = ChangeSendType(vel_temp, yaw_temp, ang_temp);
                write(fd_chassis, (unsigned char *)send_data.send_data, 12);
                usleep(300000);
            }
        } else {
            forward_setAngle = -1000;
            printf("vel_temp:%d yaw_temp:%d ang_temp:%d\n",vel_temp, yaw_temp, ang_temp);
            send_data = ChangeSendType(vel_temp, yaw_temp, ang_temp);
            printf("vel_mid: %d \n", vel_temp);
            write(fd_chassis, (unsigned char *)send_data.send_data, 12);
            printf("vel_after:%d \n", vel_temp);
            usleep(300000);
        }
        // pthread_mutex_lock(&lock);
        // if (ret > 0)
        // {
        //      printf("send success, data is  \n");
        // }
        // else
        // {
        //     printf("send error!\r\n");
        // }
        
    }
}
/**
 * @brief imu调用
 */

float normalizeAngle(float angle) {
    while (angle > 180.0f) {
        angle -= 360.0f;
    }
    while (angle < -180.0f) {
        angle += 360.0f;
    }
    return angle;
}

// 计算在y轴上的重力加速度分量
float computeGravityY(float pitchAngle) {
    // 计算重力加速度分量
    return sin(pitchAngle * M_PI / 180.0);
}


void _serial_IMU_input_task(void)
{
    int i;
    char cBuff[1];
    
    const float deltaTime = 0.01f;  // 10毫秒
    while (1)
    {
        while (serial_read_data(fd_imu, cBuff, 1))
        {
            WitSerialDataIn(cBuff[0]);
        }
        // printf("\n");
        usleep(10000);
        if (s_cDataUpdate)
        {
            // pthread_mutex_lock(&lock);
            for (i = 0; i < 3; i++)
            {
                fAcc[i] = sReg[AX + i] / 32768.0f * 16.0f;
                fGyro[i] = sReg[GX + i] / 32768.0f * 2000.0f;
                fAngle[i] = sReg[Roll + i] / 32768.0f * 180.0f;
            }
            // 更新累计z轴角度
            cumulativeZAngle += fGyro[2] * deltaTime;
            cumulativeZAngle = normalizeAngle(cumulativeZAngle);
            // 计算在y轴上的重力加速度分量
            gravityY = computeGravityY(fAngle[0]);

            // 消除重力分量
            linearAccY = fAcc[1] - gravityY;

            if(linearAccY < 0.01 && linearAccY > -0.01){
                linearAccY = 0;
            } 
            // 使用双重积分方法计算y轴位移
            // distanceY = double_integrating_acc(prevAccY, linearAccY, deltaTime);
            distanceY += vel_temp * deltaTime / 1000;

            // 更新前一个时间点的加速度
            prevAccY = linearAccY;
            if (s_cDataUpdate & ACC_UPDATE)
            {
                sprintf(imu_acc_buf, "acc:%.3f,%.3f,%.3f\r\n", fAcc[0], fAcc[1], fAcc[2]);
                printf("acc:%.3f %.3f %.3f\r\n", fAcc[0], fAcc[1], fAcc[2]);
            }
            if (s_cDataUpdate & GYRO_UPDATE)
            {
                printf("gyro:%.3f %.3f %.3f\r\n", fGyro[0], fGyro[1], fGyro[2]);
            }
            if (s_cDataUpdate & ANGLE_UPDATE)
            {
                printf("angle:%.3f %.3f %.3f\r\n", fAngle[0], fAngle[1], fAngle[2]);
            }
            if (s_cDataUpdate & MAG_UPDATE)
            {
                printf("mag:%d %d %d\r\n", sReg[HX], sReg[HY], sReg[HZ]);
            }
            pthread_mutex_unlock(&lock);
        }
    }

    serial_close(fd_imu);
}

/**
 * @brief tcp_client_socket线程
 */


char buffer[BUFFER_SIZE];

int set_non_blocking(int sockfd) {
    int flags;
#if defined(O_NONBLOCK)
    if (-1 == (flags = fcntl(sockfd, F_GETFL, 0)))
        flags = 0;
    return fcntl(sockfd, F_SETFL, flags | O_NONBLOCK);
#else
    flags = 1;
    return ioctl(sockfd, FIONBIO, &flags);
#endif
}

void _tcp_server_socket(void) {
    int server_fd, new_socket;
    struct sockaddr_in address;
    int opt = 1;
    int addrlen = sizeof(address);
    char buffer[BUFFER_SIZE] = {0};
    char *message = "Sever recevied!";

    // 创建TCP套接字
    if ((server_fd = socket(AF_INET, SOCK_STREAM, 0)) == 0) {
        perror("socket failed");
        exit(EXIT_FAILURE);
    }

    // 设置套接字选项，允许地址重用
    if (setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR | SO_REUSEPORT, &opt, sizeof(opt))) {
        perror("setsockopt");
        close(server_fd);  // 确保在错误情况下关闭套接字
        exit(EXIT_FAILURE);
    }

    address.sin_family = AF_INET;
    address.sin_addr.s_addr = INADDR_ANY;
    address.sin_port = htons(PORT);

    // 绑定套接字到指定端口
    if (bind(server_fd, (struct sockaddr *)&address, sizeof(address)) < 0) {
        perror("bind failed");
        close(server_fd);
        exit(EXIT_FAILURE);
    }

    // 监听连接
    if (listen(server_fd, 3) < 0) {
        perror("listen");
        close(server_fd);
        exit(EXIT_FAILURE);
    }

    printf("Waiting for connection...\n");

    while (1) {
        if ((new_socket = accept(server_fd, (struct sockaddr *)&address, (socklen_t*)&addrlen)) < 0) {
            perror("accept");
            continue;
        }

        printf("Connection established with client\n");

        // 循环等待客户端数据
        while (1) {
            int valread = read(new_socket, buffer, BUFFER_SIZE);
            if (valread <= 0) {
                printf("Client disconnected or read error\n");
                close(new_socket);
                break;
            }

            
            int filtered_length = 0;
            for (int i = 0; i < valread; ++i) {
                if (buffer[i] != '\r' && buffer[i] != '\n') {
                    buffer[filtered_length++] = buffer[i];
                }
            }
            buffer[filtered_length] = '\0';
            if (filtered_length > 0) {
                printf("Received from client: ");
                for (int i = 0; i < filtered_length; ++i) {
                    if (buffer[i] == '\n') {
                        printf("\\n");
                    } else if (buffer[i] == '\t') {
                        printf("\\t");
                    } else {
                        printf("%c", buffer[i]);
                    }
                }
                printf("\nFiltered length of buffer: %d\n", filtered_length);
                // pthread_mutex_lock(&lock);
                if (strcmp(buffer, "1") == 0) {
                    mode_flag = 0;
                    vel_temp = 20; yaw_temp = 0; ang_temp = 0;
                } else if (strcmp(buffer, "2") == 0) {
                    mode_flag = 0;
                    vel_temp = -20; yaw_temp = 0; ang_temp = 0;
                } else if (strcmp(buffer, "3") == 0) {
                    mode_flag = 0;
                    vel_temp = 0; yaw_temp = 0; ang_temp = 20;
                } else if (strcmp(buffer, "4") == 0) {
                    mode_flag = 0;
                    vel_temp = 0; yaw_temp = 0; ang_temp = -20;
                } else if (strcmp(buffer, "5") == 0) {
                    mode_flag = 0;
                    vel_temp = 20; yaw_temp = 200; ang_temp = 0;
                } else if (strcmp(buffer, "6") == 0) {
                    mode_flag = 0;
                    vel_temp = 20; yaw_temp = -20; ang_temp = 0;
                } else if (strcmp(buffer, "7") == 0) {
                    mode_flag = 0;
                    vel_temp = 200; yaw_temp = 0; ang_temp = 0;
                } else if (strcmp(buffer, "square") == 0) {
                    mode_flag = 1;
                } else if (strcmp(buffer, "forward") == 0) {
                    mode_flag = 2;
                } else {
                    mode_flag = 0;
                    vel_temp = 0; yaw_temp = 0; ang_temp = 0;
                }
                printf("mode_flag: %d\n", mode_flag);
                printf("Updated values: vel_temp = %d, yaw_temp = %d, ang_temp = %d\n", vel_temp, yaw_temp, ang_temp);

                // pthread_mutex_unlock(&lock);
                if (send(new_socket, message, strlen(message), 0) == -1) {
                    printf("defeat\n");
                    perror("send");
                    close(new_socket);
                    break;
                }
            }
            memset(buffer, 0, BUFFER_SIZE);
        }
    }

    close(server_fd);
}

