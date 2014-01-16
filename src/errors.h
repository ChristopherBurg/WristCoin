#pragma once

typedef enum {
    WC_ERROR_404 = 0, // Used when the JavaScript receives a 404 error from a
                      // Bitcoin exchange.
    WC_ERROR_CONNECTION_INTERRUPTED = 1, // Used when the JavaScript's 
                                         // XMLHttpRequest.status returns 
                                         // something other than four.
    WC_ERROR_EXCHANGE = 2, // The exchange API returned an error.
    WC_ERRRO_SEND = 3, // Sending a message from the Pebble to the watch failed.
} ErrorMessages;

