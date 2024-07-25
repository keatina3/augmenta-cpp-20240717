#pragma once

#include <Order.h>
#include <algorithm>
#include <list>
#include <string>

// TODO: maybe we want to make this entire thing multithreaded at some point
// would have to be very careful with priority of the queues and locks
// defintiely would need wrappers to emplace
// TODO: handle access of members correctly
class BookLevel {
  // TODO: there seems to be no price levels?
};

class BookSide {
 public:
  // TODO: need to set a size for max allowed
  using allocator_t = Allocator<Order, 1024>;
  using order_list_t = std::list<Order, allocator_t>;
  // TODO: need to figure out best way to map from
  // user and sec id
  // TODO: fix access
  // TODO: make this some wrapper class on a vector called circular buffer
  // which allocated memory smartly as needed
  // and also tracks where the next entry can go
  // TODO: find out if this simply creates the allocator itself

  // TODO: make all member variables have an underscore

  allocator_t _alloc = allocator_t();
  order_list_t _orders = order_list_t(_alloc);
  // TODO: maybe these should be iterator positions not indices
  // // or maybe they can be order referentypename Tces? but i guess that
  // doesn't help us remove them from the circular buffer
  // TODO: this might need a custom allocator also
  // Can probably be more simple
  // I'm now wondering if that's going to be faster or slower
  // than the list since it's no longer using idx
  // std::unordered_map<std::string, Order&> order_idx;

 public:
  BookSide() = default;

  Order& emplace(Order&& order) {
    // TODO: what do we do when an order gets cleared?
    // maybe allocating a circular buffer would be better and I mark the indices
    // ask back available for emplacement
    // we can extend the size if num_avail_indices = 0
    // maybe it can be configurable in size
    // I can have a worker thread managing those memory pools
    Order& order_r = _orders.emplace_back(order);
    // TODO: I think this is no longer much use
    // order_idx[order.orderId()] = order_r;

    // TODO: fixme i am broken
    // user_orders[order.user()].emplace(order_r);

    return order_r;
  }

  // TODO: maybe use remove_if
  // TODO: check if i want useful return types
  // TODO: I will definitely want some error management/exception handling
  void erase(const std::string& order_id) {
    // TODO: catch exception
    // erase_if will be handy for this in the future
    order_list_t::iterator iter = std::find_if(
        _orders.begin(), _orders.end(),
        [&order_id](Order& order) { return order.orderId() == order_id; });

    if (iter == _orders.end()) {
      // TODO: throw exception
    }

    _orders.erase(iter);
  }

  order_list_t& orders() { return _orders; }
};
