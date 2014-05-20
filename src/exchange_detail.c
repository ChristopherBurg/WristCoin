#include "exchange_detail.h"

static Window *window;
static ScrollLayer *scroll_layer;

static ExData *ex_data = NULL;

/* There are six fields currently in the ExData. Namely ex_name, low, high, avg,
 * last, and vol. At a future time I will try to create a method of dynamically
 * determining the number of fields but this will have to suffice for now.
 */
int num_fields = 6;
int num_text_layers = 0;

/* An array to hold the different values that will be displayed to the user.
 */
static char **fields = NULL;

/* An array of TextLayers used to display information to the user.
 */
static TextLayer **text_layers = NULL;

/* An array of labels used to denote what information is being dispaleyd to the
 * user.
 */
static const char *field_labels[] = {"Low:\0",
                                     "High:\0",
                                     "Average:\0",
                                     "Last:\0",
                                     "Volume:\0"
                                    };

static void click_config_provider(void *context) {

}

/* This function is charged with created all of the user interface components
 * setting them up to properly dispaly information to the user.
 */
static void window_appear(Window *window) {
  app_log(APP_LOG_LEVEL_DEBUG, "exchange_detail.c", 112, "window_appear: Entered window_appear.");
  /* To ease interface changes the values for headers, the text layers that show
   * labels, and dispalys, the text layers that show prices, are setup here. If
   * the interface needs to be changes these variables should be the only things
   * that need to be jiggered with.
   */
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
  /* The top of the screen shows the name of the exchange that the pricing data
   * applies to. This text layer is outside of the scroll layer so that it will
   * continue to show as the user scrolls through the pricing data.
   */
  text_layers[0] = text_layer_create((GRect) { .origin = { 0, 0 }, .size = { bounds.size.w, 32 } });
  text_layer_set_text_alignment(text_layers[0], GTextAlignmentCenter);
  text_layer_set_font(text_layers[0], fonts_get_system_font(FONT_KEY_GOTHIC_28_BOLD));
  text_layer_set_text(text_layers[0], fields[0]);
  layer_add_child(window_layer, text_layer_get_layer(text_layers[0]));

  app_log(APP_LOG_LEVEL_DEBUG, "exchange_detail.c", 138, "window_appear: Creating text layers to display exchange values.");
  /* This loop creates all of the text layers that are used on the screen. As
   * the first text layer, the one showing the exchange name, has already been
   * created we start with i being set to 1. Odd numbered text layers are labels
   * whereas even numbered dispalys are price information.
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

/* Frees up any memory that was allocated when this screen was first displayed.
 */
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
//  app_log(APP_LOG_LEVEL_DEBUG, "exchange_detail.c", 299, "window_load: Allocating memory for %d TextLayers.", num_text_layers);
  text_layers = (TextLayer **) calloc(num_text_layers, sizeof(TextLayer *));

//  app_log(APP_LOG_LEVEL_DEBUG, "exchange_detail.c", 304, "window_load: Allocating memory for %d fields.", num_fields);
  fields = (char **) calloc(num_fields, sizeof(char *));

  fields[0] = (char *) calloc(strlen(ex_data->ex_name) + 1, sizeof(char));
  strncpy(fields[0], ex_data->ex_name, (strlen(ex_data->ex_name) + 1));

  fields[1] = create_format_dollars(ex_data->low);
  fields[2] = create_format_dollars(ex_data->high);
  fields[3] = create_format_dollars(ex_data->avg);
  fields[4] = create_format_dollars(ex_data->last);
  fields[5] = create_format_volume(ex_data->vol);
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

/* Displays the exchange detail screen.
 *
 * ExData *data - The exchange data to show details for.
 */
void exchange_detail_show(ExData *data) {
  app_log(APP_LOG_LEVEL_DEBUG, "exchange_detail.c", 342, "exchange_detail_show: Entered exchange detail.");
  ex_data = data;

  window_stack_push(window, true);
}
