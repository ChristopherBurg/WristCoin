#pragma once

#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>

char * create_format_dollars(int64_t price);

char * create_format_volume(int64_t vol);

void destroy_format(char *format);
