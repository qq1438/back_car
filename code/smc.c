#include "smc.h"
#include <math.h>

void SMC_Init(SMC *smc)
{
    smc->k1 = 0.0f;
    smc->k2 = 0.0f;
    smc->lambda = 0.0f;
    smc->limit = 0.0f;
    smc->Ts = 0.006f;
    smc->v = 0.0f;
    smc->last_error = 0.0f;
    smc->target = Y_MID_POSITION;
}

float SMC_Update(SMC *smc, float distance)
{
    float error = smc->target - distance;
    float error_dot = (error - smc->last_error) / smc->Ts;

    float s = smc->lambda * error + error_dot;

    float sign_s = (s > 0.0f) - (s < 0.0f);

    smc->v += -smc->k2 * sign_s * smc->Ts;

    float u = -smc->k1 * sqrtf(fabsf(s)) * sign_s + smc->v;

    if (u > smc->limit)
        u = smc->limit;
    else if (u < -smc->limit)
        u = -smc->limit;

    smc->last_error = error;

    return u;
} 
