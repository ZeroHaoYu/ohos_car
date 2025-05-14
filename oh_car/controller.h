#ifndef CAR_PROJECT_CONTROLLER_H
#define CAR_PROJECT_CONTROLLER_H

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
#include "pthread_usr.h"
#include "serial.h"


typedef struct{
    float SetAngle;          //定义设定值
    double ActualAngle;        //定义实际值
    float err;                //定义偏差值
    float err_last;            //定义上一个偏差值
    float Kp,Ki,Kd;            //定义比例、积分、微分系数
    float speed;          //定义电压值（控制执行器的变量）
    float integral;            //定义积分值
    float out;
    float limits;
}pid_res;

typedef struct {
    float Setlength;
    float Actuallength;
    float Actacc;
} integral_acc;


extern pid_res pid;
extern float Act_ang;
extern float angle_before;
extern double velocityx[2];
extern double positionX[2];
extern double ay_0;
extern double ay_1;

void PID_init();
void pid_controller();  //pid 控制器  输入ERR角度   输出轮子转动
float double_integrating_acc(float ay_0, float ay_1, float deltaTime);  //双重积分 or FFT Omega算法输出里程信息
void mode_1();
void mode_2();
float pid_pos_update(pid_res *p);
#endif //CAR_PROJECT_CONTROLLER_H
