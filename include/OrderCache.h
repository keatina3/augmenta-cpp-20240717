#pragma once

#include <cstdint>
#include <map>
#include <set>
#include <string>
#include <unordered_map>
#include <vector>

class Order {

  // TODO: do i need a copy constructor for perfect forwarding?
 public:
  // do not alter signature of this constructor
  Order(const std::string& ordId, const std::string& secId,
        const std::string& side, const unsigned int qty,
        const std::string& user, const std::string& company)
      : m_orderId(ordId),
        m_securityId(secId),
        m_side(side),
        m_qty(qty),
        m_user(user),
        m_company(company) {}
  // TODO:
  // Order(Order &order){}

  // do not alter these accessor methods

  std::string orderId() const { return m_orderId; }
  std::string securityId() const { return m_securityId; }
  std::string side() const { return m_side; }
  std::string user() const { return m_user; }
  std::string company() const { return m_company; }
  unsigned int qty() const { return m_qty; }

 private:
  // use the below to hold the order data
  // do not remove the these member variables
  std::string m_orderId;     // unique order id
  std::string m_securityId;  // security identifier
  std::string m_side;        // side of the order, eg Buy or Sell
  unsigned int m_qty;        // qty for this order
  std::string m_user;        // user name who owns this order
  std::string m_company;     // company for user
};

// TODO: maybe we want to make this entire thing multithreaded at some point
// would have to be very careful with priority of the queues and locks
// defintiely would need wrappers to emplace
// TODO: handle access of members correctly
class BookLevel {
  // TODO: there seems to be no price levels?
};

class BookSide {
 public:
  // TODO: need to figure out best way to map from
  // user and sec id
  // TODO: fix access
  // TODO: make this some wrapper class on a vector called circular buffer
  // which allocated memory smartly as needed
  // and also tracks where the next entry can go
  std::vector<Order> orders;
  // TODO: maybe these should be iterator positions not indices
  // // or maybe they can be order references? but i guess that
  // doesn't help us remove them from the circular buffer
  std::unordered_map<std::string, size_t> order_idx;
  std::unordered_map<std::string, std::set<size_t>> user_orders;

 public:
  size_t emplace(Order&& order) {
    // TODO: what do we do when an order gets cleared?
    // maybe allocating a circular buffer would be better and I mark the indices
    // ask back available for emplacement
    // we can extend the size if num_avail_indices = 0
    // maybe it can be configurable in size
    // I can have a worker thread managing those memory pools
    orders.emplace_back(order);
    std::vector<Order>::size_type idx = orders.size() - 1;
    order_idx[order.orderId()] = idx;
    user_orders[order.user()].emplace(idx);

    return idx;
  }

  // TODO: check if i want useful return types
  // TODO: I will definitely want some error management/exception handling
  void erase(size_t idx) {
    // TODO: catch exception
    Order& order = orders.at(idx);
    user_orders.at(order.user()).erase(idx);
    // TODO: if user_orders is empty we can probably delete
    order_idx.erase(order.orderId());
    orders.erase(orders.begin() + idx);
  }
};

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
  size_t emplace(Order&& order) {
    // TODO: maybe I can use a if constexpr to make use of a tuple instead
    // TODO: do the correct wrapping for perfect forwarding
    auto& side = sides[OrderBook::ToSide(order.side())];
    return side.emplace(std::forward<Order>(order));
  }
};

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
  // Todo...
  std::map<std::string, OrderBook> _orderBooks{};
  std::map<std::string, std::tuple<BookSide&, size_t>> _ordIdMapToBookSide{};
  // TODO: this needs to be references not copies
  // TODO: there must be a better way of storing this...
  std::unordered_map<std::string, std::set<Order>> _userOrders;
};
