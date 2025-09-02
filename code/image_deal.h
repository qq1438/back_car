#ifndef _image_deal_h_
#define _image_deal_h_

#include "zf_common_headfile.h"

#define MAX_POINT_NUM 1000

typedef struct {
    uint8 num;
    uint16 area;
    uint8 all_point[MAX_POINT_NUM][2];
    uint32 location_x;
    uint32 location_y;
} Connected_area;

typedef struct point
{
    uint8 now_point[2];
    uint8 last_point[2];
    uint8 llast_point[2];
} point;


extern point one_point;
extern point two_point;
extern uint8 area_num;
extern uint8 is_find;
extern uint8 Image[MT9V03X_H][MT9V03X_W];

extern float error[3];

void Image_copy(void);
void Image_Binary(uint8 threshold);
void Connected_Find(uint8 start_x, uint8 start_y, Connected_area *current_area, uint8 brightness_threshold);
void Image_Deal(void);
void Update_Points(point *point1, point *point2);
void Calculate_Error(void);
uint8 Get_Max_Brightness(void);
uint8 Seek_Start(point *point1, point *point2, uint8 area);
float get_distance(float y);

#endif
