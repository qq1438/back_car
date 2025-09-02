#include "Show_Data.h"

#define IPCH 30
#define IPCW 80

float Mh = MT9V03X_H;
float Lh = IPCH;
float Mw = MT9V03X_W;
float Lw = IPCW;
uint8 Image_Use[IPCH][IPCW];


void Send_Zoomimg(void)
{
    int i, j, row, line;
    const float div_h = Mh / Lh, div_w = Mw / Lw;
    for (i = 0; i < IPCH; i++)
    {
        row = i * div_h + 0.5;
        for (j = 0; j < IPCW; j++)
        {
            line = j * div_w + 0.5;
            Image_Use[i][j] = Image[row][line];
        }
    }
    sendimg((pico_uint8 *)Image_Use, 80, 30);
}

void Show_Point(uint8 point[2],const uint16 color)
{
    uint8 x = point[0];
    uint8 y = point[1];
    for(int8 i = -1; i <= 1; i++)
    {
        for(int8 j = -1; j <= 1; j++)
        {
            if(x + i >= 0 && x + i < ips200_width_max && y + j >= 0 && y + j < ips200_height_max)
            {
                ips200_draw_point(x + i,y + j,color);
            }
        }
    }
}

void WITE(void)
{
//    Image_Binary(200);
    ips200_show_gray_image(0,0,Image[0],MT9V03X_W, MT9V03X_H, MT9V03X_W, MT9V03X_H, 0);
     Show_Point(one_point.llast_point,RGB565_WHITE);
     Show_Point(two_point.llast_point,RGB565_WHITE);

     Show_Point(one_point.last_point,RGB565_GREEN);
     Show_Point(two_point.last_point,RGB565_GREEN);

     Show_Point(one_point.now_point,RGB565_BLACK);
     Show_Point(two_point.now_point,RGB565_RED);
    ips200_show_uint(0,130,one_point.now_point[0],3);
    ips200_show_uint(60,130,one_point.now_point[1],3);
    ips200_show_uint(120,130,two_point.now_point[0],3);
    ips200_show_uint(180,130,two_point.now_point[1],3);

    ips200_show_float(0,160,error[0],3,3);
    ips200_show_float(60,160,error[1],3,3);
    ips200_show_float(120,160,error[2],3,3);

    ips200_show_uint(60,190,nec_stop_flag,3);
    ips200_show_uint(90,190,area_num,3);

}

