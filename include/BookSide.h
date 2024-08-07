#pragma once

#include <Order.h>
#include <algorithm>
#include <list>
#include <map>
#include <string>
#include <utility>

constexpr size_t NUM_ENTRIES = 1000000;

class BookSide {
 public:
  using allocator_t = Allocator<Order, NUM_ENTRIES>;
  using order_list_t = std::list<Order, allocator_t>;

  class OrderSet {
   public:
    OrderSet(uint qty, allocator_t& alloc) : _qty(qty), _orders(alloc) {}
    uint _qty = 0;
    order_list_t _orders;
  };

  using order_heap_t = std::map<std::string, OrderSet>;

 private:
  allocator_t _alloc;
  order_heap_t _orders_per_company = order_heap_t();

 public:
  BookSide(allocator_t& alloc) : _alloc(alloc){};

  Order& emplace(Order&& order) {
    auto insert =
        _orders_per_company.try_emplace(order.company(), order.qty(), _alloc);

    bool inserted = insert.second;
    OrderSet& order_set = insert.first->second;

    if (not inserted) {
      order_set._qty += order.qty();
    }

    order_list_t& orders = order_set._orders;
    Order& order_r = orders.emplace_back(order);

    return order_r;
  }

  void erase(const std::string& order_id) {
    // erase_if will be handy for this in the C++20

    for (auto& order_set : _orders_per_company) {
      order_list_t& orders = order_set.second._orders;

      auto iter = std::remove_if(
          orders.begin(), orders.end(),
          [order_id](Order& order) { return order.orderId() == order_id; });
      if (iter != orders.end()) {
        orders.erase(iter, orders.end());
        // need to break as iterator becomes invalid
        break;
      }
    }
  }

  order_heap_t& orders() { return _orders_per_company; }
  const order_heap_t& orders() const { return _orders_per_company; }
};
