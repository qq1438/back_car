#include "PID.h"
#include "clip.h"
#include <math.h>

// 模糊集标签定义（Fuzzy Sets）
#define NB 0 // 负大
#define NM 1 // 负中
#define NS 2 // 负小
#define ZO 3 // 零
#define PS 4 // 正小
#define PM 5 // 正中
#define PB 6 // 正大

// 一维误差模糊规则表
static const int FuzzyRuleKp1D[7] = {PB, PM, PS, ZO, PS, PM, PB};  // Kp规则：误差大时增大Kp
static const int FuzzyRuleKi1D[7] = {NS, NM, NB, ZO, NB, NM, NS};  // Ki规则：误差大时减小Ki，避免积分饱和；误差小时增大Ki，消除稳态误差
static const int FuzzyRuleKd1D[7] = {PS, ZO, NB, NM, NB, ZO, PS};  // Kd规则：误差变化剧烈时减小Kd，误差稳定时适当增大Kd

/**
 * @brief 初始化PID控制器
 * @param PIDX PID控制器结构体指针
 * @param KP 比例系数
 * @param KI 积分系数
 * @param KD 微分系数
 * @param limit 输出限幅值
 */
void PID_Init(PID *PIDX)
{
    PIDX->KP = 0;
    PIDX->KI = 0;
    PIDX->KD = 0;
    PIDX->limit = 0;
    PIDX->last_error = 0.0f;    // 初始化上次误差为0
    PIDX->last_output = 0.0f;   // 初始化上次输出为0
    PIDX->target = 0.0f;        // 初始化目标值为0
    PIDX->llast_error = 0.0f;
    PIDX->Integral_error = 0.0f;
}

/**
 * @brief 位置式PID控制
 * @param PIDX PID控制器结构体指针
 * @param Value 当前测量值
 * @return 经过限幅后的控制输出
 */
float PID_Position(PID *PIDX, float Value)
{
    float error = PIDX->target - Value;         // 计算当前误差
    float derror = error - PIDX->last_error;    // 计算误差变化率
    // 计算PID输出：比例项 + 微分项
    float output = PIDX->KP * error -
                  PIDX->KD * derror;
    
    PIDX->last_error = error;  // 更新上次误差
    
    // 返回经过限幅后的输出
    float y = PIDX->limit;
    output = fabs(output) > y ? fabs(output) / output * y : output;
    return output;
}

float PID_Position_speed(PID *PIDX, float Value,float now_speed)
{
    float error = PIDX->target - Value;         // 计算当前误差
    // 计算PID输出：比例项 + 微分项
    float output = PIDX->KP * error  -
                  PIDX->KD * now_speed
                  + PIDX->KPP * error * fabs(error);
    
    PIDX->last_error = error;  // 更新上次误差
    
    // 返回经过限幅后的输出
    float y = PIDX->limit;
    output = fabs(output) > y ? fabs(output) / output * y : output;
    return output;
}

/**
 * @brief 增量式PID控制
 * @param PIDX PID控制器结构体指针
 * @param Value 当前测量值
 * @return 经过限幅后的控制输出
 */
int PID_Incream(PID *PIDX, float Value)
{
    float error = PIDX->target - Value;         // 计算当前误差
    float derror = error - PIDX->last_error;    // 计算误差变化率
    // 计算PID输出：比例项 + 积分项 + 上次输出

    float output = PIDX->KP * derror +  PIDX->KI * error + PIDX->KD * (error - 2 * PIDX->last_error + PIDX->llast_error) +PIDX->last_output;
    float y = PIDX->limit;
    output = fabs(output) > y ? fabs(output) / output * y : output;

    // 正确的更新顺序：先保存当前last_error为llast_error，再更新last_error
    PIDX->llast_error = PIDX->last_error;  // 更新上上次误差
    PIDX->last_error = error;              // 更新上次误差
    PIDX->last_output = output;            // 更新上次输出
    // 返回经过限幅后的输出
    return (int)output;
}

float PID_Incream_IMU(PID *PIDX, float Value)
{
    imu660ra_get_acc();
    float acc_x = imu660ra_acc_transition(imu660ra_acc_x);
    float error = PIDX->target - Value;         // 计算当前误差
    float derror = error - PIDX->last_error;    // 计算误差变化率
    // 增量式PID：KP*error + KD*acc_y
    float output = PIDX->KP * derror + PIDX->KI * error + PIDX->KD * acc_x + PIDX->last_output;
    float y = PIDX->limit;
    output = fabs(output) > y ? fabs(output) / output * y : output;
    PIDX->llast_error = PIDX->last_error;       // 更新上上次误差
    PIDX->last_error = error;                   // 更新上次误差
    PIDX->last_output = output;

    return output;
}

int PID_Position_IMU(PID *PIDX, float Value)
{
    imu660ra_get_gyro();
    float gyro_z = imu660ra_gyro_transition(imu660ra_gyro_z);
    float error = PIDX->target - Value;         // 计算当前误差
    float output = PIDX->KP * error - PIDX->KDD * gyro_z + error*fabs(error)*PIDX->KPP + PIDX->KD * (error - PIDX->last_error);
    PIDX->last_error = error;
    float y = PIDX->limit;
    output = fabs(output) > y ? fabs(output) / output * y : output;
    // 返回经过限幅后的输出
    return (int)output;
}
