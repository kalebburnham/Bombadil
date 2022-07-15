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
        throw boost::beast::system_error(
                                         boost::beast::error_code(static_cast<int>(::ERR_get_error()),
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
        listen();
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
    // TODO
    cout << s << endl;
}

inline void binance_client::stream() {
// See this for keeping track of an orderbook.
// https://binance-docs.github.io/apidocs/spot/en/#how-to-manage-a-local-order-book-correctly
    allowStreaming = true;
    cout << "Turning streaming on" << endl;
    try {
        write("{\"method\": \"SUBSCRIBE\",\"params\":[\"btcusdt@aggTrade\",\"btcusdt@depth\"],\"id\": 1}");
        listen();
    } catch (std::exception const& e) {
        allowStreaming = false;
        std::cerr << "Error when kicking off Binance stream. Did you connect?" << std::endl;
        std::cerr << e.what() << std::endl;
    }
}

string binance_client::get_base(string ticker) {
    size_t idx = ticker.find("-");
    return ticker.substr(0, idx);
}

string binance_client::get_target(string ticker) {
    size_t idx = ticker.find("-");
    return ticker.substr(idx+1, ticker.size() - (idx+1));
}

void binance_client::perform_websockets_handshake() {
    cout << "Performing the websockets handshake..." << endl;
    ws.handshake(get_host("binance") + ":" + get_port("binance"), "/ws");
}

string binance_client::get_host(string exchange) {
    return "stream.binance.com";
}

string binance_client::get_port(string exchange) {
    return "9443";
}
