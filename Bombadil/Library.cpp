//
//  Book.cpp
//  Bombadil
//
//  Created by Kaleb Burnham on 5/20/22.
//

#include "Library.hpp"
#include <iostream>

Book::Book(string base, string target) : base(base), target(target) {
    
}

// Returns true if the insertion took place or false if the assignment took place.
void Book::add_ask(double price, double quantity) {
    asks.insert_or_assign(price, quantity);
}

void Book::add_bid(double price, double quantity) {
    bids.insert_or_assign(price, quantity);
}

unsigned long Book::remove_ask(double price) {
    return asks.erase(price);
}

unsigned long Book::remove_bid(double price) {
    return bids.erase(price);
}

pair<double, double> Book::best_bid() {
    return *prev(bids.end());
}

pair<double, double> Book::best_ask() {
    return *asks.begin();
}

Library::Library() {
    
}

void Library::create_book(string base, string target) {
    Book b = Book(base, target);
    // emplace returns std::pair<iterator, bool>
    //mutex m = mutex();
    //mutexes.emplace(<#const_iterator __p#>, <#_Args &&__args...#>)
    //mutexes.emplace(_mutex_idx++, m);
    //b._mutex = &m;
    mutexes.emplace(pair<string, string>(base, target), std::make_unique<std::mutex>());
    books.emplace(pair<string, string>(base, target), b);
}

Book* Library::checkout(string base, string target) {
    
    auto it = mutexes.find(pair<string, string>(base, target));
    if (it == mutexes.end()) {
        throw exception();
    }
    it->second->lock();
    
    try {
        return &books.at(pair<string, string>(base, target));
    } catch (const std::out_of_range& oor) {
        std::cout << "Book not found: " << base << "-" << target << std::endl;
    }
    
    return nullptr;
    //std::cerr << "Attempting to lock" << std::endl;
    //books.at(pair<string, string>(base, target))._mutex->lock();
    //std::cerr << "Locked" << std::endl;
    //return &books.at(pair<string, string>(base, target));
}

void Library::checkin(string base, string target) {
    // Do nothing until a solution is figured out.
    
    //books.at(pair<string, string>(base, target))._mutex->unlock();
    auto it = mutexes.find(pair<string, string>(base, target));
    if (it == mutexes.end()) {
        throw exception();
    }
    it->second->unlock();
}

bool Library::contains(string base, string target) {
    return books.count(pair<string, string>(base, target));
}
