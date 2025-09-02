#include "PTS_Deal.h"

bool is_start = 0;
volatile bool is_compress = false;
void PTS_Deal(void)
{
    if(mt9v03x_finish_flag)
    {
        Image_copy();
        mt9v03x_finish_flag=0;
        if(Send_state)
        {
            while(is_compress);
            image_compress(Image);
        }
        if(!is_start){
            if(Seek_Start(&one_point, &two_point, 100))is_start = 1;
        }else{
            //WITE();
            Image_Deal();
            Update_Points(&one_point, &two_point);
            Calculate_Error();
        }
        if(Send_state)
        {
            point_compress(one_point_type);
            point_compress(two_point_type);
            parameter_compress_float(add_speed,acc_target_type);
            parameter_compress_float(pid_speed,acc_pid_type);
            parameter_compress(error);
            is_compress = true;
        }
    }
}

void PTS_Test(void)
{
    if(mt9v03x_finish_flag)
    {
        Image_copy();
        mt9v03x_finish_flag=0;
        if(Send_state)
        {
            while(is_compress);
            image_compress(Image);
        }
        WITE();
        Image_Deal();
        Update_Points(&one_point, &two_point);
        Calculate_Error();
    }
    if(key_flag == Button_3)
    {
        key_flag = 0;
        uint16 boot_exp;
        mt9v03x_init();
        ExposureTime_flash_load(&boot_exp);
        mt9v03x_set_exposure_time(boot_exp);
    }
}

