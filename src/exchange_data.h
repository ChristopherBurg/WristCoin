#pragma once

#include <stdint.h>

/* #define statements used throughout this file for convenience. 
*/
#define EXCHANGE_NAME_LENGTH (10) // Length, in bytes, an exchange's name can be.

typedef struct {
    char exchange_name[EXCHANGE_NAME_LENGTH];
    int32_t high;
    int32_t low;
    int32_t last;
} ExchangeData;

