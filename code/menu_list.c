#include "menu.h"
#include "menu_list.h"
#include "zf_common_headfile.h"
#include "parameter_scheme.h"
#include "exposure_adjust.h"

// ����˵��ṹ�����
Menu mainMenu;
Menu subMenu1;
Menu subMenu2;
Menu parameterMenu;  // ���������˵�

// �˳��˵�ϵͳ�ĺ���
void exit_menu_system(void)
{
    uint8 selection = 0;     // ѡ���־ 0-�رմ洢 1-�����洢

    // ��ʾ�洢״̬ Send_state
    ips200_clear();
    ips200_set_color(RGB565_YELLOW, RGB565_BLACK);
    ips200_show_string(20, 20, "OPEN STORAGE?");
    ips200_set_color(RGB565_WHITE, RGB565_BLACK);

    // ��ʾѡ��
    ips200_show_string(40, 60, "0. CLOSE");
    ips200_show_string(40, 80, "1. OPEN");

    // ���ݵ�ǰ�洢״̬���ó�ʼѡ��
    selection = Send_state ? 1 : 0;
    ips200_set_color(RGB565_BLUE, RGB565_BLACK);
    ips200_show_string(20, selection ? 80 : 60, ">");
    ips200_set_color(RGB565_WHITE, RGB565_BLACK);
    key_flag = 0;
    // �ȴ��û�ѡ��
    while (1)
    {
        if (key_flag != 0)
        {
            switch (key_flag)
            {
                case Button_1:  // �ϼ�
                case Button_2:  // �¼�
                    // �����ǰѡ����
                    ips200_set_color(RGB565_BLACK, RGB565_BLACK);
                    ips200_show_string(20, selection ? 80 : 60, ">");

                    // �л�ѡ��
                    selection = selection ? 0 : 1;

                    // ��ʾ�µ�ѡ����
                    ips200_set_color(RGB565_BLUE, RGB565_BLACK);
                    ips200_show_string(20, selection ? 80 : 60, ">");
                    ips200_set_color(RGB565_WHITE, RGB565_BLACK);
                    break;

                case Button_3:  // ȷ�ϼ�
                    // ���´洢״̬
                    Send_state = selection;

                    // ��ʾȷ����Ϣ
                    ips200_set_color(RGB565_GREEN, RGB565_BLACK);
                    ips200_show_string(20, 120, selection ? "STORAGE OPEN" : "STORAGE CLOSE");
                    ips200_set_color(RGB565_WHITE, RGB565_BLACK);
                    system_delay_ms(1000);  // ��ʱ1��

                    // ���������־
                    key_flag = 0;
                    goto exit_selection;

                default:
                    break;
            }

            // ���������־
            key_flag = 0;
        }

        // ������ʱ
        system_delay_ms(10);
    }

exit_selection:
{   // ��ʼ���洢
    if(Send_state)
    {
        image_storage_init();
        erase_storage_block();
    }
    // ��ʾ�˳���Ϣ
    ips200_clear();
    ips200_set_color(RGB565_YELLOW, RGB565_BLACK);
    ips200_show_string(20, 20, "Exiting menu system...");
    ips200_set_color(RGB565_WHITE, RGB565_BLACK);

    // ��ʾ�����Ϣ
    ips200_set_color(RGB565_GREEN, RGB565_BLACK);
    ips200_show_string(20, 80, "Exit completed!");
    ips200_set_color(RGB565_WHITE, RGB565_BLACK);
    system_delay_ms(2000);

    // �����˳���־
    g_menu_exit_flag = 1;

    // ���ð�����־Ϊ���ؼ�
    key_flag = Button_4;
}
}

// ��Ƶ�طŹ���
void video_playback(void)
{
    uint8 key = 0;
    image_read_frame_count();
    uint32 frame_count = get_frame_count();
    // ����Ƿ��д洢��֡
    if (frame_count == 0)
    {
        ips200_set_color(RGB565_RED, RGB565_BLACK);
        ips200_show_string(20, 80, "No stored images!");
        ips200_set_color(RGB565_WHITE, RGB565_BLACK);
        system_delay_ms(1000);
        return;
    }

    ips200_set_color(RGB565_WHITE, RGB565_BLACK);

    // ��ȡ��һ֡����ʾ
    if (read_compressed_image(go) == STORAGE_OK)
    {
        image_data_analysis();
    }
    key_flag = 0;
    // �طſ���ѭ��
    while (1)
    {
        // ��鰴��
        if (key_flag != 0)
        {
            key = key_flag;
            key_flag = 0;

            switch (key)
            {
                case Button_1:  // ��һ֡
                    read_compressed_image(back);
                    break;

                case Button_2:  // ��һ֡
                    read_compressed_image(go);

                    break;

                case Button_3:  // ���10֡
                    read_compressed_image(go_go);
                    break;

                case Button_4:  // ����10֡
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
    // ����Ƿ��д洢��֡
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



// ��ʼ�����Բ˵�
void init_test_menu(void)
{
    // ��ʼ�����˵�
    strncpy(mainMenu.title, "Main Menu", MAX_MENU_NAME_LEN-1);
    mainMenu.itemCount = 0;
    mainMenu.currentSelection = 0;
    mainMenu.parentMenu = NULL;

    menu_create_item(&mainMenu, "Image Look",PTS_Test, NULL); 
    menu_create_item(&mainMenu, "Video Playback", video_playback, NULL);  // �����Ƶ�ط�ѡ��
    menu_create_item(&mainMenu, "Data Playback", Data_playback, NULL);  // ������ݻط�ѡ��
    menu_create_item(&mainMenu, "Parameter Adjust", parameter_menu_entry, NULL);  // ��Ӳ�������ѡ��
    menu_create_item(&mainMenu, "Scheme Management", scheme_menu_entry, NULL);  // ��ӷ�������
    menu_create_item(&mainMenu, "Exposure Adjust", exposure_time_adjust, NULL);  // ��������������ѡ��
    menu_create_item(&mainMenu, "Exit Menu", exit_menu_system, NULL);  // ����˳��˵�ѡ��

}

// �򿪲˵�ϵͳ
void menu_open(void)
{
    // ��ʼ���˵�ϵͳ
    menu_init();

    // �����˳���־
    menu_reset_exit_flag();

    // ��ʼ�����Բ˵�
    init_test_menu();

    // ���õ�ǰ�˵�Ϊ���˵�
    menu_set_current(&mainMenu);

    // �˵�����ѭ��
    while (1)
    {
        // ����˵�
        g_currentMenu = menu_process(g_currentMenu);
        if (g_currentMenu == NULL)
        {
            // �˳��˵�ϵͳ
            break;
        }

        // ��ʱ
        system_delay_ms(10);
    }

    // ����
    ips200_clear();
}
