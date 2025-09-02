#ifndef PID_H
#define PID_H

typedef struct {
    float KP;
    float KI;
    float KD;
    float KPP;
    float KDD;
    float target;
    float last_error;
    float llast_error;
    float Integral_error;
    int last_output;
    float limit;
}PID;

#include "zf_common_headfile.h"
void PID_Init(PID *PIDX);
float PID_Position(PID *PIDX, float Value);
int PID_Incream(PID *PIDX, float Value);
float PID_Incream_IMU(PID *PIDX, float Value);
int PID_Position_IMU(PID *PIDX, float Value);
float PID_Position_speed(PID *PIDX, float Value,float now_speed);

#endif
