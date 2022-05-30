//
//  clients.cpp
//  Bombadil
//
//  Created by Kaleb Burnham on 5/28/22.
//

#include "clients.hpp"

coinbase_client::coinbase_client(boost::asio::io_context &ioc, ssl::context &ctx) : ws(ioc, ctx) {
    boost::asio::ip::tcp::resolver resolver{ioc};
    connect(resolver);
}

inline void coinbase_client::connect(boost::asio::ip::tcp::resolver& resolver) {
    /* Performs the websocket handshake. Still need to write the subscription message. */

    // Look up the domain name
    auto const results = resolver.resolve(BASE_URL, PORT);

    // Make the connection on the IP address we get from a lookup
    auto ep = boost::asio::connect(get_lowest_layer(ws), results);

    // Set SNI Hostname (many hosts need this to handshake successfully)
    if(! SSL_set_tlsext_host_name(ws.next_layer().native_handle(), BASE_URL.c_str()))
        throw boost::beast::system_error(
                                         boost::beast::error_code(
                static_cast<int>(::ERR_get_error()),
                boost::asio::error::get_ssl_category()),
            "Failed to set SNI Hostname");

    // Update the host_ string. This will provide the value of the
    // Host HTTP header during the WebSocket handshake.
    // See https://tools.ietf.org/html/rfc7230#section-5.4
    string host = BASE_URL + ':' + to_string(ep.port());

    // Perform the SSL handshake
    ws.next_layer().handshake(ssl::stream_base::client);

    // Set a decorator to change the User-Agent of the handshake
    ws.set_option(boost::beast::websocket::stream_base::decorator(
                                                                  [](boost::beast::websocket::request_type& req)
        {
            req.set(boost::beast::http::field::user_agent,
                string(BOOST_BEAST_VERSION_STRING) +
                    " websocket-client-coro");
        }));

    // Perform the websocket handshake
    ws.handshake(BASE_URL, "/");
}

/*
 {
   "type": "l2update",
   "product_id": "BTC-USD",
   "time": "2019-08-14T20:42:27.265Z",
   "changes": [
     [
       "buy",
       "10101.80000000",
       "0.162567"
     ]
   ]
 }
 {
   "type": "snapshot",
   "product_id": "BTC-USD",
   "bids": [["10101.10", "0.45054140"]],
   "asks": [["10102.55", "0.57753524"]]
 }
 
 https://docs.cloud.coinbase.com/exchange/docs/websocket-channels
 */
inline void coinbase_client::parse(string s) {
    string base = "";
    string target = "";
         
    boost::json::value data = boost::json::parse(s);
    
    string type = boost::json::value_to<string>(data.at("type"));
    
    if (type == "snapshot") {
        string product_id = boost::json::value_to<string>(data.at("product_id"));
        boost::json::array bids = data.at("bids").as_array();
        boost::json::array asks = data.at("asks").as_array();
        
        library->create_book(get_base(product_id), get_target(product_id));
        
        Book* b = library->checkout(get_base(product_id), get_target(product_id));
        
        for (int i = 0; i < bids.size(); i++) {
            double price;
            double quantity;
            
            price = stod(boost::json::value_to<string>(bids.at(i).as_array().at(0)));
            quantity = stod(boost::json::value_to<string>(bids.at(i).as_array().at(1)));
            
            b->add_bid(price, quantity);
        }
        
        for (int i = 0; i < asks.size(); i++) {
            double price;
            double quantity;
            
            price = stod(boost::json::value_to<string>(asks.at(i).as_array().at(0)));
            quantity = stod(boost::json::value_to<string>(asks.at(i).as_array().at(1)));
            
            b->add_ask(price, quantity);
        }
        
        library->checkin(get_base(product_id), get_target(product_id));
        
    } else if (type == "l2update") {
        
        string action = boost::json::value_to<string>(data.at("changes").as_array().at(0).as_array().at(0));
        string product_id = boost::json::value_to<string>(data.at("product_id"));
        double price = stod(boost::json::value_to<string>(data.at("changes").as_array().at(0).as_array().at(1)));

        double quantity = stod(boost::json::value_to<string>(data.at("changes").as_array().at(0).as_array().at(2)));
        
        Book* b = library->checkout(get_base(product_id), get_target(product_id));
        if (action == "buy") {
            b->add_bid(price, quantity);
        } else {
            b->add_ask(price, quantity);
        }
        library->checkin(get_base(product_id), get_target(product_id));
    }
}

inline void coinbase_client::write(string text) {
    ws.write(boost::asio::buffer(text));
}

inline void coinbase_client::stream() {
    allowStreaming = true;
    write("{\"type\": \"subscribe\",\"product_ids\": [\"ETH-USD\",\"ETH-EUR\"], \"channels\":[\"level2\"]}");
    listen();
}

inline void coinbase_client::stop_stream() {
    allowStreaming = false;
}

inline void coinbase_client::listen() {
    // This buffer will hold the incoming message
    boost::beast::flat_buffer buffer;

    while (allowStreaming) {
        // Read a message into our buffer
        ws.read(buffer);

        parse(boost::beast::buffers_to_string(buffer.data()));
        // The make_printable() function helps print a ConstBufferSequence
        //cout << boost::beast::make_printable(buffer.data()) << endl;
        buffer.consume(buffer.size());
    }
    
    // Close the WebSocket connection
    boost::beast::error_code ec;

    // If this line throws an exception due to a truncated stream, ignore the error.
    // https://github.com/boostorg/beast/issues/824
    ws.close(boost::beast::websocket::close_code::normal, ec);

    // If we get here then the connection is closed gracefully
    cout << "Connection closed.\n";
}

string coinbase_client::get_base(string ticker) {
    size_t idx = ticker.find("-");
    return ticker.substr(0, idx);
}

string coinbase_client::get_target(string ticker) {
    size_t idx = ticker.find("-");
    return ticker.substr(idx+1, ticker.size() - (idx+1));
}
