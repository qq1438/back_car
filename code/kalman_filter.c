#include "kalman_filter.h"

/**
 * @brief ��ʼ�����㿨�����˲���
 */
void KF_Init(KalmanFilter *kf, int32_t Q, int32_t R, int32_t initial_x) {
    kf->Q = Q;
    kf->R = R;
    kf->P = KF_SCALE;      // ��ʼ���Э������Ϊ��������
    kf->X = initial_x;
    kf->K = 0;
}

/**
 * @brief ʹ�ö������ֵ�����˲���
 */
int32_t KF_Update(KalmanFilter *kf, int32_t measurement) {
    // Ԥ��Э����
    kf->P += kf->Q;

    // ���㿨�������� K = P / (P + R)
    kf->K = (int32_t)((kf->P * (int64_t)KF_SCALE) / (kf->P + kf->R));

    // ����״̬���� X = X + K * (measurement - X)
    kf->X += (int32_t)((kf->K * (int64_t)(measurement - kf->X)) / KF_SCALE);

    // ����Э���� P = (1 - K) * P
    kf->P = (int32_t)(((int64_t)(KF_SCALE - kf->K) * kf->P) / KF_SCALE);

    return kf->X;
}

/**
 * @brief һ���Ը���ӿڣ����븡�����ֵ������������ֵ
 */
float KF_Filter(KalmanFilter *kf, float measurement) {
    int32_t meas = (int32_t)(measurement * KF_SCALE);
    int32_t out = KF_Update(kf, meas);
    return (float)out / KF_SCALE;
} 