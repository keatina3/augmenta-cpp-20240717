#pragma once

#include <Order.h>
#include <OrderBook.h>
#include <OrderBuffer.h>
#include <set>
#include <string>
#include <unordered_map>
#include <unordered_set>
#include <vector>

// TODO: my mapping of order id needs to map to side and security id...

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
  void addOrder(Order order) override;

  void cancelOrder(const std::string& orderId) override;

  void cancelOrdersForUser(const std::string& user) override;

  void cancelOrdersForSecIdWithMinimumQty(const std::string& securityId,
                                          unsigned int minQty) override;

  unsigned int getMatchingSizeForSecurity(
      const std::string& securityId) override;

  std::vector<Order> getAllOrders() const override;

 private:
  // map of sec_id => OrderBook
  std::unordered_map<std::string, OrderBook> _orderBooks{};
  // TODO: less than ideal that this can't be passed by reference but whatever
  std::unordered_map<std::string, std::tuple<BookSide&, std::string>>
      _ordIdMapToBookSideAndUser{};
  // TODO: this needs to be references not copies
  // TODO: there must be a better way of storing this...
  std::unordered_map<std::string, std::set<std::string>> _userOrders;
  // std::unordered_set<std::string, std::set<std::string>> _companies;
};
