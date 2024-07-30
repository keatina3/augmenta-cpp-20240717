#pragma once

#include <OrderBuffer.h>
#include <functional>
#include <string>

class Order {

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
  /* TODO: broken - fix later
  // ctr for perfect forwarding when calling emplace in the set
  Order(const Order&& other)
      : m_orderId(other.m_orderId),
        m_securityId(other.m_securityId),
        m_side(other.m_side),
        m_qty(other.m_qty),
        m_user(other.m_user),
        m_company(other.m_company) {}
  */
  // TODO: check why this is being used
  Order& operator=(const Order& other) {
    m_orderId = other.m_orderId;
    m_securityId = other.m_securityId;
    m_side = other.m_side;
    m_qty = other.m_qty;
    m_user = other.m_user;
    m_company = other.m_company;
    return *this;
  }

  // do not alter these accessor methods

  // TODO: if i had my choice I would not use std::strings,
  // but if necessary I would preallocate them and have some
  // max a custom allocator
  // However, the allocator I have will only work for single inserts
  // unlike string/vector etc
  std::string orderId() const { return m_orderId; }
  std::string securityId() const { return m_securityId; }
  std::string side() const { return m_side; }
  std::string user() const { return m_user; }
  std::string company() const { return m_company; }
  unsigned int qty() const { return m_qty; }

  // working on assumption that order_id is unique across the board
  // based on the description
  // otherwise you could XOR it against security_id
  // TODO: this might not bee needed
  std::size_t hash() const { return std::hash<std::string>()(m_orderId); }

  static bool comparator(const Order& lhs, const Order& rhs) noexcept {
    return lhs.m_orderId < rhs.m_orderId;
  }

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

// TODO: this won't be needed if we don't use unordered_set
namespace std {
template <>
struct hash<Order> {
  size_t operator()(const Order& order) const noexcept { return order.hash(); }
};
}  // namespace std