#pragma once

#include <pebble.h>
#include "exchange_data.h"

void exchange_detail_init(void);

void exchange_detail_deinit(void);

void exchange_detail_show(ExchangeData *select_exchange_data);
