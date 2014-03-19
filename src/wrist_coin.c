#include <pebble.h>
#include "exchange.h"
#include "exchange_detail.h"
#include "errors.h"

static Window *window;
static MenuLayer *exchange_menu;

/* A list of keys used to exchange messages between the watch app and the 
   accompanying JavaScript loaded into the Pebble smartphone app.

   This list should mirror the list of appkeys found in the appinfo.json file.
*/
enum {
    WC_KEY_FETCH = 0,
    WC_KEY_EXCHANGE = 1,
    WC_KEY_ERROR = 2,
    WC_KEY_ERROR_MESSAGE = 3,
    WC_KEY_LOW = 100,
    WC_KEY_HIGH = 101,
    WC_KEY_LAST = 102,
    WC_KEY_AVERAGE = 103,
    WC_KEY_BUY = 104,
    WC_KEY_SELL = 105,
    WC_KEY_VOLUME = 106,
    WC_KEY_BITSTAMP = 200,
    WC_KEY_MTGOX = 201,
    WC_KEY_BTCE = 202,
};

/* This typedef is here due to a consideration to create a status field to
   decide what the menu displays instead of using whatever value is in the
   ExchangeData.last field.
*/
typedef struct {
    ExchangeData *exchange_data;
    int32_t status;
    int32_t error;
} ExchangeDataList;

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

/* Sets the price fields to "Loading..." so the displayed information read
   "Loading..." while exchange information is being fetched.
*/
static void set_status_to_loading(void) {
    for (int i = 0; i < NUMBER_OF_EXCHANGES; i++) {
        exchange_data_list[i].last = -1;
    }

    menu_layer_reload_data(exchange_menu);
}

/* Sets the price field for a selected exchange to "Error..." This is used to
   indicate an error occurring when trying to fetch data from an exchange.
*/
static void set_status_to_error(int index) {
    exchange_data_list[index].last = -2;

    menu_layer_reload_data(exchange_menu);
}

/* Asks the JavaScript code loaded on the smartphone's Pebble app to fetch
   prices from Bitcoin exchanges.
*/
static void fetch_message(void) {
    Tuplet fetch = TupletInteger(WC_KEY_FETCH, 1);

    DictionaryIterator *iter;
    app_message_outbox_begin(&iter);

    if (iter == NULL) {
        return;
    }

    dict_write_tuplet(iter, &fetch);
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
    app_log(APP_LOG_LEVEL_DEBUG, "wrist_coin.c", 114, "ERROR: error %d occurred while trying to send data to the phone.\n", reason);
}

static void select_callback(MenuLayer *menu_layer, MenuIndex *cell_index, void *data) {
    ExchangeData *selected;
    const int32_t last = 0;
    const int index = cell_index->row;
    
    if ((selected = get_data_for_exchange(index)) == NULL) {
        return;
    }

    /* If the status is showing "Loading...", "Error...", or any other status
       there's no reason to display the extended data window.
    */
    if (selected->last >= 0) {
        exchange_detail_show(selected);
    }
}

/* Sets the status for each exchange display to "Loading..." and asks the 
   JavaScript code to fetch prices from the exchanges again. 
 */
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

/* Reads the data for each exchange and prints the appropriate output to the 
   screen. Generally this value will be a Bitcoin price but messages such as
   "Loading..." and "Error..." can be dispalyed.
 */
static void draw_row_callback(GContext* ctx, Layer *cell_layer, MenuIndex *cell_index, void *data) {
    ExchangeData *exchange_data;
    const int index = cell_index->row;
    char last[PRICE_FIELD_LENGTH];

    if ((exchange_data = get_data_for_exchange(index)) == NULL) {
        return;
    }

    /* There are times when I want to display status messages on the menu. To
       accomplish this negative numbers are treated as status messages and 
       positive numbers are treated as prices to be displayed. The numbers and
       their status messages are:

       -1 = Loading...
       -2 = Error...
    */ 
    switch(exchange_data->last) {
    case -1:
        snprintf(last, PRICE_FIELD_LENGTH, "Loading...");
        break;
    case -2:
        snprintf(last, PRICE_FIELD_LENGTH, "Error...");
        break;
    default:
        format_as_dollars(last, exchange_data->last);
    }

    menu_cell_basic_draw(ctx, cell_layer, exchange_data->exchange_name, last, NULL);
}

static void in_received_handler(DictionaryIterator *received, void *context) {
    Tuple *exchange = dict_find(received, WC_KEY_EXCHANGE);
    Tuple *error = dict_find(received, WC_KEY_ERROR);
    Tuple *low = dict_find(received, WC_KEY_LOW);
    Tuple *high = dict_find(received, WC_KEY_HIGH);
    Tuple *last = dict_find(received, WC_KEY_LAST);
    Tuple *average = dict_find(received, WC_KEY_AVERAGE);
    Tuple *buy = dict_find(received, WC_KEY_BUY);
    Tuple *sell = dict_find(received, WC_KEY_SELL);
    Tuple *volume = dict_find(received, WC_KEY_VOLUME);
    int index = 0;

    if (exchange) {
        index = exchange->value->int32;
    
        if (error) {
            app_log(APP_LOG_LEVEL_DEBUG, "wrist_coin.c", 198, "Received an error from the phone.");
            set_status_to_error(index);
        } else {
            if (low) {
                exchange_data_list[index].low = low->value->int32;
            }

            if (high) {
                exchange_data_list[index].high = high->value->int32;
            }

            if (last) {
                exchange_data_list[index].last = last->value->int32;
            }

            if (average) {
                exchange_data_list[index].average = average->value->int32;
            }

            if (buy) {
                exchange_data_list[index].buy = buy->value->int32;
            }

            if (sell) {
                exchange_data_list[index].sell = sell->value->int32;
            }

           if (volume) {
               // Volume can often exceed the maximum size of a 32-bit
               // integer. Since a 32-bit integer is the largest the PebbleKit
               // JavaScript can send the volume value must be converted into 
               // a byte array. To undo this the byte array must be "unpacked"
               // which I'm doing here by shifting each byte into the proper
               // position in an int64_t variable. 
               int64_t temp = 0;
               for (unsigned int i = 0; i < volume->length; ++i) {
                   temp = volume->value->data[i];
                   temp <<= (8 * (volume->length - 1 - i));
                   exchange_data_list[index].volume |= temp;
               }

                app_log(APP_LOG_LEVEL_DEBUG, "wrist_coin.c", 247, "Volume for %d is %lld.", index, exchange_data_list[index].volume);
           } else {
               app_log(APP_LOG_LEVEL_DEBUG, "wrist_coin.c", 238, "Didn't receive a volume for exchange %d.", index);
           }
        }
    } else {
        app_log(APP_LOG_LEVEL_DEBUG, "wrist_coin.c", 193, "Didn't receive exchange.");
    }

    menu_layer_reload_data(exchange_menu);
}

static void in_dropped_handler(AppMessageResult reason, void *context) {
    app_log(APP_LOG_LEVEL_DEBUG, "wrist_coin.c", 195, "ERROR: error %d occurred while trying to receive data from the phone.\n", reason);
//    psleep(500);
//    fetch_failed_messages();
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

    set_status_to_loading();

    fetch_message();
}

static void window_unload(Window *window) {
    menu_layer_destroy(exchange_menu);
}

// Register any app message handlers.
static void app_message_init(void) {
    app_message_register_outbox_sent(out_sent_handler);
    app_message_register_outbox_failed(out_failed_handler);
    app_message_register_inbox_received(in_received_handler);
    app_message_register_inbox_dropped(in_dropped_handler);

    app_message_open(APP_MESSAGE_OUTBOX_SIZE_MINIMUM, APP_MESSAGE_INBOX_SIZE_MINIMUM);
}

static void init(void) {
    window = window_create();
    app_message_init();
    exchange_detail_init();
    window_set_click_config_provider(window, click_config_provider);
    window_set_window_handlers(window, (WindowHandlers) {
        .load = window_load,
        .unload = window_unload,
    });
    const bool animated = true;
    window_stack_push(window, animated);
}

static void deinit(void) {
    exchange_detail_deinit();
    window_destroy(window);
}

int main(void) {
  init();

  APP_LOG(APP_LOG_LEVEL_DEBUG, "Done initializing, pushed window: %p", window);

  app_event_loop();
  deinit();
}
