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

static const char *field_labels[] = {"Low:\0",
                                     "High:\0",
                                     "Average:\0",
                                     "Last:\0",
                                     "Volume:\0"
                                    };

// TODO: Remove this old crap.
/*
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
*/

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

static void window_appear(Window *window) {
  app_log(APP_LOG_LEVEL_DEBUG, "exchange_detail.c", 112, "window_appear: Entered window_appear.");
  GTextAlignment header_align = GTextAlignmentCenter;
  GTextAlignment display_align = GTextAlignmentCenter;

  const char *header_font = FONT_KEY_GOTHIC_18_BOLD;
  const char *display_font = FONT_KEY_GOTHIC_18;

  Layer *window_layer = window_get_root_layer(window);
  GRect bounds = layer_get_bounds(window_layer);

  int16_t header_width = bounds.size.w;
  int16_t display_width = bounds.size.w;
  int16_t header_height = 22;
  int16_t display_height = 22;

  scroll_layer = scroll_layer_create((GRect) { .origin = { 0, 32 }, .size = { bounds.size.w, bounds.size.h - 32 } });
  scroll_layer_set_content_size(scroll_layer, GSize(bounds.size.w, ((num_text_layers - 1) * 22)));
  layer_add_child(window_layer, scroll_layer_get_layer(scroll_layer));

  app_log(APP_LOG_LEVEL_DEBUG, "exchange_detail.c", 131, "window_appear: Creating text layer to dispaly exchange name.");
  text_layers[0] = text_layer_create((GRect) { .origin = { 0, 0 }, .size = { bounds.size.w, 32 } });
  text_layer_set_text_alignment(text_layers[0], GTextAlignmentCenter);
  text_layer_set_font(text_layers[0], fonts_get_system_font(FONT_KEY_GOTHIC_28_BOLD));
  text_layer_set_text(text_layers[0], fields[0]);
  layer_add_child(window_layer, text_layer_get_layer(text_layers[0]));

  app_log(APP_LOG_LEVEL_DEBUG, "exchange_detail.c", 138, "window_appear: Creating text layers to display exchange values.");
  /* Start at 1 since the first TextLayer has already been created.
   */
  int i = 1;

  while (i < num_text_layers) {
    int16_t field_width = 0;
    int16_t field_height = 0;

    GTextAlignment field_align = 0;

    const char *font = NULL;

    /* Odd numbered fields will be headers and even number fields will be
     * displays.
     */
    if (i % 2 == 0) {
      font = display_font;
      field_width = display_width;
      field_height = display_height;
      field_align = display_align;
    } else {
      font = header_font;
      field_width = header_width;
      field_height = header_height;
      field_align = header_align;
    }

    text_layers[i] = text_layer_create((GRect) { .origin = { 0, header_height * (i - 1) }, .size = { field_width, field_height } });
    text_layer_set_text_alignment(text_layers[i], field_align);
    text_layer_set_font(text_layers[i], fonts_get_system_font(font));

    scroll_layer_add_child(scroll_layer, text_layer_get_layer(text_layers[i]));

    i++;
  }

  app_log(APP_LOG_LEVEL_DEBUG, "exchange_detail.c", 175, "window_appear: Adding text to text layers.");

  /* Now we need to add the correct text to each text layer. This is done by
   * incrementing through each field. First a text layer has the field's label
   * set as the text. Then the next text layer has the formatted text set as its
   * label.
   */
  i = 1;
  int layer = 1;
  while (i < num_fields) {
    app_log(APP_LOG_LEVEL_DEBUG, "exchange_detail.c", 179, "window_appear: Adding header text for field %d. Text is '%s'.", i, field_labels[i - 1]);
    text_layer_set_text(text_layers[layer], field_labels[i - 1]);
    layer++;

    app_log(APP_LOG_LEVEL_DEBUG, "exchange_detail.c", 183, "window_appear: Adding display text for field %d. Text is '%s'.", i, fields[i]);
    text_layer_set_text(text_layers[layer], fields[i]);
    layer++;

    i++;
  }

  scroll_layer_set_click_config_onto_window(scroll_layer, window);
  app_log(APP_LOG_LEVEL_DEBUG, "exchange_detail.c", 188, "window_appear: Completed window_appear.");
}

static void window_unload(Window *window) {
  app_log(APP_LOG_LEVEL_DEBUG, "exchange_detail.c", 233, "window_unload: Entered window_unload.");

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
  for (int i = 0; i < num_text_layers; i++) {
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

static void window_load(Window *window) {
  app_log(APP_LOG_LEVEL_DEBUG, "exchange_detail.c", 293, "window_load: Entered window_appear.");
//  text_layer_set_text(exchange_name_display, ex_data->ex_name);

  /* With the exception of the name field each field has two text layers, one to
   * dispaly what the value is and one to dispaly the actual value.
   */
  num_text_layers = 1 + ((num_fields - 1) * 2);
  app_log(APP_LOG_LEVEL_DEBUG, "exchange_detail.c", 299, "window_load: Allocating memory for %d TextLayers.", num_text_layers);
  text_layers = (TextLayer **) malloc(sizeof(TextLayer *) * num_text_layers);

  app_log(APP_LOG_LEVEL_DEBUG, "exchange_detail.c", 304, "window_load: Allocating memory for %d fields.", num_fields);
  fields = (char **) malloc(sizeof(char *) * num_fields);

  fields[0] = (char *) malloc(sizeof(char) * (strlen(ex_data->ex_name) + 1));
  strncpy(fields[0], ex_data->ex_name, (strlen(ex_data->ex_name) + 1));
  app_log(APP_LOG_LEVEL_DEBUG, "exchange_detail.c", 248, "window_load: Exchange name (fields 0) is '%s'.", fields[0]);

  fields[1] = create_format_dollars(ex_data->low);
  app_log(APP_LOG_LEVEL_DEBUG, "exchange_detail.c", 248, "window_load: Exchange low (fields 1) is '%s'.", fields[1]);
  fields[2] = create_format_dollars(ex_data->high);
  app_log(APP_LOG_LEVEL_DEBUG, "exchange_detail.c", 248, "window_load: Exchange high (fields 2) is '%s'.", fields[2]);
  fields[3] = create_format_dollars(ex_data->avg);
  app_log(APP_LOG_LEVEL_DEBUG, "exchange_detail.c", 248, "window_load: Exchange avg (fields 3) is '%s'.", fields[3]);
  fields[4] = create_format_dollars(ex_data->last);
  app_log(APP_LOG_LEVEL_DEBUG, "exchange_detail.c", 248, "window_load: Exchange last (fields 4) is '%s'.", fields[4]);
  fields[5] = create_format_dollars(ex_data->low);
  app_log(APP_LOG_LEVEL_DEBUG, "exchange_detail.c", 248, "window_load: Exchange vol (currently set to low) (fields 5) is '%s'.", fields[5]);
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
  app_log(APP_LOG_LEVEL_DEBUG, "exchange_detail.c", 342, "exchange_detail_show: Entered exchange detail.");
  ex_data = data;

  window_stack_push(window, true);
}

/*
void OLD_exchange_detail_show(ExchangeData *selected_exchange_data) {
    exchange_data = selected_exchange_data;

    window_stack_push(window, true);
}
*/
