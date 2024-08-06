#pragma once

#include <BookSide.h>
#include <OrderBuffer.h>
#include <cstdint>
#include <string>
#include <vector>

class OrderBook {
  std::vector<BookSide> _sides;

  enum Side : std::uint8_t {
    Bid = 0,
    Ask = 1,
  };

 public:
  Side static to_side(std::string const& side) {
    return side == "Buy" ? Side::Bid : Side::Ask;
  }
  BookSide& get_side(std::string const& side) {
    return _sides[OrderBook::to_side(side)];
  }

  std::vector<BookSide>& sides() { return _sides; }
  const std::vector<BookSide>& sides() const { return _sides; }

 public:
  OrderBook(BookSide::allocator_t& alloc)
      : _sides({BookSide(alloc), BookSide(alloc)}) {}

  // TODO: why are the getters not returning references?
  Order& emplace(Order&& order) {
    // TODO: maybe I can use a if constexpr to make use of a tuple instead
    // TODO: do the correct wrapping for perfect forwarding
    auto& side = _sides[OrderBook::to_side(order.side())];
    // TODO: sort the return value here
    return side.emplace(std::forward<Order>(order));
  }
};
