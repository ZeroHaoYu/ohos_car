#ifndef CONTROLLER
#define CONTROLLER

#define max(a, b)(a>b? a:b)
#define min(a, b)(a<b? a:b)
#define range(x, a, b)(min(max(x, a), b))

#include <termios.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/types.h>
#include <fcntl.h>
#include <unistd.h>
#include <pthread.h>

typedef struct{
    float SetAngle;          //定义设定值
    float ActualAngle;        //定义实际值
    float err;                //定义偏差值
    float err_last;            //定义上一个偏差值
    float Kp,Ki,Kd;            //定义比例、积分、微分系数
    float angle;                //位置环角度值
    float integral;            //定义积分值
    float limit;                //定义限幅
}pid_res;

extern pid_res pid;


void PID_init();
void pid_controller();  //pid 控制器  输入ERR角度   输出轮子转动
void double_integrating_acc();  //双重积分Omega算法输出里程信息
void mode_1();
void mode_2();
float pid_pos_update(pid_res *p);
#endif