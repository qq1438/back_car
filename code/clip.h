#ifndef _clip_h_
#define _clip_h_

#include "zf_common_headfile.h"

#define clamp(val, lo, hi) ((val) < (lo) ? (lo) : ((val) > (hi) ? (hi) : (val)))
#define min(a, b) ((a) < (b) ? (a) : (b))
#define max(a, b) ((a) > (b) ? (a) : (b))

int clip(int x, int low, int up);
float fclip(float x, float low, float up);
float SquareRootFloat(float number);


#endif
