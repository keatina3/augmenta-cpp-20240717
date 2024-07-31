#pragma once

#include <BookSide.h>
#include <OrderBuffer.h>
#include <cstdint>
#include <string>
#include <vector>

class OrderBook {
  std::vector<BookSide> _sides = {BookSide(), BookSide()};

  enum Side : std::uint8_t {
    Bid = 0,
    Ask = 1,
  };

 public:
  Side static ToSide(std::string const& side) { return Side::Bid; }
  BookSide& get_side(std::string const& side) {
    return _sides[OrderBook::ToSide(side)];
  }

  std::vector<BookSide>& sides() { return _sides; }
  const std::vector<BookSide>& sides() const { return _sides; }

 public:
  // TODO: why are the getters not returning references?
  Order& emplace(Order&& order) {
    // TODO: maybe I can use a if constexpr to make use of a tuple instead
    // TODO: do the correct wrapping for perfect forwarding
    auto& side = _sides[OrderBook::ToSide(order.side())];
    // TODO: sort the return value here
    return side.emplace(std::forward<Order>(order));
  }
};
