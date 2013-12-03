#include <pebble.h>

/* #define statements used throughout this file for convenience. 
*/
#define EXCHANGE_NAME_LENGTH (10) // Length, in bytes, an exchange's name can be.
#define PRICE_FIELD_LENGTH (10) // Length, in bytes, a price field can be.

#define NUMBER_OF_EXCHANGES (1) // The number of exchanges. Increment when an exchange is added.
#define BITSTAMP_INDEX (0) // Index in the exchage_data_list array that contains Bitstamp data.

static Window *window;
static MenuLayer *exchange_menu;

/* A list of keys used to exchange messages between the watch app and the 
   accompanying JavaScript loaded into the Pebble smartphone app.

   This list should mirror the list of appkeys found in the appinfo.json file.
*/
enum {
    WRIST_COIN_KEY_FETCH = 0x0, // NOT CURRENTLY USED. MAY BE USED IN THE FUTURE.
    WRIST_COIN_KEY_BITSTAMP = 100, // Informs the JavaScript code to fetch prices from Bitstamp.
    WRIST_COIN_KEY_BITSTAMP_HIGH = 101, // Used by the JavaScript code to return Bitstamp's high price.
    WRIST_COIN_KEY_BITSTAMP_LOW = 102, // Used by the JavaScript code to return Bitstamp's low price.
    WRIST_COIN_KEY_BITSTAMP_LAST = 103, // Used by the JavaScript code to return Bitstamp's last price.
};

/* A structure to contain an exchange's information. Each exchange should have
   one struct whose index is based off of the above #define statements.
*/
typedef struct {
    char exchange_name[EXCHANGE_NAME_LENGTH];
    char high[PRICE_FIELD_LENGTH];
    char low[PRICE_FIELD_LENGTH];
    char last[PRICE_FIELD_LENGTH];
} ExchangeData;

static ExchangeData exchange_data_list[NUMBER_OF_EXCHANGES];

/* Returns the ExchangeData for the exchange at index. For a good time use the 
   exchange related #defines at the beginning of this file when calling this 
   function.
*/
static ExchangeData* get_data_for_exchange(int index) {
    if (index < 0 || index >= NUMBER_OF_EXCHANGES) {
        return NULL;
    }

    return &exchange_data_list[index];
}

/* Asks the JavaScript code loaded on the smartphone's Pebble app to fetch
   prices from Bitcoin exchanges.
*/
static void fetch_message(void) {
    Tuplet bitstamp_tuple = TupletInteger(WRIST_COIN_KEY_BITSTAMP, 1);

    DictionaryIterator *iter;
    app_message_outbox_begin(&iter);

    if (iter == NULL) {
      return;
    }

    dict_write_tuplet(iter, &bitstamp_tuple);
    dict_write_end(iter);

    app_message_outbox_send();
}

static void select_click_handler(ClickRecognizerRef recognizer, void *context) {
//    text_layer_set_text(bitstamp_title_text, "Loading...");
// Handled by the select_callback function now.
//    fetch_message();
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

static void select_callback(MenuLayer *menu_layer, MenuIndex *cell_index, void *data) {
    fetch_message();
    
}

static void select_long_callback(MenuLayer *menu_layer, MenuIndex *cell_index, void *data) {
    fetch_message();
}

static int16_t get_cell_height_callback(struct MenuLayer *menu_layer, MenuIndex *cell_index, void *data) {
    return 44;
}

static uint16_t get_num_rows_callback(struct MenuLayer *menu_layer, uint16_t section_index, void *data) {
    return NUMBER_OF_EXCHANGES;
}

static void draw_row_callback(GContext* ctx, Layer *cell_layer, MenuIndex *cell_index, void *data) {
    ExchangeData *exchange_data;
    const int index = cell_index->row;

    if ((exchange_data = get_data_for_exchange(index)) == NULL) {
        return;
    }

    menu_cell_basic_draw(ctx, cell_layer, exchange_data->exchange_name, exchange_data->last, NULL);
}

static void in_received_handler(DictionaryIterator *received, void *context) {
    Tuple *bitstamp_exchange = dict_find(received, WRIST_COIN_KEY_BITSTAMP);

    // Load the prices for Bitstamp into exchange_data_list.
    if (bitstamp_exchange) {
        Tuple *high = dict_find(received, WRIST_COIN_KEY_BITSTAMP_HIGH); 
        Tuple *low = dict_find(received, WRIST_COIN_KEY_BITSTAMP_LOW);
        Tuple *last = dict_find(received, WRIST_COIN_KEY_BITSTAMP_LAST);

        if (high) {
            strncpy(exchange_data_list[BITSTAMP_INDEX].high, high->value->cstring, PRICE_FIELD_LENGTH);
        } 
        if (low) {
            strncpy(exchange_data_list[BITSTAMP_INDEX].low, low->value->cstring, PRICE_FIELD_LENGTH);
        }
        if (last) {
            strncpy(exchange_data_list[BITSTAMP_INDEX].last, last->value->cstring, PRICE_FIELD_LENGTH);
        }
    }

    menu_layer_reload_data(exchange_menu);
}

static void in_dropped_handler(AppMessageResult reason, void *context) {

}

static void click_config_provider(void *context) {
//  window_single_click_subscribe(BUTTON_ID_SELECT, select_click_handler);
//  window_single_click_subscribe(BUTTON_ID_UP, up_click_handler);
//  window_single_click_subscribe(BUTTON_ID_DOWN, down_click_handler);
}

static void window_load(Window *window) {
    Layer *window_layer = window_get_root_layer(window);
    GRect bounds = layer_get_bounds(window_layer);

    strncpy(exchange_data_list[BITSTAMP_INDEX].exchange_name, "Bitstamp\0", EXCHANGE_NAME_LENGTH);
    strncpy(exchange_data_list[BITSTAMP_INDEX].high, "$0.00\0", PRICE_FIELD_LENGTH);
    strncpy(exchange_data_list[BITSTAMP_INDEX].low, "$0.00\0", PRICE_FIELD_LENGTH);
    strncpy(exchange_data_list[BITSTAMP_INDEX].last, "$0.00\0", PRICE_FIELD_LENGTH);

    exchange_menu = menu_layer_create(bounds);
    menu_layer_set_callbacks(exchange_menu, NULL, (MenuLayerCallbacks) {
        .get_cell_height = (MenuLayerGetCellHeightCallback) get_cell_height_callback,
        .draw_row = (MenuLayerDrawRowCallback) draw_row_callback,
        .get_num_rows = (MenuLayerGetNumberOfRowsInSectionsCallback) get_num_rows_callback,
        .select_click = (MenuLayerSelectCallback) select_callback,
        .select_long_click = (MenuLayerSelectCallback) select_long_callback
    });
    menu_layer_set_click_config_onto_window(exchange_menu, window);
    layer_add_child(window_layer, menu_layer_get_layer(exchange_menu));

    fetch_message();
}

static void window_unload(Window *window) {
//  text_layer_destroy(text_layer);
//    text_layer_destroy(bitstamp_title_text);
//    text_layer_destroy(bitstamp_high_text);
//    text_layer_destroy(bitstamp_low_text);
//    text_layer_destroy(bitstamp_last_text);
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
    menu_layer_destroy(exchange_menu);
}

int main(void) {
  init();

  APP_LOG(APP_LOG_LEVEL_DEBUG, "Done initializing, pushed window: %p", window);

  app_event_loop();
  deinit();
}
