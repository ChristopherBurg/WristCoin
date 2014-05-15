#include "exchange_detail.h"

static Window *window;
static ScrollLayer *scroll_layer;

static char *low = NULL;
static char *high = NULL;
static char *avg = NULL;
static char *last = NULL;
static char *vol = NULL;

static ExData *ex_data = NULL;

/* There are six fields currently in the ExData. Namely ex_name, low, high, avg,
 * last, and vol.
 */
int num_fields = 6;
int num_text_layers = 0;

/* An array to hold the different values that will be displayed to the user.
 */
static char **fields = NULL;

/* An array of TextLayers used to display information to the user.
 */
static TextLayer **text_layers = NULL;

static const char *field_labels[] = {"\0",
                                     "Low:\0",
                                     "High:\0",
                                     "Average:\0",
                                     "Last:\0",
                                     "Volume:\0"
                                    };

// TODO: Remove this old crap.
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

/*
static char low[PRICE_FIELD_LENGTH];
static char high[PRICE_FIELD_LENGTH];
static char last[PRICE_FIELD_LENGTH];
static char average[PRICE_FIELD_LENGTH];
static char buy[PRICE_FIELD_LENGTH];
static char sell[PRICE_FIELD_LENGTH];
static char volume[VOLUME_FIELD_LENGTH];
*/

/*
static ExchangeData *exchange_data;
*/

static void click_config_provider(void *context) {

}

/* Initializes a text layer, sets values for that text layer, and adds it as a
   child to the main scroll layer.

   layer - The text layer to initialize and add to the scroll layer.

   text - The text for the label to display.

   font - The font key for the font to use.

   alignment - How to align the text in the text layer.

   x - The x coordinate to draw the layer at.

   y - The y coordinate to draw the layer at.

   width - How wide the text layer should be.

   height - How tall the text layer should be.
 */
static void init_text_layer_for_scroll(TextLayer *layer, const char *text, const char *font, GTextAlignment alignment, int x, int y, int16_t width, int16_t height) {
    Layer *window_layer = window_get_root_layer(window);
    GRect bounds = layer_get_bounds(window_layer);

    layer = text_layer_create((GRect) { .origin = { x, y }, .size = { width, height } });
    text_layer_set_font(layer, fonts_get_system_font(font));
    text_layer_set_text_alignment(layer, alignment);
    text_layer_set_text(layer, text);
    scroll_layer_add_child(scroll_layer, text_layer_get_layer(layer));
}

static void window_load(Window *window) {
  GTextAlignment header_align = GTextAlignmentLeft;
  GTextAlignment display_align = GTextAlignmentRight;

  const char *header_font = FONT_KEY_GOTHIC_18_BOLD;
  const char *display_font = FONT_KEY_GOTHIC_18;

  Layer *window_layer = window_get_root_layer(window);
  GRect bounds = layer_get_bounds(window_layer);

  int16_t header_width = bounds.size.w;
  int16_t display_width = bounds.size.w;
  int16_t header_height = 22;
  int16_t display_height = 22;

  scroll_layer = scroll_layer_create((GRect) { .origin = { 0, 32 }, .size = { bounds.size.w, bounds.size.h - 32 } });
  scroll_layer_set_content_size(scroll_layer, GSize(bounds.size.w, 154));
  layer_add_child(window_layer, scroll_layer_get_layer(scroll_layer));

  text_layer[0] = text_layer_create((GRect) { .origin = { 0, 0 }, .size = { bounds.size.w, 32 } });
  text_layer_set_text_alignment(text_layer[0], GTextAlignmentCenter);
  text_layer_set_font(text_layer[0], fonts_get_system_font(FONT_KEY_GOTHIC_28_BOLD));
  text_layer_set_text(text_layer[0], fields[0]);
  layer_add_child(window_layer, text_layer_get_layer(text_layer[0]));

  for (int i = 1; i < (num_fields - 1); i++) {
    text_layer[i] = text_layer_create((GRect) { .origin = { 0, header_height * (i - 1) }, .size = { header_width, header_height } });
  }
}

static void OLD_window_load(Window *window) {
    Layer *window_layer = window_get_root_layer(window);
    GRect bounds = layer_get_bounds(window_layer);
    int half_screen = (bounds.size.w / 2);

    scroll_layer = scroll_layer_create((GRect) { .origin = { 0, 32 }, .size = { bounds.size.w, bounds.size.h - 32 } });
    scroll_layer_set_content_size(scroll_layer, GSize(bounds.size.w, 154));
    layer_add_child(window_layer, scroll_layer_get_layer(scroll_layer));

    exchange_name_display = text_layer_create((GRect) { .origin = { 0, 0 }, .size = { bounds.size.w, 32 } });
    text_layer_set_text_alignment(exchange_name_display, GTextAlignmentCenter);
    text_layer_set_font(exchange_name_display, fonts_get_system_font(FONT_KEY_GOTHIC_28_BOLD));
    layer_add_child(window_layer, text_layer_get_layer(exchange_name_display));

    // Label and text for the low value.
    init_text_layer_for_scroll(low_label, low_text, FONT_KEY_GOTHIC_18_BOLD, GTextAlignmentLeft, 0, 0, half_screen, 22);
    init_text_layer_for_scroll(low_display, low, FONT_KEY_GOTHIC_18, GTextAlignmentRight, half_screen, 0, half_screen, 22);

    // Label and text for the high value.
    init_text_layer_for_scroll(high_label, high_text, FONT_KEY_GOTHIC_18_BOLD, GTextAlignmentLeft, 0, 22, half_screen, 22);
    init_text_layer_for_scroll(high_display, high, FONT_KEY_GOTHIC_18, GTextAlignmentRight, half_screen, 22, half_screen, 22);

    // Label and text for the last value.
    init_text_layer_for_scroll(last_label, last_text, FONT_KEY_GOTHIC_18_BOLD, GTextAlignmentLeft, 0, 44, half_screen, 22);
    init_text_layer_for_scroll(last_display, last, FONT_KEY_GOTHIC_18, GTextAlignmentRight, half_screen, 44, half_screen, 22);

    // Label and text for the average value.
    init_text_layer_for_scroll(average_label, average_text, FONT_KEY_GOTHIC_18_BOLD, GTextAlignmentLeft, 0, 66, half_screen, 22);
    init_text_layer_for_scroll(average_display, average, FONT_KEY_GOTHIC_18, GTextAlignmentRight, half_screen, 66, half_screen, 22);

    // Label and text for the buy value.
    init_text_layer_for_scroll(buy_label, buy_text, FONT_KEY_GOTHIC_18_BOLD, GTextAlignmentLeft, 0, 88, half_screen, 22);
    init_text_layer_for_scroll(buy_display, buy, FONT_KEY_GOTHIC_18, GTextAlignmentRight, half_screen, 88, half_screen, 22);

    // Label and text for the sell value.
    init_text_layer_for_scroll(sell_label, sell_text, FONT_KEY_GOTHIC_18_BOLD, GTextAlignmentLeft, 0, 110, half_screen, 22);
    init_text_layer_for_scroll(sell_display, sell, FONT_KEY_GOTHIC_18, GTextAlignmentRight, half_screen, 110, half_screen, 22);

    // Label and text for the volume.
    init_text_layer_for_scroll(volume_label, volume_text, FONT_KEY_GOTHIC_18_BOLD, GTextAlignmentLeft, 0, 132, 55, 22);
    init_text_layer_for_scroll(volume_display, volume, FONT_KEY_GOTHIC_18, GTextAlignmentRight, 55, 132, bounds.size.w - 55, 22);

    scroll_layer_set_click_config_onto_window(scroll_layer, window);
}

static void window_unload(Window *window) {

  /* Destroy the field that contains the exchange name.
   */
  if (fields[0] != NULL) {
    free(fields[0]);
    fields[0] = NULL;
  }

  /* Destroy all of the format fields.
   */
  for (int i = 1; i < num_fields; i++) {
    destroy_format(fields[i]);
  }

  /* Free the memory allocated for storing all of the format fields.
   */
  if (fields != NULL) {
    free(fields);
    fields = NULL;
  }

  /* Destroy the text layers.
   */
  for (int i = 0; i < num_text_fields; i++) {
    text_layer_destroy(text_layers[i]);
  }

  /* Free the memory allocated for storing all of the text layers.
   */
  if (text_layers != NULL) {
    free(text_layers);
    text_layers = NULL;
  }

  scroll_layer_destroy(scroll_layer);
}

/*
static void OLD_window_unload(Window *window) {
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
*/

static void window_appear(Window *window) {

//  text_layer_set_text(exchange_name_display, ex_data->ex_name);

  /* With the exception of the name field each field has two text layers, one to
   * dispaly what the value is and one to dispaly the actual value.
   */
  num_text_layers = 1 + ((num_fields - 1) * 2);
  text_layers = (TextLayer **) malloc(sizeof(TextLayer *) * num_text_fields);

  app_log(APP_LOG_LEVEL_DEBUG, "exchange_detail.c", 56, "Low value is: %ld.", ex_data->low);

  fields = (char **) malloc(sizeof(char *) * num_fields);

  fields[0] = (char *) malloc(sizeof(char) * (strlen(ex_data->ex_name) + 1));
  strncpy(fields[0], ex_data->ex_name, (strlen(ex_data->ex_name) + 1));

  fields[1] = create_format_dollars(ex_data->low);
  fields[2] = create_format_dollars(ex_data->high);
  fields[3] = create_format_dollars(ex_data->avg);
  fields[4] = create_format_dollars(ex_data->last);
  fields[5] = create_format_dollars(ex_data->vol);
}

static void window_disappear(Window *window) {
  destroy_format(low);
  destroy_format(high);
  destroy_format(avg);
  destroy_format(last);
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

/* Displays the exchange detail screen.
 *
 * ExData *data - The exchange data to show details for.
 */
void exchange_detail_show(ExData *data) {
  ex_data = data;

  window_stack_push(window, true);
}

/*
void OLD_exchange_detail_show(ExchangeData *selected_exchange_data) {
    exchange_data = selected_exchange_data;

    window_stack_push(window, true);
}
*/
