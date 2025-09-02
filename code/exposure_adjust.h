#ifndef __EXPOSURE_ADJUST_H__
#define __EXPOSURE_ADJUST_H__

#include "zf_common_headfile.h"

// ʹ���ڲ� Flash ��2ҳ�洢�ع�ʱ��
#define EXPOSURE_FLASH_PAGE     2

// ���ڲ� Flash ��ȡ�ع�ʱ�䵽 exp_time, ����Ч��ʹ��Ĭ��
void ExposureTime_flash_load(uint16 *exp_time);

// ���ع�ʱ�� exp_time ���浽�ڲ� Flash
void ExposureTime_flash_save(uint16 exp_time);

// �ع�ʱ���������
void exposure_time_adjust(void);

#endif // __EXPOSURE_ADJUST_H__ 