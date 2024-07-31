// Todo: your implementation of the OrderCache...
#include <BookSide.h>
#include <OrderCache.h>
#include <algorithm>
#include <tuple>
#include <utility>
#include "OrderBook.h"

void OrderCache::addOrder(Order order) {
  // TODO: see below
  // should create in place if doesn't exist already
  // maybe we don't want that
  // this will cause one large performance hit at the beginnning
  // due to the custom allocator's preallocation
  // In reality this would be loaded up from a config file
  // containing the security ids and not done at the first order seen
  auto& book = _orderBooks[order.securityId()];
  Order& added_order = book.emplace(std::forward<Order>(order));
  // getting the side so we can map back to the order from the sec id
  auto& side = book.get_side(order.side());

  _ordIdMapToBookSideAndUser.emplace(
      std::piecewise_construct, std::forward_as_tuple(added_order.orderId()),
      std::forward_as_tuple(side, added_order.user()));

  // TODO: fix the user mappings as they are broken
  _userOrders[order.user()].emplace(added_order.orderId());
  // _companies.emplace(order.company());
  _total_orders++;
}

void OrderCache::cancelOrder(const std::string& orderId) {
  auto& [side, user] = _ordIdMapToBookSideAndUser.at(orderId);

  std::set<std::string>& ords = _userOrders.at(user);
  ords.erase(orderId);

  _ordIdMapToBookSideAndUser.erase(orderId);
  side.erase(orderId);

  _total_orders--;
}

void OrderCache::cancelOrdersForUser(const std::string& user) {
  std::set<std::string>& orders = _userOrders.at(user);

  // TODO: check this is okay because the set is getting edited as we remove
  for (auto& order : orders) {
    cancelOrder(order);
  }
}

void OrderCache::cancelOrdersForSecIdWithMinimumQty(
    const std::string& securityId, unsigned int minQty) {
  OrderBook& book = _orderBooks.at(securityId);

  for (BookSide& side : book.sides()) {
    auto& orders_per_company = side.orders();
    for (auto& ord_set : orders_per_company) {
      // TODO: implementing an iterator would fix this
      BookSide::order_list_t& orders = ord_set.second._orders;

      size_t before = orders.size();
      BookSide::order_list_t::iterator iter = std::remove_if(
          orders.begin(), orders.end(),
          [minQty](const Order& order) { return order.qty() >= minQty; });

      _total_orders -= (before - orders.size());
    }
  }
}

unsigned int OrderCache::getMatchingSizeForSecurity(
    const std::string& securityId) {
  OrderBook& book = _orderBooks.at(securityId);

  BookSide& bid = book.get_side("buy");
  BookSide& ask = book.get_side("sell");

  BookSide::order_heap_t all_bids = bid.orders();
  BookSide::order_heap_t all_asks = ask.orders();
  auto bids_companies = all_bids.cbegin();
  auto asks_companies = all_asks.crbegin();

  uint bid_qty = 0, ask_qty = 0, total_matching = 0;
  std::string skipped_company = "";
  std::string skipped_side = "";

  while (true) {
    if (bids_companies->first == asks_companies->first) {
      // for better effeciency we are iterating both
      // maps in opposite order. Once they are matching
      // company we will skip the most recently totally
      // matched side on to the next company and return to
      // this set later
      skipped_company = asks_companies->first;
      if (ask_qty == 0) {
        skipped_side = "ask";
        asks_companies++;
      } else {
        skipped_side = "bid";
        bids_companies++;
      }
    }

    bid_qty = bid_qty == 0 ? bids_companies->second._qty : bid_qty;
    ask_qty = ask_qty == 0 ? asks_companies->second._qty : ask_qty;

    if (bid_qty > ask_qty) {
      asks_companies++;
      bid_qty -= ask_qty;
      total_matching += ask_qty;
      ask_qty = 0;
    } else if (ask_qty > bid_qty) {
      bids_companies++;
      ask_qty -= bid_qty;
      total_matching += bid_qty;
      bid_qty = 0;
    } else {
      bids_companies++;
      asks_companies++;
      total_matching += ask_qty;
      ask_qty = 0;
      bid_qty = 0;
    }
  }

  // handling the skipped node
  if (bids_companies == all_bids.cend()) {
    if (skipped_side == "bid") {
      bid_qty = all_bids.at(skipped_company)._qty;

      while (bid_qty > 0) {
        if (ask_qty >= bid_qty) {
          total_matching += bid_qty;
          break;
        } else {
          bid_qty -= ask_qty;
          total_matching += ask_qty;
          asks_companies++;
          ask_qty = asks_companies->second._qty;
        }
      }
    }  // if skipped side is ask we have nothing left to match against
  } else if (asks_companies == all_asks.crend()) {
    if (skipped_side == "ask") {
      ask_qty = all_asks.at(skipped_company)._qty;

      while (ask_qty > 0) {
        if (bid_qty >= ask_qty) {
          total_matching += ask_qty;
          break;
        } else {
          ask_qty -= bid_qty;
          total_matching += bid_qty;
          bids_companies++;
          bid_qty = bids_companies->second._qty;
        }
      }
    }
  }
  return total_matching;
}

std::vector<Order> OrderCache::getAllOrders() const {
  std::vector<Order> orders = {};
  // I would use an allocator here too had I had the time to
  // write it. This isn't the most efficient function I
  // would try to avoid copying all the Orders

  // I would have also written an iterator to make the order_heap
  // easier to copy
  orders.reserve(_total_orders);

  for (const auto& book : _orderBooks) {
    const OrderBook& order_book = book.second;
    const auto& sides = order_book.sides();
    for (const auto& side : sides) {
      const auto& orders_per_company = side.orders();
      for (const auto& orders_tpl : orders_per_company) {
        const auto& order_list = orders_tpl.second._orders;
        std::copy(std::begin(order_list), std::end(order_list),
                  std::back_inserter(orders));
      }
    }
  }

  return orders;
}