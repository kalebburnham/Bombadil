//
//  main.cpp
//  Bombadil
//
//  Created by Kaleb Burnham on 5/13/22.
//

#include <iostream>
#include <chrono>
#include <thread>
#include <fstream>

#include <boost/json.hpp>
#include <boost/json/src.hpp>
#include <boost/property_tree/ptree.hpp>
#include <boost/property_tree/json_parser.hpp>
#include <boost/foreach.hpp>

#include "Library.hpp"
#include "clients.cpp"
#include "arbitrage.hpp"

bool streaming_is_enabled = true;

void analyze(std::string s, triarb* arb) {
    arb->start();
}

void connect(std::string s, coinbase_client* cb) {
    cb->connect();
}

void help(std::string s) {
    std::cout << "Available commands are 'analyze', 'connect', 'help', 'stream', 'stop' or 'quit'." << std::endl;
}

void shutdown(std::string s) {
    std::cout << "Shutting down." << std::endl;
    exit(0);
}

void stream(std::string s, coinbase_client* c) {
    //std::thread t1(&coinbase_client::stream, c);
    //std::cout << "Kicked off. Detaching thread..." << std::endl;
    //t1.detach();
    c->stream();
}

void stop(std::string s, coinbase_client* cb, triarb* arb) {
    cb->stop_stream();
    arb->stop();
}

int main() {
    cout << "Welcome!" << endl;
    boost::asio::ssl::context ctx(boost::asio::ssl::context::tlsv12);
    boost::asio::io_context ioc;
    binance_client binanceClient{ioc, ctx};
    //binanceClient.connect();
    coinbase_client client{ioc, ctx};
    //binanceClient.stream();
    
    
    boost::property_tree::ptree pt;
    boost::property_tree::read_json("/Users/kalebburnham/Workspaces/Bombadil/Bombadil/config.json", pt);
    BOOST_FOREACH(boost::property_tree::ptree::value_type &v, pt.get_child("clients")) {
            // The data function is used to access the data stored in a node.
            //m_modules.insert(v.second.data());
        cout << v.second.get_value<string>() << endl;
        
    }
    Library* ob_library = new Library();
    client.library = ob_library;

    triarb* arb = new triarb();
    arb->library = ob_library;
    arb->books[0] = make_pair("ETH", "USD");
    arb->books[1] = make_pair("ETH", "BTC");
    arb->books[2] = make_pair("BTC", "USD");
    
    std::ifstream t("/Users/kalebburnham/Workspaces/Bombadil/Bombadil/config.json");
    string content( (std::istreambuf_iterator<char>(t) ),
                           (std::istreambuf_iterator<char>()    ) );
    
    cout << content << endl;
    
    boost::json::value config = boost::json::parse(content);
    cout << config.at("clients").as_object() << endl;
    
    boost::asio::io_service ios;
    
    
    while (1) {
        std::string input = "";
        std::cin >> input;
        
        if (input == "help" || input == "h") {
            help(input);
        } else if (input == "connect" || input == "c") {
            connect(input, &client);
        } else if (input == "stream" || input == "s") {
            std::cout << "Starting Stream" << std::endl;
            stream(input, &client);
        } else if (input == "analyze" || input == "a") {
            std::cout << "Analyzing..." << std::endl;
            analyze(input, arb);
        } else if (input == "stop") {
            std::cout << "Stopping stream" << std::endl;
            stop(input, &client, arb);
            std::this_thread::sleep_for(std::chrono::milliseconds(500));
        } else if (input == "quit") {
            shutdown(input);
        } else {
            std::cout << "Unrecognized command: " + input << std::endl;
        }
    }

    return 0;
}

