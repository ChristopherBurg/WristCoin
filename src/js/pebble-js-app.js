/* Takes a hexadecimal string and converts it to a byte array. This function is
   primarily used as a way to send numbers larger than 32-bit to the Pebble.
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

Pebble.addEventListener("ready",
    function(e) {
        console.log("Wrist Coin is ready.");
    }
);

function sendMessageToPebble(message) {
    var successHandler = function(event) {
        console.log("Successfully sent " + event.data.transactionId + " to Pebble.");
    }

    /* The Pebble can only process one message at a time. Since message come in
       as they arrive from the exchange they can oftentimes send before a 
       previous message has complete processing. To work around this any
       message that fails to send will cause the application to wait one second
       for the message buffer to clear before resending the message.

       It's not pretty or elegant but it works.
    */
    var errorHandler = function(event) {
        console.log("Failed to send " + event.data.transactionId + " to Pebble.");
        console.log("Error message for " + event.data.transactionId + " was " + event.error + ".");
        setTimeout(sendMessageToPebble(message), 1000);
    }

    console.log("Sending message to Pebble");
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

                sendMessageToPebble({"exchange" : 0,
                                     "high" : high, 
                                     "low" : low, 
                                     "last" : last,
                                     "average" : average,
                                     "buy" : buy,
                                     "sell" : sell,
                                     "volume" : volume_bytes
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

// Gets the current price list from Mt. Gox.
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

                var volume = parseInt(response.ticker.vol * 100000000).toString(16);
                var volume_bytes = convertHexStringToByteArray(volume);

                console.log(volume);

                sendMessageToPebble({"exchange" : 2,
                                     "high" : high,
                                     "low" : low,
                                     "last" : last,
                                     "average" : average,
                                     "buy" : buy,
                                     "sell" : sell,
                                     "volume" : volume_bytes
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

Pebble.addEventListener("appmessage",
    function(e) {
        console.log("Received a message from the watch.");
        console.log(e.payload.exchange);

        if (e.payload.fetch) {
            console.log("Received request to fetch prices from exchanges.");
            fetchBitstampPrice();
            fetchMtGoxPrice();
            fetchBtcePrice();
        }
    }
);

