#include "controller.h"

pid_res pid;

void PID_init(){
    printf("PID_init begin \n");
    pid.SetAngle=0.0;
    pid.ActualAngle=0.0;
    pid.err=0.0;
    pid.err_last=0.0;
    pid.integral=0.0;
    pid.angle= 0.0;
    pid.Kp=0.2;
    pid.Ki=0.015;
    pid.Kd=0.2;
    pid.limit=250;
    printf("PID_init end \n");
}
/*
void pid_controller(float target){
    pid.SetSpeed=speed;
    pid.err=pid.SetSpeed-pid.ActualSpeed;
    pid.integral+=pid.err;
    pid.voltage=pid.Kp*pid.err+pid.Ki*pid.integral+pid.Kd*(pid.err-pid.err_last);
    pid.err_last=pid.err;
    pid.ActualSpeed=pid.voltage*1.0;
    return pid.ActualSpeed;
}
*/

float pid_pos_update(pid_res *p){
    float pe,ie,de;
    float out =0;
    if(p->ActualAngle+p->SetAngle>180){
        p->SetAngle-=(360-p->ActualAngle);
    }
    //计算当前误差
    p->err =p->SetAngle-p->ActualAngle;
    //误差积分
    p->integral+=p->err;
    de=p->err-p->err_last;
    pe=p->err_last;
    ie=p->integral;
    p->err_last=p->err;
    //输出
    out=pe*(p->Kp)+ie*(p->Ki)+de*(p->Kd);
    //限幅
    out=range(out,-p->limit,p->limit);
    return out;
}