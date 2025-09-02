#ifndef __KEY_H__
#define __KEY_H__

typedef enum
{
    Button_1 = 1,
    Button_2 = 2,
    Button_3 = 3,
    Button_4 = 4,
}key_enum;


#include "zf_common_headfile.h"

void my_key_init(void);
void key_scan(void);

extern volatile uint8 key_flag;
extern volatile bool nec_stop_flag;
#endif
