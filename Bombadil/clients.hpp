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
#include <boost/bind.hpp>
#include <boost/asio/placeholders.hpp>

#include <boost/json/src.hpp>
#include <boost/json.hpp>
#include <boost/json/value.hpp>

#include "Library.hpp"

using namespace std;

class client {
public:
    
    const string BASE_URL = "stream.binance.com";
    const string PORT = "9443";
    bool allowStreaming = false;
    
    Library* library;
    
    boost::beast::websocket::stream<boost::beast::ssl_stream<boost::asio::ip::tcp::socket>> ws;
    boost::asio::io_context io;
    
    client(boost::asio::io_context &ioc, ssl::context &ctx);
    void connect();
    void disconnect();
    void stop_stream();
    void write(string message);
    void listen();
    
private:
    boost::asio::ip::tcp::resolver::results_type resolve_ip_addresses();
    void connect_to_ip_addresses(boost::asio::ip::tcp::resolver::results_type endpoints);
    void set_sni_hostname();
    void perform_ssl_handshake();
    virtual string get_host(string exchange);
    virtual string get_port(string exchange);
    
    virtual void perform_websockets_handshake();
    virtual void parse(string s);
};

class coinbase_client : public client {
public:
    coinbase_client(boost::asio::io_context &ioc, ssl::context &ctx);
    void parse(string s);
    void stream();
    
private:
    void perform_websockets_handshake();
    string get_base(string ticker);
    string get_target(string ticker);
    string get_host(string exchange);
    string get_port(string exchange);
};

class binance_client : public client {
public:
    binance_client(boost::asio::io_context &ioc, ssl::context &ctx);
    void parse(string s);
    string getDepthSnapshot(string ticker);
    void stream();
    
private:
    void perform_websockets_handshake();
    string get_base(string ticker);
    string get_target(string ticker);
    string get_host(string exchange);
    string get_port(string exchange);
};


#endif /* clients_hpp */
