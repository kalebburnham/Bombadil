//
//  Book.cpp
//  Bombadil
//
//  Created by Kaleb Burnham on 5/20/22.
//

#include "Library.hpp"

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

Library::Library() {
    
}

void Library::create_book(string base, string target) {
    Book b = Book(base, target);
    books.emplace(pair<string, string>(base, target), b);
}

Book* Library::checkout(string base, string target) {
    if (!books.at(pair<string, string>(base, target)).isCheckedOut) {
        books.at(pair<string, string>(base, target)).isCheckedOut = true;
        return &books.at(pair<string, string>(base, target));
    }
    
    return nullptr;
}

void Library::checkin(string base, string target) {
    books.at(pair<string, string>(base, target)).isCheckedOut = false;
}

bool Library::contains(string base, string target) {
    return books.count(pair<string, string>(base, target));
}
