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
                var high, last, bid, low, ask;

                high = response.high;
                last = response.last;
                bid = response.bid;
                low = response.low;
                ask = response.ask;

                Pebble.sendAppMessage({"bitstamp" : "1",
                                       "bitstampHigh" : "$" + high.toString(), 
                                       "bitstampLow" : "$" + low.toString(), 
                                       "bitstampLast" : "$" + last.toString()
                                      });

            } else {
                console.log("HTTP status returned was not 200. Received " + req.status + " instead.");
            }
        } else {
            console.log("Didn't receieve ready status of 4. Receieved " + req.readyStatus + " instead.");
        }
    }

    req.send(null);
}

// Gets the current price list from Mt. Gox.
function fetchMtGox() {
    var response;
    var req = new XMLHttpRequest();

    console.log("Fetching Mt. Gox prices.");

    req.open("GET", "https://data.mtgox.com/api/2/BTCUSD/money/ticker", true);

    req.onload = function(e) {
        if (req.readyState == 4) {
            if (req.status == 200) {



            } else {
                console.log("HTTP status returned was not 200. Received " + req.status + " instead.");
            }

        } else {
            console.log("Didn't receieve ready status of 4. Received " + req.readyState + " instead.");
        }

    }

}

Pebble.addEventListener("appmessage",
    function(e) {
        console.log("Received a message from the watch.");
        console.log(e.payload);

//        if (e.payload.fetch) {
//            console.log("Received request to fetch Bitstamp prices.");
//            fetchBitstampPrice();
//        }
        if (e.payload.bitstamp) {
            console.log("Received request to fetch Bitstamp prices.");
            fetchBitstampPrice();
        }
    }
);

