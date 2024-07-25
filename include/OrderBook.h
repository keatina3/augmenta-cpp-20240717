#pragma once

#include <BookSide.h>
#include <OrderBuffer.h>
#include <cstdint>
#include <string>
#include <vector>

class OrderBook {
  std::vector<BookSide> sides = {BookSide(), BookSide()};

  enum Side : std::uint8_t {
    Bid = 0,
    Ask = 1,
  };

 public:
  Side static ToSide(std::string const& side) { return Side::Bid; }
  BookSide& getSide(std::string const& side) {
    return sides[OrderBook::ToSide(side)];
  }

 public:
  // TODO: why are the getters not returning references?
  Order& emplace(Order&& order) {
    // TODO: maybe I can use a if constexpr to make use of a tuple instead
    // TODO: do the correct wrapping for perfect forwarding
    auto& side = sides[OrderBook::ToSide(order.side())];
    return side.emplace(std::forward<Order>(order));
  }
};
