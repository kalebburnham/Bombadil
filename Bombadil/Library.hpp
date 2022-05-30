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

using namespace std;

class Book {
public:
    bool isCheckedOut = false;
    string base;
    string target;
    map<double, double> bids;
    map<double, double> asks;
    
    Book(string base, string target);
    void add_ask(double price, double quantity);
    void add_bid(double price, double quantity);
    
    unsigned long remove_ask(double price);
    unsigned long remove_bid(double price);
};

// Handles checkin/checkout functionality of the order books.
class Library {
public:
    
    map<pair<string, string>, Book> books;

    Library();
    
    void create_book(string base, string target);
    
    Book* checkout(string base, string target);
    
    void checkin(string base, string target);
    
    bool contains(string base, string target);
};

#endif /* Library_hpp */
