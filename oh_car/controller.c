#include "controller.h"

pid_res pid;
extern PC_SendType send_data;
extern float fAcc[3], fGyro[3], fAngle[3];
extern float cumulativeZAngle;
integral_acc integral_length;
void PID_init(){
    printf("PID_init begin \n");
    pid.SetAngle = 0.0;
    pid.ActualAngle = 0.0;
    pid.err = 0.0;
    pid.err_last = 0.0;
    pid.speed = 20.0;
    pid.integral = 0.0;
    pid.Kp = 0.4;
    pid.Ki = 0.000;
    pid.Kd = 0.001;
    pid.limits=250;
    pid.out=20;

    integral_length.Setlength = 2000;
    printf("PID_init end \n");
   // printf("PID_init begin \n");
   // pid.SetAngle=0.0;
   // pid.ActualAngle=0.0;
   // pid.err=0.0;
   // pid.err_last=0.0;
   // pid.integral=0.0;
   // pid.angle= 0.0;
   // pid.Kp=0.2;
   // pid.Ki=0.015;
   // pid.Kd=0.2;
   // pid.limit=250;
   // printf("PID_init end \n");
}

float pid_pos_update(pid_res *p){

    float out = 0;
    //pid.ActualAngle=Act_ang;
    //计算当前误差
    p->ActualAngle = cumulativeZAngle;
    p->err =p->SetAngle-p->ActualAngle;
    //误差积分
    p->integral+=p->err;
    //pid公式
    pid.speed= pid.Kp * pid.err + pid.Ki * pid.integral + pid.Kd * (pid.err - pid.err_last);
    //误差迭代
    p->err_last=p->err;
    //输出
    out=pid.speed*1.0f;
    //限幅s
    if(out>=0)out+=90;
    else out-=90;
    out = range(out, -p->limits, p->limits);
    pid.out=out;
    return out;
}

// 存储速度和位移
double velocityx[2] = {0.0f, 0.0f};
double positionX[2] = {0.0f, 0.0f};
double ay_0 = 0;
double ay_1 = 0;

float double_integrating_acc(float ay_0, float ay_1, float deltaTime) {
    // first integration (calculate velocity)
    velocityx[1] = velocityx[0] + (ay_0 + (ay_1 - ay_0) / 2.0f) * deltaTime;
    
    // second integration (calculate position)
    positionX[1] = positionX[0] + (velocityx[0] + (velocityx[1] - velocityx[0]) / 2.0f) * deltaTime;
    
    // 更新前一个时间点的数据
    velocityx[0] = velocityx[1];
    positionX[0] = positionX[1];
    
    return positionX[1];
}


void mode_1(){
    //turn;
    
    printf("setangle=%lf",pid.SetAngle);
    pid_pos_update(&pid);
    printf("PID out dead at %f\n", pid.out);
    send_data = ChangeSendType(0, 0, pid.out);
    write(fd_imu, (unsigned char *) send_data.send_data, 12);
}

void mode_2(){
    //forward move
    ay_1=integral_length.Actacc;
    // integral_length.Actuallength=double_integrating_acc(ay_1,ay_0);
    ay_0=ay_1;
    send_data = ChangeSendType(20, 0, 0);
    write(fd_imu, (unsigned char *) send_data.send_data, 12);
}