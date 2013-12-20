#pragma once

#include <stdint.h>
#include <stdio.h>

/* #define statements used throughout this file for convenience. 
*/
#define EXCHANGE_NAME_LENGTH (10) // Length, in bytes, an exchange's name can be.

typedef struct {
    char exchange_name[EXCHANGE_NAME_LENGTH];
    int32_t high;
    int32_t low;
    int32_t last;
} ExchangeData;

/* Takes an integer value, creates a decimal string representation, and copies
   that string into a destination.

   char *dest - The string to copy the decimal representation into.

   int length - The maximum length of the string. 

   int value - The integer value to convert.
*/
void exchange_data_display_as_currency(char *dest, int length, int value);
