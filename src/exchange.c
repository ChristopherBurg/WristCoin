#include "exchange.h"

/* Allocates memory for an ExData struct, zeros out the allocated chunk, and
 * returns a pointer to the allocated memory.
 */
ExData * create_ex_data(void) {
  ExData *data = (ExData *) calloc(1, sizeof(ExData));

  return data;
}

/* Used to tear down an ExData struct. It takes a pointer to an ExData struct,
 * frees any memory that was allocated to it, and frees the ExData struct's
 * allocated memory.
 *
 * ExData *data - A pointer to the ExData struct to destroy.
 */
void destroy_ex_data(ExData *data) {
//  app_log(APP_LOG_LEVEL_DEBUG, "exchange.c", 17, "destroy_ex_data: Received order to destroying ExData struct %p.", data);
  if (data != NULL) {
    if (data->ex_name != NULL) {
//      app_log(APP_LOG_LEVEL_DEBUG, "exchange.c", 20, "destroy_ex_data: Destroying ExData->ex_name %p.", data->ex_name);
      free(data->ex_name);
//      data->ex_name = NULL;
    }

//    app_log(APP_LOG_LEVEL_DEBUG, "wrist_coin.c", 25, "destroy_ex_data: Destroying ExData struct %p now.", data);
    free(data);
    data = NULL;
//    app_log(APP_LOG_LEVEL_DEBUG, "wrist_coin.c", 28, "destroy_ex_data: ExData struct destroyed. Pointer is now %p.", data);
  }
}

/* Used to set the ex_name value in an ExData struct. It first checks to see if
 * the ex_name pointer is NULL. If not it frees ex_name. Memory is than
 * allocated to fit src and the contents of src are copied into the newly
 * allocated memory for ex_name.
 *
 * ExData *data - The ExData struct to modify the ex_name value of.
 *
 * char *src - The new value to copy into ex_name.
 */
void set_ex_name(ExData *data, char *src) {
  if (data->ex_name != NULL) {
    free(data->ex_name);
    data->ex_name = NULL;
  }

//  app_log(APP_LOG_LEVEL_DEBUG, "exchange.c", 35, "set_ex_name: Allocating %d bytes for ex_name.", (strlen(src) + 1));
  data->ex_name = (char *) calloc(strlen(src) + 1, sizeof(char));

//  app_log(APP_LOG_LEVEL_DEBUG, "exchange.", 38, "set_ex_name: Copying %s into ex_name.", src);
  strncpy(data->ex_name, src, strlen(src) + 1);

//  app_log(APP_LOG_LEVEL_DEBUG, "exchange.c", 41, "set_ex_name: ex_name now contains %s.", data->ex_name);
}
