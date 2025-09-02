#ifndef _INFRARED_NEC_H_
#define _INFRARED_NEC_H_

#include "zf_common_headfile.h"

/**
 * @brief 初始化NEC红外接收模块，配置P10_6引脚为输入并启动系统定时器
 */
void NEC_Init(void);

/**
 * @brief 读取NEC红外接收模块的输入电平
 * @return 返回输入电平状态
 */
bool IR_Read(void);

/**
 * @brief 接收一帧NEC红外遥控信号并解析为32位数据
 * @param data 指向接收数据的指针
 * @return 接收成功返回true，否则返回false
 */
bool NEC_Receive(uint16 *data);

#endif // _INFRARED_NEC_H_ 