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

bool streaming_is_enabled = true;

void analyze(std::string s) {
    
}

void help(std::string s) {
    std::cout << "Available commands are 'analyze', 'help', 'stream', 'stop' or 'quit'." << std::endl;
}

void shutdown(std::string s) {
    std::cout << "Shutting down." << std::endl;
    exit(0);
}

void stream(std::string s, coinbase_client* cb) {
    std::thread t1(&coinbase_client::stream, cb);
    t1.detach();
}

void stop(std::string s, coinbase_client* cb) {
    cb->stop_stream();
}

int main() {
    boost::asio::ssl::context ctx(boost::asio::ssl::context::tlsv12);
    boost::asio::io_context ioc;
    coinbase_client cb{ioc, ctx};
    
    Library* ob_library = new Library();
    cb.library = ob_library;
    
    while (1) {
        std::string input = "";
        std::cin >> input;
        
        if (input == "help" || input == "h") {
            help(input);
        } else if (input == "stream" || input == "s") {
            std::cout << "Starting Stream" << std::endl;
            stream(input, &cb);
        } else if (input == "analyze") {
            std::cout << "TODO: Analyze" << std::endl;
        } else if (input == "stop") {
            std::cout << "Stopping stream" << std::endl;
            stop(input, &cb);
            std::this_thread::sleep_for(std::chrono::milliseconds(500));
            
            Book* b = ob_library->checkout("ETH", "USD");
            std::cout << "BIDS" << std::endl;
            for (auto const &pair: b->bids) {
                std::cout << "{" << pair.first << ": " << pair.second << "}\n";
            }
            
            std::cout << "\n\n\nASKS" << std::endl;
            for (auto const &pair: b->asks) {
                    std::cout << "{" << pair.first << ": " << pair.second << "}\n";
            }
            ob_library->checkin("ETH","USD");
            
        } else if (input == "quit") {
            shutdown(input);
        } else {
            std::cout << "Unrecognized command: " + input << std::endl;
        }
    }

    return 0;
}
