#pragma once

// I need the app_log function from pebble.h.
#include <pebble.h>
#include <stdint.h>
#include <stdio.h>

/* Struct that stores exchange data.
 */
typedef struct {
  char *ex_name;
  int32_t low;
  int32_t high;
  int32_t avg;
  int32_t last;
  int64_t vol;
} ExData;

ExData * create_ex_data(void);

void destroy_ex_data(ExData *data);

void set_ex_name(ExData *data, char *src);
