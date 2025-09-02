#ifndef _All_Init_h_
#define _All_Init_h_

#include "zf_common_headfile.h"

extern volatile bool Send_state;
extern float FOC_PWM;

void Device_Init(void);
void send_value(void);

#endif

