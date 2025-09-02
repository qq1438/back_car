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

// W25N04������ض���
#define W25N04_BACKUP_PAGE_ADDR 0  

// ���������洢��
typedef struct {
    uint8  index; // ��Ӧ param_config �е� index
    float  value;
} SchemeParam;

// �޸ĺ�ķ������ݽṹ �� ֱ�Ӵ洢�ض�������ָ���ֵ
typedef struct {
    uint8       is_used;                          // 1 ��ʾ��ʹ��, 0 ��ʾ����
    char        name[MAX_SCHEME_NAME_LEN];        // ��������
    uint8       param_count;                      // ʵ�ʴ洢�Ĳ�������
    SchemeParam params[MAX_SCHEME_PARAMS];        // ����ָ���ֵ������
} ParameterScheme;

// ��������ҳ�������ں���
void scheme_menu_entry(void);

#endif // _PARAMETER_SCHEME_H_ 
