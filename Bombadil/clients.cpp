//
//  clients.cpp
//  Bombadil
//
//  Created by Kaleb Burnham on 5/28/22.
//

#include "clients.hpp"


client::client(boost::asio::io_context &ioc, ssl::context &ctx) : ws(ioc, ctx) {}
void client::connect() {
    connect_to_ip_addresses(resolve_ip_addresses());
    set_sni_hostname();
    perform_ssl_handshake();
    perform_websockets_handshake();
}

void client::disconnect() {
    boost::beast::error_code ec;
    // If this line throws an exception due to a truncated stream, ignore the error.
    // https://github.com/boostorg/beast/issues/824
    ws.close(boost::beast::websocket::close_code::normal, ec);
    // If we get here then the connection is closed gracefully
    cout << "Connection closed.\n";
}

void client::write(string text) {
    ws.write(boost::asio::buffer(text));
}

void client::stop_stream() {
    allowStreaming = false;
}

void client::listen() {
    boost::beast::flat_buffer buffer;

    while (allowStreaming) {
        ws.read(buffer);
        parse(boost::beast::buffers_to_string(buffer.data()));
        // The make_printable() function helps print a ConstBufferSequence
        //cout << boost::beast::make_printable(buffer.data()) << endl;
        buffer.consume(buffer.size());
    }
    
    disconnect();
}

boost::asio::ip::tcp::resolver::results_type client::resolve_ip_addresses() {
    cout << "Looking up the domain name..." << endl;
    boost::asio::ip::tcp::resolver resolver{io};
    // If you error here, double check you are connected to internet.
    boost::asio::ip::tcp::resolver::results_type results = resolver.resolve(get_host("test"), get_port("test"));
    return results;
}

void client::connect_to_ip_addresses(boost::asio::ip::tcp::resolver::results_type endpoints) {
    boost::asio::connect(get_lowest_layer(ws), endpoints);
}

void client::set_sni_hostname() {
    cout << "Setting SNI host names..." << endl;
    if (!SSL_set_tlsext_host_name(ws.next_layer().native_handle(),
                                          get_host("test").c_str()))
        throw boost::beast::system_error(boost::beast::error_code(static_cast<int>(::ERR_get_error()),
                                         boost::asio::error::get_ssl_category()),
                                         "Failed to set SNI Hostname");
}

void client::perform_ssl_handshake() {
    cout << "Performing the SSL handshake..." << endl;
    ws.next_layer().handshake(ssl::stream_base::client);
    ws.set_option(boost::beast::websocket::stream_base::decorator(
                                                                  [](boost::beast::websocket::request_type& req)
        {
            req.set(boost::beast::http::field::user_agent,
                string(BOOST_BEAST_VERSION_STRING) +
                    " websocket-client-coro");
        }));
}

void client::perform_websockets_handshake() {}
void client::parse(string s) {}
string client::get_host(string exchange) { return ""; }
string client::get_port(string exchange) { return ""; }


/* *************************************************************** */

/* *************************************************************** */


coinbase_client::coinbase_client(boost::asio::io_context &ioc, ssl::context &ctx) : client(ioc, ctx) {
    
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
void coinbase_client::parse(string s) {
    string base = "";
    string target = "";
    
    std::cout<< s << std::endl;
         
    boost::json::value data = boost::json::parse(s);
    
    string type = boost::json::value_to<string>(data.at("type"));
    
    if (type == "snapshot") {
        string product_id = boost::json::value_to<string>(data.at("product_id"));
        boost::json::array bids = data.at("bids").as_array();
        boost::json::array asks = data.at("asks").as_array();
        
        library->create_book(get_base(product_id), get_target(product_id));
        
        Book* b = library->checkout(get_base(product_id), get_target(product_id));
        
        for (int i = 0; i < bids.size(); i++) {
            double price = stod(boost::json::value_to<string>(bids.at(i).as_array().at(0)));
            double quantity = stod(boost::json::value_to<string>(bids.at(i).as_array().at(1)));
            if (quantity == 0) {
                b->remove_bid(price);
            } else {
                b->add_bid(price, quantity);
            }
        }
        
        for (int i = 0; i < asks.size(); i++) {            
            double price = stod(boost::json::value_to<string>(asks.at(i).as_array().at(0)));
            double quantity = stod(boost::json::value_to<string>(asks.at(i).as_array().at(1)));
            if (quantity == 0) {
                b->remove_ask(price);
            } else {
                b->add_ask(price, quantity);
            }
        }
        
        library->checkin(get_base(product_id), get_target(product_id));
        
    } else if (type == "l2update") {
        
        string action = boost::json::value_to<string>(data.at("changes").as_array().at(0).as_array().at(0));
        string product_id = boost::json::value_to<string>(data.at("product_id"));
        double price = stod(boost::json::value_to<string>(data.at("changes").as_array().at(0).as_array().at(1)));

        double quantity = stod(boost::json::value_to<string>(data.at("changes").as_array().at(0).as_array().at(2)));
        
        Book* b = library->checkout(get_base(product_id), get_target(product_id));
        if (action == "buy") {
            if (quantity == 0) {
                b->remove_bid(price);
            } else {
                b->add_bid(price, quantity);
            }
        } else {
            if (quantity == 0) {
                b->remove_ask(price);
            } else {
                b->add_ask(price, quantity);
            }
        }
        library->checkin(get_base(product_id), get_target(product_id));
    }
}

inline void coinbase_client::stream() {
    allowStreaming = true;
    try {
        write("{\"type\": \"subscribe\",\"product_ids\": [\"ETH-USD\",\"ETH-BTC\", \"BTC-USD\"], \"channels\":[\"level2\"]}");
        
        std::thread t(&coinbase_client::listen, this);
        std::cout << "Kicked off. Detaching thread..." << std::endl;
        t.detach();
        //listen();
    } catch (std::exception const& e) {
        allowStreaming = false;
        std::cerr << "Error when kicking off streaming. Did you connect?" << std::endl;
        std::cerr << e.what() << std::endl;
    }
    
}

void coinbase_client::perform_websockets_handshake() {
    cout << "Performing the websockets handshake..." << endl;
    ws.handshake(get_host("coinbase"), "/");
}

string coinbase_client::get_base(string ticker) {
    size_t idx = ticker.find("-");
    return ticker.substr(0, idx);
}

string coinbase_client::get_target(string ticker) {
    size_t idx = ticker.find("-");
    return ticker.substr(idx+1, ticker.size() - (idx+1));
}

string coinbase_client::get_host(string exchange) {
    return "ws-feed.exchange.coinbase.com";
}

string coinbase_client::get_port(string exchange) {
    return "443";
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
///

binance_client::binance_client(boost::asio::io_context &ioc, ssl::context &ctx) : client(ioc, ctx) {

}

inline void binance_client::parse(string s) {
    // How to manage a local order book correctly
    // Open a stream to wss://stream.binance.com:9443/ws/bnbbtc@depth.
    // Buffer the events you receive from the stream.
    // Get a depth snapshot from https://api.binance.com/api/v3/depth?symbol=BNBBTC&limit=1000 .
    // Drop any event where u is <= lastUpdateId in the snapshot.
    // The first processed event should have U <= lastUpdateId+1 AND u >= lastUpdateId+1.
    // While listening to the stream, each new event's U should be equal to the previous event's u+1.
    // The data in each event is the absolute quantity for a price level.
    // If the quantity is 0, remove the price level.
    // Receiving an event that removes a price level that is not in your local order book can happen and is normal.
    cout << s << endl;
    
    string base = "";
    string target = "";
         
    boost::json::value data = boost::json::parse(s);
    // If the message looks like this, break and move on.
    // {"result":null,"id":1}
    string event = boost::json::value_to<string>(data.at("e"));
    
    if (event == "depthUpdate") {
        // A complete replica of the local order book for the current price.
        string symbol = boost::json::value_to<string>(data.at("s"));
        int first_update_id = boost::json::value_to<int>(data.at("U"));
        int last_update_id = boost::json::value_to<int>(data.at("u"));
        boost::json::array bids = data.at("b").as_array();
        boost::json::array asks = data.at("a").as_array();
        
        Book* b = library->checkout(get_base(symbol), get_target(symbol));
        
        for (int i = 0; i < bids.size(); i++) {
            double price = stod(boost::json::value_to<string>(bids.at(i).as_array().at(0)));
            double quantity = stod(boost::json::value_to<string>(bids.at(i).as_array().at(1)));
            if (quantity == 0) {
                b->remove_bid(price);
            } else {
                b->add_bid(price, quantity);
            }
        }
        
        for (int i = 0; i < asks.size(); i++) {
            double price = stod(boost::json::value_to<string>(asks.at(i).as_array().at(0)));
            double quantity = stod(boost::json::value_to<string>(asks.at(i).as_array().at(1)));
            if (quantity == 0) {
                b->remove_ask(price);
            } else {
                b->add_ask(price, quantity);
            }
        }
        
        library->checkin(get_base(symbol), get_target(symbol));
    }
}

inline void binance_client::stream() {
// See this for keeping track of an orderbook.
// https://binance-docs.github.io/apidocs/spot/en/#how-to-manage-a-local-order-book-correctly
    // Binance is trickier because we need to subscribe, then
    
    // Detaching the thread may prove a problem in the future. One alternative
    // would be to pass in the thread that should be used as an argumnet.
    allowStreaming = true;
    cout << "Turning streaming on" << endl;
    try {
        write("{\"method\": \"SUBSCRIBE\",\"params\":[\"btcusdt@aggTrade\", \"btcusdt@depth\"],\"id\": 1}");
        //write_http();
        std::cout<< "Wrote GET request" << endl;
        std::thread t(&binance_client::listen, this);
        std::cout << "Kicked off. Detaching thread..." << std::endl;
        t.detach();
    } catch (std::exception const& e) {
        allowStreaming = false;
        std::cerr << "Error when kicking off Binance stream. Did you connect?" << std::endl;
        std::cerr << e.what() << std::endl;
    }
}

void binance_client::perform_websockets_handshake() {
    cout << "Performing the websockets handshake..." << endl;
    ws.handshake(get_host("binance") + ":" + get_port("binance"), "/ws");
}

void
binance_client::write_http() {
    // Code copied from
    // https://www.boost.org/doc/libs/master/libs/beast/example/http/client/sync-ssl/http_client_sync_ssl.cpp
    boost::asio::io_context ioc;
    
    std::string port = "443";
    std::string target = "/api/v3/depth?symbol=BNBBTC&limit=1000";
    std::string host = "api.binance.com";
    int version = 11; // HTTP 1.1
    
    // The SSL context is required, and holds certificates
    ssl::context ctx(ssl::context::tlsv12_client);

    // This holds the root certificate used for verification
    load_root_certificates(ctx);
    
    // Verify the remote server's certificate
    ctx.set_verify_mode(ssl::verify_peer);

    // These objects perform our I/O
    boost::asio::ip::tcp::resolver resolver(ioc);
    boost::beast::ssl_stream<boost::beast::tcp_stream> stream(ioc, ctx);

    // Set SNI Hostname (many hosts need this to handshake successfully)
    if(! SSL_set_tlsext_host_name(stream.native_handle(), host.c_str()))
    {
        boost::beast::error_code ec{static_cast<int>(::ERR_get_error()), boost::asio::error::get_ssl_category()};
        throw boost::beast::system_error{ec};
    }

    // Look up the domain name
    auto const results = resolver.resolve(host, port);
    // Make the connection on the IP address we get from a lookup
    boost::beast::get_lowest_layer(stream).connect(results);
    
    // Perform the SSL handshake
    stream.handshake(ssl::stream_base::client);
    
    // Set up an HTTP GET request message
    boost::beast::http::request<boost::beast::http::string_body> req{boost::beast::http::verb::get, target, version};
    req.set(boost::beast::http::field::host, host);
    req.set(boost::beast::http::field::user_agent, BOOST_BEAST_VERSION_STRING);

    // Send the HTTP request to the remote host
    boost::beast::http::write(stream, req);

    // This buffer is used for reading and must be persisted
    boost::beast::flat_buffer buffer;

    // Declare a container to hold the response
    boost::beast::http::response<boost::beast::http::dynamic_body> res;

    // Receive the HTTP response
    boost::beast::http::read(stream, buffer, res);

    // Write the message to standard out
    std::cout << res << std::endl;
    
    // Gracefully close the stream
    boost::beast::error_code ec;
    stream.shutdown(ec);
    if(ec == boost::asio::error::eof)
    {
        // Rationale:
        // http://stackoverflow.com/questions/25587403/boost-asio-ssl-async-shutdown-always-finishes-with-an-error
        ec = {};
    }
    if(ec)
        throw boost::beast::system_error{ec};

    // If we get here then the connection is closed gracefully
}

string binance_client::get_base(string ticker) {
    if (ticker == "BTCUSDT") {
        return "BTC";
    } else {
        throw std::invalid_argument("Ticker not recognized for Binance.");
    }
}

string binance_client::get_target(string ticker) {
    if (ticker == "BTCUSDT") {
        return "USDT";
    } else {
        throw std::invalid_argument("Ticker not recognized for Binance.");
    }
}

string binance_client::get_host(string exchange) {
    return "stream.binance.com";
}

string binance_client::get_port(string exchange) {
    return "9443";
}

/* ******************************************************** */
