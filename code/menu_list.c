#include "menu.h"
#include "menu_list.h"
#include "zf_common_headfile.h"
#include "parameter_scheme.h"
#include "exposure_adjust.h"

// 定义菜单结构体变量
Menu mainMenu;
Menu subMenu1;
Menu subMenu2;
Menu parameterMenu;  // 参数调整菜单

// 退出菜单系统的函数
void exit_menu_system(void)
{
    uint8 selection = 0;     // 选择标志 0-关闭存储 1-开启存储

    // 显示存储状态 Send_state
    ips200_clear();
    ips200_set_color(RGB565_YELLOW, RGB565_BLACK);
    ips200_show_string(20, 20, "OPEN STORAGE?");
    ips200_set_color(RGB565_WHITE, RGB565_BLACK);

    // 显示选项
    ips200_show_string(40, 60, "0. CLOSE");
    ips200_show_string(40, 80, "1. OPEN");

    // 根据当前存储状态设置初始选择
    selection = Send_state ? 1 : 0;
    ips200_set_color(RGB565_BLUE, RGB565_BLACK);
    ips200_show_string(20, selection ? 80 : 60, ">");
    ips200_set_color(RGB565_WHITE, RGB565_BLACK);
    key_flag = 0;
    // 等待用户选择
    while (1)
    {
        if (key_flag != 0)
        {
            switch (key_flag)
            {
                case Button_1:  // 上键
                case Button_2:  // 下键
                    // 清除当前选择标记
                    ips200_set_color(RGB565_BLACK, RGB565_BLACK);
                    ips200_show_string(20, selection ? 80 : 60, ">");

                    // 切换选择
                    selection = selection ? 0 : 1;

                    // 显示新的选择标记
                    ips200_set_color(RGB565_BLUE, RGB565_BLACK);
                    ips200_show_string(20, selection ? 80 : 60, ">");
                    ips200_set_color(RGB565_WHITE, RGB565_BLACK);
                    break;

                case Button_3:  // 确认键
                    // 更新存储状态
                    Send_state = selection;

                    // 显示确认信息
                    ips200_set_color(RGB565_GREEN, RGB565_BLACK);
                    ips200_show_string(20, 120, selection ? "STORAGE OPEN" : "STORAGE CLOSE");
                    ips200_set_color(RGB565_WHITE, RGB565_BLACK);
                    system_delay_ms(1000);  // 延时1秒

                    // 清除按键标志
                    key_flag = 0;
                    goto exit_selection;

                default:
                    break;
            }

            // 清除按键标志
            key_flag = 0;
        }

        // 短暂延时
        system_delay_ms(10);
    }

exit_selection:
{   // 初始化存储
    if(Send_state)
    {
        image_storage_init();
        erase_storage_block();
    }
    // 显示退出信息
    ips200_clear();
    ips200_set_color(RGB565_YELLOW, RGB565_BLACK);
    ips200_show_string(20, 20, "Exiting menu system...");
    ips200_set_color(RGB565_WHITE, RGB565_BLACK);

    // 显示完成信息
    ips200_set_color(RGB565_GREEN, RGB565_BLACK);
    ips200_show_string(20, 80, "Exit completed!");
    ips200_set_color(RGB565_WHITE, RGB565_BLACK);
    system_delay_ms(2000);

    // 设置退出标志
    g_menu_exit_flag = 1;

    // 设置按键标志为返回键
    key_flag = Button_4;
}
}

// 视频回放功能
void video_playback(void)
{
    uint8 key = 0;
    image_read_frame_count();
    uint32 frame_count = get_frame_count();
    // 检查是否有存储的帧
    if (frame_count == 0)
    {
        ips200_set_color(RGB565_RED, RGB565_BLACK);
        ips200_show_string(20, 80, "No stored images!");
        ips200_set_color(RGB565_WHITE, RGB565_BLACK);
        system_delay_ms(1000);
        return;
    }

    ips200_set_color(RGB565_WHITE, RGB565_BLACK);

    // 读取第一帧并显示
    if (read_compressed_image(go) == STORAGE_OK)
    {
        image_data_analysis();
    }
    key_flag = 0;
    // 回放控制循环
    while (1)
    {
        // 检查按键
        if (key_flag != 0)
        {
            key = key_flag;
            key_flag = 0;

            switch (key)
            {
                case Button_1:  // 上一帧
                    read_compressed_image(back);
                    break;

                case Button_2:  // 下一帧
                    read_compressed_image(go);

                    break;

                case Button_3:  // 快进10帧
                    read_compressed_image(go_go);
                    break;

                case Button_4:  // 快退10帧
                    read_compressed_image(back_back);
                    break;

                default:
                    break;
            }
        }
        image_data_analysis();
        WITE();
    }
}

void Data_playback(void)
{
    send_signel();
    image_read_frame_count();
    uint32 frame_count = get_frame_count();
    // 检查是否有存储的帧
    if (frame_count == 0)
    {
        ips200_set_color(RGB565_RED, RGB565_BLACK);
        ips200_show_string(20, 80, "No stored images!");
        ips200_set_color(RGB565_WHITE, RGB565_BLACK);
        system_delay_ms(1000);
        return;
    }

    ips200_set_color(RGB565_WHITE, RGB565_BLACK);
    while (1)
    {
        if(read_compressed_data() == STORAGE_END)break;
        data_analysis();
        send_value();
    }
}



// 初始化测试菜单
void init_test_menu(void)
{
    // 初始化主菜单
    strncpy(mainMenu.title, "Main Menu", MAX_MENU_NAME_LEN-1);
    mainMenu.itemCount = 0;
    mainMenu.currentSelection = 0;
    mainMenu.parentMenu = NULL;

    menu_create_item(&mainMenu, "Image Look",PTS_Test, NULL); 
    menu_create_item(&mainMenu, "Video Playback", video_playback, NULL);  // 添加视频回放选项
    menu_create_item(&mainMenu, "Data Playback", Data_playback, NULL);  // 添加数据回放选项
    menu_create_item(&mainMenu, "Parameter Adjust", parameter_menu_entry, NULL);  // 添加参数调整选项
    menu_create_item(&mainMenu, "Scheme Management", scheme_menu_entry, NULL);  // 添加方案管理
    menu_create_item(&mainMenu, "Exposure Adjust", exposure_time_adjust, NULL);  // 添加相机参数调整选项
    menu_create_item(&mainMenu, "Exit Menu", exit_menu_system, NULL);  // 添加退出菜单选项

}

// 打开菜单系统
void menu_open(void)
{
    // 初始化菜单系统
    menu_init();

    // 重置退出标志
    menu_reset_exit_flag();

    // 初始化测试菜单
    init_test_menu();

    // 设置当前菜单为主菜单
    menu_set_current(&mainMenu);

    // 菜单处理循环
    while (1)
    {
        // 处理菜单
        g_currentMenu = menu_process(g_currentMenu);
        if (g_currentMenu == NULL)
        {
            // 退出菜单系统
            break;
        }

        // 延时
        system_delay_ms(10);
    }

    // 清屏
    ips200_clear();
}
