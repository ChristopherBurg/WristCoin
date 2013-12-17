var messageQueue = [];

Pebble.addEventListener("ready",
    function(e) {
        console.log("Wrist Coin is ready.");
    }
);

function addMessageToQueue(message) {
    messageQueue.push(message);
}

function sendMessageFromQueue() {
    console.log("Sending messages. There are " + messageQueue.length + " message currently in the queue.");

    var successHandler = function(e) {
        if (messageQueue.length > 0) {
            sendMessageFromQueue();
        }
    }

    var errorHandler = function(e) {
        console.log("An error occurred sending a message to the phone. The error was: " + e.error.message);
    }

    if (messageQueue.length > 0) {
        Pebble.sendAppMessage(messageQueue.shift(), successHandler, errorHandler);
    }
}

// Gets the curret price list from Bitstamp.
function fetchBitstampPrice() {
    var response;
    var req = new XMLHttpRequest();

    console.log("Fetching Bitstamp prices.");

    req.open("GET", "https://www.bitstamp.net/api/ticker/", false);

    req.onload = function(e) {
        if (req.readyState == 4) {
            if (req.status == 200) {
                console.log(req.responseText);            

                response = JSON.parse(req.responseText);

                var high = response.high;
                var last = response.last;
                var low = response.low;

                addMessageToQueue({"bitstamp" : "1",
                                   "high" : "$" + high.toString(), 
                                   "low" : "$" + low.toString(), 
                                   "last" : "$" + last.toString()
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

    req.send(null);
}

// Gets the current price list from Mt. Gox.
function fetchMtGoxPrice() {
    var req = new XMLHttpRequest();

    var onloadHandler = function(event) {
        console.log("Mt. Gox response received!");

        if (req.readyState == 4) {
            if (req.status == 200) {

                console.log(req.responseText);

                var response = JSON.parse(req.responseText);

                if (response.result == "success") {
                    var high = response.data.high.display;
                    var low = response.data.low.display;
                    var last = response.data.last.display;

                    console.log("High: " + high.toString());
                    console.log("Low: " + low.toString());
                    console.log("Last: " + last.toString());

                    addMessageToQueue({"mtgox" : "1",
                                       "high" : high.toString(),
                                       "low" : low.toString(),
                                       "last" : last.toString()
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
    
    req.open("GET", "https://data.mtgox.com/api/2/BTCUSD/money/ticker", false);

    req.send(null);
}

// Gets the current prices from BTC-e.
function fetchBtcePrice() {
    var req = new XMLHttpRequest();

    var onloadHandler = function(event) {
        console.log("BTC-e response received!");
 
        if (req.readyState == 4) {
            if (req.status == 200) {

                console.log(req.responseText);

                var response = JSON.parse(req.responseText);

                var high = response.ticker.high;
                var low = response.ticker.low;
                var last = response.ticker.last;

                addMessageToQueue({"btce" : "1",
                                   "high" : "$" + high.toString(),
                                   "low" : "$" + low.toString(),
                                   "last" : "$"+ last.toString()
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

    req.open("GET", "https://btc-e.com/api/2/btc_usd/ticker", false);

    req.send(null);
}

Pebble.addEventListener("appmessage",
    function(e) {
        console.log("Received a message from the watch.");
        console.log(e.payload);

        if (e.payload.bitstamp) {
            console.log("Received request to fetch Bitstamp prices.");
            fetchBitstampPrice();
        }
        if (e.payload.mtgox) {
            console.log("Received request to fetch Mt. Gox prices.");
            fetchMtGoxPrice();
        }
        if (e.payload.btce) {
            console.log("Received request to fetch BTC-e prices.");
            fetchBtcePrice();
        }

        sendMessageFromQueue();
    }
);

