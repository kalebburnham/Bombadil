//
//  main.cpp
//  Bombadil
//
//  Created by Kaleb Burnham on 5/13/22.
//

#include <iostream>
#include <chrono>
#include <thread>

#include "Library.hpp"
#include "clients.cpp"
#include "arbitrage.hpp"

bool streaming_is_enabled = true;

void analyze(std::string s, triarb* arb) {
    std::thread t2(&triarb::start, arb);
    std::cout << "Kicked off analyzer. Detaching thread..." << std::endl;
    t2.detach();
    //arb->start();
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

void stream(std::string s, coinbase_client* cb) {
    std::thread t1(&coinbase_client::stream, cb);
    std::cout << "Kicked off. Detaching thread..." << std::endl;
    t1.detach();
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
    binanceClient.connect();
    coinbase_client client{ioc, ctx};
    //binanceClient.stream();
    
    Library* ob_library = new Library();
    client.library = ob_library;

    triarb* arb = new triarb();
    arb->library = ob_library;
    arb->books[0] = make_pair("ETH", "USD");
    arb->books[1] = make_pair("ETH", "BTC");
    arb->books[2] = make_pair("BTC", "USD");
    
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
