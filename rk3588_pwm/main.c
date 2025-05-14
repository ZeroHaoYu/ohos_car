#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include "um_pwm.h"

#define INIT_DUTY 0
#define MAX_DUTY 2500000

int main (int argc, char **argv){
    int pwm_nums = -1;
    int pwm_period = 25;
    int current_duty = INIT_DUTY;
    int flag = 1;
    int pwm_duty = 0;
    int cur_duty = 0;
    printf("%d\n",argc);
    if (argc != 4) {
        printf("error\n");
        return 0;
    } else {
         // 从命令行参数中获取PWM引脚编号和使能状态
        pwm_nums = atoi(argv[1]);
        pwm_period = atoi(argv[2]);
        pwm_duty = atoi(argv[3]);
    }
      // 根据使能状态初始化和控制PWM
    init_pwm(pwm_nums);
        // 设置PWM使能
    set_pwm_enable(pwm_nums,PWM_IS_ENABLED);
    
    // 设置PWM周期
    set_pwm_period(pwm_nums,pwm_period);
    // 设置PWM极性为正常极性
    set_pwm_polarity(pwm_nums, PWM_POLARITY_NORMAL);
    
    // 获取PWM占空比
    current_duty = get_pwm_dutyCycle(pwm_nums);
    printf("current duty is %d\n",current_duty);

    // 设置PWM占空比
    set_pwm_dutyCycle(pwm_nums, current_duty);
    while (1)
    {
        // 根据标志位增减占空比
        if (flag){
            current_duty += pwm_duty;
            set_pwm_dutyCycle(pwm_nums,current_duty);
        } else {
            current_duty -= pwm_duty;
            set_pwm_dutyCycle(pwm_nums,current_duty);
        }
        // 切换标志位以控制占空比增减方向
        if (current_duty <= INIT_DUTY){
            flag = 1;
        } else if (current_duty >= MAX_DUTY){
            flag = 0;
        }
        printf("current duty is %d\n",current_duty);
        usleep(50000);
        cur_duty = get_pwm_dutyCycle(pwm_nums);
        if (cur_duty <= INIT_DUTY) {
            set_pwm_dutyCycle(pwm_nums, INIT_DUTY);
            cur_duty = get_pwm_dutyCycle(pwm_nums);
        }
        float cur_angle = 1.0 * (cur_duty - INIT_DUTY);
        printf("The current angle %.2f, about to turn:%.2f.\n", cur_angle, 1.0 * pwm_duty);
    }
    return 0;
}