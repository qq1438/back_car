#include "ladrc.h"

/**
 * @brief ��ʼ��LADRC������
 * @param ladrc LADRC�������ṹ��ָ��
 * @param Ts ��������
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
 * @brief ����LADRC������Ŀ��ֵ
 * @param ladrc LADRC�������ṹ��ָ��
 * @param target Ŀ��ֵ
 */
void LADRC_SetTarget(LADRC *ladrc, float target)
{
    ladrc->target = target;
}

/**
 * @brief LADRC���������º���
 * @param ladrc LADRC�������ṹ��ָ��
 * @param y ϵͳ�������ֵ
 * @return �������u
 */
float LADRC_Update(LADRC *ladrc, float y)
{
    float wc = ladrc->wc;
    float w0 = ladrc->w0;
    float b = ladrc->b;
    float Ts = ladrc->Ts;
    
    
    // ����״̬�۲�����ESO��
    // �۲������
    float e = y - ladrc->z1;
    
    // ESO���·��̣���ɢ����
    // z1(k+1) = z1(k) + Ts * (z2(k) + 2*w0*e + b*u(k))
    // z2(k+1) = z2(k) + Ts * (w0^2 * e)
    float z1_new = ladrc->z1 + Ts * (ladrc->z2 + 2.0f * w0 * e + b * ladrc->last_u);
    float z2_new = ladrc->z2 + Ts * (w0 * w0 * e);
    
    // ����ESO״̬
    ladrc->z1 = z1_new;
    ladrc->z2 = z2_new;
    
    // ����״̬����������
    // u0 = wc * (r - z1)
    float u0 = wc * (ladrc->target - ladrc->z1);
    
    // �Ŷ�����
    // u = (u0 - z2) / b
    float u = (u0 - ladrc->z2) / b;
    
    // ����޷�
    if (u > ladrc->limit)
    {
        u = ladrc->limit;
    }
    else if (u < -ladrc->limit)
    {
        u = -ladrc->limit;
    }
    
    // �������������´�ESO����ʹ��
    ladrc->last_u = u;
    
    return u;
}

float LADRC_Update_2(LADRC *ladrc, float y) {
    float wc = ladrc->wc;
    float w0 = ladrc->w0;
    float b = ladrc->b;
    float Ts = ladrc->Ts;
    
    // ����ESO������z3��
    float e = y - ladrc->z1;
    
    // ESO��ɢ���·��̣�������ʽ��
    float z1_new = ladrc->z1 + Ts * (ladrc->z2 + 3.0f*w0*e);          // ԭ2*w0��Ϊ3*w0
    float z2_new = ladrc->z2 + Ts * (ladrc->z3 + 3.0f*w0*w0*e + b*ladrc->last_u); // ����z3��
    float z3_new = ladrc->z3 + Ts * (w0*w0*w0*e);                     // ����z3��̬
    
    ladrc->z1 = z1_new;
    ladrc->z2 = z2_new;
    ladrc->z3 = z3_new;

    float e1 = ladrc->target - ladrc->z1;
    float e2 = ladrc->target_dot - ladrc->z2;
    float u0 = wc*wc*e1 + 2.0f*wc*e2;          // PD����
    float u = (u0 - ladrc->z3) / b;
    
    // ����޷�
    if (u > ladrc->limit)
    {
        u = ladrc->limit;
    }
    else if (u < -ladrc->limit)
    {
        u = -ladrc->limit;
    }
    
    // �������������´�ESO����ʹ��
    ladrc->last_u = u;
    return u;
}

/**
 * @brief ����LADRC������״̬
 * @param ladrc LADRC�������ṹ��ָ��
 */
void LADRC_Reset(LADRC *ladrc)
{
    ladrc->z1 = 0.0f;
    ladrc->z2 = 0.0f;
    ladrc->last_u = 0.0f;
} 

/**
 * @brief ����LADRC�������Ĳ���
 * @param dest Ŀ��LADRC�������ṹ��ָ��
 * @param src ԴLADRC�������ṹ��ָ��
 */
void LADRC_CopyParams(LADRC *dest, const LADRC *src)
{
    if (dest == NULL || src == NULL)
    {
        // �����ָ����󣬻���ֱ�ӷ���
        return;
    }

    // ���Ʋ�����������״̬���� (z1, z2, last_u)
    dest->w0 = src->w0;
    dest->wc = src->wc;
    dest->b = src->b;
    dest->limit = src->limit;
    dest->target = src->target; // Ŀ��ֵҲ��Ϊ������һ���֣����Ա�����
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

