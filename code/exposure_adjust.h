#ifndef __EXPOSURE_ADJUST_H__
#define __EXPOSURE_ADJUST_H__

#include "zf_common_headfile.h"

// 使用内部 Flash 第2页存储曝光时间
#define EXPOSURE_FLASH_PAGE     2

// 从内部 Flash 读取曝光时间到 exp_time, 若无效则使用默认
void ExposureTime_flash_load(uint16 *exp_time);

// 将曝光时间 exp_time 保存到内部 Flash
void ExposureTime_flash_save(uint16 exp_time);

// 曝光时间调整功能
void exposure_time_adjust(void);

#endif // __EXPOSURE_ADJUST_H__ 