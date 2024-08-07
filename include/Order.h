#pragma once

#include <OrderBuffer.h>
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

  Order() = default;

  // this is being used in remove_if
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

  // if i had my choice I would not use std::strings,
  // but if necessary I would preallocate them and have some
  // max size, using custom allocator
  // However, the allocator I have will only work for single inserts
  // unlike string/vector etc
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