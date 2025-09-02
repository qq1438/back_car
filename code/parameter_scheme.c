#include <string.h>
#include <stdio.h>
#include "parameter_scheme.h"
#include "zf_device_ips200.h"
#include "key.h"

//===================================================宏定义和静态变量===================================================

// 方案管理状态
typedef enum {
    SCHEME_STATE_MAIN_MENU,     // 主菜单（四个选项）
    SCHEME_STATE_SAVE_CURRENT,  // 保存当前方案
    SCHEME_STATE_DELETE_LIST,   // 删除方案列表
    SCHEME_STATE_LOAD_LIST,     // 加载方案列表
    SCHEME_STATE_VIEW_LIST,     // 查看方案列表
    SCHEME_STATE_VIEW_DETAILS,  // 查看方案详情
    SCHEME_STATE_DELETE_CONFIRM,// 删除确认
    SCHEME_STATE_SAVE_CONFIRM,  // 保存确认
    SCHEME_STATE_BACKUP_CONFIRM,// 备份确认
    SCHEME_STATE_RESTORE_CONFIRM,// 恢复确认
} SchemeState;

// UI布局常量 (与 parameter_flash.c 保持一致)
#define PARAM_LINE_HEIGHT   18
#define PARAM_LEFT_MARGIN   10
#define PARAM_VALUE_X_OFFSET 130
#define PARAM_TITLE_Y       10
#define PARAM_START_Y       35
#define PARAM_LINES_PER_PAGE 12

// 静态变量
static ParameterScheme s_schemes[MAX_SCHEMES];
static SchemeState s_scheme_state = SCHEME_STATE_MAIN_MENU;
static uint8 s_selected_scheme = 0;
static uint8 s_selected_menu = 0;      // 主菜单选择
static uint8 s_view_start_param = 0;   // 用于在查看视图中滚动参数
static uint8 s_ui_initialized = 0;

// 主菜单选项
static const char* main_menu_items[] = {
    "1. Save Current Scheme",
    "2. Delete Scheme", 
    "3. Load Scheme",
    "4. View Scheme Details",
    "5. Backup to W25N04",
    "6. Restore from W25N04"
};
#define MAIN_MENU_COUNT 6

//===================================================Flash读写===================================================

// 从Flash加载所有方案
void scheme_load_from_flash(void)
{
    flash_read_page_to_buffer(0, SCHEME_STORAGE_INDEX);
    memcpy(s_schemes, flash_union_buffer, sizeof(s_schemes));

    // 基本的数据有效性检查
    for(int i = 0; i < MAX_SCHEMES; i++) {
        if(s_schemes[i].is_used != 0 && s_schemes[i].is_used != 1) {
            // 如果数据无效，则清空所有方案
            memset(s_schemes, 0, sizeof(s_schemes));
            return;
        }
    }
}

// 保存所有方案到Flash
void scheme_save_to_flash(void)
{
    flash_erase_page(0, SCHEME_STORAGE_INDEX);
    memcpy(flash_union_buffer, s_schemes, sizeof(s_schemes));
    flash_write_page_from_buffer(0, SCHEME_STORAGE_INDEX);
}

//===================================================方案命名和管理===================================================

// 根据参数名称生成方案名称，如果重复则添加版本后缀
void generate_scheme_name(char* scheme_name, const char* base_name)
{
    uint8 version = 1;
    char temp_name[MAX_SCHEME_NAME_LEN];
    
    // 生成base_name_v1格式的名称
    snprintf(temp_name, MAX_SCHEME_NAME_LEN, "%s_v%d", base_name, version);
    
    // 检查是否已存在同名方案，如果存在则递增版本号
    uint8 name_exists = 0;
    do {
        name_exists = 0;
        for(uint8 i = 0; i < MAX_SCHEMES; i++) {
            if(s_schemes[i].is_used && strcmp(s_schemes[i].name, temp_name) == 0) {
                name_exists = 1;
                break;
            }
        }
        
        if(name_exists) {
            // 生成新的版本名称
            version++;
            snprintf(temp_name, MAX_SCHEME_NAME_LEN, "%s_v%d", base_name, version);
        }
    } while(name_exists && version < 100); // 最多尝试100个版本
    
    strncpy(scheme_name, temp_name, MAX_SCHEME_NAME_LEN - 1);
    scheme_name[MAX_SCHEME_NAME_LEN - 1] = '\0';
}

// 查找第一个主要参数的名称作为方案基础名称
bool get_primary_param_name(char* base_name)
{
    // 通过指针地址比较查找&speed_base对应的参数值
    float speed_base_value = 0.0f;
    uint8 found_speed_base = 0;
    
    for(uint8 i = 0; i < parameter_count; i++) {
        if(param_config[i].target_var == &speed_base) {
            uint8 idx = param_config[i].index;
            speed_base_value = parameter[idx];
            found_speed_base = 1;
            break;
        }
    }
    if(found_speed_base) {
        int int_part = (int)speed_base_value;
        int frac_part = (int)((speed_base_value - int_part) * 100); // 取两位小数
        
        if(frac_part == 0) {
            snprintf(base_name, MAX_SCHEME_NAME_LEN - 10, "%d", int_part);
        } else {
            snprintf(base_name, MAX_SCHEME_NAME_LEN - 10, "%d.%02d", int_part, frac_part);
        }
    } else {
        return false;
    }
    return true;
}

//===================================================核心方案操作===================================================

// 将当前参数保存为新方案
uint8 scheme_save_current(void)
{
    Parameter_flash_load();
    int8 first_empty_slot = -1;
    for(uint8 i = 0; i < MAX_SCHEMES; i++) {
        if(!s_schemes[i].is_used) {
            first_empty_slot = i;
            break;
        }
    }

    if(first_empty_slot == -1) return 0; // 空间已满

    ParameterScheme* new_scheme = &s_schemes[first_empty_slot];
    new_scheme->is_used = 1;
    
    // 生成基于参数名称的方案名称
    char base_name[MAX_SCHEME_NAME_LEN];
    if(!get_primary_param_name(base_name)) return 0;
    generate_scheme_name(new_scheme->name, base_name);

    // 复制参数值和名称，但不超过MAX_SCHEME_PARAMS的限制
    uint8 params_to_save = (parameter_count > MAX_SCHEME_PARAMS) ? MAX_SCHEME_PARAMS : parameter_count;
    new_scheme->param_count = params_to_save;
    for(uint8 i = 0; i < params_to_save; i++) {
        uint8 idx = param_config[i].index;
        new_scheme->params[i].index = param_config[i].index;
        new_scheme->params[i].value = parameter[idx];
    }
    
    scheme_save_to_flash();
    return 1; // 成功
}

// 加载指定方案到当前系统
uint8 scheme_apply(uint8 scheme_index)
{
    if(scheme_index >= MAX_SCHEMES || !s_schemes[scheme_index].is_used) {
        return 0; // 失败
    }

    ParameterScheme* scheme = &s_schemes[scheme_index];

    // 初始化所有参数为0，然后通过索引匹配来更新参数
    for(uint8 i = 0; i < parameter_count; i++) {
        uint8 idx = param_config[i].index;
        parameter[idx] = 0.0f; // 默认设为0
        if(param_config[i].target_var != NULL) {
            *(param_config[i].target_var) = 0.0f;
        }
    }

    // 使用索引匹配来更新参数，只处理方案中实际存储的参数数量
    for(uint8 scheme_idx = 0; scheme_idx < scheme->param_count; scheme_idx++) {
        uint8 idx = scheme->params[scheme_idx].index;
        float value = scheme->params[scheme_idx].value;

        // 查找对应的参数配置
        for(uint8 config_idx = 0; config_idx < parameter_count; config_idx++) {
            if(param_config[config_idx].index == idx) {
                float val = value;
                if(val > param_config[config_idx].max_value) val = param_config[config_idx].max_value;
                if(val < param_config[config_idx].min_value) val = param_config[config_idx].min_value;
                val = truncate_by_step(val, param_config[config_idx].step);

                parameter[idx] = val;

                if(param_config[config_idx].target_var != NULL) {
                    *(param_config[config_idx].target_var) = val;
                }
                break;
            }
        }
    }
    return 1; // 成功
}

// 删除指定方案
void scheme_delete(uint8 scheme_index)
{
    if(scheme_index < MAX_SCHEMES) {
        s_schemes[scheme_index].is_used = 0;
        memset(s_schemes[scheme_index].name, 0, MAX_SCHEME_NAME_LEN);
        memset(s_schemes[scheme_index].params, 0, sizeof(s_schemes[scheme_index].params));
        s_schemes[scheme_index].param_count = 0;
        scheme_save_to_flash();
    }
}

//===================================================工具函数===================================================

// 获取已使用的方案数量
uint8 get_used_scheme_count(void)
{
    uint8 count = 0;
    for(uint8 i = 0; i < MAX_SCHEMES; i++) {
        if(s_schemes[i].is_used) count++;
    }
    return count;
}

// 获取第n个已使用的方案索引
uint8 get_nth_used_scheme_index(uint8 nth)
{
    uint8 count = 0;
    for(uint8 i = 0; i < MAX_SCHEMES; i++) {
        if(s_schemes[i].is_used) {
            if(count == nth) return i;
            count++;
        }
    }
    return 0; // 默认返回0
}

//===================================================UI绘制函数===================================================

// 绘制UI框架和标题
void draw_scheme_ui_frame(const char* title)
{
    ips200_clear();
    ips200_set_color(RGB565_YELLOW, RGB565_BLACK);
    ips200_show_string(PARAM_LEFT_MARGIN, PARAM_TITLE_Y, title);
    ips200_set_color(RGB565_WHITE, RGB565_BLACK);
}

// 绘制主菜单
void draw_main_menu(void)
{
    draw_scheme_ui_frame("Scheme Management");
    
    for(uint8 i = 0; i < MAIN_MENU_COUNT; i++) {
        uint16 y_pos = PARAM_START_Y + i * PARAM_LINE_HEIGHT;
        
        if(i == s_selected_menu) {
            ips200_set_color(RGB565_BLUE, RGB565_BLACK);
        } else {
            ips200_set_color(RGB565_WHITE, RGB565_BLACK);
        }
        
        ips200_show_string(PARAM_LEFT_MARGIN, y_pos, main_menu_items[i]);
    }
    
    // 显示统计信息
    uint8 used_count = get_used_scheme_count();
    char info_str[40];
    sprintf(info_str, "Schemes: %d/%d", used_count, MAX_SCHEMES);
    ips200_set_color(RGB565_CYAN, RGB565_BLACK);
    ips200_show_string(PARAM_LEFT_MARGIN, PARAM_START_Y + (MAIN_MENU_COUNT + 1) * PARAM_LINE_HEIGHT, info_str);
    
    // 操作提示
    ips200_set_color(RGB565_GRAY, RGB565_BLACK);
    ips200_show_string(PARAM_LEFT_MARGIN, ips200_height_max - 40, "1:Up 2:Down 3:Select 4:Back");
    ips200_set_color(RGB565_WHITE, RGB565_BLACK);
}

// 局部更新主菜单选中项（只更新菜单项的颜色，不重绘整个屏幕）
void update_main_menu_selection(uint8 old_selection, uint8 new_selection)
{
    // 清除旧选中项的高亮
    if(old_selection < MAIN_MENU_COUNT) {
        uint16 old_y_pos = PARAM_START_Y + old_selection * PARAM_LINE_HEIGHT;
        ips200_set_color(RGB565_WHITE, RGB565_BLACK);
        ips200_show_string(PARAM_LEFT_MARGIN, old_y_pos, main_menu_items[old_selection]);
    }
    
    // 设置新选中项的高亮
    if(new_selection < MAIN_MENU_COUNT) {
        uint16 new_y_pos = PARAM_START_Y + new_selection * PARAM_LINE_HEIGHT;
        ips200_set_color(RGB565_BLUE, RGB565_BLACK);
        ips200_show_string(PARAM_LEFT_MARGIN, new_y_pos, main_menu_items[new_selection]);
    }
    
    ips200_set_color(RGB565_WHITE, RGB565_BLACK);
}

// 绘制方案列表（用于删除、加载、查看）
void draw_scheme_list(const char* title, const char* hint)
{
    draw_scheme_ui_frame(title);
    
    uint8 used_count = get_used_scheme_count();
    if(used_count == 0) {
        ips200_set_color(RGB565_RED, RGB565_BLACK);
        ips200_show_string(PARAM_LEFT_MARGIN, PARAM_START_Y, "No schemes available");
        ips200_set_color(RGB565_WHITE, RGB565_BLACK);
    } else {
        uint8 displayed = 0;
        for(uint8 i = 0; i < MAX_SCHEMES && displayed < PARAM_LINES_PER_PAGE; i++) {
            if(s_schemes[i].is_used) {
                uint16 y_pos = PARAM_START_Y + displayed * PARAM_LINE_HEIGHT;
                
                if(displayed == s_selected_scheme) {
                    ips200_set_color(RGB565_BLUE, RGB565_BLACK);
                } else {
                    ips200_set_color(RGB565_WHITE, RGB565_BLACK);
                }
                
                char display_str[50];
                sprintf(display_str, "%d. %s", displayed + 1, s_schemes[i].name);
                ips200_show_string(PARAM_LEFT_MARGIN, y_pos, display_str);
                displayed++;
            }
        }
    }
    
    // 操作提示
    ips200_set_color(RGB565_GRAY, RGB565_BLACK);
    ips200_show_string(PARAM_LEFT_MARGIN, ips200_height_max - 40, hint);
    ips200_set_color(RGB565_WHITE, RGB565_BLACK);
}

// 绘制方案详情
void draw_scheme_details(uint8 scheme_index)
{
    ParameterScheme* scheme = &s_schemes[scheme_index];
    
    char title[50];
    sprintf(title, "Details: %s", scheme->name);
    draw_scheme_ui_frame(title);
    
    uint8 lines_drawn = 0;
    uint8 max_params = parameter_count; // 显示param_config中的所有参数
    for(uint8 i = s_view_start_param; i < max_params && lines_drawn < PARAM_LINES_PER_PAGE; i++) {
        uint16 y_pos = PARAM_START_Y + lines_drawn * PARAM_LINE_HEIGHT;
        uint16 value_x = PARAM_LEFT_MARGIN + PARAM_VALUE_X_OFFSET;

        // 显示参数名称
        ips200_show_string(PARAM_LEFT_MARGIN, y_pos, (char*)param_config[i].name);
        
        // 通过索引匹配查找方案中对应的值
        float display_value = 0.0f; // 默认为0
        for(uint8 j = 0; j < scheme->param_count; j++) {
            uint8 idx = scheme->params[j].index;
            if(param_config[i].index == idx) {
                display_value = scheme->params[j].value;
                break;
            }
        }
        
        char value_str[16];
        format_float_truncate(value_str, display_value, 7, 2);
        ips200_show_string(value_x, y_pos, value_str);
        lines_drawn++;
    }

    // 操作提示
    ips200_set_color(RGB565_GRAY, RGB565_BLACK);
    ips200_show_string(PARAM_LEFT_MARGIN, ips200_height_max - 40, "1:Up 2:Down 4:Back");
    ips200_set_color(RGB565_WHITE, RGB565_BLACK);
}

// 绘制确认对话框
void draw_confirm_dialog(const char* message, const char* hint, uint16 color)
{
    ips200_clear();
    
    // 居中显示消息
    uint16 msg_y = ips200_height_max / 2 - 30;
    ips200_set_color(color, RGB565_BLACK);
    ips200_show_string(PARAM_LEFT_MARGIN, msg_y, message);
    
    // 居中显示提示
    uint16 hint_y = msg_y + 25;
    ips200_set_color(RGB565_WHITE, RGB565_BLACK);
    ips200_show_string(PARAM_LEFT_MARGIN + 20, hint_y, hint);
    
    // 显示边框
    ips200_set_color(RGB565_GRAY, RGB565_BLACK);
    ips200_draw_line(5, msg_y - 10, ips200_width_max - 5, msg_y - 10, RGB565_GRAY);
    ips200_draw_line(5, hint_y + 20, ips200_width_max - 5, hint_y + 20, RGB565_GRAY);
    
    ips200_set_color(RGB565_WHITE, RGB565_BLACK);
}

// 绘制状态消息
void draw_status_message(const char* message, uint16 color, uint16 delay_ms)
{
    ips200_clear();
    uint16 msg_y = ips200_height_max / 2 - 10;
    ips200_set_color(color, RGB565_BLACK);
    ips200_show_string(PARAM_LEFT_MARGIN, msg_y, message);
    ips200_set_color(RGB565_WHITE, RGB565_BLACK);
    system_delay_ms(delay_ms);
}

// 局部更新方案列表选中项（只更新选中项的颜色，不重绘整个屏幕）
void update_scheme_list_selection(uint8 old_selection, uint8 new_selection)
{
    uint8 displayed_old = 0, displayed_new = 0;
    uint8 actual_displayed = 0;
    
    // 找到对应的显示索引并更新颜色
    for(uint8 i = 0; i < MAX_SCHEMES && actual_displayed <= (old_selection > new_selection ? old_selection : new_selection); i++) {
        if(s_schemes[i].is_used) {
            // 清除旧选中项的高亮
            if(actual_displayed == old_selection) {
                uint16 old_y_pos = PARAM_START_Y + actual_displayed * PARAM_LINE_HEIGHT;
                ips200_set_color(RGB565_WHITE, RGB565_BLACK);
                char display_str[50];
                sprintf(display_str, "%d. %s", actual_displayed + 1, s_schemes[i].name);
                ips200_show_string(PARAM_LEFT_MARGIN, old_y_pos, display_str);
            }
            
            // 设置新选中项的高亮
            if(actual_displayed == new_selection) {
                uint16 new_y_pos = PARAM_START_Y + actual_displayed * PARAM_LINE_HEIGHT;
                ips200_set_color(RGB565_BLUE, RGB565_BLACK);
                char display_str[50];
                sprintf(display_str, "%d. %s", actual_displayed + 1, s_schemes[i].name);
                ips200_show_string(PARAM_LEFT_MARGIN, new_y_pos, display_str);
            }
            
            actual_displayed++;
        }
    }
    
    ips200_set_color(RGB565_WHITE, RGB565_BLACK);
}

//===================================================W25N04备份/恢复功能===================================================

/**
 * @brief 将所有方案数据备份到W25N04存储器
 * 
 * @return uint8 0:失败，1:成功
 */
uint8 scheme_backup_to_w25n04(void)
{
    uint8 result;
    uint8 *backup_data = (uint8*)s_schemes;
    uint16 data_size = sizeof(s_schemes);
    uint16 written = 0;
    uint32 page_addr = W25N04_BACKUP_PAGE_ADDR;
    image_storage_init();
    ips200_clear();
    ips200_show_string(PARAM_LEFT_MARGIN, PARAM_START_Y, "Backup to W25N04...");
    result = w25n04_block_erase(W25N04_BACKUP_PAGE_ADDR);
    if (result != 1) {
        return 0; // 擦除失败
    }
    
    // 分页写入数据(每页最大2048字节)
    while (written < data_size) {
        uint16 write_size = (data_size - written > 2048) ? 2048 : (data_size - written);
        
        result = w25n04_write_page(page_addr, 0, backup_data + written, write_size);
        if (result != 1) {
            return 0; // 写入失败
        }
        
        written += write_size;
        page_addr++;
    }
    
    return 1; // 备份成功
}

/**
 * @brief 从W25N04存储器恢复方案数据
 * 
 * @return uint8 0:失败，1:成功，2:数据校验失败
 */
uint8 scheme_restore_from_w25n04(void)
{
    uint8 result;
    ParameterScheme temp_schemes[MAX_SCHEMES];
    uint8 *restore_data = (uint8*)temp_schemes;
    uint16 data_size = sizeof(temp_schemes);
    uint16 read_count = 0;
    uint32 page_addr = W25N04_BACKUP_PAGE_ADDR;
    image_storage_init();
    // 分页读取数据
    while (read_count < data_size) {
        uint16 read_size = (data_size - read_count > 2048) ? 2048 : (data_size - read_count);
        
        result = w25n04_read_page(page_addr, 0, restore_data + read_count, read_size);
        if (result == 0 || result == 3) {
            return 0; // 读取失败或ECC无法纠错
        }
        
        read_count += read_size;
        page_addr++;
    }
    
    // 基本数据有效性检查
    for(int i = 0; i < MAX_SCHEMES; i++) {
        if(temp_schemes[i].is_used != 0 && temp_schemes[i].is_used != 1) {
            return 2; // 数据校验失败
        }
    }
    
    // 数据验证通过，复制到当前方案数组
    memcpy(s_schemes, temp_schemes, sizeof(s_schemes));
    
    // 保存到系统Flash
    scheme_save_to_flash();
    
    return 1; // 恢复成功
}

//===================================================主循环和事件处理===================================================

void scheme_menu_entry(void)
{
    key_flag = 0;
    if(!s_ui_initialized) {
        scheme_load_from_flash();
        s_scheme_state = SCHEME_STATE_MAIN_MENU;
        s_selected_menu = 0;
        s_selected_scheme = 0;
        s_view_start_param = 0;
        s_ui_initialized = 1;
        draw_main_menu();
    }

    uint8 exit_flag = 0;
    while(!exit_flag)
    {
        if(key_flag) {
            uint8 key = key_flag;
            key_flag = 0;

            switch(s_scheme_state)
            {
                case SCHEME_STATE_MAIN_MENU:
                    switch(key) {
                        case Button_1: // 上
                            {
                                uint8 old_selection = s_selected_menu;
                                s_selected_menu = (s_selected_menu > 0) ? (s_selected_menu - 1) : (MAIN_MENU_COUNT - 1);
                                update_main_menu_selection(old_selection, s_selected_menu);
                            }
                            break;
                        case Button_2: // 下
                            {
                                uint8 old_selection = s_selected_menu;
                                s_selected_menu = (s_selected_menu < MAIN_MENU_COUNT - 1) ? (s_selected_menu + 1) : 0;
                                update_main_menu_selection(old_selection, s_selected_menu);
                            }
                            break;
                        case Button_3: // 选择
                            switch(s_selected_menu) {
                                case 0: // 保存当前方案
                                    s_scheme_state = SCHEME_STATE_SAVE_CURRENT;
                                    draw_confirm_dialog("Save Current Scheme?", "3:Yes  4:No", RGB565_GREEN);
                                    break;
                                case 1: // 删除方案
                                    if(get_used_scheme_count() > 0) {
                                        s_scheme_state = SCHEME_STATE_DELETE_LIST;
                                        s_selected_scheme = 0;
                                        draw_scheme_list("Delete Scheme", "1:Up 2:Down 3:Delete 4:Back");
                                    } else {
                                        draw_status_message("No schemes to delete!", RGB565_RED, 1000);
                                        draw_main_menu();
                                    }
                                    break;
                                case 2: // 加载方案
                                    if(get_used_scheme_count() > 0) {
                                        s_scheme_state = SCHEME_STATE_LOAD_LIST;
                                        s_selected_scheme = 0;
                                        draw_scheme_list("Load Scheme", "1:Up 2:Down 3:Load 4:Back");
                                    } else {
                                        draw_status_message("No schemes to load!", RGB565_RED, 1000);
                                        draw_main_menu();
                                    }
                                    break;
                                case 3: // 查看方案详情
                                    if(get_used_scheme_count() > 0) {
                                        s_scheme_state = SCHEME_STATE_VIEW_LIST;
                                        s_selected_scheme = 0;
                                        draw_scheme_list("View Scheme Details", "1:Up 2:Down 3:View 4:Back");
                                    } else {
                                        draw_status_message("No schemes to view!", RGB565_RED, 1000);
                                        draw_main_menu();
                                    }
                                    break;
                                case 4: // 备份到W25N04
                                    s_scheme_state = SCHEME_STATE_BACKUP_CONFIRM;
                                    draw_confirm_dialog("Backup to W25N04?", "3:Yes  4:No", RGB565_CYAN);
                                    break;
                                case 5: // 从W25N04恢复
                                    s_scheme_state = SCHEME_STATE_RESTORE_CONFIRM;
                                    draw_confirm_dialog("Restore from W25N04?", "3:Yes  4:No", RGB565_MAGENTA);
                                    break;
                            }
                            break;
                        case Button_4: // 返回
                            s_ui_initialized = 0;
                            exit_flag = 1;
                            break;
                    }
                    break;

                case SCHEME_STATE_SAVE_CURRENT:
                    if(key == Button_3) { // 确认保存
                        if(scheme_save_current()) {
                            draw_status_message("Scheme saved successfully!", RGB565_GREEN, 1500);
                        } else {
                            draw_status_message("Failed! No empty slot.", RGB565_RED, 1500);
                        }
                        s_scheme_state = SCHEME_STATE_MAIN_MENU;
                        draw_main_menu();
                    } else if(key == Button_4) { // 取消
                        s_scheme_state = SCHEME_STATE_MAIN_MENU;
                        draw_main_menu();
                    } else {
                        // 重新显示确认对话框
                        draw_confirm_dialog("Save Current Scheme?", "3:Yes  4:No", RGB565_GREEN);
                    }
                    break;

                case SCHEME_STATE_DELETE_LIST:
                    switch(key) {
                        case Button_1: // 上
                            {
                                uint8 old_selection = s_selected_scheme;
                                s_selected_scheme = (s_selected_scheme > 0) ? (s_selected_scheme - 1) : (get_used_scheme_count() - 1);
                                update_scheme_list_selection(old_selection, s_selected_scheme);
                            }
                            break;
                        case Button_2: // 下
                            {
                                uint8 old_selection = s_selected_scheme;
                                s_selected_scheme = (s_selected_scheme < get_used_scheme_count() - 1) ? (s_selected_scheme + 1) : 0;
                                update_scheme_list_selection(old_selection, s_selected_scheme);
                            }
                            break;
                        case Button_3: // 删除
                            s_scheme_state = SCHEME_STATE_DELETE_CONFIRM;
                            draw_confirm_dialog("Delete Scheme?", "3:Yes  4:No", RGB565_RED);
                            break;
                        case Button_4: // 返回
                            s_scheme_state = SCHEME_STATE_MAIN_MENU;
                            draw_main_menu();
                            break;
                    }
                    break;

                case SCHEME_STATE_LOAD_LIST:
                    switch(key) {
                        case Button_1: // 上
                            {
                                uint8 old_selection = s_selected_scheme;
                                s_selected_scheme = (s_selected_scheme > 0) ? (s_selected_scheme - 1) : (get_used_scheme_count() - 1);
                                update_scheme_list_selection(old_selection, s_selected_scheme);
                            }
                            break;
                        case Button_2: // 下
                            {
                                uint8 old_selection = s_selected_scheme;
                                s_selected_scheme = (s_selected_scheme < get_used_scheme_count() - 1) ? (s_selected_scheme + 1) : 0;
                                update_scheme_list_selection(old_selection, s_selected_scheme);
                            }
                            break;
                        case Button_3: // 加载
                            {
                                uint8 actual_index = get_nth_used_scheme_index(s_selected_scheme);
                                if(scheme_apply(actual_index)) {
                                    Parameter_flash_save(); // 加载后保存到主参数区
                                    draw_status_message("Scheme loaded & saved!", RGB565_GREEN, 1500);
                                } else {
                                    draw_status_message("Failed to load scheme!", RGB565_RED, 1500);
                                }
                            }
                            s_scheme_state = SCHEME_STATE_MAIN_MENU;
                            draw_main_menu();
                            break;
                        case Button_4: // 返回
                            s_scheme_state = SCHEME_STATE_MAIN_MENU;
                            draw_main_menu();
                            break;
                    }
                    break;

                case SCHEME_STATE_VIEW_LIST:
                    switch(key) {
                        case Button_1: // 上
                            {
                                uint8 old_selection = s_selected_scheme;
                                s_selected_scheme = (s_selected_scheme > 0) ? (s_selected_scheme - 1) : (get_used_scheme_count() - 1);
                                update_scheme_list_selection(old_selection, s_selected_scheme);
                            }
                            break;
                        case Button_2: // 下
                            {
                                uint8 old_selection = s_selected_scheme;
                                s_selected_scheme = (s_selected_scheme < get_used_scheme_count() - 1) ? (s_selected_scheme + 1) : 0;
                                update_scheme_list_selection(old_selection, s_selected_scheme);
                            }
                            break;
                        case Button_3: // 查看详情
                            {
                                uint8 actual_index = get_nth_used_scheme_index(s_selected_scheme);
                                s_scheme_state = SCHEME_STATE_VIEW_DETAILS;
                                s_view_start_param = 0;
                                draw_scheme_details(actual_index);
                            }
                            break;
                        case Button_4: // 返回
                            s_scheme_state = SCHEME_STATE_MAIN_MENU;
                            draw_main_menu();
                            break;
                    }
                    break;

                case SCHEME_STATE_VIEW_DETAILS:
                    {
                        uint8 actual_index = get_nth_used_scheme_index(s_selected_scheme);
                        switch(key) {
                            case Button_1: // 向上滚动
                                if(s_view_start_param > 0) {
                                    s_view_start_param--;
                                    draw_scheme_details(actual_index);
                                }
                                break;
                            case Button_2: // 向下滚动
                                if(s_view_start_param < parameter_count - PARAM_LINES_PER_PAGE) {
                                    s_view_start_param++;
                                    draw_scheme_details(actual_index);
                                }
                                break;
                            case Button_4: // 返回
                                s_scheme_state = SCHEME_STATE_VIEW_LIST;
                                draw_scheme_list("View Scheme Details", "1:Up 2:Down 3:View 4:Back");
                                break;
                        }
                    }
                    break;
                
                case SCHEME_STATE_DELETE_CONFIRM:
                    if(key == Button_3) { // 确认删除
                        uint8 actual_index = get_nth_used_scheme_index(s_selected_scheme);
                        scheme_delete(actual_index);
                        draw_status_message("Scheme deleted!", RGB565_YELLOW, 1000);
                        s_scheme_state = SCHEME_STATE_MAIN_MENU;
                        draw_main_menu();
                    } else if(key == Button_4) { // 取消
                        s_scheme_state = SCHEME_STATE_MAIN_MENU;
                        draw_main_menu();
                    }
                    break;

                case SCHEME_STATE_BACKUP_CONFIRM:
                    if(key == Button_3) { // 确认备份
                        uint8 backup_result = scheme_backup_to_w25n04();
                        if(backup_result == 1) {
                            draw_status_message("Backup to W25N04 successful!", RGB565_GREEN, 1500);
                        } else {
                            draw_status_message("Backup to W25N04 failed!", RGB565_RED, 1500);
                        }
                        s_scheme_state = SCHEME_STATE_MAIN_MENU;
                        draw_main_menu();
                    } else if(key == Button_4) { // 取消
                        s_scheme_state = SCHEME_STATE_MAIN_MENU;
                        draw_main_menu();
                    } else {
                        // 重新显示确认对话框
                        draw_confirm_dialog("Backup to W25N04?", "3:Yes  4:No", RGB565_CYAN);
                    }
                    break;

                case SCHEME_STATE_RESTORE_CONFIRM:
                    if(key == Button_3) { // 确认恢复
                        uint8 restore_result = scheme_restore_from_w25n04();
                        if(restore_result == 1) {
                            draw_status_message("Restore successful!", RGB565_GREEN, 1500);
                        } else if(restore_result == 2) {
                            draw_status_message("Data validation failed!", RGB565_RED, 1500);
                        } else {
                            draw_status_message("Restore failed!", RGB565_RED, 1500);
                        }
                        s_scheme_state = SCHEME_STATE_MAIN_MENU;
                        draw_main_menu();
                    } else if(key == Button_4) { // 取消
                        s_scheme_state = SCHEME_STATE_MAIN_MENU;
                        draw_main_menu();
                    } else {
                        // 重新显示确认对话框
                        draw_confirm_dialog("Restore from W25N04?", "3:Yes  4:No", RGB565_MAGENTA);
                    }
                    break;
            }
        }
        system_delay_ms(20);
    }
    key_flag = Button_4; // 通知上层菜单返回
}


