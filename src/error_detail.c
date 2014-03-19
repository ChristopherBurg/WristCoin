#include <error_detail.h>

static Window *window;

static TextLayer *error_explanation;

ErrorMessages error;

static void click_config_provider(void *context) {

}

static void window_load(Window *window) {
    Layer *window_layer = window_get_root_layer(window);
    GRect bounds = layer_get_bounds(window_layer);

    error_explanation = text_layer_create((GRect) { .origin = { 0, 0 }, .size = { bounds.size.w, bounds.size.h } });
    text_layer_set_font(error_explanation, fonts_get_system_font(FONT_KEY_GOTHIC_18));
    text_layer_set_text_alignment(error_explanation, GTextAlignmentCenter);
    text_layer_set_overflow_mode(error_explanation, GTextOverflowModeWordWrap);

    layer_add_child(window_layer, text_layer_get_layer(error_explanation));
}

static void window_unload(Window *window) {
    text_layer_destroy(error_explanation);
}

static void window_appear(Window *window) {

}

static void window_disappear(Window *window) {

}

void error_detail_init(void) {
    window = window_create();
    window_set_click_config_provider(window, click_config_provider);
    window_set_window_handlers(window, (WindowHandlers) {
        .load = window_load,
        .unload = window_unload,
        .appear = window_appear,
        .disappear = window_disappear,
    });
}

void error_detail_deinit(void) {
    window_destroy(window);
}

void error_detail_show(ErrorMessages selected_error) {
    error = selected_error;

    window_stack_push(window, true);
}

