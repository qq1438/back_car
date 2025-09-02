#include "ladrc.h"

/**
 * @brief 初始化LADRC控制器
 * @param ladrc LADRC控制器结构体指针
 * @param Ts 采样周期
 */
void LADRC_Init(LADRC *ladrc,float Ts)
{
    ladrc->wc = 0;
    ladrc->w0 = 0;
    ladrc->b = 0;
    ladrc->kd = 0;
    ladrc->Ts = Ts;
    ladrc->limit = 0;
    
    ladrc->z1 = 0.0f;
    ladrc->z2 = 0.0f;
    ladrc->target = 0.0f;
    ladrc->last_u = 0.0f;
}

/**
 * @brief 设置LADRC控制器目标值
 * @param ladrc LADRC控制器结构体指针
 * @param target 目标值
 */
void LADRC_SetTarget(LADRC *ladrc, float target)
{
    ladrc->target = target;
}

/**
 * @brief LADRC控制器更新函数
 * @param ladrc LADRC控制器结构体指针
 * @param y 系统输出测量值
 * @return 控制输出u
 */
float LADRC_Update(LADRC *ladrc, float y)
{
    float wc = ladrc->wc;
    float w0 = ladrc->w0;
    float b = ladrc->b;
    float Ts = ladrc->Ts;
    
    
    // 扩张状态观测器（ESO）
    // 观测器误差
    float e = y - ladrc->z1;
    
    // ESO更新方程（离散化）
    // z1(k+1) = z1(k) + Ts * (z2(k) + 2*w0*e + b*u(k))
    // z2(k+1) = z2(k) + Ts * (w0^2 * e)
    float z1_new = ladrc->z1 + Ts * (ladrc->z2 + 2.0f * w0 * e + b * ladrc->last_u);
    float z2_new = ladrc->z2 + Ts * (w0 * w0 * e);
    
    // 更新ESO状态
    ladrc->z1 = z1_new;
    ladrc->z2 = z2_new;
    
    // 线性状态反馈控制律
    // u0 = wc * (r - z1)
    float u0 = wc * (ladrc->target - ladrc->z1);
    
    // 扰动补偿
    // u = (u0 - z2) / b
    float u = (u0 - ladrc->z2) / b;
    
    // 输出限幅
    if (u > ladrc->limit)
    {
        u = ladrc->limit;
    }
    else if (u < -ladrc->limit)
    {
        u = -ladrc->limit;
    }
    
    // 保存控制输出供下次ESO更新使用
    ladrc->last_u = u;
    
    return u;
}

float LADRC_Update_2(LADRC *ladrc, float y) {
    float wc = ladrc->wc;
    float w0 = ladrc->w0;
    float b = ladrc->b;
    float Ts = ladrc->Ts;
    
    // 三阶ESO（新增z3）
    float e = y - ladrc->z1;
    
    // ESO离散更新方程（三阶形式）
    float z1_new = ladrc->z1 + Ts * (ladrc->z2 + 3.0f*w0*e);          // 原2*w0改为3*w0
    float z2_new = ladrc->z2 + Ts * (ladrc->z3 + 3.0f*w0*w0*e + b*ladrc->last_u); // 新增z3项
    float z3_new = ladrc->z3 + Ts * (w0*w0*w0*e);                     // 新增z3动态
    
    ladrc->z1 = z1_new;
    ladrc->z2 = z2_new;
    ladrc->z3 = z3_new;

    float e1 = ladrc->target - ladrc->z1;
    float e2 = ladrc->target_dot - ladrc->z2;
    float u0 = wc*wc*e1 + 2.0f*wc*e2;          // PD控制
    float u = (u0 - ladrc->z3) / b;
    
    // 输出限幅
    if (u > ladrc->limit)
    {
        u = ladrc->limit;
    }
    else if (u < -ladrc->limit)
    {
        u = -ladrc->limit;
    }
    
    // 保存控制输出供下次ESO更新使用
    ladrc->last_u = u;
    return u;
}

/**
 * @brief 重置LADRC控制器状态
 * @param ladrc LADRC控制器结构体指针
 */
void LADRC_Reset(LADRC *ladrc)
{
    ladrc->z1 = 0.0f;
    ladrc->z2 = 0.0f;
    ladrc->last_u = 0.0f;
} 

/**
 * @brief 复制LADRC控制器的参数
 * @param dest 目标LADRC控制器结构体指针
 * @param src 源LADRC控制器结构体指针
 */
void LADRC_CopyParams(LADRC *dest, const LADRC *src)
{
    if (dest == NULL || src == NULL)
    {
        // 处理空指针错误，或者直接返回
        return;
    }

    // 复制参数，不复制状态变量 (z1, z2, last_u)
    dest->w0 = src->w0;
    dest->wc = src->wc;
    dest->b = src->b;
    dest->limit = src->limit;
    dest->target = src->target; // 目标值也视为参数的一部分，可以被复制
}


void Diff_speed_ladrc(int16 speed,int16 diff_speed,LADRC *left_speed, LADRC *right_speed)
{
    if(speed > 0)
    {
        if(diff_speed < 0)
        {
            LADRC_SetTarget(left_speed, speed + abs(diff_speed));
            LADRC_SetTarget(right_speed, speed - abs(diff_speed));
        }
        else
        {
            LADRC_SetTarget(left_speed, speed - abs(diff_speed));
            LADRC_SetTarget(right_speed, speed + abs(diff_speed));
        }
    }
    else
    {
        if(diff_speed < 0)
        {
            LADRC_SetTarget(left_speed, speed - abs(diff_speed));
            LADRC_SetTarget(right_speed, speed + abs(diff_speed));
        }
        else
        {
            LADRC_SetTarget(left_speed, speed + abs(diff_speed));
            LADRC_SetTarget(right_speed, speed - abs(diff_speed));
        }
    }
}

