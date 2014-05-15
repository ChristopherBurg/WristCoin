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
  WC_KEY_COMMAND = 0,
  WC_KEY_CONFIG = 1,
  WC_KEY_NUM_EX = 2,
  WC_KEY_EX_INDEX = 100,
  WC_KEY_EX_NAME = 101,
  WC_KEY_EX_LOW = 102,
  WC_KEY_EX_HIGH = 103,
  WC_KEY_EX_AVG = 104,
  WC_KEY_EX_LAST = 105,
  WC_KEY_EX_VOL = 106,
};

/* This is a list of constants that don't appear in appinfo.json.
 */
enum {
  WC_KEY_GLOBAL_CONFIG = 0,
  WC_KEY_EX_CONFIG = 1,
};

const char *stat_loading = "Loading...\0";
const char *stat_error = "Error...\0";

/* Struct that stores exchange data.
 */
typedef struct {
  char *ex_name;
  char *ex_status;
  int32_t low;
  int32_t high;
  int32_t avg;
  int32_t last;
  int64_t vol;
} ExData;

/* The number of exchanges that the Pebble app has told this app the user has
 * selected. This number should only be altered by the update_global_config
 * function.
 */
static int32_t num_ex = 0;

/* An array that stores exchange related date. Because the user can initiate a
 * change to this array from the Pebble app no other variables should tied
 * directly to this data. When you need data form this array copy it to another
 * variable because at any point this whole thing could be destroyed.
 */
static ExData *ex_data_list = NULL;

/* Frees all of the data allocated for the ex_data_list. First all of the
 * allocated variables for all of the ExData items are freed then ex_data_list
 * itself is freed. All pointers are then set to NULL. This function should only
 * be called from update_global_config and window_unload.
 */
static void free_ex_data_list(void) {
  if (ex_data_list != NULL) {
    // Iterate through each ExData item and free its allocated variables and set
    // their pointers to NULL.
    for (int i = 0; i < num_ex; i++) {
      if (ex_data_list[i].ex_name != NULL) {
        free(ex_data_list[i].ex_name);
        ex_data_list[i].ex_name = NULL;
      }

      if (ex_data_list[i].ex_status != NULL) {
        free(ex_data_list[i].ex_status);
        ex_data_list[i].ex_status = NULL;
      }
    }

    // Free the ex_data_list itself and set its pointer to NULL.
    free(ex_data_list);
    ex_data_list = NULL;
  }
}

/* Dynamic memory string copy. This function takes a dest and a source then
 * frees dest and sets it to NULL before allocating new memory and copying the
 * contents of source into dest. Obviously this function is destruction as all
 * get out. Whatever is in dest won't be there afer this function finishes.
 *
 * char *dest - The pointer to copy the contents of source to. This variable
 *              will be totally destroyed before the copy.
 *
 * char *source - The contents to copy into dest.
 */
static char * strdyncpy(char *dest, const char *source) {
  if (source != NULL) {
    // We're going to deallocate and reallocate dest. This will ensure the size
    // of dest and source are equal and the data is reliably copied.
    if (dest != NULL) {
      free(dest);
      dest = NULL;
    }

//    app_log(APP_LOG_LEVEL_DEBUG, "wrist_coin.c", 132, "Copying '%s' into dest.", source);
//    app_log(APP_LOG_LEVEL_DEBUG, "wrist_coin.c", 132, "Reallocating memory for dest. Will allocate %d byets.", (strlen(source) + 1));
    dest = (char *) malloc(sizeof(char) * (strlen(source) + 1));

    if (dest != NULL) {
//      app_log(APP_LOG_LEVEL_DEBUG, "wrist_coin.c", 136, "Memory for dest allocated. Copying contents from source into dest.");
      strncpy(dest, source, strlen(source) + 1);
//      app_log(APP_LOG_LEVEL_DEBUG, "wrist_coin.c", 138, "dest now contains '%s'.", dest);
    } else {
      app_log(APP_LOG_LEVEL_DEBUG, "wrist_coin.c", 374, "Something went horribly wrong. ex_data_list[i].ex_status memory wasn't allocated.");
    }

  } else {
    app_log(APP_LOG_LEVEL_DEBUG, "wrist_coin.c", 124, "You can't pass a NULL source value into set_ex_status, dummy.");
  }

  return dest;
}

/* Some price values, namely 24-hour volumes, can exceed the maximum size of a
 * 32-bit integer. Since the largest integer the Pebble JavaScript application
 * can send is 32-bit this creates a problem. To get around this problem large
 * integers are broken up into byte arrays and fed to this function to be
 * converted to a 64-bit integer.
 *
 * Tuple *bytes - A Tuple containing a byte array.
 */
static int64_t convert_bytes_to_int64(Tuple *bytes) {
  int64_t unpacked = 0;
  int64_t temp = 0;

  // Unpack the byte array by shifting each byte into the int64_t value.
  for (unsigned int i = 0; i < bytes->length; ++i) {
    temp = bytes->value->data[i];
    temp <<= (8 * (bytes->length - 1 - i));
    unpacked |= temp;
  }

  return unpacked;
}

static char * format_dollars(char *dest, int32_t price) {
  char *dollars = NULL;
  int32_t digits = 0;
  int32_t characteristic = 0;
  int32_t mantissa = 0;

  // In order to know how much memory to allocate we need to know how many
  // digits long the price is.
  if (price < 100) {
    // The minimum value that can be printed by this function is '$ 0.00'. If
    // the price is less than 100 then three digits should be reserved.
    digits = 3;
  } else {
    int32_t temp = price;
    digits++;

    while (temp >= 10) {
      digits++;
      temp /= 10;
    }
  }

//  app_log(APP_LOG_LEVEL_DEBUG, "wrist_coin.c", 200, "%ld is %ld digits long. Allocating %ld bytes for dollar string.", price, digits, (digits + 4));
  dollars = (char *) malloc(digits + 4);

  characteristic = price / 100;
  mantissa = price - (characteristic * 100);
  snprintf(dollars, digits + 4, "$ %ld.%02ld", characteristic, mantissa);
//  app_log(APP_LOG_LEVEL_DEBUG, "wrist_coin.c", 208, "Formatted dollar string is '%s'.", dollars);

//  app_log(APP_LOG_LEVEL_DEBUG, "wrist_coin.c", 207, "Copying dollar string into dest.");
  dest = strdyncpy(dest, dollars);

//  app_log(APP_LOG_LEVEL_DEBUG, "wrist_coin.c", 209, "Freeing temporary dollar string.");
  free(dollars);
  dollars = NULL;

  return dest;
}

/* The nuclear option. This function wipes out the current configuration and
 * loads a new configuration. It should only be activated from a global
 * configuration initiated by the Pebble app.
 *
 * int exNum - The number of exchanges that the user has selected. This is used
 *             to allocate the appropriate amount of memory for ex_data_list.
 */
static ExData* update_global_config(int32_t new_num_ex) {
  num_ex = new_num_ex;

  app_log(APP_LOG_LEVEL_DEBUG, "wrist_coin.c", 75, "Global configuration change requested. Shit is going down.");
  // If ex_data_list has already been allocated dellocate it now.
  free_ex_data_list();

  app_log(APP_LOG_LEVEL_DEBUG, "wrist_coin.c", 82, "Allocating ex_data_list.");

  // Allocate enough memory to store the configuration of all user selected
  // exchanges.
  ex_data_list = (ExData*) malloc(sizeof(ExData) * num_ex);

  if (ex_data_list == NULL) {
    app_log(APP_LOG_LEVEL_DEBUG, "wrist_coin.c", 89, "Something went horribly wrong. ex_data_list is NULL.");
  } else {
    app_log(APP_LOG_LEVEL_DEBUG, "wrist_coin.c", 91, "ex_data_list allocated. Zeroing values in memory.");

    // Zero out all of the data in the newly allocated array so we don't have
    // a bunch of random shit happen due to unknown memory contents.
    for (int i = 0; i < num_ex; i++) {
      ex_data_list[i].ex_name = NULL;
      ex_data_list[i].ex_status = NULL;
      ex_data_list[i].low = 0;
      ex_data_list[i].high = 0;
      ex_data_list[i].avg = 0;
      ex_data_list[i].last = 0;
    }
  }

  return ex_data_list;
}

/* Returns the ExData record for the exchange at index.
 */
static ExData* get_ex_data(int index) {
  if (index < 0 || index >= num_ex) {
    return NULL;
  }

  return &ex_data_list[index];
}

/* Sets the status field for a selected exchange to "Loading...".
 *
 * int index - The index of the exchange to set the status for.
 */
static void set_stat_to_loading(int index) {
  ex_data_list[index].ex_status = strdyncpy(ex_data_list[index].ex_status, stat_loading);
}

/* Sets the status field for a selected exchange to "Error...".
 *
 * int index - The index of the exchange to set the status for.
 */
static void set_stat_to_error(int index) {
  ex_data_list[index].ex_status = strdyncpy(ex_data_list[index].ex_status, stat_error);
}

/* Fetch the current configuration from the phone application.
 *
 */
static void fetch_global_config(void) {
  Tuplet fetch = TupletInteger(WC_KEY_COMMAND, 0);
  Tuplet config_type = TupletInteger(WC_KEY_CONFIG, WC_KEY_GLOBAL_CONFIG);

  DictionaryIterator *iter;
  app_message_outbox_begin(&iter);

  if (iter == NULL) {
    return;
  }

  dict_write_tuplet(iter, &fetch);
  dict_write_tuplet(iter, &config_type);
  dict_write_end(iter);

  app_message_outbox_send();
}

/* Fetch the current exchange configuration. This should only be called after
 * fetch_global_config has been properly processed.
 */
static void fetch_ex_config(void) {
  Tuplet fetch = TupletInteger(WC_KEY_COMMAND, 0);
  Tuplet config_type = TupletInteger(WC_KEY_CONFIG, WC_KEY_EX_CONFIG);

  DictionaryIterator *iter;
  app_message_outbox_begin(&iter);

  if (iter != NULL) {
    dict_write_tuplet(iter, &fetch);
    dict_write_tuplet(iter, &config_type);
    dict_write_end(iter);
  }

  app_message_outbox_send();

  return;
}

/* Fetch the current price information from the selected exchange.
 *
 * int exchange - The index of the exchange to fetch prices for.
 */
static void fetch_ex_price(int exchange) {
  Tuplet command = TupletInteger(WC_KEY_COMMAND, 1);
  Tuplet index = TupletInteger(WC_KEY_EX_INDEX, exchange);

  app_log(APP_LOG_LEVEL_DEBUG, "wrist_coin.c", 310, "fetch_ex_price: Fetching prices for %s. Sending index value %d.", ex_data_list[exchange].ex_name, exchange);

  DictionaryIterator *iter;
  app_message_outbox_begin(&iter);

  if (iter == NULL) {
    return;
  }

  dict_write_tuplet(iter, &command);
  dict_write_tuplet(iter, &index);
  dict_write_end(iter);

  app_message_outbox_send();
}

static void select_callback(MenuLayer *menu_layer, MenuIndex *cell_index, void *data) {
  ExData *selected;
  //    const int32_t last = 0;
  const int index = cell_index->row;

  if ((selected = get_ex_data(index)) == NULL) {
    return;
  }

  /* If the status is showing "Loading...", "Error...", or any other status
  there's no reason to display the extended data window.
  */
  if (selected->last >= 0) {
//    exchange_detail_show(selected);
  }
}

/* Sets the status for each exchange display to "Loading..." and asks the
JavaScript code to fetch prices from the exchanges again.
*/
static void select_long_callback(MenuLayer *menu_layer, MenuIndex *cell_index, void *data) {
//  set_status_to_loading();
//  fetch_message();
}

static int16_t get_cell_height_callback(struct MenuLayer *menu_layer, MenuIndex *cell_index, void *data) {
  return 44;
}

/* Returns the number of rows in the menu. The number of rows is equal to the
 * number of exchanges the user has selected.
 */
static uint16_t get_num_rows_callback(struct MenuLayer *menu_layer, uint16_t section_index, void *data) {
  return num_ex;
}

/* Redraws a menu item that has changed. Typically this means that the status
 * tied to the row's exchange has changed. This function simply recopies the
 * exchange name and exchange status to the menu row.
 */
static void draw_row_callback(GContext* ctx, Layer *cell_layer, MenuIndex *cell_index, void *data) {
  menu_cell_basic_draw(ctx, cell_layer, ex_data_list[cell_index->row].ex_name, ex_data_list[cell_index->row].ex_status, NULL);
}

/*
* Loads global configuration information sent from the Pebble app.
*
* DictionaryIterator *config - A DictionaryIterator containing the
*                              configuration information.
*/
static void load_global_config(DictionaryIterator *config) {
  Tuple *num_ex = dict_find(config, WC_KEY_NUM_EX);

  app_log(APP_LOG_LEVEL_DEBUG, "wrist_coin.c", 300, "Global configuration changing. User has selected %ld exchanges.", num_ex->value->int32);

  update_global_config(num_ex->value->int32);

  // Global configuration data has been loaded. Set the status for each exchange
  // to "Loading...".
  for (int i = 0; i < num_ex->value->int32; i++) {
    app_log(APP_LOG_LEVEL_DEBUG, "wrist_coin.c", 404, "Copying 'Loading...' into exchange %d.", i);
    set_stat_to_loading(i);
    app_log(APP_LOG_LEVEL_DEBUG, "wrist_coin.c", 411, "After strdyncpy ex_status now contains %p.", ex_data_list[i].ex_status);
  }

  // When the global configuration is updated the exchange_menu is rebuilt. It's
  // a good idea to set the index to zero in this case.
  menu_layer_set_selected_index(exchange_menu, (MenuIndex) {0, 0}, MenuRowAlignNone, false);

  // Now that the global configuration has been changed the configuration for
  // the exchanges need to be updated.
  fetch_ex_config();
}

/* Load exchange configuration information sent from the Pebble app.
 *
 * DictionaryIterator *config - The exchange configuration information.
 */
static void load_ex_config(DictionaryIterator *config) {
  Tuple *ex_index = dict_find(config, WC_KEY_EX_INDEX);
  Tuple *ex_name = dict_find(config, WC_KEY_EX_NAME);

  if (ex_index) {
    int32_t index = ex_index->value->int32;

    app_log(APP_LOG_LEVEL_DEBUG, "wrist_coin.c", 247, "Received exchange index %ld.", index);

    if (ex_name) {
      app_log(APP_LOG_LEVEL_DEBUG, "wrist_coin.c", 435, "Exchange name is '%s'. Copying it now.", ex_name->value->cstring);
      ex_data_list[index].ex_name = strdyncpy(ex_data_list[index].ex_name, ex_name->value->cstring);
      app_log(APP_LOG_LEVEL_DEBUG, "write_coin.c", 438, "'%s' finshed copying.", ex_data_list[index].ex_name);
    }

    app_log(APP_LOG_LEVEL_DEBUG, "write_coin.c", 442, "Fetching current prices for %s.", ex_data_list[index].ex_name);
    // Now that the configuration has been loaded it's time to fetch the current
    // prices.
    fetch_ex_price(index);
  }
}

/* Looks at the configuration information to determine what type it is. If it is
 * a global configuration then config is sent to load_global_config. If it is an
 * exchange configuration then config is sent to load_ex_config.
 *
 * DictionaryIterator *config - Configuration information to be processed.
 */
static void load_config(DictionaryIterator *config) {
  Tuple *config_type = dict_find(config, WC_KEY_CONFIG);

  if (config_type) {
    if (config_type->value->int32 == WC_KEY_GLOBAL_CONFIG) {
      app_log(APP_LOG_LEVEL_DEBUG, "wrist_coin.c", 429, "Config record is global. This is going to get interesting.");
      load_global_config(config);
    } else if (config_type->value->int32 == WC_KEY_EX_CONFIG) {
      app_log(APP_LOG_LEVEL_DEBUG, "wrist_coin.c", 431, "Config record is for an exchange. Should be no biggy.");
      load_ex_config(config);
    }
  } else {
    app_log(APP_LOG_LEVEL_DEBUG, "wrist_coin.c", 387, "Configuration data didn't contain configuration type value.");
  }

  return;

}

/*
* Loads price information for an exchange.
*
* DictionaryIterator *prices - A DictionaryIterator containing price data for
*                              an exchange.
*/
static void load_ex_prices(DictionaryIterator *prices) {
  Tuple *ex_index = dict_find(prices, WC_KEY_EX_INDEX);

  app_log(APP_LOG_LEVEL_DEBUG, "wrist_coin.c", 233, "Entered load_ex_prices.");

  if (ex_index) {
    char *stat = NULL;

    Tuple *ex_low = dict_find(prices, WC_KEY_EX_LOW);
    Tuple *ex_high = dict_find(prices, WC_KEY_EX_HIGH);
    Tuple *ex_avg = dict_find(prices, WC_KEY_EX_AVG);
    Tuple *ex_last = dict_find(prices, WC_KEY_EX_LAST);
    Tuple *ex_vol = dict_find(prices, WC_KEY_EX_VOL);
    int index = ex_index->value->int32;

    app_log(APP_LOG_LEVEL_DEBUG, "wrist_coin.c", 482, "Received price information for %s.", ex_data_list[index].ex_name);

    if (ex_low) {
      app_log(APP_LOG_LEVEL_DEBUG, "wrist_coin.c", 482, "%s has a low of %ld.", ex_data_list[index].ex_name, ex_low->value->int32);
      ex_data_list[index].low = ex_low->value->int32;
    }

    if (ex_high) {
      ex_data_list[index].high = ex_high->value->int32;
      app_log(APP_LOG_LEVEL_DEBUG, "wrist_coin.c", 482, "%s has a high of %ld.", ex_data_list[index].ex_name, ex_high->value->int32);
    }

    if (ex_avg) {
      ex_data_list[index].avg = ex_avg->value->int32;
      app_log(APP_LOG_LEVEL_DEBUG, "wrist_coin.c", 482, "%s has an average of %ld.", ex_data_list[index].ex_name, ex_avg->value->int32);
    }

    if (ex_last) {
      ex_data_list[index].last = ex_last->value->int32;
      app_log(APP_LOG_LEVEL_DEBUG, "wrist_coin.c", 482, "%s has a last of %ld.", ex_data_list[index].ex_name, ex_last->value->int32);
    }

    if (ex_vol) {
      ex_data_list[index].vol = convert_bytes_to_int64(ex_vol);
      app_log(APP_LOG_LEVEL_DEBUG, "wrist_coin.c", 491, "%s has a volume of %lld.", ex_data_list[index].ex_name, ex_data_list[index].vol);
    }

    app_log(APP_LOG_LEVEL_DEBUG, "wrist_coin.c", 586, "Loading %ld into temporary status variable.", ex_data_list[index].avg);
    stat = format_dollars(stat, ex_data_list[index].avg);
    app_log(APP_LOG_LEVEL_DEBUG, "wrist_coin.c", 591, "Loading formatted string '%s' into status field.", stat);
    ex_data_list[index].ex_status = strdyncpy(ex_data_list[index].ex_status, stat);
  }
}

/* A handler for incoming messages from the Pebble app. It looks at the message
 * type and passes the DictionaryIterator off to the appropriate function for
 * processing.
 *
 */
static void in_received_handler(DictionaryIterator *received, void *context) {
  Tuple *command = dict_find(received, WC_KEY_COMMAND);

  /*
   * Processes command type messages.
   */
  if (command) {
    int32_t command_type = command->value->int32;

    app_log(APP_LOG_LEVEL_DEBUG, "wrist_coin.c", 203, "Received a command from the phone.");

    /*
     * Command 0 contains configuration information processable by the
     * load_ex_config function.
     */
    if (command_type == 0) {
      app_log(APP_LOG_LEVEL_DEBUG, "wrist_coin.c", 207, "Command was an exchange configuration.");
      load_config(received);
    }

    /*
     * Command 1 contains price information for an exchange that is
     * processable by the load_ex_prices function.
     */
    if (command_type == 1) {
      app_log(APP_LOG_LEVEL_DEBUG, "wrist_coin.c", 250, "Command was an exchange price list.");
      load_ex_prices(received);
    }
  }

  menu_layer_reload_data(exchange_menu);
}

static void in_dropped_handler(AppMessageResult reason, void *context) {
  app_log(APP_LOG_LEVEL_DEBUG, "wrist_coin.c", 195, "in_dropped_handler: error %d occurred while trying to receive data from the phone.\n", reason);
}

static void out_sent_handler(DictionaryIterator *sent, void *context) {
  app_log(APP_LOG_LEVEL_DEBUG, "wrist_coin.c", 677, "out_sent_handler: Sent a record from the Pebble to the phone.");
}

static void out_failed_handler(DictionaryIterator *failed, AppMessageResult reason, void *context) {
  app_log(APP_LOG_LEVEL_DEBUG, "wrist_coin.c", 114, "out_failed_handler: error %d occurred while trying to send data to the phone.\n", reason);

  Tuple *command = dict_find(failed, WC_KEY_COMMAND);
  Tuple *ex_index = dict_find(failed, WC_KEY_EX_INDEX);

  if (command) {
    app_log(APP_LOG_LEVEL_DEBUG, "wrist_coin.c", 677, "out_failed_handler: The command that failed to send was %ld.", command->value->int32);
  }

  if (ex_index) {
    app_log(APP_LOG_LEVEL_DEBUG, "wrist_coin.c", 681, "out_failed_handler: The exchange index that failed to send was %ld.", ex_index->value->int32);
  }
}

static void click_config_provider(void *context) {
  //  window_single_click_subscribe(BUTTON_ID_SELECT, select_click_handler);
  //  window_single_click_subscribe(BUTTON_ID_UP, up_click_handler);
  //  window_single_click_subscribe(BUTTON_ID_DOWN, down_click_handler);
}

static void window_load(Window *window) {
  Layer *window_layer = window_get_root_layer(window);
  GRect bounds = layer_get_bounds(window_layer);

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

  fetch_global_config();
}

static void window_unload(Window *window) {
  menu_layer_destroy(exchange_menu);

  free_ex_data_list();
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
