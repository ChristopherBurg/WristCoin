#include "exchange_detail.h"

static Window *window;
static TextLayer *last_text_layer;

static ExchangeData *exchange_data;

static void click_config_provider(void *context) {

}

static void window_load(Window *window) {
    Layer *window_layer = window_get_root_layer(window);
    GRect bounds = layer_get_bounds(window_layer);

    last_text_layer = text_layer_create((GRect) { .origin = { 0, 20 }, .size = { bounds.size.w, 50 } });

    text_layer_set_text(last_text_layer, exchange_data->exchange_name); 

    text_layer_set_text_alignment(last_text_layer, GTextAlignmentCenter); 
    text_layer_set_font(last_text_layer, fonts_get_system_font(FONT_KEY_BITHAM_30_BLACK));
    layer_add_child(window_layer, text_layer_get_layer(last_text_layer));
}

static void window_unload(Window *window) {
    text_layer_destroy(last_text_layer);
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

