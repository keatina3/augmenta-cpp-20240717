#pragma once

#include <Order.h>
#include <OrderBook.h>
#include <OrderBuffer.h>
#include <list>
#include <set>
#include <string>
#include <unordered_map>
#include <vector>
#include "BookSide.h"


// Provide an implementation for the OrderCacheInterface interface class.
// Your implementation class should hold all relevant data structures you think
// are needed.
class OrderCacheInterface {

 public:
  // implement the 6 methods below, do not alter signatures

  // add order to the cache
  virtual void addOrder(Order order) = 0;

  // remove order with this unique order id from the cache
  virtual void cancelOrder(const std::string& orderId) = 0;

  // remove all orders in the cache for this user
  virtual void cancelOrdersForUser(const std::string& user) = 0;

  // remove all orders in the cache for this security with qty >= minQty
  virtual void cancelOrdersForSecIdWithMinimumQty(const std::string& securityId,
                                                  unsigned int minQty) = 0;

  // return the total qty that can match for the security id
  virtual unsigned int getMatchingSizeForSecurity(
      const std::string& securityId) = 0;

  // return all orders in cache in a vector
  virtual std::vector<Order> getAllOrders() const = 0;
};

// Todo: Your implementation of the OrderCache...
class OrderCache : public OrderCacheInterface {
 public:
  OrderCache() {
    _alloc.set_object_size(sizeof(Order) + 16);
  }  // +16 for size of nodes in std::list

  void addOrder(Order order) override;

  void cancelOrder(const std::string& orderId) override;

  void cancelOrdersForUser(const std::string& user) override;

  void cancelOrdersForSecIdWithMinimumQty(const std::string& securityId,
                                          unsigned int minQty) override;

  unsigned int getMatchingSizeForSecurity(
      const std::string& securityId) override;

  std::vector<Order> getAllOrders() const override;

 private:
  void deleteOrder(const std::string& order_id, bool skip_user_orders);
  // map of sec_id => OrderBook
  std::unordered_map<std::string, OrderBook> _orderBooks{};
  // orderId => [ BookSide&, user_id ]
  std::unordered_map<std::string, std::tuple<BookSide&, std::string>>
      _ordIdMapToBookSideAndUser{};
  // user => order_ids
  std::unordered_map<std::string, std::set<std::string>> _userOrders;

  BookSide::allocator_t _alloc = BookSide::allocator_t();
  std::size_t _total_orders = 0;
};
