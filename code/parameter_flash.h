#ifndef _PARAMETER_FLASH_H_
#define _PARAMETER_FLASH_H_

#include "zf_common_headfile.h"

//===================================================�궨��===================================================
#define MAX_PARAMETERS 64 // Flash�пɴ洢������������
#define PARAMETER_INDEX 1 // ����������ʼ��ַ

//===================================================�ṹ�嶨��===================================================

// �������������ýṹ
typedef struct {
    const char* name;       // ��������
    uint8       index;      // ��parameter�����Flash�е�����
    float*      target_var; // ָ��ʵ��Ҫ�޸ĵ�Ŀ�������ָ��
    float       default_value; // Ĭ��ֵ
    float       min_value;  // ��Сֵ
    float       max_value;  // ���ֵ
    float       step;       // ��������
} ParameterConfig;

// ����ҳ���״̬ö��
typedef enum {
    PARAM_STATE_VIEW, // �鿴ģʽ
    PARAM_STATE_EDIT  // �༭ģʽ
} ParameterState;

//===================================================�ⲿ��������===================================================

// �����ⲿ������ʹ������������ļ��з���
extern float parameter[MAX_PARAMETERS];
extern ParameterConfig param_config[];
extern uint8 parameter_count;
extern volatile uint8 key_flag; // ������־λҲ��Ҫ������UIģ�����

//===================================================�ⲿ��������===================================================

void Parameter_flash_load(void);
void Parameter_flash_save(void);
void Parameter_reset_to_default(void);
void parameter_menu_entry(void);

// ���õĸ�������
float truncate_by_step(float value, float step);
void format_float_truncate(char* buffer, float value, uint8 width, uint8 precision);
uint8 check_parameter_index_conflicts(void);

#endif 
