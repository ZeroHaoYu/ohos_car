/**
  ******************************************************************************
  * @file           : pthread_usr.c
  * @author         : Dylan
  * @brief          : 用户串口协议-底盘机串口协议
  * @attention      : 串口接收未完成
  * @date           : 2023.3.22
  ******************************************************************************
  */

#include "serial_protocal.h"



void dec_to_hex(int vel, int yaw, int ang, unsigned char *send_data)
{
    int low;
    int high;
    low = vel & 0xff;
    high = (vel >>= 8) & 0xff;
    send_data[0] = (char)low;
    send_data[1] = (char)high;
    low = yaw & 0xff;
    high = (yaw >>= 8) & 0xff;
    send_data[2] = (char)low;
    send_data[3] = (char)high;
    low = ang & 0xff;
    high = (ang >>= 8) & 0xff;
    send_data[4] = (char)low;
    send_data[5] = (char)high;
}


// 获取校验位
void xor_cal(PC_SendType *pc_send)
{
    int temp = 0;
    temp ^= pc_send->data_length;
    temp ^= pc_send->response_id;
    for (int i = 0; i < pc_send->data_length; i++)
    {
        temp ^= pc_send->data[i];
    }
    pc_send->xor_num = temp;
}

// 获取发送数据
PC_SendType ChangeSendType(int vel, int yaw, int ang)
{
    PC_SendType SendData;
    memset(SendData.data, 0, 32);
    memset(SendData.send_data, 0, 40);

    SendData.header0 = 0X0A;
    SendData.header1 = 0X0C;
    SendData.data_length = 0X06;
    SendData.response_id = 0X02;
    // 参数保存入data
    dec_to_hex(vel, yaw, ang, SendData.data);
    // 获取校验位
    xor_cal(&SendData);
    // 保存入send_data
    SendData.send_data[0] = 0X0A;
    SendData.send_data[1] = 0X0C;
    SendData.send_data[2] = 0X06;
    SendData.send_data[3] = 0X00;
    SendData.send_data[4] = 0X02;
    for (int i = 5; i <= 10; i++)
    {
        SendData.send_data[i] = SendData.data[i - 5];
    }
    SendData.send_data[11] = SendData.xor_num;
    return SendData;
}

PC_SendType Power(int state) {
    PC_SendType SendData;
    memset(SendData.data, 0, 32);
    memset(SendData.send_data, 0, 40);
    if(state==1){
        SendData.send_data[0]=0x0A;
        SendData.send_data[1]=0x0C;
        SendData.send_data[2]=0x01;
        SendData.send_data[3]=0x00;
        SendData.send_data[4]=0x01;
        SendData.send_data[5]=0x01;
        SendData.send_data[6]=0x01;
    }
    else if(state==0){
        SendData.send_data[0]=0x0A;
        SendData.send_data[1]=0x0C;
        SendData.send_data[2]=0x01;
        SendData.send_data[3]=0x00;
        SendData.send_data[4]=0x01;
        SendData.send_data[5]=0x00;
        SendData.send_data[6]=0x00;
    }
    return SendData;
}
