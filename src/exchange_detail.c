#include "exchange_detail.h"

static Window *window;
static ScrollLayer *scroll_layer;

static TextLayer *exchange_name_display;
static TextLayer *low_label;
static TextLayer *low_display;
static TextLayer *high_label;
static TextLayer *high_display;
static TextLayer *last_label;
static TextLayer *last_display;
static TextLayer *average_label;
static TextLayer *average_display;
static TextLayer *buy_label;
static TextLayer *buy_display;
static TextLayer *sell_label;
static TextLayer *sell_display;
static TextLayer *volume_label;
static TextLayer *volume_display;

static const char const *low_text = "Low:";
static const char const *high_text = "High:";
static const char const *last_text = "Last:";
static const char const *average_text = "Average:";
static const char const *buy_text = "Buy:";
static const char const *sell_text = "Sell:";
static const char const *volume_text = "Volume:";

static char low[PRICE_FIELD_LENGTH];
static char high[PRICE_FIELD_LENGTH];
static char last[PRICE_FIELD_LENGTH];
static char average[PRICE_FIELD_LENGTH];
static char buy[PRICE_FIELD_LENGTH];
static char sell[PRICE_FIELD_LENGTH];
static char volume[VOLUME_FIELD_LENGTH];

static ExchangeData *exchange_data;

static void click_config_provider(void *context) {

}

static void window_load(Window *window) {
    Layer *window_layer = window_get_root_layer(window);
    GRect bounds = layer_get_bounds(window_layer);
    int half_screen = (bounds.size.w / 2);

    scroll_layer = scroll_layer_create((GRect) { .origin = { 0, 32 }, .size = { bounds.size.w, bounds.size.h - 32 } });
    scroll_layer_set_content_size(scroll_layer, GSize(bounds.size.w, 176));
    layer_add_child(window_layer, scroll_layer_get_layer(scroll_layer));

    exchange_name_display = text_layer_create((GRect) { .origin = { 0, 0 }, .size = { bounds.size.w, 32 } });
    text_layer_set_text_alignment(exchange_name_display, GTextAlignmentCenter);
    text_layer_set_font(exchange_name_display, fonts_get_system_font(FONT_KEY_GOTHIC_28_BOLD));
    layer_add_child(window_layer, text_layer_get_layer(exchange_name_display));

    // Label and text for the low value.
    low_label = text_layer_create((GRect) { .origin = { 0, 0 }, .size = { half_screen, 22 } });
    text_layer_set_font(low_label, fonts_get_system_font(FONT_KEY_GOTHIC_18_BOLD));
    text_layer_set_text(low_label, low_text);
    scroll_layer_add_child(scroll_layer, text_layer_get_layer(low_label));

    low_display = text_layer_create((GRect) { .origin = { half_screen, 0 }, .size = { half_screen, 22 } });
    text_layer_set_font(low_display, fonts_get_system_font(FONT_KEY_GOTHIC_18));
    text_layer_set_text(low_display, low);
    scroll_layer_add_child(scroll_layer, text_layer_get_layer(low_display));

    // Label and text for the high value.
    high_label = text_layer_create((GRect) { .origin = { 0, 22 }, .size = { half_screen, 22 } });
    text_layer_set_font(high_label, fonts_get_system_font(FONT_KEY_GOTHIC_18_BOLD));
    text_layer_set_text(high_label, high_text);
    scroll_layer_add_child(scroll_layer, text_layer_get_layer(high_label));

    high_display = text_layer_create((GRect) { .origin = { half_screen, 22 }, .size = { half_screen, 22 } });
    text_layer_set_font(high_display, fonts_get_system_font(FONT_KEY_GOTHIC_18));
    text_layer_set_text(high_display, high);
    scroll_layer_add_child(scroll_layer, text_layer_get_layer(high_display));

    // Label and text for the last value.
    last_label = text_layer_create((GRect) { .origin = { 0, 44 }, .size = { half_screen, 22 } });
    text_layer_set_font(last_label, fonts_get_system_font(FONT_KEY_GOTHIC_18_BOLD));
    text_layer_set_text(last_label, last_text);
    scroll_layer_add_child(scroll_layer, text_layer_get_layer(last_label));

    last_display = text_layer_create((GRect) { .origin = { half_screen, 44 }, .size = { half_screen, 22 } });
    text_layer_set_font(last_display, fonts_get_system_font(FONT_KEY_GOTHIC_18));
    text_layer_set_text(last_display, last);
    scroll_layer_add_child(scroll_layer, text_layer_get_layer(last_display));

    // Label and text for the average value.
    average_label = text_layer_create((GRect) { .origin = { 0, 66 }, .size = { half_screen, 22 } });
    text_layer_set_font(average_label, fonts_get_system_font(FONT_KEY_GOTHIC_18_BOLD));
    text_layer_set_text(average_label, average_text);
    scroll_layer_add_child(scroll_layer, text_layer_get_layer(average_label));

    average_display = text_layer_create((GRect) { .origin = { half_screen, 66 }, .size = { half_screen, 22 } });
    text_layer_set_font(average_display, fonts_get_system_font(FONT_KEY_GOTHIC_18));
    text_layer_set_text(average_display, average);
    scroll_layer_add_child(scroll_layer, text_layer_get_layer(average_display));

    // Label and text for the buy value.
    buy_label = text_layer_create((GRect) { .origin = { 0, 88 }, .size = { half_screen, 22 } });
    text_layer_set_font(buy_label, fonts_get_system_font(FONT_KEY_GOTHIC_18_BOLD));
    text_layer_set_text(buy_label, buy_text);
    scroll_layer_add_child(scroll_layer, text_layer_get_layer(buy_label));

    buy_display = text_layer_create((GRect) { .origin = { half_screen, 88 }, .size = { half_screen, 22 } });
    text_layer_set_font(buy_display, fonts_get_system_font(FONT_KEY_GOTHIC_18));
    text_layer_set_text(buy_display, buy);
    scroll_layer_add_child(scroll_layer, text_layer_get_layer(buy_display));

    // Label and text for the sell value.
    sell_label = text_layer_create((GRect) { .origin = { 0, 110 }, .size = { half_screen, 22 } });
    text_layer_set_font(sell_label, fonts_get_system_font(FONT_KEY_GOTHIC_18_BOLD));
    text_layer_set_text(sell_label, sell_text);
    scroll_layer_add_child(scroll_layer, text_layer_get_layer(sell_label));

    sell_display = text_layer_create((GRect) { .origin = { half_screen, 110 }, .size = { half_screen, 22 } });
    text_layer_set_font(sell_display, fonts_get_system_font(FONT_KEY_GOTHIC_18));
    text_layer_set_text(sell_display, sell);
    scroll_layer_add_child(scroll_layer, text_layer_get_layer(sell_display));

    // Label and text for the volume.
    volume_label = text_layer_create((GRect) { .origin = { 0, 132 }, .size = { half_screen, 22 } });
    text_layer_set_font(volume_label, fonts_get_system_font(FONT_KEY_GOTHIC_18_BOLD));
    text_layer_set_text(volume_label, volume_text);
    scroll_layer_add_child(scroll_layer, text_layer_get_layer(volume_label));

    volume_display = text_layer_create((GRect) { .origin = { 10, 154 }, .size = { bounds.size.w - 10, 22 } });
    text_layer_set_font(volume_display, fonts_get_system_font(FONT_KEY_GOTHIC_18));
    text_layer_set_text(volume_display, volume);
    scroll_layer_add_child(scroll_layer, text_layer_get_layer(volume_display));

    scroll_layer_set_click_config_onto_window(scroll_layer, window);
}

static void window_unload(Window *window) {
    text_layer_destroy(exchange_name_display);
    text_layer_destroy(low_label);
    text_layer_destroy(low_display);
    text_layer_destroy(high_label);
    text_layer_destroy(high_display);
    text_layer_destroy(last_label);
    text_layer_destroy(last_display);
    text_layer_destroy(average_label);
    text_layer_destroy(average_display);
    text_layer_destroy(buy_label);
    text_layer_destroy(buy_display);
    text_layer_destroy(sell_label);
    text_layer_destroy(sell_display);
    text_layer_destroy(volume_label);
    text_layer_destroy(volume_display);
    scroll_layer_destroy(scroll_layer);
}

static void window_appear(Window *window) {

    text_layer_set_text(exchange_name_display, exchange_data->exchange_name);

    app_log(APP_LOG_LEVEL_DEBUG, "exchange_detail.c", 56, "Low value is: %ld.", exchange_data->low);

    format_as_dollars(low, exchange_data->low);
    format_as_dollars(high, exchange_data->high);
    format_as_dollars(last, exchange_data->last);
    format_as_dollars(average, exchange_data->average);
    format_as_dollars(buy, exchange_data->buy);
    format_as_dollars(sell, exchange_data->sell);
    format_as_bitcoin(volume, exchange_data->volume);
}

static void window_disappear(Window *window) {

}

void exchange_detail_init(void) {
    window = window_create();
    window_set_click_config_provider(window, click_config_provider);
    window_set_window_handlers(window, (WindowHandlers) {
        .load = window_load,
        .unload = window_unload,
        .appear = window_appear,
        .disappear = window_disappear,
    });
}

void exchange_detail_deinit(void) {
    window_destroy(window);
}

void exchange_detail_show(ExchangeData *selected_exchange_data) {
    exchange_data = selected_exchange_data;

    window_stack_push(window, true);
}

