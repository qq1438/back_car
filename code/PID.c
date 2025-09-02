#include "PID.h"
#include "clip.h"
#include <math.h>

// ģ������ǩ���壨Fuzzy Sets��
#define NB 0 // ����
#define NM 1 // ����
#define NS 2 // ��С
#define ZO 3 // ��
#define PS 4 // ��С
#define PM 5 // ����
#define PB 6 // ����

// һά���ģ�������
static const int FuzzyRuleKp1D[7] = {PB, PM, PS, ZO, PS, PM, PB};  // Kp��������ʱ����Kp
static const int FuzzyRuleKi1D[7] = {NS, NM, NB, ZO, NB, NM, NS};  // Ki��������ʱ��СKi��������ֱ��ͣ����Сʱ����Ki��������̬���
static const int FuzzyRuleKd1D[7] = {PS, ZO, NB, NM, NB, ZO, PS};  // Kd�������仯����ʱ��СKd������ȶ�ʱ�ʵ�����Kd

/**
 * @brief ��ʼ��PID������
 * @param PIDX PID�������ṹ��ָ��
 * @param KP ����ϵ��
 * @param KI ����ϵ��
 * @param KD ΢��ϵ��
 * @param limit ����޷�ֵ
 */
void PID_Init(PID *PIDX)
{
    PIDX->KP = 0;
    PIDX->KI = 0;
    PIDX->KD = 0;
    PIDX->limit = 0;
    PIDX->last_error = 0.0f;    // ��ʼ���ϴ����Ϊ0
    PIDX->last_output = 0.0f;   // ��ʼ���ϴ����Ϊ0
    PIDX->target = 0.0f;        // ��ʼ��Ŀ��ֵΪ0
    PIDX->llast_error = 0.0f;
    PIDX->Integral_error = 0.0f;
}

/**
 * @brief λ��ʽPID����
 * @param PIDX PID�������ṹ��ָ��
 * @param Value ��ǰ����ֵ
 * @return �����޷���Ŀ������
 */
float PID_Position(PID *PIDX, float Value)
{
    float error = PIDX->target - Value;         // ���㵱ǰ���
    float derror = error - PIDX->last_error;    // �������仯��
    // ����PID����������� + ΢����
    float output = PIDX->KP * error -
                  PIDX->KD * derror;
    
    PIDX->last_error = error;  // �����ϴ����
    
    // ���ؾ����޷�������
    float y = PIDX->limit;
    output = fabs(output) > y ? fabs(output) / output * y : output;
    return output;
}

float PID_Position_speed(PID *PIDX, float Value,float now_speed)
{
    float error = PIDX->target - Value;         // ���㵱ǰ���
    // ����PID����������� + ΢����
    float output = PIDX->KP * error  -
                  PIDX->KD * now_speed
                  + PIDX->KPP * error * fabs(error);
    
    PIDX->last_error = error;  // �����ϴ����
    
    // ���ؾ����޷�������
    float y = PIDX->limit;
    output = fabs(output) > y ? fabs(output) / output * y : output;
    return output;
}

/**
 * @brief ����ʽPID����
 * @param PIDX PID�������ṹ��ָ��
 * @param Value ��ǰ����ֵ
 * @return �����޷���Ŀ������
 */
int PID_Incream(PID *PIDX, float Value)
{
    float error = PIDX->target - Value;         // ���㵱ǰ���
    float derror = error - PIDX->last_error;    // �������仯��
    // ����PID����������� + ������ + �ϴ����

    float output = PIDX->KP * derror +  PIDX->KI * error + PIDX->KD * (error - 2 * PIDX->last_error + PIDX->llast_error) +PIDX->last_output;
    float y = PIDX->limit;
    output = fabs(output) > y ? fabs(output) / output * y : output;

    // ��ȷ�ĸ���˳���ȱ��浱ǰlast_errorΪllast_error���ٸ���last_error
    PIDX->llast_error = PIDX->last_error;  // �������ϴ����
    PIDX->last_error = error;              // �����ϴ����
    PIDX->last_output = output;            // �����ϴ����
    // ���ؾ����޷�������
    return (int)output;
}

float PID_Incream_IMU(PID *PIDX, float Value)
{
    imu660ra_get_acc();
    float acc_x = imu660ra_acc_transition(imu660ra_acc_x);
    float error = PIDX->target - Value;         // ���㵱ǰ���
    float derror = error - PIDX->last_error;    // �������仯��
    // ����ʽPID��KP*error + KD*acc_y
    float output = PIDX->KP * derror + PIDX->KI * error + PIDX->KD * acc_x + PIDX->last_output;
    float y = PIDX->limit;
    output = fabs(output) > y ? fabs(output) / output * y : output;
    PIDX->llast_error = PIDX->last_error;       // �������ϴ����
    PIDX->last_error = error;                   // �����ϴ����
    PIDX->last_output = output;

    return output;
}

int PID_Position_IMU(PID *PIDX, float Value)
{
    imu660ra_get_gyro();
    float gyro_z = imu660ra_gyro_transition(imu660ra_gyro_z);
    float error = PIDX->target - Value;         // ���㵱ǰ���
    float output = PIDX->KP * error - PIDX->KDD * gyro_z + error*fabs(error)*PIDX->KPP + PIDX->KD * (error - PIDX->last_error);
    PIDX->last_error = error;
    float y = PIDX->limit;
    output = fabs(output) > y ? fabs(output) / output * y : output;
    // ���ؾ����޷�������
    return (int)output;
}
