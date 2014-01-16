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

/* Initializes a text layer, sets values for that text layer, and adds it as a
   child to the main scroll layer.

   layer - The text layer to initialize and add to the scroll layer.

   font - The font key for the font to use.

   text - The text for the label to display.

   x - The x coordinate to draw the layer at.

   y - The y coordinate to draw the layer at.
 */
static void init_text_layer_for_scroll(TextLayer *layer, const char *font, const char *text, int x, int y) {
    Layer *window_layer = window_get_root_layer(window);
    GRect bounds = layer_get_bounds(window_layer);

    layer = text_layer_create((GRect) { .origin = { x, y }, .size = { bounds.size.w, 22 } });
    text_layer_set_font(layer, fonts_get_system_font(font));
    text_layer_set_text(layer, text);
    scroll_layer_add_child(scroll_layer, text_layer_get_layer(layer));
}

static void window_load(Window *window) {
    Layer *window_layer = window_get_root_layer(window);
    GRect bounds = layer_get_bounds(window_layer);
    int half_screen = (bounds.size.w / 2);

    scroll_layer = scroll_layer_create((GRect) { .origin = { 0, 32 }, .size = { bounds.size.w, bounds.size.h - 32 } });
    scroll_layer_set_content_size(scroll_layer, GSize(bounds.size.w, 308));
    layer_add_child(window_layer, scroll_layer_get_layer(scroll_layer));

    exchange_name_display = text_layer_create((GRect) { .origin = { 0, 0 }, .size = { bounds.size.w, 32 } });
    text_layer_set_text_alignment(exchange_name_display, GTextAlignmentCenter);
    text_layer_set_font(exchange_name_display, fonts_get_system_font(FONT_KEY_GOTHIC_28_BOLD));
    layer_add_child(window_layer, text_layer_get_layer(exchange_name_display));

    // Label and text for the low value.
    init_text_layer_for_scroll(low_label, FONT_KEY_GOTHIC_18_BOLD, low_text, 0, 0);
    init_text_layer_for_scroll(low_display, FONT_KEY_GOTHIC_18, low, 0, 22);

    // Label and text for the high value.
    init_text_layer_for_scroll(high_label, FONT_KEY_GOTHIC_18_BOLD, high_text, 0, 44);
    init_text_layer_for_scroll(high_display, FONT_KEY_GOTHIC_18, high, 0, 66);

    // Label and text for the last value.
    init_text_layer_for_scroll(last_label, FONT_KEY_GOTHIC_18_BOLD, last_text, 0, 88);
    init_text_layer_for_scroll(last_display, FONT_KEY_GOTHIC_18, last, 0, 110);

    // Label and text for the average value.
    init_text_layer_for_scroll(average_label, FONT_KEY_GOTHIC_18_BOLD, average_text, 0, 132);
    init_text_layer_for_scroll(average_display, FONT_KEY_GOTHIC_18, average, 0, 154);

    // Label and text for the buy value.
    init_text_layer_for_scroll(buy_label, FONT_KEY_GOTHIC_18_BOLD, buy_text, 0, 176);
    init_text_layer_for_scroll(buy_display, FONT_KEY_GOTHIC_18, buy, 0, 198);

    // Label and text for the sell value.
    init_text_layer_for_scroll(sell_label, FONT_KEY_GOTHIC_18_BOLD, sell_text, 0, 220);
    init_text_layer_for_scroll(sell_display, FONT_KEY_GOTHIC_18, sell, 0, 242);

    // Label and text for the volume.
    init_text_layer_for_scroll(volume_label, FONT_KEY_GOTHIC_18_BOLD, volume_text, 0, 264);
    init_text_layer_for_scroll(volume_display, FONT_KEY_GOTHIC_18, volume, 0, 286);

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

