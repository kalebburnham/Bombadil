//
//  arbitrage.cpp
//  Bombadil
//
//  Created by Kaleb Burnham on 5/30/22.
//

#include "arbitrage.hpp"

triarb::triarb() {

}

void
triarb::start() {
    std::thread t(&triarb::analyze, this);
    std::cout << "Kicked off. Detaching thread..." << std::endl;
    t.detach();
}

void
triarb::stop() {
    continue_analyzing = false;
}

void
triarb::analyze() {
    try {
        continue_analyzing = true;
        
        // Conditional variables may be able to replace the constant loop.
        // One can be set to only run this block when an update to one of the books occurs.
        while (continue_analyzing) {
            Book* b = library->checkout(books[0].first, books[0].second);
            pair<double, double> best_ask0 = b->best_ask();
            library->checkin(books[0].first, books[0].second);
            
            b = library->checkout(books[1].first, books[1].second);
            pair<double, double> best_ask1 = b->best_ask();
            library->checkin(books[1].first, books[1].second);
            
            b = library->checkout(books[2].first, books[2].second);
            pair<double, double> best_ask2 = b->best_ask();
            library->checkin(books[2].first, books[2].second);
            
            
            if ((((1 / best_ask0.first) * best_ask1.first * best_ask2.first) - 1) * 100 > _tmp_best_ask) {
                _tmp_best_ask = (((1 / best_ask0.first) * best_ask1.first * best_ask2.first) - 1) * 100;
            }
            std::cout << (((1 / best_ask0.first) * best_ask1.first * best_ask2.first) - 1) * 100 << "%" << " Best: " << _tmp_best_ask << "%" << std::endl;
        }
    } catch (exception e) {
        std::cout << e.what() << std::endl;
    }
}
