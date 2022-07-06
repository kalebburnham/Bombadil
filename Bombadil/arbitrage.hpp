//
//  arbitrage.hpp
//  Bombadil
//
//  Created by Kaleb Burnham on 5/30/22.
//

#ifndef arbitrage_hpp
#define arbitrage_hpp

#include <stdio.h>
#include <iostream>

#include "Library.hpp"

using namespace std;

class triarb {
public:
    Library* library;
    bool continue_analyzing = false;
    double _tmp_best_ask = 0;
    
    pair<string, string> books[3];
    triarb();
    void start();
    void stop();
};

#endif /* arbitrage_hpp */
