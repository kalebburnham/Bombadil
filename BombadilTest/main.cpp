//
//  main.cpp
//  BombadilTest
//
//  Created by Kaleb Burnham on 5/27/22.
//

//main.cpp
#include <gtest/gtest.h>

//#include "../Bombadil/Library.hpp"

TEST(BookTests, InsertOneBid) {
    Book book = Book("USD", "BTC");
    book.add_bid(100.0, 1.0);
    
    std::map<double, double> bids;
    bids.insert(std::pair<int, int>(100, 1));
    
    EXPECT_EQ(book.bids, bids);
}

TEST(BookTests, InsertTwoBids) {
    Book book = Book("USD", "BTC");
    book.add_bid(100.0, 1.0);
    book.add_bid(200.0, 2.0);
    
    std::map<double, double> bids;
    bids.insert(std::pair<int, int>(100, 1));
    bids.insert(std::pair<int, int>(200, 2));
    
    EXPECT_EQ(book.bids, bids);
}

TEST(BookTests, InsertAndRemoveBid) {
    Book book = Book("USD", "BTC");
    book.add_bid(100.0, 1.0);
    book.remove_bid(100);
    
    std::map<double, double> bids;
    
    EXPECT_EQ(book.bids, bids);
}

TEST(BookTests, InsertOneAsk) {
    Book book = Book("USD", "BTC");
    book.add_ask(100.0, 1.0);
    
    std::map<double, double> asks;
    asks.insert(std::pair<int, int>(100, 1));
    
    EXPECT_EQ(book.asks, asks);
}

TEST(BookTests, InsertTwoAsks) {
    Book book = Book("USD", "BTC");
    book.add_ask(100.0, 1.0);
    book.add_ask(200.0, 2.0);
    
    std::map<double, double> asks;
    asks.insert(std::pair<int, int>(100, 1));
    asks.insert(std::pair<int, int>(200, 2));
    
    EXPECT_EQ(book.asks, asks);
}

TEST(BookTests, InsertAndRemoveAsk) {
    Book book = Book("USD", "BTC");
    book.add_ask(100.0, 1.0);
    book.remove_ask(100);
    
    std::map<double, double> asks;
    
    EXPECT_EQ(book.asks, asks);
}

TEST(LibraryTests, CheckoutAndCheckinBook) {
    
}

TEST(LibraryTests, CheckoutIsExclusive) {
    
}

TEST(LibraryTests, ContainsBook) {
    
}

TEST(LibraryTests, DoesNotContainBook) {
    
}


int main(int argc, const char * argv[]) {
    testing::InitGoogleTest(&argc, (char**)argv);
    return RUN_ALL_TESTS();
}
