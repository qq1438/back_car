#ifndef _LADRC_H_
#define _LADRC_H_

typedef struct 
{
    // ����������
    float wc;         // ����������
    float w0;         // �۲�������
    float b;          // ��������
    float kd;         // �Ŷ�����ϵ��
    
    // ����״̬�۲���(ESO)״̬����
    float z1;         // ϵͳ�������ֵ
    float z2;         // ϵͳ״̬���Ŷ�����ֵ
    float z3;         // ϵͳ״̬���Ŷ�����ֵ
    
    // ����ʱ��
    float Ts;         // ��������
    
    // Ŀ��ֵ������޷�
    float target;     // Ŀ��ֵ (r)
    float target_dot; // Ŀ��ֵ�仯�� (r_dot)
    float limit;      // ����޷�
    
    // ��һ�εĿ������
    float last_target; // ��һ�ε�Ŀ��ֵ
    float last_u;     // ��һ�εĿ������
} LADRC;


#include "zf_common_headfile.h"

// һ��LADRC��������
void LADRC_Init(LADRC *ladrc, float Ts);
void LADRC_SetTarget(LADRC *ladrc, float target);
float LADRC_Update(LADRC *ladrc, float y);
void LADRC_Reset(LADRC *ladrc);
void Diff_speed_ladrc(int16 speed,int16 diff_speed,LADRC *left_speed, LADRC *right_speed);
void LADRC_CopyParams(LADRC *dest, const LADRC *src);
float LADRC_Update_2(LADRC *ladrc, float y);

#endif // _LADRC_H_ 
