#pragma once

#include <stdint.h>
#include <stdio.h>

/* #define statements used throughout this file for convenience. 
*/
#define EXCHANGE_NAME_LENGTH (10) // Length, in bytes, an exchange's name can be.
#define PRICE_FIELD_LENGTH (15) // Length, in bytes, a price field can be.
#define VOLUME_FIELD_LENGTH (20) // Length, in bytes, a volume field can be.

#define NUMBER_OF_EXCHANGES (3) // The number of exchanges. Increment when an exchange is added.
#define BITSTAMP_INDEX (0) // Index in the exchage_data_list array that contains Bitstamp data.
#define MTGOX_INDEX (1) // Index in the exchange_data_list array that contians Mt. Gox data.
#define BTCE_INDEX (2) // Index in the exchange_data_list array that contains BTC-e data.

typedef struct {
    char exchange_name[EXCHANGE_NAME_LENGTH];
    int32_t high;
    int32_t low;
    int32_t last;
    int32_t average;
    int32_t buy;
    int32_t sell;
    int64_t volume;
} ExchangeData;

/* Takes an integer value, creates a decimal string representation, and copies
   that string into a destination.

   char *dest - The string to copy the decimal representation into.

   int value - The integer value to convert.
*/
void format_as_dollars(char *dest, int32_t value);

//void format_as_bitcoin_with_precision(char *dest, int64_t value, uint8_t precision);

void format_as_bitcoin(char *dest, int64_t value);

