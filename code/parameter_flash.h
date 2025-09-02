#ifndef _PARAMETER_FLASH_H_
#define _PARAMETER_FLASH_H_

#include "zf_common_headfile.h"

//===================================================宏定义===================================================
#define MAX_PARAMETERS 64 // Flash中可存储的最大参数数量
#define PARAMETER_INDEX 1 // 参数扇区起始地址

//===================================================结构体定义===================================================

// 单个参数的配置结构
typedef struct {
    const char* name;       // 参数名称
    uint8       index;      // 在parameter数组和Flash中的索引
    float*      target_var; // 指向实际要修改的目标变量的指针
    float       default_value; // 默认值
    float       min_value;  // 最小值
    float       max_value;  // 最大值
    float       step;       // 调整步长
} ParameterConfig;

// 参数页面的状态枚举
typedef enum {
    PARAM_STATE_VIEW, // 查看模式
    PARAM_STATE_EDIT  // 编辑模式
} ParameterState;

//===================================================外部变量声明===================================================

// 声明外部变量，使其可以在其他文件中访问
extern float parameter[MAX_PARAMETERS];
extern ParameterConfig param_config[];
extern uint8 parameter_count;
extern volatile uint8 key_flag; // 按键标志位也需要被其他UI模块访问

//===================================================外部函数声明===================================================

void Parameter_flash_load(void);
void Parameter_flash_save(void);
void Parameter_reset_to_default(void);
void parameter_menu_entry(void);

// 公用的辅助函数
float truncate_by_step(float value, float step);
void format_float_truncate(char* buffer, float value, uint8 width, uint8 precision);
uint8 check_parameter_index_conflicts(void);

#endif 
