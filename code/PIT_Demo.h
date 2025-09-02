#ifndef _PIT_Demo_h_
#define _PIT_Demo_h_

#include "zf_common_headfile.h"

#define X_MID_POSITION 94.0f
#define Y_MID_POSITION 87.0f

extern LADRC Lspeed_ladrc;
extern LADRC Rspeed_ladrc;
extern PID acc_pid; 
extern PID angle_pid;
extern PID position_pid;
extern PID diff_pid;
extern float all_speed;
extern int16 angle_pwm;
extern float speed_base;
extern float voltage;
extern int left_pwm;
extern int right_pwm;
extern float acc_x;
extern float add_speed;
extern float pid_speed;
extern int left_speed;
extern int right_speed;


#endif
