#ifndef _INFRARED_NEC_H_
#define _INFRARED_NEC_H_

#include "zf_common_headfile.h"

/**
 * @brief ��ʼ��NEC�������ģ�飬����P10_6����Ϊ���벢����ϵͳ��ʱ��
 */
void NEC_Init(void);

/**
 * @brief ��ȡNEC�������ģ��������ƽ
 * @return ���������ƽ״̬
 */
bool IR_Read(void);

/**
 * @brief ����һ֡NEC����ң���źŲ�����Ϊ32λ����
 * @param data ָ��������ݵ�ָ��
 * @return ���ճɹ�����true�����򷵻�false
 */
bool NEC_Receive(uint16 *data);

#endif // _INFRARED_NEC_H_ 