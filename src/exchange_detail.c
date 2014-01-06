#include "exchange_detail.h"

static Window *window;
static TextLayer *exchange_name_text;
static TextLayer *low_label;
static TextLayer *low_text;
static TextLayer *high_label;
static TextLayer *high_text;
static TextLayer *last_label;
static TextLayer *last_text;
static TextLayer *average_label;
static TextLayer *average_text;
static TextLayer *buy_label;
static TextLayer *buy_text;
static TextLayer *sell_label;
static TextLayer *sell_text;

static char low[15];
static char high[15];
static char last[15];
static char average[15];
static char buy[15];
static char sell[15];

static ExchangeData *exchange_data;

static void click_config_provider(void *context) {

}

static void window_load(Window *window) {
    Layer *window_layer = window_get_root_layer(window);
    GRect bounds = layer_get_bounds(window_layer);
    int half_screen = (bounds.size.w / 2);

    exchange_name_text = text_layer_create((GRect) { .origin = {0, 0}, .size = {bounds.size.w, 32 } });
    text_layer_set_text_alignment(exchange_name_text, GTextAlignmentCenter);
    text_layer_set_font(exchange_name_text, fonts_get_system_font(FONT_KEY_GOTHIC_28_BOLD));
    layer_add_child(window_layer, text_layer_get_layer(exchange_name_text));

    // Label and text for the low value.
    low_label = text_layer_create((GRect) { .origin = { 0, 32 }, .size = { half_screen, 22 } });
    text_layer_set_font(low_label, fonts_get_system_font(FONT_KEY_GOTHIC_18_BOLD));
    text_layer_set_text(low_label, "Low:");
    layer_add_child(window_layer, text_layer_get_layer(low_label));

    low_text = text_layer_create((GRect) { .origin = { half_screen, 32 }, .size = { half_screen, 22 } });
    text_layer_set_font(low_text, fonts_get_system_font(FONT_KEY_GOTHIC_18));
    text_layer_set_text(low_text, low);
    layer_add_child(window_layer, text_layer_get_layer(low_text));

    // Label and text for the high value.
    high_label = text_layer_create((GRect) { .origin = { 0, 54 }, .size = { half_screen, 22 } });
    text_layer_set_font(high_label, fonts_get_system_font(FONT_KEY_GOTHIC_18_BOLD));
    text_layer_set_text(high_label, "High:");
    layer_add_child(window_layer, text_layer_get_layer(high_label));

    high_text = text_layer_create((GRect) { .origin = { half_screen, 54 }, .size = { half_screen, 22 } });
    text_layer_set_font(high_text, fonts_get_system_font(FONT_KEY_GOTHIC_18));
    text_layer_set_text(high_text, high);
    layer_add_child(window_layer, text_layer_get_layer(high_text));

    // Label and text for the last value.
    last_label = text_layer_create((GRect) { .origin = { 0, 76 }, .size = { half_screen, 22 } });
    text_layer_set_font(last_label, fonts_get_system_font(FONT_KEY_GOTHIC_18_BOLD));
    text_layer_set_text(last_label, "Last:");
    layer_add_child(window_layer, text_layer_get_layer(last_label));

    last_text = text_layer_create((GRect) { .origin = { half_screen, 76 }, .size = { half_screen, 22 } });
    text_layer_set_font(last_text, fonts_get_system_font(FONT_KEY_GOTHIC_18));
    text_layer_set_text(last_text, last);
    layer_add_child(window_layer, text_layer_get_layer(last_text));

    // Label and text for the average value.
    average_label = text_layer_create((GRect) { .origin = { 0, 98 }, .size = { half_screen, 22 } });
    text_layer_set_font(average_label, fonts_get_system_font(FONT_KEY_GOTHIC_18_BOLD));
    text_layer_set_text(average_label, "Average:");
    layer_add_child(window_layer, text_layer_get_layer(average_label));

    average_text = text_layer_create((GRect) { .origin = { half_screen, 98 }, .size = { half_screen, 22 } });
    text_layer_set_font(average_text, fonts_get_system_font(FONT_KEY_GOTHIC_18));
    text_layer_set_text(average_text, average);
    layer_add_child(window_layer, text_layer_get_layer(average_text));

    // Label and text for the buy value.
    buy_label = text_layer_create((GRect) { .origin = { 0, 120 }, .size = { half_screen, 22 } });
    text_layer_set_font(buy_label, fonts_get_system_font(FONT_KEY_GOTHIC_18_BOLD));
    text_layer_set_text(buy_label, "Buy:");
    layer_add_child(window_layer, text_layer_get_layer(buy_label));

    buy_text = text_layer_create((GRect) { .origin = { half_screen, 120 }, .size = { half_screen, 22 } });
    text_layer_set_font(buy_text, fonts_get_system_font(FONT_KEY_GOTHIC_18));
    text_layer_set_text(buy_text, buy);
    layer_add_child(window_layer, text_layer_get_layer(buy_text));

    // Label and text for the sell value.
    sell_label = text_layer_create((GRect) { .origin = { 0, 142 }, .size = { half_screen, 22 } });
    text_layer_set_font(sell_label, fonts_get_system_font(FONT_KEY_GOTHIC_18_BOLD));
    text_layer_set_text(sell_label, "Sell:");
    layer_add_child(window_layer, text_layer_get_layer(sell_label));

    sell_text = text_layer_create((GRect) { .origin = { half_screen, 142 }, .size = { half_screen, 22 } });
    text_layer_set_font(sell_text, fonts_get_system_font(FONT_KEY_GOTHIC_18));
    text_layer_set_text(sell_text, sell);
    layer_add_child(window_layer, text_layer_get_layer(sell_text));
}

static void window_unload(Window *window) {
    text_layer_destroy(exchange_name_text);
    text_layer_destroy(last_text);
}

static void window_appear(Window *window) {
    
    text_layer_set_text(exchange_name_text, exchange_data->exchange_name);

    app_log(APP_LOG_LEVEL_DEBUG, "exchange_detail.c", 56, "Low value is: %ld.", exchange_data->low);

    exchange_data_display_as_currency(low, 15, exchange_data->low);
    exchange_data_display_as_currency(high, 15, exchange_data->high);
    exchange_data_display_as_currency(last, 15, exchange_data->last);
    exchange_data_display_as_currency(average, 15, exchange_data->average);
    exchange_data_display_as_currency(buy, 15, exchange_data->buy);
    exchange_data_display_as_currency(sell, 15, exchange_data->sell);
}

void exchange_detail_init(void) {
    window = window_create();
    window_set_click_config_provider(window, click_config_provider);
    window_set_window_handlers(window, (WindowHandlers) {
        .load = window_load,
        .unload = window_unload,
        .appear = window_appear,
    });
}

void exchange_detail_deinit(void) {
    window_destroy(window);
}

void exchange_detail_show(ExchangeData *selected_exchange_data) {
    exchange_data = selected_exchange_data;

    window_stack_push(window, true);
}

