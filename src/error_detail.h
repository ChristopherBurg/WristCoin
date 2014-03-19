#pragma once

#include <pebble.h>
#include <errors.h>

void error_detail_init(void);
 
void error_detail_deinit(void);
 
void error_detail_show(ErrorMessages selected_error);

