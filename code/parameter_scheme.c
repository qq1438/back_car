#include <string.h>
#include <stdio.h>
#include "parameter_scheme.h"
#include "zf_device_ips200.h"
#include "key.h"

//===================================================�궨��;�̬����===================================================

// ��������״̬
typedef enum {
    SCHEME_STATE_MAIN_MENU,     // ���˵����ĸ�ѡ�
    SCHEME_STATE_SAVE_CURRENT,  // ���浱ǰ����
    SCHEME_STATE_DELETE_LIST,   // ɾ�������б�
    SCHEME_STATE_LOAD_LIST,     // ���ط����б�
    SCHEME_STATE_VIEW_LIST,     // �鿴�����б�
    SCHEME_STATE_VIEW_DETAILS,  // �鿴��������
    SCHEME_STATE_DELETE_CONFIRM,// ɾ��ȷ��
    SCHEME_STATE_SAVE_CONFIRM,  // ����ȷ��
    SCHEME_STATE_BACKUP_CONFIRM,// ����ȷ��
    SCHEME_STATE_RESTORE_CONFIRM,// �ָ�ȷ��
} SchemeState;

// UI���ֳ��� (�� parameter_flash.c ����һ��)
#define PARAM_LINE_HEIGHT   18
#define PARAM_LEFT_MARGIN   10
#define PARAM_VALUE_X_OFFSET 130
#define PARAM_TITLE_Y       10
#define PARAM_START_Y       35
#define PARAM_LINES_PER_PAGE 12

// ��̬����
static ParameterScheme s_schemes[MAX_SCHEMES];
static SchemeState s_scheme_state = SCHEME_STATE_MAIN_MENU;
static uint8 s_selected_scheme = 0;
static uint8 s_selected_menu = 0;      // ���˵�ѡ��
static uint8 s_view_start_param = 0;   // �����ڲ鿴��ͼ�й�������
static uint8 s_ui_initialized = 0;

// ���˵�ѡ��
static const char* main_menu_items[] = {
    "1. Save Current Scheme",
    "2. Delete Scheme", 
    "3. Load Scheme",
    "4. View Scheme Details",
    "5. Backup to W25N04",
    "6. Restore from W25N04"
};
#define MAIN_MENU_COUNT 6

//===================================================Flash��д===================================================

// ��Flash�������з���
void scheme_load_from_flash(void)
{
    flash_read_page_to_buffer(0, SCHEME_STORAGE_INDEX);
    memcpy(s_schemes, flash_union_buffer, sizeof(s_schemes));

    // ������������Ч�Լ��
    for(int i = 0; i < MAX_SCHEMES; i++) {
        if(s_schemes[i].is_used != 0 && s_schemes[i].is_used != 1) {
            // ���������Ч����������з���
            memset(s_schemes, 0, sizeof(s_schemes));
            return;
        }
    }
}

// �������з�����Flash
void scheme_save_to_flash(void)
{
    flash_erase_page(0, SCHEME_STORAGE_INDEX);
    memcpy(flash_union_buffer, s_schemes, sizeof(s_schemes));
    flash_write_page_from_buffer(0, SCHEME_STORAGE_INDEX);
}

//===================================================���������͹���===================================================

// ���ݲ����������ɷ������ƣ�����ظ�����Ӱ汾��׺
void generate_scheme_name(char* scheme_name, const char* base_name)
{
    uint8 version = 1;
    char temp_name[MAX_SCHEME_NAME_LEN];
    
    // ����base_name_v1��ʽ������
    snprintf(temp_name, MAX_SCHEME_NAME_LEN, "%s_v%d", base_name, version);
    
    // ����Ƿ��Ѵ���ͬ���������������������汾��
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
            // �����µİ汾����
            version++;
            snprintf(temp_name, MAX_SCHEME_NAME_LEN, "%s_v%d", base_name, version);
        }
    } while(name_exists && version < 100); // ��ೢ��100���汾
    
    strncpy(scheme_name, temp_name, MAX_SCHEME_NAME_LEN - 1);
    scheme_name[MAX_SCHEME_NAME_LEN - 1] = '\0';
}

// ���ҵ�һ����Ҫ������������Ϊ������������
bool get_primary_param_name(char* base_name)
{
    // ͨ��ָ���ַ�Ƚϲ���&speed_base��Ӧ�Ĳ���ֵ
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
        int frac_part = (int)((speed_base_value - int_part) * 100); // ȡ��λС��
        
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

//===================================================���ķ�������===================================================

// ����ǰ��������Ϊ�·���
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

    if(first_empty_slot == -1) return 0; // �ռ�����

    ParameterScheme* new_scheme = &s_schemes[first_empty_slot];
    new_scheme->is_used = 1;
    
    // ���ɻ��ڲ������Ƶķ�������
    char base_name[MAX_SCHEME_NAME_LEN];
    if(!get_primary_param_name(base_name)) return 0;
    generate_scheme_name(new_scheme->name, base_name);

    // ���Ʋ���ֵ�����ƣ���������MAX_SCHEME_PARAMS������
    uint8 params_to_save = (parameter_count > MAX_SCHEME_PARAMS) ? MAX_SCHEME_PARAMS : parameter_count;
    new_scheme->param_count = params_to_save;
    for(uint8 i = 0; i < params_to_save; i++) {
        uint8 idx = param_config[i].index;
        new_scheme->params[i].index = param_config[i].index;
        new_scheme->params[i].value = parameter[idx];
    }
    
    scheme_save_to_flash();
    return 1; // �ɹ�
}

// ����ָ����������ǰϵͳ
uint8 scheme_apply(uint8 scheme_index)
{
    if(scheme_index >= MAX_SCHEMES || !s_schemes[scheme_index].is_used) {
        return 0; // ʧ��
    }

    ParameterScheme* scheme = &s_schemes[scheme_index];

    // ��ʼ�����в���Ϊ0��Ȼ��ͨ������ƥ�������²���
    for(uint8 i = 0; i < parameter_count; i++) {
        uint8 idx = param_config[i].index;
        parameter[idx] = 0.0f; // Ĭ����Ϊ0
        if(param_config[i].target_var != NULL) {
            *(param_config[i].target_var) = 0.0f;
        }
    }

    // ʹ������ƥ�������²�����ֻ��������ʵ�ʴ洢�Ĳ�������
    for(uint8 scheme_idx = 0; scheme_idx < scheme->param_count; scheme_idx++) {
        uint8 idx = scheme->params[scheme_idx].index;
        float value = scheme->params[scheme_idx].value;

        // ���Ҷ�Ӧ�Ĳ�������
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
    return 1; // �ɹ�
}

// ɾ��ָ������
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

//===================================================���ߺ���===================================================

// ��ȡ��ʹ�õķ�������
uint8 get_used_scheme_count(void)
{
    uint8 count = 0;
    for(uint8 i = 0; i < MAX_SCHEMES; i++) {
        if(s_schemes[i].is_used) count++;
    }
    return count;
}

// ��ȡ��n����ʹ�õķ�������
uint8 get_nth_used_scheme_index(uint8 nth)
{
    uint8 count = 0;
    for(uint8 i = 0; i < MAX_SCHEMES; i++) {
        if(s_schemes[i].is_used) {
            if(count == nth) return i;
            count++;
        }
    }
    return 0; // Ĭ�Ϸ���0
}

//===================================================UI���ƺ���===================================================

// ����UI��ܺͱ���
void draw_scheme_ui_frame(const char* title)
{
    ips200_clear();
    ips200_set_color(RGB565_YELLOW, RGB565_BLACK);
    ips200_show_string(PARAM_LEFT_MARGIN, PARAM_TITLE_Y, title);
    ips200_set_color(RGB565_WHITE, RGB565_BLACK);
}

// �������˵�
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
    
    // ��ʾͳ����Ϣ
    uint8 used_count = get_used_scheme_count();
    char info_str[40];
    sprintf(info_str, "Schemes: %d/%d", used_count, MAX_SCHEMES);
    ips200_set_color(RGB565_CYAN, RGB565_BLACK);
    ips200_show_string(PARAM_LEFT_MARGIN, PARAM_START_Y + (MAIN_MENU_COUNT + 1) * PARAM_LINE_HEIGHT, info_str);
    
    // ������ʾ
    ips200_set_color(RGB565_GRAY, RGB565_BLACK);
    ips200_show_string(PARAM_LEFT_MARGIN, ips200_height_max - 40, "1:Up 2:Down 3:Select 4:Back");
    ips200_set_color(RGB565_WHITE, RGB565_BLACK);
}

// �ֲ��������˵�ѡ���ֻ���²˵������ɫ�����ػ�������Ļ��
void update_main_menu_selection(uint8 old_selection, uint8 new_selection)
{
    // �����ѡ����ĸ���
    if(old_selection < MAIN_MENU_COUNT) {
        uint16 old_y_pos = PARAM_START_Y + old_selection * PARAM_LINE_HEIGHT;
        ips200_set_color(RGB565_WHITE, RGB565_BLACK);
        ips200_show_string(PARAM_LEFT_MARGIN, old_y_pos, main_menu_items[old_selection]);
    }
    
    // ������ѡ����ĸ���
    if(new_selection < MAIN_MENU_COUNT) {
        uint16 new_y_pos = PARAM_START_Y + new_selection * PARAM_LINE_HEIGHT;
        ips200_set_color(RGB565_BLUE, RGB565_BLACK);
        ips200_show_string(PARAM_LEFT_MARGIN, new_y_pos, main_menu_items[new_selection]);
    }
    
    ips200_set_color(RGB565_WHITE, RGB565_BLACK);
}

// ���Ʒ����б�����ɾ�������ء��鿴��
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
    
    // ������ʾ
    ips200_set_color(RGB565_GRAY, RGB565_BLACK);
    ips200_show_string(PARAM_LEFT_MARGIN, ips200_height_max - 40, hint);
    ips200_set_color(RGB565_WHITE, RGB565_BLACK);
}

// ���Ʒ�������
void draw_scheme_details(uint8 scheme_index)
{
    ParameterScheme* scheme = &s_schemes[scheme_index];
    
    char title[50];
    sprintf(title, "Details: %s", scheme->name);
    draw_scheme_ui_frame(title);
    
    uint8 lines_drawn = 0;
    uint8 max_params = parameter_count; // ��ʾparam_config�е����в���
    for(uint8 i = s_view_start_param; i < max_params && lines_drawn < PARAM_LINES_PER_PAGE; i++) {
        uint16 y_pos = PARAM_START_Y + lines_drawn * PARAM_LINE_HEIGHT;
        uint16 value_x = PARAM_LEFT_MARGIN + PARAM_VALUE_X_OFFSET;

        // ��ʾ��������
        ips200_show_string(PARAM_LEFT_MARGIN, y_pos, (char*)param_config[i].name);
        
        // ͨ������ƥ����ҷ����ж�Ӧ��ֵ
        float display_value = 0.0f; // Ĭ��Ϊ0
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

    // ������ʾ
    ips200_set_color(RGB565_GRAY, RGB565_BLACK);
    ips200_show_string(PARAM_LEFT_MARGIN, ips200_height_max - 40, "1:Up 2:Down 4:Back");
    ips200_set_color(RGB565_WHITE, RGB565_BLACK);
}

// ����ȷ�϶Ի���
void draw_confirm_dialog(const char* message, const char* hint, uint16 color)
{
    ips200_clear();
    
    // ������ʾ��Ϣ
    uint16 msg_y = ips200_height_max / 2 - 30;
    ips200_set_color(color, RGB565_BLACK);
    ips200_show_string(PARAM_LEFT_MARGIN, msg_y, message);
    
    // ������ʾ��ʾ
    uint16 hint_y = msg_y + 25;
    ips200_set_color(RGB565_WHITE, RGB565_BLACK);
    ips200_show_string(PARAM_LEFT_MARGIN + 20, hint_y, hint);
    
    // ��ʾ�߿�
    ips200_set_color(RGB565_GRAY, RGB565_BLACK);
    ips200_draw_line(5, msg_y - 10, ips200_width_max - 5, msg_y - 10, RGB565_GRAY);
    ips200_draw_line(5, hint_y + 20, ips200_width_max - 5, hint_y + 20, RGB565_GRAY);
    
    ips200_set_color(RGB565_WHITE, RGB565_BLACK);
}

// ����״̬��Ϣ
void draw_status_message(const char* message, uint16 color, uint16 delay_ms)
{
    ips200_clear();
    uint16 msg_y = ips200_height_max / 2 - 10;
    ips200_set_color(color, RGB565_BLACK);
    ips200_show_string(PARAM_LEFT_MARGIN, msg_y, message);
    ips200_set_color(RGB565_WHITE, RGB565_BLACK);
    system_delay_ms(delay_ms);
}

// �ֲ����·����б�ѡ���ֻ����ѡ�������ɫ�����ػ�������Ļ��
void update_scheme_list_selection(uint8 old_selection, uint8 new_selection)
{
    uint8 displayed_old = 0, displayed_new = 0;
    uint8 actual_displayed = 0;
    
    // �ҵ���Ӧ����ʾ������������ɫ
    for(uint8 i = 0; i < MAX_SCHEMES && actual_displayed <= (old_selection > new_selection ? old_selection : new_selection); i++) {
        if(s_schemes[i].is_used) {
            // �����ѡ����ĸ���
            if(actual_displayed == old_selection) {
                uint16 old_y_pos = PARAM_START_Y + actual_displayed * PARAM_LINE_HEIGHT;
                ips200_set_color(RGB565_WHITE, RGB565_BLACK);
                char display_str[50];
                sprintf(display_str, "%d. %s", actual_displayed + 1, s_schemes[i].name);
                ips200_show_string(PARAM_LEFT_MARGIN, old_y_pos, display_str);
            }
            
            // ������ѡ����ĸ���
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

//===================================================W25N04����/�ָ�����===================================================

/**
 * @brief �����з������ݱ��ݵ�W25N04�洢��
 * 
 * @return uint8 0:ʧ�ܣ�1:�ɹ�
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
        return 0; // ����ʧ��
    }
    
    // ��ҳд������(ÿҳ���2048�ֽ�)
    while (written < data_size) {
        uint16 write_size = (data_size - written > 2048) ? 2048 : (data_size - written);
        
        result = w25n04_write_page(page_addr, 0, backup_data + written, write_size);
        if (result != 1) {
            return 0; // д��ʧ��
        }
        
        written += write_size;
        page_addr++;
    }
    
    return 1; // ���ݳɹ�
}

/**
 * @brief ��W25N04�洢���ָ���������
 * 
 * @return uint8 0:ʧ�ܣ�1:�ɹ���2:����У��ʧ��
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
    // ��ҳ��ȡ����
    while (read_count < data_size) {
        uint16 read_size = (data_size - read_count > 2048) ? 2048 : (data_size - read_count);
        
        result = w25n04_read_page(page_addr, 0, restore_data + read_count, read_size);
        if (result == 0 || result == 3) {
            return 0; // ��ȡʧ�ܻ�ECC�޷�����
        }
        
        read_count += read_size;
        page_addr++;
    }
    
    // ����������Ч�Լ��
    for(int i = 0; i < MAX_SCHEMES; i++) {
        if(temp_schemes[i].is_used != 0 && temp_schemes[i].is_used != 1) {
            return 2; // ����У��ʧ��
        }
    }
    
    // ������֤ͨ�������Ƶ���ǰ��������
    memcpy(s_schemes, temp_schemes, sizeof(s_schemes));
    
    // ���浽ϵͳFlash
    scheme_save_to_flash();
    
    return 1; // �ָ��ɹ�
}

//===================================================��ѭ�����¼�����===================================================

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
                        case Button_1: // ��
                            {
                                uint8 old_selection = s_selected_menu;
                                s_selected_menu = (s_selected_menu > 0) ? (s_selected_menu - 1) : (MAIN_MENU_COUNT - 1);
                                update_main_menu_selection(old_selection, s_selected_menu);
                            }
                            break;
                        case Button_2: // ��
                            {
                                uint8 old_selection = s_selected_menu;
                                s_selected_menu = (s_selected_menu < MAIN_MENU_COUNT - 1) ? (s_selected_menu + 1) : 0;
                                update_main_menu_selection(old_selection, s_selected_menu);
                            }
                            break;
                        case Button_3: // ѡ��
                            switch(s_selected_menu) {
                                case 0: // ���浱ǰ����
                                    s_scheme_state = SCHEME_STATE_SAVE_CURRENT;
                                    draw_confirm_dialog("Save Current Scheme?", "3:Yes  4:No", RGB565_GREEN);
                                    break;
                                case 1: // ɾ������
                                    if(get_used_scheme_count() > 0) {
                                        s_scheme_state = SCHEME_STATE_DELETE_LIST;
                                        s_selected_scheme = 0;
                                        draw_scheme_list("Delete Scheme", "1:Up 2:Down 3:Delete 4:Back");
                                    } else {
                                        draw_status_message("No schemes to delete!", RGB565_RED, 1000);
                                        draw_main_menu();
                                    }
                                    break;
                                case 2: // ���ط���
                                    if(get_used_scheme_count() > 0) {
                                        s_scheme_state = SCHEME_STATE_LOAD_LIST;
                                        s_selected_scheme = 0;
                                        draw_scheme_list("Load Scheme", "1:Up 2:Down 3:Load 4:Back");
                                    } else {
                                        draw_status_message("No schemes to load!", RGB565_RED, 1000);
                                        draw_main_menu();
                                    }
                                    break;
                                case 3: // �鿴��������
                                    if(get_used_scheme_count() > 0) {
                                        s_scheme_state = SCHEME_STATE_VIEW_LIST;
                                        s_selected_scheme = 0;
                                        draw_scheme_list("View Scheme Details", "1:Up 2:Down 3:View 4:Back");
                                    } else {
                                        draw_status_message("No schemes to view!", RGB565_RED, 1000);
                                        draw_main_menu();
                                    }
                                    break;
                                case 4: // ���ݵ�W25N04
                                    s_scheme_state = SCHEME_STATE_BACKUP_CONFIRM;
                                    draw_confirm_dialog("Backup to W25N04?", "3:Yes  4:No", RGB565_CYAN);
                                    break;
                                case 5: // ��W25N04�ָ�
                                    s_scheme_state = SCHEME_STATE_RESTORE_CONFIRM;
                                    draw_confirm_dialog("Restore from W25N04?", "3:Yes  4:No", RGB565_MAGENTA);
                                    break;
                            }
                            break;
                        case Button_4: // ����
                            s_ui_initialized = 0;
                            exit_flag = 1;
                            break;
                    }
                    break;

                case SCHEME_STATE_SAVE_CURRENT:
                    if(key == Button_3) { // ȷ�ϱ���
                        if(scheme_save_current()) {
                            draw_status_message("Scheme saved successfully!", RGB565_GREEN, 1500);
                        } else {
                            draw_status_message("Failed! No empty slot.", RGB565_RED, 1500);
                        }
                        s_scheme_state = SCHEME_STATE_MAIN_MENU;
                        draw_main_menu();
                    } else if(key == Button_4) { // ȡ��
                        s_scheme_state = SCHEME_STATE_MAIN_MENU;
                        draw_main_menu();
                    } else {
                        // ������ʾȷ�϶Ի���
                        draw_confirm_dialog("Save Current Scheme?", "3:Yes  4:No", RGB565_GREEN);
                    }
                    break;

                case SCHEME_STATE_DELETE_LIST:
                    switch(key) {
                        case Button_1: // ��
                            {
                                uint8 old_selection = s_selected_scheme;
                                s_selected_scheme = (s_selected_scheme > 0) ? (s_selected_scheme - 1) : (get_used_scheme_count() - 1);
                                update_scheme_list_selection(old_selection, s_selected_scheme);
                            }
                            break;
                        case Button_2: // ��
                            {
                                uint8 old_selection = s_selected_scheme;
                                s_selected_scheme = (s_selected_scheme < get_used_scheme_count() - 1) ? (s_selected_scheme + 1) : 0;
                                update_scheme_list_selection(old_selection, s_selected_scheme);
                            }
                            break;
                        case Button_3: // ɾ��
                            s_scheme_state = SCHEME_STATE_DELETE_CONFIRM;
                            draw_confirm_dialog("Delete Scheme?", "3:Yes  4:No", RGB565_RED);
                            break;
                        case Button_4: // ����
                            s_scheme_state = SCHEME_STATE_MAIN_MENU;
                            draw_main_menu();
                            break;
                    }
                    break;

                case SCHEME_STATE_LOAD_LIST:
                    switch(key) {
                        case Button_1: // ��
                            {
                                uint8 old_selection = s_selected_scheme;
                                s_selected_scheme = (s_selected_scheme > 0) ? (s_selected_scheme - 1) : (get_used_scheme_count() - 1);
                                update_scheme_list_selection(old_selection, s_selected_scheme);
                            }
                            break;
                        case Button_2: // ��
                            {
                                uint8 old_selection = s_selected_scheme;
                                s_selected_scheme = (s_selected_scheme < get_used_scheme_count() - 1) ? (s_selected_scheme + 1) : 0;
                                update_scheme_list_selection(old_selection, s_selected_scheme);
                            }
                            break;
                        case Button_3: // ����
                            {
                                uint8 actual_index = get_nth_used_scheme_index(s_selected_scheme);
                                if(scheme_apply(actual_index)) {
                                    Parameter_flash_save(); // ���غ󱣴浽��������
                                    draw_status_message("Scheme loaded & saved!", RGB565_GREEN, 1500);
                                } else {
                                    draw_status_message("Failed to load scheme!", RGB565_RED, 1500);
                                }
                            }
                            s_scheme_state = SCHEME_STATE_MAIN_MENU;
                            draw_main_menu();
                            break;
                        case Button_4: // ����
                            s_scheme_state = SCHEME_STATE_MAIN_MENU;
                            draw_main_menu();
                            break;
                    }
                    break;

                case SCHEME_STATE_VIEW_LIST:
                    switch(key) {
                        case Button_1: // ��
                            {
                                uint8 old_selection = s_selected_scheme;
                                s_selected_scheme = (s_selected_scheme > 0) ? (s_selected_scheme - 1) : (get_used_scheme_count() - 1);
                                update_scheme_list_selection(old_selection, s_selected_scheme);
                            }
                            break;
                        case Button_2: // ��
                            {
                                uint8 old_selection = s_selected_scheme;
                                s_selected_scheme = (s_selected_scheme < get_used_scheme_count() - 1) ? (s_selected_scheme + 1) : 0;
                                update_scheme_list_selection(old_selection, s_selected_scheme);
                            }
                            break;
                        case Button_3: // �鿴����
                            {
                                uint8 actual_index = get_nth_used_scheme_index(s_selected_scheme);
                                s_scheme_state = SCHEME_STATE_VIEW_DETAILS;
                                s_view_start_param = 0;
                                draw_scheme_details(actual_index);
                            }
                            break;
                        case Button_4: // ����
                            s_scheme_state = SCHEME_STATE_MAIN_MENU;
                            draw_main_menu();
                            break;
                    }
                    break;

                case SCHEME_STATE_VIEW_DETAILS:
                    {
                        uint8 actual_index = get_nth_used_scheme_index(s_selected_scheme);
                        switch(key) {
                            case Button_1: // ���Ϲ���
                                if(s_view_start_param > 0) {
                                    s_view_start_param--;
                                    draw_scheme_details(actual_index);
                                }
                                break;
                            case Button_2: // ���¹���
                                if(s_view_start_param < parameter_count - PARAM_LINES_PER_PAGE) {
                                    s_view_start_param++;
                                    draw_scheme_details(actual_index);
                                }
                                break;
                            case Button_4: // ����
                                s_scheme_state = SCHEME_STATE_VIEW_LIST;
                                draw_scheme_list("View Scheme Details", "1:Up 2:Down 3:View 4:Back");
                                break;
                        }
                    }
                    break;
                
                case SCHEME_STATE_DELETE_CONFIRM:
                    if(key == Button_3) { // ȷ��ɾ��
                        uint8 actual_index = get_nth_used_scheme_index(s_selected_scheme);
                        scheme_delete(actual_index);
                        draw_status_message("Scheme deleted!", RGB565_YELLOW, 1000);
                        s_scheme_state = SCHEME_STATE_MAIN_MENU;
                        draw_main_menu();
                    } else if(key == Button_4) { // ȡ��
                        s_scheme_state = SCHEME_STATE_MAIN_MENU;
                        draw_main_menu();
                    }
                    break;

                case SCHEME_STATE_BACKUP_CONFIRM:
                    if(key == Button_3) { // ȷ�ϱ���
                        uint8 backup_result = scheme_backup_to_w25n04();
                        if(backup_result == 1) {
                            draw_status_message("Backup to W25N04 successful!", RGB565_GREEN, 1500);
                        } else {
                            draw_status_message("Backup to W25N04 failed!", RGB565_RED, 1500);
                        }
                        s_scheme_state = SCHEME_STATE_MAIN_MENU;
                        draw_main_menu();
                    } else if(key == Button_4) { // ȡ��
                        s_scheme_state = SCHEME_STATE_MAIN_MENU;
                        draw_main_menu();
                    } else {
                        // ������ʾȷ�϶Ի���
                        draw_confirm_dialog("Backup to W25N04?", "3:Yes  4:No", RGB565_CYAN);
                    }
                    break;

                case SCHEME_STATE_RESTORE_CONFIRM:
                    if(key == Button_3) { // ȷ�ϻָ�
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
                    } else if(key == Button_4) { // ȡ��
                        s_scheme_state = SCHEME_STATE_MAIN_MENU;
                        draw_main_menu();
                    } else {
                        // ������ʾȷ�϶Ի���
                        draw_confirm_dialog("Restore from W25N04?", "3:Yes  4:No", RGB565_MAGENTA);
                    }
                    break;
            }
        }
        system_delay_ms(20);
    }
    key_flag = Button_4; // ֪ͨ�ϲ�˵�����
}


