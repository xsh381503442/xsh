//
// Created by Paint on 2019-11-11.
//

#ifndef COD_SDK_COD_TIME_H
#define COD_SDK_COD_TIME_H

#include "../cod_ble_bean.h"

uint64_t cod_time2timestamp(cod_ble_time *input);

void cod_timestamp2time(uint64_t timestamp, cod_ble_time* output);

void cod_timestamp2time_array(uint64_t timestamp, uint8_t* output, uint8_t output_size);


#endif //COD_SDK_COD_TIME_H
