//
//  Book.hpp
//  Bombadil
//
//  Created by Kaleb Burnham on 5/20/22.
//

#ifndef Library_hpp
#define Library_hpp

#include <stdio.h>
#include <string>
#include <map>
#include <mutex>
#include <list>

using namespace std;

class Book {
public:
    long long updateId;
    
    string base;
    string target;
    map<double, double> bids;
    map<double, double> asks;
    
    Book(string base, string target);
    void add_ask(double price, double quantity);
    void add_bid(double price, double quantity);
    
    unsigned long remove_ask(double price);
    unsigned long remove_bid(double price);
    
    pair<double, double> best_bid();
    pair<double, double> best_ask();
};

// Handles checkin/checkout functionality of the order books.
class Library {
public:
    
    // Mutexes to control book access.
    map<pair<string, string>, std::unique_ptr<std::mutex>> mutexes;
    map<pair<string, string>, Book> books;

    Library();
    
    void create_book(string base, string target);
    
    Book* checkout(string base, string target);
    
    void checkin(string base, string target);
    
    bool contains(string base, string target);
};

#endif /* Library_hpp */
