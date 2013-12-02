#include <pebble.h>

static Window *window;
//static TextLayer *text_layer;
static TextLayer *bitstamp_title_text;
static TextLayer *bitstamp_high_text;
static TextLayer *bitstamp_low_text;
static TextLayer *bitstamp_last_text;

static char bitstamp_high_price[10];
static char bitstamp_low_price[10];
static char bitstamp_last_price[10];

enum {
    WRIST_COIN_KEY_FETCH = 0x0,
    WRIST_COIN_KEY_BITSTAMP = 100,
    WRIST_COIN_KEY_BITSTAMP_HIGH = 101,
    WRIST_COIN_KEY_BITSTAMP_LOW = 102,
    WRIST_COIN_KEY_BITSTAMP_LAST = 103,
};

static void fetch_message(void) {
//    Tuplet fetch_tuple = TupletInteger(WRIST_COIN_KEY_FETCH, 1);
    Tuplet bitstamp_tuple = TupletInteger(WRIST_COIN_KEY_BITSTAMP, 1);

    DictionaryIterator *iter;
    app_message_outbox_begin(&iter);

    if (iter == NULL) {
      return;
    }

//    dict_write_tuplet(iter, &fetch_tuple);
    dict_write_tuplet(iter, &bitstamp_tuple);
    dict_write_end(iter);

    app_message_outbox_send();
}

static void select_click_handler(ClickRecognizerRef recognizer, void *context) {
    text_layer_set_text(bitstamp_title_text, "Loading...");
    fetch_message();
}

static void up_click_handler(ClickRecognizerRef recognizer, void *context) {
//  text_layer_set_text(text_layer, "Up");
}

static void down_click_handler(ClickRecognizerRef recognizer, void *context) {
//  text_layer_set_text(text_layer, "Down");
}

static void out_sent_handler(DictionaryIterator *sent, void *context) {

}

static void out_failed_handler(DictionaryIterator *failed, AppMessageResult reason, void *context) {

}

static void in_received_handler(DictionaryIterator *received, void *context) {
    Tuple *bitstamp_high_tuple = dict_find(received, WRIST_COIN_KEY_BITSTAMP_HIGH);
    Tuple *bitstamp_low_tuple = dict_find(received, WRIST_COIN_KEY_BITSTAMP_LOW);
    Tuple *bitstamp_last_tuple = dict_find(received, WRIST_COIN_KEY_BITSTAMP_LAST);

    if (bitstamp_high_tuple) {
        strncpy(bitstamp_high_price, bitstamp_high_tuple->value->cstring, 10);
        text_layer_set_text(bitstamp_high_text, bitstamp_high_price);
    } 
    if (bitstamp_low_tuple) {
        strncpy(bitstamp_low_price, bitstamp_low_tuple->value->cstring, 10);
        text_layer_set_text(bitstamp_low_text, bitstamp_low_price);
    }
    if (bitstamp_last_tuple) {
        strncpy(bitstamp_last_price, bitstamp_last_tuple->value->cstring, 10);
        text_layer_set_text(bitstamp_last_text, bitstamp_last_price);
    }

    text_layer_set_text(bitstamp_title_text, "Bitstamp");
}

static void in_dropped_handler(AppMessageResult reason, void *context) {

}

static void click_config_provider(void *context) {
  window_single_click_subscribe(BUTTON_ID_SELECT, select_click_handler);
  window_single_click_subscribe(BUTTON_ID_UP, up_click_handler);
  window_single_click_subscribe(BUTTON_ID_DOWN, down_click_handler);
}

static void window_load(Window *window) {
    Layer *window_layer = window_get_root_layer(window);
    GRect bounds = layer_get_bounds(window_layer);

//  text_layer = text_layer_create((GRect) { .origin = { 0, 72 }, .size = { bounds.size.w, 20 } });
//  text_layer_set_text(text_layer, "Welcome to Wrist Coin.");
//  text_layer_set_text_alignment(text_layer, GTextAlignmentCenter);
//  layer_add_child(window_layer, text_layer_get_layer(text_layer));

    bitstamp_title_text = text_layer_create((GRect) { .origin = { 0, 10 }, .size = { bounds.size.w, 20 } });
    text_layer_set_text(bitstamp_title_text, "Bitstamp");
    text_layer_set_text_alignment(bitstamp_title_text, GTextAlignmentCenter);
    layer_add_child(window_layer, text_layer_get_layer(bitstamp_title_text));

    bitstamp_high_text = text_layer_create((GRect) { .origin = { 0, 30 }, .size = { bounds.size.w, 20 } });
    text_layer_set_text(bitstamp_high_text, "High");
    text_layer_set_text_alignment(bitstamp_high_text, GTextAlignmentCenter);
    layer_add_child(window_layer, text_layer_get_layer(bitstamp_high_text));

    bitstamp_low_text = text_layer_create((GRect) { .origin = { 0, 50 }, .size = { bounds.size.w, 20 } });
    text_layer_set_text(bitstamp_low_text, "Low");
    text_layer_set_text_alignment(bitstamp_low_text, GTextAlignmentCenter);     
    layer_add_child(window_layer, text_layer_get_layer(bitstamp_low_text));

    bitstamp_last_text = text_layer_create((GRect) { .origin = { 0, 70 }, .size = { bounds.size.w, 20 } });
    text_layer_set_text(bitstamp_high_text, "Last");
    text_layer_set_text_alignment(bitstamp_last_text, GTextAlignmentCenter);
    layer_add_child(window_layer, text_layer_get_layer(bitstamp_last_text));

    fetch_message();
}

static void window_unload(Window *window) {
//  text_layer_destroy(text_layer);
    text_layer_destroy(bitstamp_title_text);
    text_layer_destroy(bitstamp_high_text);
    text_layer_destroy(bitstamp_low_text);
    text_layer_destroy(bitstamp_last_text);
}

// Register any app message handlers.
static void app_message_init(void) {
    app_message_register_outbox_sent(out_sent_handler);
    app_message_register_outbox_failed(out_failed_handler);
    app_message_register_inbox_received(in_received_handler);
    app_message_register_inbox_dropped(in_dropped_handler);

    app_message_open(64, 64);
}

static void init(void) {
  window = window_create();
  app_message_init();
  window_set_click_config_provider(window, click_config_provider);
  window_set_window_handlers(window, (WindowHandlers) {
    .load = window_load,
    .unload = window_unload,
  });
  const bool animated = true;
  window_stack_push(window, animated);
}

static void deinit(void) {
  window_destroy(window);
}

int main(void) {
  init();

  APP_LOG(APP_LOG_LEVEL_DEBUG, "Done initializing, pushed window: %p", window);

  app_event_loop();
  deinit();
}
