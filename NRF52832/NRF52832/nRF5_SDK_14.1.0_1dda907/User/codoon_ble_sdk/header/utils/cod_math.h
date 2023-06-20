//
// Created by Paint on 2019-11-21.
//

#ifndef COD_SDK_COD_MATH_H
#define COD_SDK_COD_MATH_H


#include <stdint.h>

#define cod_max(v, compare) (v>compare?v:compare)
#define cod_min(v, compare) (v<compare?v:compare)

int32_t cod_max_min(int32_t v, int32_t min, int32_t max);

float cod_rad(float d);

float cod_latlong2dis(float pre_lat, float pre_lon, float lat, float lon);

/**
 * fuf 表示，输入为float、uint32_t，输出为float
 * @param a
 * @param b
 * @return
 */
float cod_safe_div_fuf(float a, uint32_t b);

#endif //COD_SDK_COD_MATH_H
