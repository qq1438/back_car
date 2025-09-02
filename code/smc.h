#ifndef _SMC_H_
#define _SMC_H_

typedef struct {
    float k1;
    float k2;
    float lambda;
    float limit;
    float Ts;
    float v;
    float last_error;
    float target;
} SMC;

#include "zf_common_headfile.h"

void SMC_Init(SMC *smc);
float SMC_Update(SMC *smc, float distance);

#endif 
