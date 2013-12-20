#include "exchange_detail.h"

static Window *window;
static TextLayer *exchange_name_text;
static TextLayer *last_text;

static ExchangeData *exchange_data;

static void click_config_provider(void *context) {

}

static void window_load(Window *window) {
    Layer *window_layer = window_get_root_layer(window);
    GRect bounds = layer_get_bounds(window_layer);

    exchange_name_text = text_layer_create((GRect) { .origin = {0, 10}, .size = {bounds.size.w, 40 } });
    text_layer_set_text(exchange_name_text, exchange_data->exchange_name);
    text_layer_set_text_alignment(exchange_name_text, GTextAlignmentCenter);
    text_layer_set_font(exchange_name_text, fonts_get_system_font(FONT_KEY_GOTHIC_28_BOLD));
    layer_add_child(window_layer, text_layer_get_layer(exchange_name_text));

    last_text = text_layer_create((GRect) { .origin = { 0, 70 }, .size = { bounds.size.w, 20 } });
    text_layer_set_text(last_text, exchange_data->exchange_name); 
    text_layer_set_text_alignment(last_text, GTextAlignmentCenter); 
//    text_layer_set_font(last_text, fonts_get_system_font(FONT_KEY_BITHAM_30_BLACK));
    layer_add_child(window_layer, text_layer_get_layer(last_text));
}

static void window_unload(Window *window) {
    text_layer_destroy(exchange_name_text);
    text_layer_destroy(last_text);
}

void exchange_detail_init(void) {
    window = window_create();
    window_set_click_config_provider(window, click_config_provider);
    window_set_window_handlers(window, (WindowHandlers) {
        .load = window_load,
        .unload = window_unload,
    });
}

void exchange_detail_deinit(void) {
    window_destroy(window);
}

void exchange_detail_show(ExchangeData *selected_exchange_data) {
    exchange_data = selected_exchange_data;
    exchange_detail_init();
    window_stack_push(window, true);
}

