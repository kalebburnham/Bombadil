//
//  clients.hpp
//  Bombadil
//
//  Created by Kaleb Burnham on 5/28/22.
//

#ifndef clients_hpp
#define clients_hpp

#include <stdio.h>
#include <iostream>

#include "root_certificates.hpp"

#include <boost/beast/core.hpp>
#include <boost/beast/core/buffers_to_string.hpp>
#include <boost/beast/ssl.hpp>
#include <boost/beast/websocket.hpp>
#include <boost/beast/websocket/ssl.hpp>
#include <boost/asio/connect.hpp>
#include <boost/asio/ip/tcp.hpp>
#include <boost/asio/ssl/stream.hpp>

#include <boost/json/src.hpp>
#include <boost/json.hpp>
#include <boost/json/value.hpp>

#include "Library.hpp"

using namespace std;

class coinbase_client {
public:
    
    // CONFIG
    const string BASE_URL = "ws-feed.exchange.coinbase.com";
    const string PORT = "443";
    // END CONFIG
    
    bool allowStreaming = false;
    
    Library* library;
    
    boost::beast::websocket::stream<boost::beast::ssl_stream<boost::asio::ip::tcp::socket>> ws;
    boost::asio::io_context io;
    
    coinbase_client(boost::asio::io_context &ioc, ssl::context &ctx);
    void connect(boost::asio::ip::tcp::resolver& resolver);
    void parse(string s);
    void write(string text);
    
    void stream();
    void stop_stream();
    void listen();
    
private:
    string get_base(string ticker);
    string get_target(string ticker);
};

#endif /* clients_hpp */