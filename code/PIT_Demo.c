#include "PIT_Demo.h"

PID angle_pid;
PID position_pid;
PID acc_pid;
PID diff_pid;
LADRC Lspeed_ladrc;
LADRC Rspeed_ladrc;

bool start = 0;
float all_speed = 0,speed_base = 0;
int16 angle_pwm = 0;
int left_speed = 0,right_speed = 0;
int left_pwm = 0,right_pwm = 0;
int mid_speed = 0;
float voltage = 0,acc_x = 0;
float prop = 0;
float add_speed,pid_speed,diff_speed;
uint16 adc_value = 0;
uint8 time = 0;

IFX_INTERRUPT(cc60_pit_ch0_isr, 0, CCU6_0_CH0_ISR_PRIORITY)
{
    interrupt_global_enable(0);                     // ÆôÓÃÇ¶Ì×ÖÐ¶Ï
    pit_clear_flag(CCU60_CH0);
    time++;
    right_speed = -encoder_get_count(TIM4_ENCODER);
    left_speed = encoder_get_count(TIM2_ENCODER);

    encoder_clear_count(TIM4_ENCODER);
    encoder_clear_count(TIM2_ENCODER);
     if(!start && is_start){
         start = 1;
         add_speed = speed_base*16.8;
     }
     if(start && time == 2)
     {
         time = 0;
         if(error[1] > Y_MID_POSITION)pid_speed = PID_Position_speed(&acc_pid,error[1] - Y_MID_POSITION,(left_speed + right_speed)/2 - add_speed);
         else{
             pid_speed = PID_Position_speed(&position_pid,get_distance(Y_MID_POSITION) - error[2],(left_speed + right_speed)/2 - add_speed);
         }
     }
     if(area_num != 2 && is_start)
     {
         angle_pwm = 0;
         all_speed = 0;
         pwm_set_duty(ATOM0_CH0_P21_2,0);
         pwm_set_duty(ATOM0_CH1_P21_3,0);
         nec_stop_flag = 1;
     }
     all_speed = add_speed + pid_speed;
     angle_pwm = PID_Position_IMU(&angle_pid,(error[0] - X_MID_POSITION));
     if(is_start)diff_speed = PID_Position_IMU(&diff_pid,(error[0] - X_MID_POSITION));
    Diff_speed_ladrc(all_speed,diff_speed,&Lspeed_ladrc,&Rspeed_ladrc);
    left_pwm = (int)LADRC_Update(&Lspeed_ladrc,(float)left_speed);
    right_pwm = (int)LADRC_Update(&Rspeed_ladrc,(float)right_speed);
    pwm_set_duty(ATOM2_CH3_P11_6,angle_pwm + 2900);
    gpio_set_level(P02_5,right_pwm>0?0:1);
    pwm_set_duty(ATOM0_CH4_P02_4,clip(abs(right_pwm),0,9900));
    gpio_set_level(P02_7,left_pwm>0?1:0);
    pwm_set_duty(ATOM0_CH6_P02_6,clip(abs(left_pwm),0,9900));
}
