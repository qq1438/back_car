#ifndef _KALMAN_FILTER_H_
#define _KALMAN_FILTER_H_

#include "zf_common_headfile.h"
#include <stdint.h>

// 定点格式缩放因子（Q10 定点格式）
#define KF_SCALE (1 << 10)

// 卡尔曼滤波器结构体，用定点数表示
typedef struct {
    int32_t P;   // 误差协方差
    int32_t Q;   // 过程噪声方差
    int32_t R;   // 测量噪声方差
    int32_t X;   // 状态估计值
    int32_t K;   // 卡尔曼增益
} KalmanFilter;

/**
 * @brief 初始化定点卡尔曼滤波器
 * @param kf         滤波器实例指针
 * @param Q          过程噪声方差（定点值，已乘以 KF_SCALE）
 * @param R          测量噪声方差（定点值，已乘以 KF_SCALE）
 * @param initial_x  初始状态估计值（定点值，已乘以 KF_SCALE）
 */
void KF_Init(KalmanFilter *kf, int32_t Q, int32_t R, int32_t initial_x);

/**
 * @brief 使用定点测量值更新滤波器
 * @param kf           滤波器实例指针
 * @param measurement  当前测量值（定点值，已乘以 KF_SCALE）
 * @return             更新后的状态估计值（定点值）
 */
int32_t KF_Update(KalmanFilter *kf, int32_t measurement);

/**
 * @brief 一次性浮点接口：输入浮点测量值，输出浮点估计值
 * @param kf           滤波器实例指针
 * @param measurement  浮点测量值
 * @return             浮点估计值
 */
float KF_Filter(KalmanFilter *kf, float measurement);

#endif // _KALMAN_FILTER_H_ 