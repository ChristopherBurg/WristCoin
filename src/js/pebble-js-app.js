Pebble.addEventListener("ready",
    function(e) {
        console.log("Wrist Coin is ready.");
    }
);

// Gets the curret price list from Bitstamp.
function fetchBitstampPrice() {
    var response;
    var req = new XMLHttpRequest();

    console.log("Fetching Bitstamp prices.");

    req.open("GET", "https://www.bitstamp.net/api/ticker/", true);

    req.onload = function(e) {
        if (req.readyState == 4) {
            if (req.status == 200) {
                console.log(req.responseText);            

                response = JSON.parse(req.responseText);

                var high = response.high;
                var last = response.last;
                var low = response.low;

                Pebble.sendAppMessage({"bitstamp" : "1",
                                       "bitstampHigh" : "$" + high.toString(), 
                                       "bitstampLow" : "$" + low.toString(), 
                                       "bitstampLast" : "$" + last.toString()
                                      });

            } else {
                console.log("HTTP status returned was not 200. Received " + req.status + " instead.");
                Pebble.sendAppMessage({"bitstamp" : "1",
                                       "bitstampError" : "1",
                                       "bitstampErrorMessage" : "Failed to get response from Bitstamp server."
                                      });
            }
        } else {
            console.log("Didn't receieve ready status of 4. Receieved " + req.readyStatus + " instead.");
            Pebble.sendAppMessage({"bitstamp" : "1",
                                   "bitstampError" : "1",
                                   "bitstampErrorMessage" : "Failed to connect to Bitstamp server."
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

                    Pebble.sendAppMessage({"mtgox" : "1",
                                           "mtgoxHigh" : high.toString(),
                                           "mtgoxLow" : low.toString(),
                                           "mtgoxLast" : last.toString()
                                          });

                } else {
                    console.log("Mt. Gox API didn't return success. Received " + response.success.toString() + " instead.");
                    Pebble.sendAppMessage({"mtgox" : "1",
                                           "mtgoxError" : "1",
                                           "mtgoxErrorMessage" : "Recived bad response from Mt. Gox server."
                                          });
                }

            } else {
                console.log("HTTP status returned was not 200. Received " + req.status.toString() + " instead.");
                Pebble.sendAppMessage({"mtgox" : "1",
                                       "mtgoxError" : "1",
                                       "mtgoxErrorMessage" : "Failed to get response from Mt. Gox server."
                                      });
            }

        } else {
            console.log("Didn't receieve ready status of 4. Received " + req.readyState.toString() + " instead.");
            Pebble.sendAppMessage({"mtgox" : "1",
                                   "mtgoxError" : "1",
                                   "mtgoxErrorMessage" : "Failed to connect to Mt. Gox server."
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
        console.log("BTC-e response received!");
 
        if (req.readyState == 4) {
            if (req.status == 200) {

                console.log(req.responseText);

                var response = JSON.parse(req.responseText);

                var high = response.ticker.high;
                var low = response.ticker.low;
                var last = response.ticker.last;

                Pebble.sendAppMessage({"btce" : "1",
                                       "btceHigh" : high.toString(),
                                       "btceLow" : low.toString(),
                                       "btceLast" : last.toString()
                                      });

            } else {
                console.log("HTTP status returned was not 200. Received " + req.status.toString() + " instead.");
                Pebble.sendAppMessage({"btce" : "1",
                                       "btceError" : "1",
                                       "btceErrorMessage" : "Failed to get response from BTC-e server."
                                      });
            }
        } else {
            console.log("Didn't received ready status of 4. Received " + req.readyState.toString() + " instead.");
            Pebble.sendAppMessage({"btce" : "1",
                                   "btceError" : "1",
                                   "btceErrorMessage" : "Failed to connect to BTC-e server."
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
    }
);

