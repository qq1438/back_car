#include "parameter_flash.h"
#include "zf_device_ips200.h"
#include "key.h"
#include "parameter_scheme.h"
#include <string.h>

//===================================================参数处理函数===================================================

// 按步长截断浮点数
// 适用于参数步进调整，防止精度误差累积
float truncate_by_step(float value, float step)
{
    if(step <= 0.0f) return value;  // 步长无效则直接返回

    // 处理负数
    uint8 is_negative = 0;
    if(value < 0.0f)
    {
        is_negative = 1;
        value = -value;
    }

    float offset = step * 0.0001f;      // 1e-4 * step 用于微小偏移
    float multiplier = (value + offset) / step;
    uint32 int_multiplier = (uint32)multiplier;  // 取整
    float result = (float)int_multiplier * step;

    return is_negative ? -result : result;
}

// 浮点数格式化输出（截断而非四舍五入）
// 例如：format_float_truncate(buffer, 1.2345, 7, 2) -> "   1.23 "
void format_float_truncate(char* buffer, float value, uint8 width, uint8 precision)
{
    // 处理负数
    uint8 is_negative = 0;
    if(value < 0.0f)
    {
        is_negative = 1;
        value = -value;
    }
    
    // 整数部分
    uint32 int_part = (uint32)value;
    
    // 小数部分
    float frac_part = value - (float)int_part;
    
    // 计算小数位
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
    
    // 格式化字符串
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
    
    // 宽度对齐，末尾补空格
    sprintf(buffer, "%*s ", width, temp);
}

//===================================================参数存储相关===================================================

// 参数存储数组
float parameter[MAX_PARAMETERS];

// 参数配置 - 用于参数菜单和存储
ParameterConfig param_config[] = {
    // 名称                索引  目标变量         默认值   最小值    最大值     步长
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

// 参数菜单状态
static uint8 s_curr_param = 0;
static ParameterState s_param_state = PARAM_STATE_VIEW;
static uint8 s_param_initialized = 0;
static float s_backup_params[MAX_PARAMETERS];



// 参数值变化检测
static float s_last_param_values[MAX_PARAMETERS];  // 上次显示的参数值
static uint8 s_param_values_initialized = 0;
static uint8 s_update_pending = 0;  // 是否有待更新的参数行
static uint32 s_last_update_time = 0;  // 上次更新时间戳
#define UPDATE_INTERVAL_MS 50  // 参数值刷新最小间隔(ms)

//===================================================参数加载与保存===================================================

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
        // 加载到内存并同步到目标变量
        for(uint8 i = 0; i < parameter_count; i++)
        {
            uint8 idx = param_config[i].index;
            // 步长截断
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
        // 步长截断后存储
        float truncated_value = truncate_by_step(parameter[idx], param_config[i].step);
        parameter[idx] = truncated_value;  // 同步到内存
        flash_union_buffer[idx].float_type = truncated_value;
        // 同步到目标变量
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

//===================================================参数菜单显示===================================================

// 参数菜单页面状态
static uint8 s_current_page = 0;  // 当前页
static uint8 s_last_page = 255;  // 上次页面
static uint8 s_last_curr_param = 255;  // 上次选中参数
static ParameterState s_last_state = PARAM_STATE_VIEW;  // 上次状态

// 参数菜单显示布局
#define PARAM_LINE_HEIGHT   18      // 每行高度
#define PARAM_LEFT_MARGIN   10      // 左边距
#define PARAM_VALUE_X_OFFSET 130    // 参数值显示X偏移
#define PARAM_TITLE_Y       10      // 标题Y坐标
#define PARAM_START_Y       35      // 参数列表起始Y
#define PARAM_LINES_PER_PAGE 12     // 每页参数数

// 清除参数行显示
void clear_param_line(uint8 line_index)
{
    uint16 y_pos = PARAM_START_Y + line_index * PARAM_LINE_HEIGHT;
    uint16 line_width = ips200_width_max - PARAM_LEFT_MARGIN - 10;  // 右边留10像素
    
    // 用黑色覆盖该行
    for(uint8 i = 0; i < PARAM_LINE_HEIGHT && (y_pos + i) < ips200_height_max; i++)
    {
        ips200_draw_line(PARAM_LEFT_MARGIN, y_pos + i, PARAM_LEFT_MARGIN + line_width, y_pos + i, RGB565_BLACK);
    }
}

// 绘制参数行
void draw_param_line(uint8 param_index, uint8 line_index, uint8 curr_param, ParameterState state)
{
    if(param_index >= parameter_count) return;
    
    uint16 y_pos = PARAM_START_Y + line_index * PARAM_LINE_HEIGHT;
    uint16 value_x = PARAM_LEFT_MARGIN + PARAM_VALUE_X_OFFSET;
    
    // 超出屏幕则不绘制
    if(y_pos + PARAM_LINE_HEIGHT > ips200_height_max) return;
    
    // 当前选中参数高亮
    if(param_index == curr_param)
    {
        ips200_set_color(state == PARAM_STATE_EDIT ? RGB565_GREEN : RGB565_BLUE, RGB565_BLACK);
    }
    else
    {
        ips200_set_color(RGB565_WHITE, RGB565_BLACK);
    }
    
    // 显示参数名称
    ips200_show_string(PARAM_LEFT_MARGIN, y_pos, (char*)param_config[param_index].name);
    
    // 显示参数值
    char value_str[16];
    format_float_truncate(value_str, parameter[param_config[param_index].index], 7, 2);
    ips200_show_string(value_x, y_pos, value_str);
}

// 更新操作提示
void update_operation_hint(ParameterState state)
{
    uint16 hint_y = ips200_height_max - 40;  // 距底部40像素
    uint16 hint_width = ips200_width_max - PARAM_LEFT_MARGIN * 2;
    
    // 清除提示区域
    for(uint8 i = 0; i < 16 && (hint_y + i) < ips200_height_max; i++)
    {
        ips200_draw_line(PARAM_LEFT_MARGIN, hint_y + i, PARAM_LEFT_MARGIN + hint_width, hint_y + i, RGB565_BLACK);
    }
    
    // 显示提示
    ips200_set_color(RGB565_GRAY, RGB565_BLACK);
    if(state == PARAM_STATE_VIEW)
        ips200_show_string(PARAM_LEFT_MARGIN, hint_y, "1:Up 2:Down 3:Edit 4:Menu");
    else
        ips200_show_string(PARAM_LEFT_MARGIN, hint_y, "1:+ 2:- 3:Save 4:Cancel");
}

// 更新页面指示
void update_page_indicator(uint8 current_page, uint8 total_pages)
{
    uint16 page_x = ips200_width_max - 80;  // 右上角
    uint16 page_width = 70;
    
    // 清除区域
    for(uint8 i = 0; i < 16; i++)
    {
        ips200_draw_line(page_x, PARAM_TITLE_Y + i, page_x + page_width, PARAM_TITLE_Y + i, RGB565_BLACK);
    }
    
    // 显示页码
    ips200_set_color(RGB565_CYAN, RGB565_BLACK);
    char page_str[20];
    sprintf(page_str, "Page %d/%d", current_page + 1, total_pages);
    ips200_show_string(page_x, PARAM_TITLE_Y, page_str);
    ips200_set_color(RGB565_WHITE, RGB565_BLACK);
}

// 获取参数所在页码
uint8 get_param_page(uint8 param_index)
{
    return param_index / PARAM_LINES_PER_PAGE;
}

// 获取总页数
uint8 get_total_pages(void)
{
    return (parameter_count + PARAM_LINES_PER_PAGE - 1) / PARAM_LINES_PER_PAGE;  // 向上取整
}

// 绘制整页参数（含高亮）
void draw_full_page(uint8 page, uint8 curr_param, ParameterState state)
{
    uint8 start_idx = page * PARAM_LINES_PER_PAGE;
    uint8 end_idx = start_idx + PARAM_LINES_PER_PAGE;
    if(end_idx > parameter_count) end_idx = parameter_count;
    
    // 绘制本页所有参数
    for(uint8 i = start_idx; i < end_idx; i++)
    {
        draw_param_line(i, i - start_idx, curr_param, state);
    }
    
    // 更新页码指示
    update_page_indicator(page, get_total_pages());
}

// 只更新参数行（切换选中时）
void update_single_param_line(uint8 old_param, uint8 new_param, ParameterState state)
{
    uint8 old_page = get_param_page(old_param);
    uint8 new_page = get_param_page(new_param);
    
    // 同一页才需要刷新
    if(old_page == new_page && old_page == s_current_page)
    {
        // 旧参数行
        uint8 old_line = old_param % PARAM_LINES_PER_PAGE;
        draw_param_line(old_param, old_line, new_param, state);
        
        // 新参数行
        uint8 new_line = new_param % PARAM_LINES_PER_PAGE;
        draw_param_line(new_param, new_line, new_param, state);
    }
}

// 只更新参数值（不刷新整行）
void update_param_value_only(uint8 param_index)
{
    if(get_param_page(param_index) == s_current_page)
    {
        // 初始化上次参数值
        if(!s_param_values_initialized)
        {
            for(uint8 i = 0; i < parameter_count; i++)
                s_last_param_values[i] = -999999.0f;  // 不可能的值
            s_param_values_initialized = 1;
        }
        
        // 标记待更新
        s_update_pending = param_index + 1;  // +1防止0被误判为无更新
    }
}

// 执行参数值刷新
void perform_param_value_update(void)
{
    static uint32 current_time = 0;
    
    if(s_update_pending == 0) return;
    
    // 获取当前时间
    current_time = system_getval_ms();
    
    // 限制刷新频率
    if(current_time - s_last_update_time < UPDATE_INTERVAL_MS) return;
    
    uint8 param_index = s_update_pending - 1;
    
    // 检查参数值是否变化
    uint8 idx = param_config[param_index].index;
    if(parameter[idx] != s_last_param_values[param_index])
    {
        uint8 line_index = param_index % PARAM_LINES_PER_PAGE;
        uint16 y_pos = PARAM_START_Y + line_index * PARAM_LINE_HEIGHT;
        uint16 value_x = PARAM_LEFT_MARGIN + PARAM_VALUE_X_OFFSET;
        
        // 高亮显示
        ips200_set_color(RGB565_GREEN, RGB565_BLACK);
        
        // 显示新值
        char value_str[16];
        format_float_truncate(value_str, parameter[idx], 7, 2);
        ips200_show_string(value_x, y_pos, value_str);
        
        ips200_set_color(RGB565_WHITE, RGB565_BLACK);
        
        // 记录本次值
        s_last_param_values[param_index] = parameter[idx];
    }
    
    // 清除待更新标记
    s_update_pending = 0;
    s_last_update_time = current_time;
}

// 初始化参数菜单显示
void init_param_display(uint8 curr_param, ParameterState state)
{
    ips200_clear();
    
    // 重置参数值刷新状态
    s_param_values_initialized = 0;
    
    // 显示标题
    ips200_set_color(RGB565_YELLOW, RGB565_BLACK);
    ips200_show_string(PARAM_LEFT_MARGIN, PARAM_TITLE_Y, "Parameter Config");
    
    // 计算当前页
    s_current_page = get_param_page(curr_param);
    
    // 绘制参数列表
    draw_full_page(s_current_page, curr_param, state);
    
    // 显示操作提示
    update_operation_hint(state);
    
    ips200_set_color(RGB565_WHITE, RGB565_BLACK);
    
    // 记录状态
    s_last_page = s_current_page;
    s_last_curr_param = curr_param;
    s_last_state = state;
}

// 参数菜单主绘制函数
void draw_param_page(uint8 curr_param, ParameterState state)
{
    // 首次进入菜单
    if(s_last_curr_param == 255)
    {
        init_param_display(curr_param, state);
        return;
    }
    
    // 计算新页码
    uint8 new_page = get_param_page(curr_param);
    
    // 切换页面
    if(new_page != s_current_page)
    {
        s_current_page = new_page;
        // 清屏
        ips200_clear();
        
        // 显示标题
        ips200_set_color(RGB565_YELLOW, RGB565_BLACK);
        ips200_show_string(PARAM_LEFT_MARGIN, PARAM_TITLE_Y, "Parameter Config");
        
        // 绘制参数列表
        draw_full_page(s_current_page, curr_param, state);
        
        // 显示操作提示
        update_operation_hint(state);
        
        ips200_set_color(RGB565_WHITE, RGB565_BLACK);
    }
    // 同页切换参数或状态
    else
    {
        // 只切换选中参数
        if(s_last_curr_param != curr_param && state == s_last_state)
        {
            update_single_param_line(s_last_curr_param, curr_param, state);
        }
        // 状态切换（如进入/退出编辑）
        else if(state != s_last_state)
        {
            // 只刷新当前行
            uint8 line = curr_param % PARAM_LINES_PER_PAGE;
            clear_param_line(line);
            draw_param_line(curr_param, line, curr_param, state);
            
            // 更新操作提示
            update_operation_hint(state);
        }
    }
    
    // 记录状态
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
        
        // 初始化页面状态
        s_current_page = 0;
        s_last_page = 255;
        s_last_curr_param = 255;
        s_last_state = PARAM_STATE_VIEW;
        s_update_pending = 0;  // 无待更新
        
        // 备份参数
        for(uint8 i = 0; i < parameter_count; i++)
            s_backup_params[i] = parameter[param_config[i].index];
            
        draw_param_page(s_curr_param, s_param_state);
        return 0;
    }
    
    // 检查参数值是否需要刷新
    perform_param_value_update();
    
    if(key_flag)
    {
        uint8 key = key_flag;
        key_flag = 0;
        
        if(s_param_state == PARAM_STATE_VIEW)
        {
            switch(key)
            {
                case Button_1: // 上移
                    s_curr_param = (s_curr_param > 0) ? (s_curr_param - 1) : (parameter_count - 1);
                    draw_param_page(s_curr_param, s_param_state);
                    break;
                    
                case Button_2: // 下移
                    s_curr_param = (s_curr_param < parameter_count - 1) ? (s_curr_param + 1) : 0;
                    draw_param_page(s_curr_param, s_param_state);
                    break;
                    
                case Button_3: // 进入编辑
                    s_param_state = PARAM_STATE_EDIT;
                    draw_param_page(s_curr_param, s_param_state);
                    break;
                    
                case Button_4: // 返回菜单
                    s_param_initialized = 0;
                    // 重置状态
                    s_last_curr_param = 255;
                    s_last_page = 255;
                    return 1;
            }
        }
        else // PARAM_STATE_EDIT
        {
            switch(key)
            {
                case Button_1: // 增加
                {
                    uint8 idx = param_config[s_curr_param].index;
                    float new_val = parameter[idx] + param_config[s_curr_param].step;
                    if(new_val <= param_config[s_curr_param].max_value)
                    {
                        // 步长截断
                        new_val = truncate_by_step(new_val, param_config[s_curr_param].step);
                        parameter[idx] = new_val;
                        if(param_config[s_curr_param].target_var != NULL)
                            *(param_config[s_curr_param].target_var) = new_val;
                        // 刷新显示
                        update_param_value_only(s_curr_param);
                    }
                    break;
                }
                
                case Button_2: // 减少
                {
                    uint8 idx = param_config[s_curr_param].index;
                    float new_val = parameter[idx] - param_config[s_curr_param].step;
                    if(new_val >= param_config[s_curr_param].min_value)
                    {
                        // 步长截断
                        new_val = truncate_by_step(new_val, param_config[s_curr_param].step);
                        parameter[idx] = new_val;
                        if(param_config[s_curr_param].target_var != NULL)
                            *(param_config[s_curr_param].target_var) = new_val;
                        // 刷新显示
                        update_param_value_only(s_curr_param);
                    }
                    break;
                }
                
                case Button_3: // 保存
                    s_param_state = PARAM_STATE_VIEW;
                    Parameter_flash_save();
                    draw_param_page(s_curr_param, s_param_state);
                    break;
                    
                case Button_4: // 取消
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





//===================================================参数菜单入口===================================================

void parameter_menu_entry(void)
{
    ips200_clear();
    // 显示加载提示
    uint16 msg_y = ips200_height_max / 2 - 10;
    ips200_show_string(PARAM_LEFT_MARGIN, msg_y, "Loading parameters...");

    Parameter_flash_load();
    system_delay_ms(500);

    s_param_initialized = 0;
    uint8 s_params_modified = 0;
    uint8 exit_flag = 0;
    key_flag = 0;

    // 记录初始参数
    float initial_params[MAX_PARAMETERS];
    for(uint8 i = 0; i < parameter_count; i++)
        initial_params[i] = parameter[param_config[i].index];

    while(!exit_flag)
    {
        exit_flag = parameter_adjust_page();
        system_delay_ms(10);
    }

    // 检查参数是否有修改
    for(uint8 i = 0; i < parameter_count; i++)
    {
        if(parameter[param_config[i].index] != initial_params[i])
        {
            s_params_modified = 1;
            break;
        }
    }

    // 有修改则提示保存
    if(s_params_modified)
    {
        ips200_clear();
        ips200_set_color(RGB565_GREEN, RGB565_BLACK);
        ips200_show_string(PARAM_LEFT_MARGIN, msg_y, "Parameters Saved!");
        ips200_show_string(PARAM_LEFT_MARGIN, msg_y + 20, "Returning to menu...");
        ips200_set_color(RGB565_WHITE, RGB565_BLACK);
        system_delay_ms(500);
    }

    // 返回主菜单时模拟按下Button_4
    key_flag = Button_4;
}



//===================================================参数索引冲突检测===================================================

// 检查参数索引是否有冲突
uint8 check_parameter_index_conflicts(void)
{
    uint8 index_used[MAX_PARAMETERS] = {0};  // 标记每个索引是否被使用
    uint8 conflicts = 0;
    
    for(uint8 i = 0; i < parameter_count; i++)
    {
        uint8 idx = param_config[i].index;
        
        // 检查索引是否越界
        if(idx >= MAX_PARAMETERS)
        {
            conflicts++;
            continue;
        }
        
        // 检查是否重复
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
