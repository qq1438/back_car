#ifndef _PARAMETER_SCHEME_H_
#define _PARAMETER_SCHEME_H_

#include "zf_common_headfile.h"
#include "parameter_flash.h"

#ifndef MAX_PARAMETERS
#define MAX_PARAMETERS 64
#endif

#define MAX_SCHEMES 10
#define MAX_SCHEME_NAME_LEN 16
#define MAX_SCHEME_PARAMS 30
#define SCHEME_STORAGE_INDEX 3

// W25N04备份相关定义
#define W25N04_BACKUP_PAGE_ADDR 0  

// 单个参数存储项
typedef struct {
    uint8  index; // 对应 param_config 中的 index
    float  value;
} SchemeParam;

// 修改后的方案数据结构 — 直接存储特定参数的指针和值
typedef struct {
    uint8       is_used;                          // 1 表示已使用, 0 表示空闲
    char        name[MAX_SCHEME_NAME_LEN];        // 方案名称
    uint8       param_count;                      // 实际存储的参数数量
    SchemeParam params[MAX_SCHEME_PARAMS];        // 参数指针和值的数组
} ParameterScheme;

// 方案管理页面的主入口函数
void scheme_menu_entry(void);

#endif // _PARAMETER_SCHEME_H_ 
