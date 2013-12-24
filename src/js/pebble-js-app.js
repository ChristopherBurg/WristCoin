Pebble.addEventListener("ready",
    function(e) {
        console.log("Wrist Coin is ready.");
    }
);

function sendMessageToPebble(message) {
    var successHandler = function(event) {
        console.log("Successfully sent " + event.data.transactionId + " to Pebble.");
    }

    var errorHandler = function(event) {
        console.log("Failed to send " + event.data.transactionId + " to Pebble.");
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

                var response = JSON.parse(req.responseText);

                var high = parseInt(response.high * 100);
                var last = parseInt(response.last * 100);
                var low = parseInt(response.low * 100);
                // Bitstamp doesn't provide an average so I calculate one.
                var average = parseInt((high + low) / 2);
                console.log("Bitstamp average: " + average.toString());

                sendMessageToPebble({"exchange" : 0,
                                     "high" : high, 
                                     "low" : low, 
                                     "last" : last,
                                     "average" : average
                                    });

            } else {
                console.log("HTTP status returned was not 200. Received " + req.status + " instead.");
                addMessageToQueue({"bitstamp" : "1",
                                   "error" : "1",
                                   "errorMessage" : "Failed to get response from Bitstamp server."
                                  });
            }
        } else {
            console.log("Didn't receieve ready status of 4. Receieved " + req.readyStatus + " instead.");
            addMessageToQueue({"bitstamp" : "1",
                               "error" : "1",
                               "errorMessage" : "Failed to connect to Bitstamp server."
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

                var response = JSON.parse(req.responseText);

                if (response.result == "success") {
                    var high = parseInt(response.data.high.value * 100);
                    var low = parseInt(response.data.low.value * 100);
                    var last = parseInt(response.data.last.value * 100);
                    var average = parseInt(response.data.avg.value * 100);

                    console.log("High: " + high);
                    console.log("Low: " + low);
                    console.log("Last: " + last);

                    sendMessageToPebble({"exchange" : 1,
                                         "high" : high,
                                         "low" : low,
                                         "last" : last,
                                         "average" : average
                                        });

                } else {
                    console.log("Mt. Gox API didn't return success. Received " + response.success.toString() + " instead.");
                    addMessageToQueue({"mtgox" : "1",
                                       "error" : "1",
                                       "errorMessage" : "Recived bad response from Mt. Gox server."
                                      });
                }

            } else {
                console.log("HTTP status returned was not 200. Received " + req.status.toString() + " instead.");
                addMessageToQueue({"mtgox" : "1",
                                   "error" : "1",
                                   "errorMessage" : "Failed to get response from Mt. Gox server."
                                  });
            }

        } else {
            console.log("Didn't receieve ready status of 4. Received " + req.readyState.toString() + " instead.");
            addMessageToQueue({"mtgox" : "1",
                               "error" : "1",
                               "errorMessage" : "Failed to connect to Mt. Gox server."
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

                var response = JSON.parse(req.responseText);

                var high = parseInt(response.ticker.high * 100);
                var low = parseInt(response.ticker.low * 100);
                var last = parseInt(response.ticker.last * 100);
                var average = parseInt(response.ticker.avg * 100);

                sendMessageToPebble({"exchange" : 2,
                                     "high" : high,
                                     "low" : low,
                                     "last" : last,
                                     "average" : average
                                    });

            } else {
                console.log("HTTP status returned was not 200. Received " + req.status.toString() + " instead.");
                addMessageToQueue({"btce" : "1",
                                   "error" : "1",
                                   "errorMessage" : "Failed to get response from BTC-e server."
                                  });
            }
        } else {
            console.log("Didn't received ready status of 4. Received " + req.readyState.toString() + " instead.");
            addMessageToQueue({"btce" : "1",
                               "error" : "1",
                               "errorMessage" : "Failed to connect to BTC-e server."
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
        if (e.payload.resendFailed) {
            console.log("Pebble asked me to resend failed messages.");
            while (failedMessageQueue.length > 0) {
                var message = failedMessageQueue.shift();
                sendMessageToPebble(message); 
            }
        }
    }
);

