#ifndef _KALMAN_FILTER_H_
#define _KALMAN_FILTER_H_

#include "zf_common_headfile.h"
#include <stdint.h>

// �����ʽ�������ӣ�Q10 �����ʽ��
#define KF_SCALE (1 << 10)

// �������˲����ṹ�壬�ö�������ʾ
typedef struct {
    int32_t P;   // ���Э����
    int32_t Q;   // ������������
    int32_t R;   // ������������
    int32_t X;   // ״̬����ֵ
    int32_t K;   // ����������
} KalmanFilter;

/**
 * @brief ��ʼ�����㿨�����˲���
 * @param kf         �˲���ʵ��ָ��
 * @param Q          ���������������ֵ���ѳ��� KF_SCALE��
 * @param R          ���������������ֵ���ѳ��� KF_SCALE��
 * @param initial_x  ��ʼ״̬����ֵ������ֵ���ѳ��� KF_SCALE��
 */
void KF_Init(KalmanFilter *kf, int32_t Q, int32_t R, int32_t initial_x);

/**
 * @brief ʹ�ö������ֵ�����˲���
 * @param kf           �˲���ʵ��ָ��
 * @param measurement  ��ǰ����ֵ������ֵ���ѳ��� KF_SCALE��
 * @return             ���º��״̬����ֵ������ֵ��
 */
int32_t KF_Update(KalmanFilter *kf, int32_t measurement);

/**
 * @brief һ���Ը���ӿڣ����븡�����ֵ������������ֵ
 * @param kf           �˲���ʵ��ָ��
 * @param measurement  �������ֵ
 * @return             �������ֵ
 */
float KF_Filter(KalmanFilter *kf, float measurement);

#endif // _KALMAN_FILTER_H_ 