#include "parameter_flash.h"
#include "zf_device_ips200.h"
#include "key.h"
#include "parameter_scheme.h"
#include <string.h>

//===================================================����������===================================================

// �������ضϸ�����
// �����ڲ���������������ֹ��������ۻ�
float truncate_by_step(float value, float step)
{
    if(step <= 0.0f) return value;  // ������Ч��ֱ�ӷ���

    // ������
    uint8 is_negative = 0;
    if(value < 0.0f)
    {
        is_negative = 1;
        value = -value;
    }

    float offset = step * 0.0001f;      // 1e-4 * step ����΢Сƫ��
    float multiplier = (value + offset) / step;
    uint32 int_multiplier = (uint32)multiplier;  // ȡ��
    float result = (float)int_multiplier * step;

    return is_negative ? -result : result;
}

// ��������ʽ��������ض϶����������룩
// ���磺format_float_truncate(buffer, 1.2345, 7, 2) -> "   1.23 "
void format_float_truncate(char* buffer, float value, uint8 width, uint8 precision)
{
    // ������
    uint8 is_negative = 0;
    if(value < 0.0f)
    {
        is_negative = 1;
        value = -value;
    }
    
    // ��������
    uint32 int_part = (uint32)value;
    
    // С������
    float frac_part = value - (float)int_part;
    
    // ����С��λ
    uint32 frac_digits = 0;
    if(precision > 0)
    {
        uint32 multiplier = 1;
        for(uint8 i = 0; i < precision; i++)
            multiplier *= 10;
        const float epsilon = 1e-6f;
        float scaled = (frac_part + epsilon) * (float)multiplier;
        if(scaled >= (float)multiplier)
            scaled = (float)multiplier - 1.0f;
        frac_digits = (uint32)scaled;
    }
    
    // ��ʽ���ַ���
    char temp[32];
    char fmt[20];

    if(precision == 0)
    {
        sprintf(fmt, "%s%%lu", is_negative ? "-" : "");
        sprintf(temp, fmt, (unsigned long)int_part);
    }
    else
    {
        sprintf(fmt, "%s%%lu.%%0%dlu", is_negative ? "-" : "", precision);
        sprintf(temp, fmt, (unsigned long)int_part, frac_digits);
    }
    
    // ��ȶ��룬ĩβ���ո�
    sprintf(buffer, "%*s ", width, temp);
}

//===================================================�����洢���===================================================

// �����洢����
float parameter[MAX_PARAMETERS];

// �������� - ���ڲ����˵��ʹ洢
ParameterConfig param_config[] = {
    // ����                ����  Ŀ�����         Ĭ��ֵ   ��Сֵ    ���ֵ     ����
    {"Acc_pid:-P",        0, &acc_pid.KP,    0.0f,     0.0f,     1000.0f,   0.01f},
    {"     -KPP",           1, &acc_pid.KPP,    0.0f,     0.0f,     1000.0f,   0.01f},
    {"     -D",           2, &acc_pid.KD,    0.0f,     0.0f,     1000.0f,   0.1f},
    {"     -Limit",       3, &acc_pid.limit, 0.0f,     0.0f,     10000.0f,  10.0f},
    {"Position_pid:-P",        14, &position_pid.KP,    0.0f,     0.0f,     1000.0f,   1.0f},
    {"     -KPP",           15, &position_pid.KPP,    0.0f,     0.0f,     1000.0f,   0.01f},
    {"     -D",           16, &position_pid.KD,    0.0f,     0.0f,     1000.0f,   0.1f},
    {"     -Limit",       17, &position_pid.limit, 0.0f,     0.0f,     10000.0f,  10.0f},
    {"Motor_ladrc:-Wc",   4, &Lspeed_ladrc.wc, 0.0f,     0.0f,     1000.0f,   1.0f},
    {"     -W0",          5, &Lspeed_ladrc.w0, 0.0f,     0.0f,     1000.0f,   4.0f},
    {"     -b",           6, &Lspeed_ladrc.b,  0.0f,     0.0f,     10.0f,     0.01f},
    {"     -Limit",       7, &Lspeed_ladrc.limit, 0.0f,  0.0f,     10000.0f,  100.f},
    {"Angle_pid:-P",      8, &angle_pid.KP,    0.0f,     0.0f,     100.0f,    0.1f},
    {"     -KDD",         9, &angle_pid.KDD,   0.0f,     0.0f,     100.0f,    0.01f},
    {"      -D",          10, &angle_pid.KD,    0.0f,     0.0f,     100.0f,    0.1f},
    {"      -Limit",      11, &angle_pid.limit, 450.0f,   450.0f,   450.0f,    50.0f},
    {"Diff_pid:-P",      18, &diff_pid.KP,    0.0f,     0.0f,     100.0f,    0.01f},
    {"     -KDD",         19, &diff_pid.KDD,   0.0f,     0.0f,     100.0f,    0.01f},
    {"      -D",          20, &diff_pid.KD,    0.0f,     0.0f,     100.0f,    0.01f},
    {"      -Limit",      21, &diff_pid.limit, 0.0f,   0.0f,   450.0f,    10.0f},
    {"speed_base",        12, &speed_base,      0.0f,     0.0f,     10.0f,     0.25f},
    {"FOC_PWM",           13,  &FOC_PWM,         0.0f,    0.0f,    1000.0f,    10.0f},
};

uint8 parameter_count = sizeof(param_config) / sizeof(ParameterConfig);

// �����˵�״̬
static uint8 s_curr_param = 0;
static ParameterState s_param_state = PARAM_STATE_VIEW;
static uint8 s_param_initialized = 0;
static float s_backup_params[MAX_PARAMETERS];



// ����ֵ�仯���
static float s_last_param_values[MAX_PARAMETERS];  // �ϴ���ʾ�Ĳ���ֵ
static uint8 s_param_values_initialized = 0;
static uint8 s_update_pending = 0;  // �Ƿ��д����µĲ�����
static uint32 s_last_update_time = 0;  // �ϴθ���ʱ���
#define UPDATE_INTERVAL_MS 50  // ����ֵˢ����С���(ms)

//===================================================���������뱣��===================================================

void Parameter_flash_load(void)
{
    flash_read_page_to_buffer(0, PARAMETER_INDEX);

    uint8 valid_data = 1;
    for(uint8 i = 0; i < parameter_count; i++)
    {
        uint8 idx = param_config[i].index;
        float value = flash_union_buffer[idx].float_type;
        if(value != value || value > 1e10 || value < -1e10)
        {
            valid_data = 0;
            break;
        }
        parameter[idx] = value;
    }

    if(!valid_data)
    {
        Parameter_reset_to_default();
    }
    else
    {
        // ���ص��ڴ沢ͬ����Ŀ�����
        for(uint8 i = 0; i < parameter_count; i++)
        {
            uint8 idx = param_config[i].index;
            // �����ض�
            parameter[idx] = truncate_by_step(parameter[idx], param_config[i].step);
            if(param_config[i].target_var != NULL)
                *(param_config[i].target_var) = parameter[idx];
        }
    }
}

void Parameter_flash_save(void)
{
    flash_erase_page(0, PARAMETER_INDEX);

    for(uint8 i = 0; i < parameter_count; i++)
    {
        uint8 idx = param_config[i].index;
        // �����ضϺ�洢
        float truncated_value = truncate_by_step(parameter[idx], param_config[i].step);
        parameter[idx] = truncated_value;  // ͬ�����ڴ�
        flash_union_buffer[idx].float_type = truncated_value;
        // ͬ����Ŀ�����
        if(param_config[i].target_var != NULL)
            *(param_config[i].target_var) = truncated_value;
    }

    flash_write_page_from_buffer(0, PARAMETER_INDEX);
}

void Parameter_reset_to_default(void)
{
    for(uint8 i = 0; i < parameter_count; i++)
    {
        uint8 idx = param_config[i].index;
        parameter[idx] = 0;
        //Parameter_flash_save();
    }
}

//===================================================�����˵���ʾ===================================================

// �����˵�ҳ��״̬
static uint8 s_current_page = 0;  // ��ǰҳ
static uint8 s_last_page = 255;  // �ϴ�ҳ��
static uint8 s_last_curr_param = 255;  // �ϴ�ѡ�в���
static ParameterState s_last_state = PARAM_STATE_VIEW;  // �ϴ�״̬

// �����˵���ʾ����
#define PARAM_LINE_HEIGHT   18      // ÿ�и߶�
#define PARAM_LEFT_MARGIN   10      // ��߾�
#define PARAM_VALUE_X_OFFSET 130    // ����ֵ��ʾXƫ��
#define PARAM_TITLE_Y       10      // ����Y����
#define PARAM_START_Y       35      // �����б���ʼY
#define PARAM_LINES_PER_PAGE 12     // ÿҳ������

// �����������ʾ
void clear_param_line(uint8 line_index)
{
    uint16 y_pos = PARAM_START_Y + line_index * PARAM_LINE_HEIGHT;
    uint16 line_width = ips200_width_max - PARAM_LEFT_MARGIN - 10;  // �ұ���10����
    
    // �ú�ɫ���Ǹ���
    for(uint8 i = 0; i < PARAM_LINE_HEIGHT && (y_pos + i) < ips200_height_max; i++)
    {
        ips200_draw_line(PARAM_LEFT_MARGIN, y_pos + i, PARAM_LEFT_MARGIN + line_width, y_pos + i, RGB565_BLACK);
    }
}

// ���Ʋ�����
void draw_param_line(uint8 param_index, uint8 line_index, uint8 curr_param, ParameterState state)
{
    if(param_index >= parameter_count) return;
    
    uint16 y_pos = PARAM_START_Y + line_index * PARAM_LINE_HEIGHT;
    uint16 value_x = PARAM_LEFT_MARGIN + PARAM_VALUE_X_OFFSET;
    
    // ������Ļ�򲻻���
    if(y_pos + PARAM_LINE_HEIGHT > ips200_height_max) return;
    
    // ��ǰѡ�в�������
    if(param_index == curr_param)
    {
        ips200_set_color(state == PARAM_STATE_EDIT ? RGB565_GREEN : RGB565_BLUE, RGB565_BLACK);
    }
    else
    {
        ips200_set_color(RGB565_WHITE, RGB565_BLACK);
    }
    
    // ��ʾ��������
    ips200_show_string(PARAM_LEFT_MARGIN, y_pos, (char*)param_config[param_index].name);
    
    // ��ʾ����ֵ
    char value_str[16];
    format_float_truncate(value_str, parameter[param_config[param_index].index], 7, 2);
    ips200_show_string(value_x, y_pos, value_str);
}

// ���²�����ʾ
void update_operation_hint(ParameterState state)
{
    uint16 hint_y = ips200_height_max - 40;  // ��ײ�40����
    uint16 hint_width = ips200_width_max - PARAM_LEFT_MARGIN * 2;
    
    // �����ʾ����
    for(uint8 i = 0; i < 16 && (hint_y + i) < ips200_height_max; i++)
    {
        ips200_draw_line(PARAM_LEFT_MARGIN, hint_y + i, PARAM_LEFT_MARGIN + hint_width, hint_y + i, RGB565_BLACK);
    }
    
    // ��ʾ��ʾ
    ips200_set_color(RGB565_GRAY, RGB565_BLACK);
    if(state == PARAM_STATE_VIEW)
        ips200_show_string(PARAM_LEFT_MARGIN, hint_y, "1:Up 2:Down 3:Edit 4:Menu");
    else
        ips200_show_string(PARAM_LEFT_MARGIN, hint_y, "1:+ 2:- 3:Save 4:Cancel");
}

// ����ҳ��ָʾ
void update_page_indicator(uint8 current_page, uint8 total_pages)
{
    uint16 page_x = ips200_width_max - 80;  // ���Ͻ�
    uint16 page_width = 70;
    
    // �������
    for(uint8 i = 0; i < 16; i++)
    {
        ips200_draw_line(page_x, PARAM_TITLE_Y + i, page_x + page_width, PARAM_TITLE_Y + i, RGB565_BLACK);
    }
    
    // ��ʾҳ��
    ips200_set_color(RGB565_CYAN, RGB565_BLACK);
    char page_str[20];
    sprintf(page_str, "Page %d/%d", current_page + 1, total_pages);
    ips200_show_string(page_x, PARAM_TITLE_Y, page_str);
    ips200_set_color(RGB565_WHITE, RGB565_BLACK);
}

// ��ȡ��������ҳ��
uint8 get_param_page(uint8 param_index)
{
    return param_index / PARAM_LINES_PER_PAGE;
}

// ��ȡ��ҳ��
uint8 get_total_pages(void)
{
    return (parameter_count + PARAM_LINES_PER_PAGE - 1) / PARAM_LINES_PER_PAGE;  // ����ȡ��
}

// ������ҳ��������������
void draw_full_page(uint8 page, uint8 curr_param, ParameterState state)
{
    uint8 start_idx = page * PARAM_LINES_PER_PAGE;
    uint8 end_idx = start_idx + PARAM_LINES_PER_PAGE;
    if(end_idx > parameter_count) end_idx = parameter_count;
    
    // ���Ʊ�ҳ���в���
    for(uint8 i = start_idx; i < end_idx; i++)
    {
        draw_param_line(i, i - start_idx, curr_param, state);
    }
    
    // ����ҳ��ָʾ
    update_page_indicator(page, get_total_pages());
}

// ֻ���²����У��л�ѡ��ʱ��
void update_single_param_line(uint8 old_param, uint8 new_param, ParameterState state)
{
    uint8 old_page = get_param_page(old_param);
    uint8 new_page = get_param_page(new_param);
    
    // ͬһҳ����Ҫˢ��
    if(old_page == new_page && old_page == s_current_page)
    {
        // �ɲ�����
        uint8 old_line = old_param % PARAM_LINES_PER_PAGE;
        draw_param_line(old_param, old_line, new_param, state);
        
        // �²�����
        uint8 new_line = new_param % PARAM_LINES_PER_PAGE;
        draw_param_line(new_param, new_line, new_param, state);
    }
}

// ֻ���²���ֵ����ˢ�����У�
void update_param_value_only(uint8 param_index)
{
    if(get_param_page(param_index) == s_current_page)
    {
        // ��ʼ���ϴβ���ֵ
        if(!s_param_values_initialized)
        {
            for(uint8 i = 0; i < parameter_count; i++)
                s_last_param_values[i] = -999999.0f;  // �����ܵ�ֵ
            s_param_values_initialized = 1;
        }
        
        // ��Ǵ�����
        s_update_pending = param_index + 1;  // +1��ֹ0������Ϊ�޸���
    }
}

// ִ�в���ֵˢ��
void perform_param_value_update(void)
{
    static uint32 current_time = 0;
    
    if(s_update_pending == 0) return;
    
    // ��ȡ��ǰʱ��
    current_time = system_getval_ms();
    
    // ����ˢ��Ƶ��
    if(current_time - s_last_update_time < UPDATE_INTERVAL_MS) return;
    
    uint8 param_index = s_update_pending - 1;
    
    // ������ֵ�Ƿ�仯
    uint8 idx = param_config[param_index].index;
    if(parameter[idx] != s_last_param_values[param_index])
    {
        uint8 line_index = param_index % PARAM_LINES_PER_PAGE;
        uint16 y_pos = PARAM_START_Y + line_index * PARAM_LINE_HEIGHT;
        uint16 value_x = PARAM_LEFT_MARGIN + PARAM_VALUE_X_OFFSET;
        
        // ������ʾ
        ips200_set_color(RGB565_GREEN, RGB565_BLACK);
        
        // ��ʾ��ֵ
        char value_str[16];
        format_float_truncate(value_str, parameter[idx], 7, 2);
        ips200_show_string(value_x, y_pos, value_str);
        
        ips200_set_color(RGB565_WHITE, RGB565_BLACK);
        
        // ��¼����ֵ
        s_last_param_values[param_index] = parameter[idx];
    }
    
    // ��������±��
    s_update_pending = 0;
    s_last_update_time = current_time;
}

// ��ʼ�������˵���ʾ
void init_param_display(uint8 curr_param, ParameterState state)
{
    ips200_clear();
    
    // ���ò���ֵˢ��״̬
    s_param_values_initialized = 0;
    
    // ��ʾ����
    ips200_set_color(RGB565_YELLOW, RGB565_BLACK);
    ips200_show_string(PARAM_LEFT_MARGIN, PARAM_TITLE_Y, "Parameter Config");
    
    // ���㵱ǰҳ
    s_current_page = get_param_page(curr_param);
    
    // ���Ʋ����б�
    draw_full_page(s_current_page, curr_param, state);
    
    // ��ʾ������ʾ
    update_operation_hint(state);
    
    ips200_set_color(RGB565_WHITE, RGB565_BLACK);
    
    // ��¼״̬
    s_last_page = s_current_page;
    s_last_curr_param = curr_param;
    s_last_state = state;
}

// �����˵������ƺ���
void draw_param_page(uint8 curr_param, ParameterState state)
{
    // �״ν���˵�
    if(s_last_curr_param == 255)
    {
        init_param_display(curr_param, state);
        return;
    }
    
    // ������ҳ��
    uint8 new_page = get_param_page(curr_param);
    
    // �л�ҳ��
    if(new_page != s_current_page)
    {
        s_current_page = new_page;
        // ����
        ips200_clear();
        
        // ��ʾ����
        ips200_set_color(RGB565_YELLOW, RGB565_BLACK);
        ips200_show_string(PARAM_LEFT_MARGIN, PARAM_TITLE_Y, "Parameter Config");
        
        // ���Ʋ����б�
        draw_full_page(s_current_page, curr_param, state);
        
        // ��ʾ������ʾ
        update_operation_hint(state);
        
        ips200_set_color(RGB565_WHITE, RGB565_BLACK);
    }
    // ͬҳ�л�������״̬
    else
    {
        // ֻ�л�ѡ�в���
        if(s_last_curr_param != curr_param && state == s_last_state)
        {
            update_single_param_line(s_last_curr_param, curr_param, state);
        }
        // ״̬�л��������/�˳��༭��
        else if(state != s_last_state)
        {
            // ֻˢ�µ�ǰ��
            uint8 line = curr_param % PARAM_LINES_PER_PAGE;
            clear_param_line(line);
            draw_param_line(curr_param, line, curr_param, state);
            
            // ���²�����ʾ
            update_operation_hint(state);
        }
    }
    
    // ��¼״̬
    s_last_page = s_current_page;
    s_last_curr_param = curr_param;
    s_last_state = state;
}

uint8 parameter_adjust_page(void)
{
    if(!s_param_initialized)
    {
        s_curr_param = 0;
        s_param_state = PARAM_STATE_VIEW;
        s_param_initialized = 1;
        
        // ��ʼ��ҳ��״̬
        s_current_page = 0;
        s_last_page = 255;
        s_last_curr_param = 255;
        s_last_state = PARAM_STATE_VIEW;
        s_update_pending = 0;  // �޴�����
        
        // ���ݲ���
        for(uint8 i = 0; i < parameter_count; i++)
            s_backup_params[i] = parameter[param_config[i].index];
            
        draw_param_page(s_curr_param, s_param_state);
        return 0;
    }
    
    // ������ֵ�Ƿ���Ҫˢ��
    perform_param_value_update();
    
    if(key_flag)
    {
        uint8 key = key_flag;
        key_flag = 0;
        
        if(s_param_state == PARAM_STATE_VIEW)
        {
            switch(key)
            {
                case Button_1: // ����
                    s_curr_param = (s_curr_param > 0) ? (s_curr_param - 1) : (parameter_count - 1);
                    draw_param_page(s_curr_param, s_param_state);
                    break;
                    
                case Button_2: // ����
                    s_curr_param = (s_curr_param < parameter_count - 1) ? (s_curr_param + 1) : 0;
                    draw_param_page(s_curr_param, s_param_state);
                    break;
                    
                case Button_3: // ����༭
                    s_param_state = PARAM_STATE_EDIT;
                    draw_param_page(s_curr_param, s_param_state);
                    break;
                    
                case Button_4: // ���ز˵�
                    s_param_initialized = 0;
                    // ����״̬
                    s_last_curr_param = 255;
                    s_last_page = 255;
                    return 1;
            }
        }
        else // PARAM_STATE_EDIT
        {
            switch(key)
            {
                case Button_1: // ����
                {
                    uint8 idx = param_config[s_curr_param].index;
                    float new_val = parameter[idx] + param_config[s_curr_param].step;
                    if(new_val <= param_config[s_curr_param].max_value)
                    {
                        // �����ض�
                        new_val = truncate_by_step(new_val, param_config[s_curr_param].step);
                        parameter[idx] = new_val;
                        if(param_config[s_curr_param].target_var != NULL)
                            *(param_config[s_curr_param].target_var) = new_val;
                        // ˢ����ʾ
                        update_param_value_only(s_curr_param);
                    }
                    break;
                }
                
                case Button_2: // ����
                {
                    uint8 idx = param_config[s_curr_param].index;
                    float new_val = parameter[idx] - param_config[s_curr_param].step;
                    if(new_val >= param_config[s_curr_param].min_value)
                    {
                        // �����ض�
                        new_val = truncate_by_step(new_val, param_config[s_curr_param].step);
                        parameter[idx] = new_val;
                        if(param_config[s_curr_param].target_var != NULL)
                            *(param_config[s_curr_param].target_var) = new_val;
                        // ˢ����ʾ
                        update_param_value_only(s_curr_param);
                    }
                    break;
                }
                
                case Button_3: // ����
                    s_param_state = PARAM_STATE_VIEW;
                    Parameter_flash_save();
                    draw_param_page(s_curr_param, s_param_state);
                    break;
                    
                case Button_4: // ȡ��
                {
                    uint8 idx = param_config[s_curr_param].index;
                    parameter[idx] = s_backup_params[s_curr_param];
                    if(param_config[s_curr_param].target_var != NULL)
                        *(param_config[s_curr_param].target_var) = parameter[idx];
                    s_param_state = PARAM_STATE_VIEW;
                    draw_param_page(s_curr_param, s_param_state);
                    break;
                }
            }
        }
    }
    
    return 0;
}





//===================================================�����˵����===================================================

void parameter_menu_entry(void)
{
    ips200_clear();
    // ��ʾ������ʾ
    uint16 msg_y = ips200_height_max / 2 - 10;
    ips200_show_string(PARAM_LEFT_MARGIN, msg_y, "Loading parameters...");

    Parameter_flash_load();
    system_delay_ms(500);

    s_param_initialized = 0;
    uint8 s_params_modified = 0;
    uint8 exit_flag = 0;
    key_flag = 0;

    // ��¼��ʼ����
    float initial_params[MAX_PARAMETERS];
    for(uint8 i = 0; i < parameter_count; i++)
        initial_params[i] = parameter[param_config[i].index];

    while(!exit_flag)
    {
        exit_flag = parameter_adjust_page();
        system_delay_ms(10);
    }

    // �������Ƿ����޸�
    for(uint8 i = 0; i < parameter_count; i++)
    {
        if(parameter[param_config[i].index] != initial_params[i])
        {
            s_params_modified = 1;
            break;
        }
    }

    // ���޸�����ʾ����
    if(s_params_modified)
    {
        ips200_clear();
        ips200_set_color(RGB565_GREEN, RGB565_BLACK);
        ips200_show_string(PARAM_LEFT_MARGIN, msg_y, "Parameters Saved!");
        ips200_show_string(PARAM_LEFT_MARGIN, msg_y + 20, "Returning to menu...");
        ips200_set_color(RGB565_WHITE, RGB565_BLACK);
        system_delay_ms(500);
    }

    // �������˵�ʱģ�ⰴ��Button_4
    key_flag = Button_4;
}



//===================================================����������ͻ���===================================================

// �����������Ƿ��г�ͻ
uint8 check_parameter_index_conflicts(void)
{
    uint8 index_used[MAX_PARAMETERS] = {0};  // ���ÿ�������Ƿ�ʹ��
    uint8 conflicts = 0;
    
    for(uint8 i = 0; i < parameter_count; i++)
    {
        uint8 idx = param_config[i].index;
        
        // ��������Ƿ�Խ��
        if(idx >= MAX_PARAMETERS)
        {
            conflicts++;
            continue;
        }
        
        // ����Ƿ��ظ�
        if(index_used[idx])
        {
            conflicts++;
        }
        else
        {
            index_used[idx] = 1;
        }
    }
    
    return conflicts;
} 
