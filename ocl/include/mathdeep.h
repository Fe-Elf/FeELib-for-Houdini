
#ifndef __MATHDEEP_H__
#define __MATHDEEP_H__

#include <mathdeep.h>


static
void swapf(
    float *arg0,
    float *arg1
) {
    float temp = *arg0;
    *arg0 = *arg1;
    *arg1 = temp;
}

static
void swapi(
    int *arg0,
    int *arg1
) {
    int temp = *arg0;
    *arg0 = *arg1;
    *arg1 = temp;
}


inline
float efitf(
    float val,
    float src0,
    float src1,
    float dest0,
    float dest1
    ) {
    return (val-src0) / (src1-src0) * (dest1-dest0) + dest0;
}

inline
float fitf(
    float val,
    float src0,
    float src1,
    float dest0,
    float dest1
    ) {
    return val <= src0 ? dest0 : (val >= src1 ? dest1 : efitf(val, src0, src1, dest0, dest1));
}

inline
float maxf(
    float a,
    float b
    ) {
    return a > b ? a : b;
}

inline
float absf(
    float arg
    ) {
    return arg < 0 ? -arg : arg;
}

inline
int absi(
    int arg
    ) {
    return arg < 0 ? -arg : arg;
}


inline
float lerpf(
    float val0,
    float val1,
    float bias
    ) {
    return val0 * (1-bias) + val1 * bias;
}


inline
float clampf(
    float val,
    float min,
    float max
    ) {
    return val < min ? min : (val > max ? max : val);
}

/*
//Bresenham's line algorithm
//https://zh.wikipedia.org/zh-cn/布雷森漢姆直線演算法
int x0 = gidx;
int y0 = gidy;
int x1 = (int)(gidx + dirx[gid] * 10);
int y1 = (int)(gidy + diry[gid] * 10);
int steep = absi(y1 - y0) > absi(x1 - x0);
if ( steep ) {
    swapi(&x0, &y0);
    swapi(&x1, &y1);
}
if ( x0 > x1 ) {
    swapi(&x0, &x1);
    swapi(&y0, &y1);
}
int deltax = x1 - x0;
int deltay = absi(y1 - y0);
int error = deltax / 2;
int ystep = y0 < y1 ? 1 : -1;
int y = y0;
for (int x = x0; x < x1; ++x) {
    int idx_middle;
    if ( steep ) {
        //if ( x < 0 || x >= density_res_y || y < 0 || y >= density_res_x ) continue;
        idx_middle = density_stride_offset
                   + y * density_stride_x
                   + x * density_stride_y;
    } else {
        //if ( x < 0 || x >= density_res_x || y < 0 || y >= density_res_y ) continue;
        idx_middle = density_stride_offset
                   + x * density_stride_x
                   + y * density_stride_y;
    }
    if ( mask[idx_middle] < THRESHOLD_MASK ) break;
    error -= deltay;
    if ( error < 0 ) {
        y += ystep;
        error += deltax;
    }
}
*/




#endif
