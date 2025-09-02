#include "kalman_filter.h"

/**
 * @brief 初始化定点卡尔曼滤波器
 */
void KF_Init(KalmanFilter *kf, int32_t Q, int32_t R, int32_t initial_x) {
    kf->Q = Q;
    kf->R = R;
    kf->P = KF_SCALE;      // 初始误差协方差设为缩放因子
    kf->X = initial_x;
    kf->K = 0;
}

/**
 * @brief 使用定点测量值更新滤波器
 */
int32_t KF_Update(KalmanFilter *kf, int32_t measurement) {
    // 预测协方差
    kf->P += kf->Q;

    // 计算卡尔曼增益 K = P / (P + R)
    kf->K = (int32_t)((kf->P * (int64_t)KF_SCALE) / (kf->P + kf->R));

    // 更新状态估计 X = X + K * (measurement - X)
    kf->X += (int32_t)((kf->K * (int64_t)(measurement - kf->X)) / KF_SCALE);

    // 更新协方差 P = (1 - K) * P
    kf->P = (int32_t)(((int64_t)(KF_SCALE - kf->K) * kf->P) / KF_SCALE);

    return kf->X;
}

/**
 * @brief 一次性浮点接口：输入浮点测量值，输出浮点估计值
 */
float KF_Filter(KalmanFilter *kf, float measurement) {
    int32_t meas = (int32_t)(measurement * KF_SCALE);
    int32_t out = KF_Update(kf, meas);
    return (float)out / KF_SCALE;
} 