#pragma once

#include <string>
#include <vector>
#include <map>
#include <set>
#include <cstdint>

class Order
{

public:
  // do not alter signature of this constructor
  Order(
      const std::string &ordId,
      const std::string &secId,
      const std::string &side,
      const unsigned int qty,
      const std::string &user,
      const std::string &company)
      : m_orderId(ordId),
        m_securityId(secId),
        m_side(side),
        m_qty(qty),
        m_user(user),
        m_company(company) {}

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
  std::string m_orderId;    // unique order id
  std::string m_securityId; // security identifier
  std::string m_side;       // side of the order, eg Buy or Sell
  unsigned int m_qty;       // qty for this order
  std::string m_user;       // user name who owns this order
  std::string m_company;    // company for user
};

// Provide an implementation for the OrderCacheInterface interface class.
// Your implementation class should hold all relevant data structures you think
// are needed.
class OrderCacheInterface
{

public:
  // implement the 6 methods below, do not alter signatures

  // add order to the cache
  virtual void addOrder(Order order) = 0;

  // remove order with this unique order id from the cache
  virtual void cancelOrder(const std::string &orderId) = 0;

  // remove all orders in the cache for this user
  virtual void cancelOrdersForUser(const std::string &user) = 0;

  // remove all orders in the cache for this security with qty >= minQty
  virtual void cancelOrdersForSecIdWithMinimumQty(const std::string &securityId, unsigned int minQty) = 0;

  // return the total qty that can match for the security id
  virtual unsigned int getMatchingSizeForSecurity(const std::string &securityId) = 0;

  // return all orders in cache in a vector
  virtual std::vector<Order> getAllOrders() const = 0;
};

class BookLevel
{
  // TODO: there seems to be no price levels?
};

class BookSide
{
  // TODO: need to figure out best way to map from
  // user and sec id
  // TODO: fix access
  std::vector<Order> orders;
  std::map<std::string, size_t> order_idx;
  std::map<std::string, std::set<size_t>> user_orders;
};

class OrderBook
{
  enum Side : std::uint8_t
  {
    Bid = 0,
    Ask = 1,
  };

  static Side ToSide(std::string const side)
  {
    return Side::Bid;
  }

  std::vector<BookSide> const sides = {BookSide(), BookSide()};

  // TODO: why are the getters not returning references?
  void emplace(Order &&order)
  {
    auto &side = sides[OrderBook::ToSide(order.side())];
  }
};

// Todo: Your implementation of the OrderCache...
class OrderCache : public OrderCacheInterface
{
public:
  void addOrder(Order order) override;

  void cancelOrder(const std::string &orderId) override;

  void cancelOrdersForUser(const std::string &user) override;

  void cancelOrdersForSecIdWithMinimumQty(const std::string &securityId, unsigned int minQty) override;

  unsigned int getMatchingSizeForSecurity(const std::string &securityId) override;

  std::vector<Order> getAllOrders() const override;

private:
  // Todo...
  std::map<std::string, OrderBook> _orderBooks{};
};
