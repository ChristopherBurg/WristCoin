#include <pebble.h>

/* #define statements used throughout this file for convenience. 
*/
#define EXCHANGE_NAME_LENGTH (10) // Length, in bytes, an exchange's name can be.
#define PRICE_FIELD_LENGTH (15) // Length, in bytes, a price field can be.

#define NUMBER_OF_EXCHANGES (3) // The number of exchanges. Increment when an exchange is added.
#define BITSTAMP_INDEX (0) // Index in the exchage_data_list array that contains Bitstamp data.
#define MTGOX_INDEX (1) // Index in the exchange_data_list array that contians Mt. Gox data.
#define BTCE_INDEX (2) // Index in the exchange_data_list array that contains BTC-e data.

static Window *window;
static MenuLayer *exchange_menu;

/* A list of keys used to exchange messages between the watch app and the 
   accompanying JavaScript loaded into the Pebble smartphone app.

   This list should mirror the list of appkeys found in the appinfo.json file.
*/
enum {
    WC_KEY_FETCH = 0,
    WC_KEY_ERROR = 1,
    WC_KEY_ERROR_MESSAGE = 2,
    WC_KEY_LOW = 100,
    WC_KEY_HIGH = 101,
    WC_KEY_LAST = 102,
    WC_KEY_BITSTAMP = 200,
    WC_KEY_MTGOX = 201,
    WC_KEY_BTCE = 202,
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
    Tuplet bitstamp_tuplet = TupletInteger(WC_KEY_BITSTAMP, 1);
    Tuplet mtgox_tuplet = TupletInteger(WC_KEY_MTGOX, 1);
    Tuplet btce_tuplet = TupletInteger(WC_KEY_BTCE, 1);

    DictionaryIterator *iter;
    app_message_outbox_begin(&iter);

    if (iter == NULL) {
      return;
    }

    dict_write_tuplet(iter, &bitstamp_tuplet);
    dict_write_tuplet(iter, &mtgox_tuplet);
    dict_write_tuplet(iter, &btce_tuplet);
    dict_write_end(iter);

    app_message_outbox_send();
}

/* Sets the price fields to "Loading..." so the displayed information read
   "Loading..." while exchange information is being fetched.
*/
static void set_status_to_loading(void) {
    for (int i = 0; i < NUMBER_OF_EXCHANGES; i++) {
        strncpy(exchange_data_list[i].last, "Loading...\0", PRICE_FIELD_LENGTH);
    }
}

/* Sets the price field for a selected exchange to "Error..." This is used to
   indicate an error occurring when trying to fetch data from an exchange.
*/
static void set_status_to_error(int index) {
    strncpy(exchange_data_list[index].last, "Error...\0", PRICE_FIELD_LENGTH);
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
    set_status_to_loading();
    fetch_message(); 
}

static void select_long_callback(MenuLayer *menu_layer, MenuIndex *cell_index, void *data) {
    set_status_to_loading();
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

    app_log(APP_LOG_LEVEL_DEBUG, "wrist_coin.c", 140, "Row number is %d.\n", index);

    if ((exchange_data = get_data_for_exchange(index)) == NULL) {
        app_log(APP_LOG_LEVEL_DEBUG, "wrist_coin.c", 143, "Index of %d did not return any data.\n", index);
        return;
    }

    app_log(APP_LOG_LEVEL_DEBUG, "wrist_coin.c", 146, "Exchange name being refreshed is %s.\n", exchange_data->exchange_name);

    menu_cell_basic_draw(ctx, cell_layer, exchange_data->exchange_name, exchange_data->last, NULL);
}

static void in_received_handler(DictionaryIterator *received, void *context) {
    Tuple *bitstamp_exchange = dict_find(received, WC_KEY_BITSTAMP);
    Tuple *mtgox_exchange = dict_find(received, WC_KEY_MTGOX);
    Tuple *btce_exchange = dict_find(received, WC_KEY_BTCE);

    int index = 0;

    if (bitstamp_exchange) {
        app_log(APP_LOG_LEVEL_DEBUG, "wrist_coin.c", 157, "Received message for Bitstamp.\n");
        index = BITSTAMP_INDEX;
    } else if (mtgox_exchange) {
        app_log(APP_LOG_LEVEL_DEBUG, "wrist_coin.c", 157, "Received message for Mt. Gox.\n");
        index = MTGOX_INDEX;
    } else if (btce_exchange) {
        app_log(APP_LOG_LEVEL_DEBUG, "wrist_coin.c", 157, "Received message for BTC-e.\n");
        index = BTCE_INDEX;
    } else {
        return;
    }

    app_log(APP_LOG_LEVEL_DEBUG, "wrist_coin.c", 164, "Index is %d\n", index);

    Tuple *error = dict_find(received, WC_KEY_ERROR); 

    if (error) {
        app_log(APP_LOG_LEVEL_DEBUG, "wrist_coin.c", 171, "An error occurred.\n");
        set_status_to_error(index);
    } else {
        Tuple *low = dict_find(received, WC_KEY_LOW);
        Tuple *high = dict_find(received, WC_KEY_HIGH);
        Tuple *last = dict_find(received, WC_KEY_LAST);

        app_log(APP_LOG_LEVEL_DEBUG, "wrist_coin.c", 177, "Low is %s.\n", low->value->cstring);

        strncpy(exchange_data_list[index].low, low->value->cstring, PRICE_FIELD_LENGTH);
        strncpy(exchange_data_list[index].high, high->value->cstring, PRICE_FIELD_LENGTH);
        strncpy(exchange_data_list[index].last, last->value->cstring, PRICE_FIELD_LENGTH);
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
    strncpy(exchange_data_list[MTGOX_INDEX].exchange_name, "Mt. Gox\0", EXCHANGE_NAME_LENGTH);
    strncpy(exchange_data_list[BTCE_INDEX].exchange_name, "BTC-e\0", EXCHANGE_NAME_LENGTH);

    for(int i = 0; i < NUMBER_OF_EXCHANGES; i++) {
        strncpy(exchange_data_list[i].high, "$0.00\0", PRICE_FIELD_LENGTH);
        strncpy(exchange_data_list[i].low, "$0.00\0", PRICE_FIELD_LENGTH);
        strncpy(exchange_data_list[i].last, "$0.00\0", PRICE_FIELD_LENGTH);
    }

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
