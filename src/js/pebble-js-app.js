/* Takes a hexadecimal string and converts it to a byte array. This function is
 * primarily used as a way to send numbers larger than 32-bit to the Pebble.
 */
function convertHexStringToByteArray(string) {
  var byte_array = new Array();
  var i = 0;

  if (string.length % 2 != 0) {
    console.log("Odd number of bytes. Pushing: " + parseInt(string.charAt(i), 16).toString(16));
    byte_array.push(parseInt(string.charAt(i), 16));
    ++i;
  }

  while (i < string.length) {
    console.log("Substring is " + string.substr(i, 2));
    var byte_to_push = parseInt(string.substr(i, 2), 16);
    console.log("Pushing byte " + byte_to_push.toString(16) + " " + byte_to_push.toString(10));
    byte_array.push(byte_to_push);
    i += 2;
  }

  console.log(byte_array);

  return byte_array;
}

/* The Pebble is only capable of receiving one message at a time. Since much of
 * this app is asynchronous it's possible, and in testing quite common, for
 * multiple messages to be generated simultaneously. To ensure problems related
 * related to sending message at the same time are avoided this program uses a
 * message stack. When a function wants to send a message to the Pebble it calls
 * sendMessageToPebble, which pushes the message onto the stack and then tells
 * the app to begin sending messages if it's not already doing so.
 */
var messageStack = new Array();
/* A counter to determine the number of times the sendMessage function has been
 * called on an empty messageStack. After 10 intervals the messageSender should
 * be cleared and this variable reset to 0.
 */
var iterations = 0;
var messageSender;

/* This function is charged with actually sending message to the Pebble. When
 * called it pops the first message off of the stack and attempts to send it.
 * This function is called continously via setInterval until the stack is empty.
 * When the stack appears to be empty the sendMessage interval is clocked down
 * so any stragling messages can be pushed onto the stack. If no messages appear
 * for 10 iterations then messageSender is cleared.
 */
function sendMessage() {
  var messageSender;
  if (messageStack.length == 0) {
    /* Keep checking the message stack for straglers.
     */
    if (iterations < 5) {
      console.log("Message stack appears to be empty.");
      iterations++;
    /* If no messages appear on the stack after five iterations clock the
     * interval down to once a second.
     */
    } else if ((iterations >= 5) && (iterations < 10)) {
      console.log("Clocking down sendMessage interval time to 1 second.");
      iterations++;
      clearInterval(messageSender);
      messageSender = setInterval(sendMessage, 1000);
    /* If nothing has appeared on the stack after 10 iterations then it's
     * probably not going to happen. Clear messageSender and reset the interval
     * count to 0.
     */
    } else if (iterations >= 10) {
      console.log("No messages appeared on the stack in the last 10 seconds. Clearing sendMessage interval.");
      iterations = 0;
      clearInterval(messageSender);
    }
  } else {
    var messageToSend = messageStack.pop();

    var successHandler = function(event) {
      console.log("Successfully sent " + event.data.transactionId + " to Pebble.");
    }

    var errorHandler = function(event) {
      console.log("Failed to send " + event.data.transactionId + " to Pebble.");
      messageStack.push(messageToSend);
    }

    /* This function is only called when another function is trying to send a
     * message to the Pebble. Since that necessarily means the stack isn't empty
     * reset iterations to 0.
     */
    iterations = 0;

    console.log("Sending message to Pebble");
    console.log("Message contains command '" + messageToSend.command + "' and exchange index '" + messageToSend.exIndex + ".");
    Pebble.sendAppMessage(messageToSend,
                          successHandler,
                          errorHandler);
  }
}

/* This function takes a message and pushes it onto the message stack. If the
 * program isn't already sending messages this function then tells it to start
 * sending. Otherwise the function returns.
 *
 * message - The message to push onto the message stack.
 */
function OLD_sendMessageToPebble(message) {
  messageStack.push(message);

  clearInterval(messageSender);
  /* Call setIntervale here. Doing it in the sendMessage function causes the
   * setInterval function not to work for some reason. Unless the app is being
   * monitored with 'pebble logs' the sendMessage function never gets called.
   *
   * I'll look into this more later.
   */
  messageSender = setInterval(sendMessage, 250);
}

/* This variable is set to true when sendMessageToPebble's internal sending
 * function, messageSender, is running. Use this flag to ensure there isn't
 * several instances of messageSender running simultaneously.
 */
var isSending = false;

/* When a function wants to send a message to the Pebble it should send that
 * message to this function. Upon being called this function pushes the new
 * message to messageStack. If isSending is false then this function calls its
 * internal helper function, messageSender. Otherwise it returns.
 */
function sendMessageToPebble(message) {
  /* When messageStack is empty the messageSender function will check it a few
   * more times to make sure no straglers show up. By default it checks ten
   * times. After the fifth time it clocks down from checking it every
   * defaultTime milliseconds to extendedTime milliseconds.
   */
  var defaultTime = 100;
  var extendedTime = 1000;

  var messageSender = function(iEmpty) {
    console.log("messageSender: Entered messageSender. iEmpty is " + iEmpty + ".");
    if (messageStack.length == 0) {
      /* Keep checking the message stack for straglers.
       */
      if (iEmpty < 5) {
        console.log("Message stack appears to be empty.");
        setTimeout(function() { messageSender(iEmpty + 1) }, defaultTime);
      /* If no messages appear on the stack after five iterations clock the
       * interval down to once a second.
       */
      } else if ((iEmpty >= 5) && (iEmpty < 10)) {
        console.log("Clocking down sendMessage interval time to 1 second.");
        setTimeout(function() { messageSender(iEmpty + 1) }, extendedTime)
      /* If nothing has appeared on the stack after 10 iterations then it's
       * probably not going to happen. Clear messageSender and reset the interval
       * count to 0.
       */
      } else if (iEmpty >= 10) {
        isSending = false;
      }
    } else {
      var messageToSend = messageStack.pop();

      var successHandler = function(event) {
        console.log("Successfully sent " + event.data.transactionId + " to Pebble.");
        setTimeout(function() { messageSender(0) }, defaultTime);
      }

      var errorHandler = function(event) {
        console.log("Failed to send " + event.data.transactionId + " to Pebble.");
        messageStack.push(messageToSend);
        setTimeout(function() { messageSender(0) }, defaultTime);
      }

      isSending = true;
      console.log("Sending message to Pebble");
      console.log("Message contains command '" + messageToSend.command + "' and exchange index '" + messageToSend.exIndex + ".");
      Pebble.sendAppMessage(messageToSend,
                            successHandler,
                            errorHandler);

    }
  }

  messageStack.push(message);

  if (!isSending) {
    setTimeout(function() { messageSender(0) }, 0);
  }
}

function OLDER_sendMessageToPebble(message) {
  var successHandler = function(event) {
    console.log("Successfully sent " + event.data.transactionId + " to Pebble.");
  }

  /* The Pebble can only process one message at a time. Since messages come in
   * as they arrive from the exchange they can oftentimes send before a
   * previous message has complete processing. To work around this any
   * message that fails to send will cause the application to wait for a period
   * between one and two seconds before resending.
   *
   * It's not pretty or elegant but it works.
  */
  var errorHandler = function(event) {
    var delay = Math.floor(Math.random() * (2000 - 1000) + 1000);
    console.log("Failed to send " + event.data.transactionId + " to Pebble.");
    console.log("Error message for " + event.data.transactionId + " was " + event.error + ".");
    console.log("Delaying resend for " + (delay / 1000) + " seconds.");
    setTimeout(sendMessageToPebble(message), delay);
  }

  console.log("Sending message to Pebble");
  console.log("Message contains command '" + message.command + "' and exchange index '" + message.exIndex + ".");
  Pebble.sendAppMessage(message,
                        successHandler,
                        errorHandler);
}

// Gets the curret price list from Bitstamp.
function fetchBitstampPrice() {
  var req = new XMLHttpRequest();

  var onloadHandler = function(event) {
    if (req.readyState == 4) {
      if (req.status == 200) {
        console.log(req.responseText);

        // Sometimes we can get unexpected results. This constitutes a
        // "type 2" error. Please note that this try-catch block will
        // return immediately upon error.
        try {
          var response = JSON.parse(req.responseText);
        } catch (e) {
          sendMessageToPebble({"exchange" : 0,
                               "error" : 2
                              });
          return;
        }

        var high = parseInt(response.high * 100);
        var last = parseInt(response.last * 100);
        var low = parseInt(response.low * 100);
        // Bitstamp doesn't provide an average so I calculate one.
        var average = parseInt((high + low) / 2);
        var buy = parseInt(response.bid * 100);
        var sell = parseInt(response.ask * 100);

        var volume = parseInt(response.volume * 100000000).toString(16);
        var volume_bytes = convertHexStringToByteArray(volume);

        console.log(volume);

        console.log("Sending prices for Bitstamp. Hold on to your butts.");
        sendMessageToPebble({"command" : 1,
                             "exIndex" : getEnabledIndexByName("Bitstamp"),
                             "exLow" : low,
                             "exHigh" : high,
                             "exAvg" : average,
                             "exLast" : last
                            });

      } else {
        console.log("HTTP status returned was not 200. Received " + req.status + " instead.");
        sendMessageToPebble({"exchange" : 0,
                           "error" : 0
                            });
      }
    } else {
      console.log("Didn't receieve ready status of 4. Receieved " + req.readyStatus + " instead.");
      sendMessageToPebble({"exchange" : 0,
                           "error" : 1
                          });
    }
  }

  console.log("Fetching Bitstamp prices.");

  req.onload = onloadHandler;

  req.open("GET", "https://www.bitstamp.net/api/ticker/", true);

  req.send(null);
}

// Gets the current prices from BTC-e.
function fetchBtcePrice() {
  var req = new XMLHttpRequest();

  var onloadHandler = function(event) {
    var successHandler = function(event) {
      console.log("Successfully sent BTC-e prices to Pebble.");
    }

    var errorHandler = function(event) {
      console.log("Error sending BTC-e prices to Pebble.");
    }

    console.log("BTC-e response received!");

    if (req.readyState == 4) {
      if (req.status == 200) {

        console.log(req.responseText);

        try {
          var response = JSON.parse(req.responseText);
        } catch (e) {
          sendMessageToPebble({"exchange" : 2,
          "error" : 2
        });
        return;
      }

      var high = parseInt(response.ticker.high * 100);
      var low = parseInt(response.ticker.low * 100);
      var last = parseInt(response.ticker.last * 100);
      var average = parseInt(response.ticker.avg * 100);
      var buy = parseInt(response.ticker.buy * 100);
      var sell = parseInt(response.ticker.sell * 100);

      var volume = parseInt(response.ticker.vol_cur * 100000000).toString(16);
      var volume_bytes = convertHexStringToByteArray(volume);

      console.log(volume);

      console.log("Sending prices for BTC-e. Hold on to your butts.");
      sendMessageToPebble({"command" : 1,
                           "exIndex" : getEnabledIndexByName("BTC-e"),
                           "exLow" : low,
                           "exHigh" : high,
                           "exAvg" : average,
                           "exLast" : last
                          });

      } else {
        console.log("HTTP status returned was not 200. Received " + req.status.toString() + " instead.");
        sendMessageToPebble({"exchange" : 2,
                             "error" : 0
                            });
      }
    } else {
      console.log("Didn't received ready status of 4. Received " + req.readyState.toString() + " instead.");
      sendMessageToPebble({"exchange" : 2,
                           "error" : 1
                          });
    }
  }

  console.log("Fetching BTC-e prices.");

  req.onload = onloadHandler;

  req.open("GET", "https://btc-e.com/api/2/btc_usd/ticker", true);

  req.send(null);
}

function fetchBitfinexPrice() {
  var req = new XMLHttpRequest();

  var onloadHandler = function(event) {
    var successHandler = function(event) {
      console.log("Successfully sent Bitfinex prices to Pebble.");
    }

    var errorHandler = function(event) {
      console.log("Error sending Bitfinex prices to Pebble.");
    }

    console.log("Bitfinex response received!");

    if (req.readyState == 4) {
      if (req.status == 200) {

        console.log(req.responseText);

        try {
          var response = JSON.parse(req.responseText);
        } catch (e) {
          sendMessageToPebble({"exchange" : 2,
                               "error" : 2
        });
        return;
      }

      var high = parseInt(response.high * 100);
      var low = parseInt(response.low * 100);
      var last = parseInt(response.last_price * 100);
      // Like Bitstamp, Bitfinex doesn't report an average that I know of.
      var average = parseInt((high + low) / 2);
//      var buy = parseInt(response.ticker.buy * 100);
//      var sell = parseInt(response.ticker.sell * 100);

      var volume = parseInt(response.volume * 100000000).toString(16);
      var volume_bytes = convertHexStringToByteArray(volume);

      console.log(volume);

      console.log("Sending prices for Bitfinex. Hold on to your butts.");
      sendMessageToPebble({"command" : 1,
                           "exIndex" : getEnabledIndexByName("Bitfinex"),
                           "exLow" : low,
                           "exHigh" : high,
                           "exAvg" : average,
                           "exLast" : last
                          });

      } else {
        console.log("HTTP status returned was not 200. Received " + req.status.toString() + " instead.");
        sendMessageToPebble({"exchange" : 2,
                             "error" : 0
                            });
      }
    } else {
      console.log("Didn't received ready status of 4. Received " + req.readyState.toString() + " instead.");
      sendMessageToPebble({"exchange" : 2,
                           "error" : 1
                          });
    }
  }

  console.log("Fetching Bitfinex prices.");

  req.onload = onloadHandler;

  req.open("GET", "https://api.bitfinex.com/v1/pubticker/btcusd", true);

  req.send(null);
}

/* Remember when Mt. Gox was a thing? Well it's not anymore but this code
 * remains as a method of testing disabled exchanges. It will eventually be
 * removed.
 */
function fetchMtGoxPrice() {
  var req = new XMLHttpRequest();

  var onloadHandler = function(event) {
    console.log("Mt. Gox response received!");
    var successHandler = function(event) {
      console.log("Successfully sent Mt.Gox prices to Pebble.");
    }

    var errorHandler = function(event) {
      console.log("Error sending Mt. Gox prices to Pebble.");
    }


    if (req.readyState == 4) {
      if (req.status == 200) {

        console.log(req.responseText);

        try {
          var response = JSON.parse(req.responseText);
        } catch (e) {
          sendMessageToPebble({"exchange" : 1,
          "error" : 2
        });
        return;
      }

      if (response.result == "success") {
        var high = parseInt(response.data.high.value * 100);
        var low = parseInt(response.data.low.value * 100);
        var last = parseInt(response.data.last.value * 100);
        var average = parseInt(response.data.avg.value * 100);
        var buy = parseInt(response.data.buy.value * 100);
        var sell = parseInt(response.data.sell.value * 100);

        var volume = parseInt(response.data.vol.value_int).toString(16);
        var volume_bytes = convertHexStringToByteArray(volume);

        console.log(volume);

        //                    console.log("High: " + high);
        //                    console.log("Low: " + low);
        //                    console.log("Last: " + last);
        //                    console.log("Volume: " + volume);

        sendMessageToPebble({"exchange" : 1,
                             "high" : high,
                             "low" : low,
                             "last" : last,
                             "average" : average,
                             "buy" : buy,
                             "sell" : sell,
                             "volume" : volume_bytes
                            });

    } else {
      console.log("Mt. Gox API didn't return success. Received " + response.success.toString() + " instead.");
      sendMessageToPebble({"exchange" : 1,
                           "error" : 2
                          });
    }

  } else {
    console.log("HTTP status returned was not 200. Received " + req.status.toString() + " instead.");
    sendMessageToPebble({"exchange" : 1,
                         "error" : 0
                        });
  }

  } else {
    console.log("Didn't receieve ready status of 4. Received " + req.readyState.toString() + " instead.");
    sendMessageToPebble({"exchange" : 1,
                         "error" : 1
                        });
  }

}

console.log("Fetching Mt. Gox prices.");

req.onload = onloadHandler;

req.open("GET", "https://data.mtgox.com/api/2/BTCUSD/money/ticker", true);

req.send(null);
}

/* An array containing a list of all available exchanges.
 *
 * Each exchange has an associated hashtable. Each hashtable includes the
 * exchanges name and the function called to fetches its current prices.
 */
var exchanges = [{"exName" : "Bitstamp",
                  "priceLookup" : function() { fetchBitstampPrice() }
                 },
                 {"exName" : "BTC-e",
                  "priceLookup" : function () { fetchBtcePrice() }
                 },
                 {"exName" : "Bitfinex",
                  "priceLookup" : function () { fetchBitfinexPrice() }
                 },
                 {"exName" : "Mt. Gox",
                  "priceLookup" : function () { fetchMtGoxPrice() }
                 }
                ];

/* An array of enabled exchanges. Exchanges added to this array will be
 * considered enabled and therefore have their prices looked up and send to the
 * Pebble.
 */
var enabledEx = new Array();

/* Checks what exchanges the user has enabled and pushes them into the enabledEx
 * array. This function should be called at startup and whenever the user
 * enables or disables an exchange.
 */
function loadEnabledExchanges() {
  // TODO: Swap out this hardcoded test with user selected exchanges.
  enabledEx.push(exchanges[0]);
  enabledEx.push(exchanges[1]);
  enabledEx.push(exchanges[2]);
}

/* The price lookup functions need the ability to include the index of their
 * cooresponding exchange from the enabledEx array. This function facilitates
 * that ability to taking the name of the exchange and returning its index in
 * the enabledEx array.
 *
 * name - The name of the exchange as it appears in enabledEx.exName.
 */
function getEnabledIndexByName(name) {
  var index = 0;

  while ((enabledEx[index].exName != name) && (index < enabledEx.length)) {
    index++;
  }

  return index;
}

/* This function send global configuration information to the Pebble. Global
 * configuration information is basically anything that doesn't explicitly deal
 * with an individual exchange.
 */
function sendGlobalConfig() {
//  var numExEnabled = getNumExEnabled();

  console.log("Sending global configuration.");
  sendMessageToPebble({"command" : 0,
                       "config" : 0,
                       "numEx" : enabledEx.length,
                      });
}

/* This function sends configuration information for each enabled exchange.
 */
function sendExConfig() {
  console.log("Pebble has requested exchange configurations. Sending test configs now.");

  for (var i = 0; i < enabledEx.length; i++) {
    console.log(enabledEx[i].exName + " is enabled. Sending its configuraiton information.");
    sendMessageToPebble({"command" : 0,
                         "config" : 1,
                         "exIndex" : i,
                         "exName" : enabledEx[i].exName
                        });
  }
}

/* Calls the appropriate function to look up the current prices for the desired
 * exchange.
 *
 * index - The index of the exchange to look up prices for.
 */
function sendExPrices(index) {
//  var ex = getExEnabled(index);
  console.log("Pebble requested prices for exchange " + enabledEx[index].exName + ". Sending prices now.");

  enabledEx[index].priceLookup();
}

Pebble.addEventListener("ready",
  function(e) {
    console.log("Wrist Coin is ready.");
    console.log("Enabling test exchanges.");
    loadEnabledExchanges();
  }
);

Pebble.addEventListener("appmessage",
  function(e) {
    console.log("Received a message from the watch.");
    console.log(e.type);
    console.log(e.payload.command);
    console.log(e.payload.config);
    console.log(e.payload.exIndex);

    if (e.payload.command == 0) {
      if (e.payload.config == 0) {
        console.log("Pebble requested global configuration information.");
        sendGlobalConfig();
      }

      if (e.payload.config == 1) {
        console.log("Pebble requested exchange configuration information.");
        sendExConfig();
      }
    }

    if (e.payload.command == 1) {
      console.log("Pebble requested prices for an exchange.");
      sendExPrices(e.payload.exIndex);
    }
  }
);
