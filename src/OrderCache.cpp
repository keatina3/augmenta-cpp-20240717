// Todo: your implementation of the OrderCache...
#include <BookSide.h>
#include <OrderCache.h>
#include <algorithm>
#include <utility>
#include "OrderBook.h"

void OrderCache::addOrder(Order order) {
  // In the real world I would have a config with all
  // security IDs and not add them ad-hoc upon the
  // first order add
  auto& book =
      _orderBooks.try_emplace(order.securityId(), _alloc).first->second;
  Order& added_order = book.emplace(std::forward<Order>(order));
  // getting the side so we can map back to the order from the sec id
  auto& side = book.get_side(order.side());

  _ordIdMapToBookSideAndUser.try_emplace(added_order.orderId(), side,
                                         added_order.user());

  _userOrders[order.user()].emplace(added_order.orderId());
  _total_orders++;
}

void OrderCache::deleteOrder(const std::string& order_id,
                             bool skip_user_orders) {
  auto iter = _ordIdMapToBookSideAndUser.find(order_id);
  if (iter == _ordIdMapToBookSideAndUser.end()) {
    // I would in normal circumstances make this throw an exception
    return;
  }

  auto& [side, user] = iter->second;

  if (not skip_user_orders) {
    std::set<std::string>& ords = _userOrders.at(user);
    ords.erase(order_id);
  }

  side.erase(order_id);
  _ordIdMapToBookSideAndUser.erase(order_id);

  _total_orders--;
}

void OrderCache::cancelOrder(const std::string& orderId) {
  deleteOrder(orderId, false);
}

void OrderCache::cancelOrdersForUser(const std::string& user) {
  auto iter = _userOrders.find(user);
  if (iter == _userOrders.end()) {
    return;
  }
  std::set<std::string> orders = iter->second;

  for (auto order_id : orders) {
    deleteOrder(order_id, true);
  }
}

void OrderCache::cancelOrdersForSecIdWithMinimumQty(
    const std::string& securityId, unsigned int minQty) {
  OrderBook& book = _orderBooks.at(securityId);

  for (BookSide& side : book.sides()) {
    auto& orders_per_company = side.orders();
    for (auto& ord_set : orders_per_company) {
      BookSide::order_list_t& orders = ord_set.second._orders;
      size_t before = orders.size();

      BookSide::order_list_t::iterator iter = std::remove_if(
          orders.begin(), orders.end(),
          [minQty](Order& order) { return order.qty() >= minQty; });

      orders.erase(iter, orders.end());
      _total_orders -= (before - orders.size());
    }
  }
}

// the algo here iterates from start and end of both
// the bid and ask sides. it will skip the company
// entry when both iterators have the same key
// Once the shorter iterator reaches the end we
// return to the skipped company and match what's left
// on the longer iterator
unsigned int OrderCache::getMatchingSizeForSecurity(
    const std::string& securityId) {
  // the test supported providing requests for empty books
  // otherwise I would use .at() and throw an exception
  auto iter = _orderBooks.find(securityId);
  if (iter == _orderBooks.end()) {
    return 0;
  }
  OrderBook& book = iter->second;

  BookSide& bid = book.get_side("Buy");
  BookSide& ask = book.get_side("Sell");

  BookSide::order_heap_t all_bids = bid.orders();
  BookSide::order_heap_t all_asks = ask.orders();
  auto bids_companies = all_bids.cbegin();
  auto asks_companies = all_asks.crbegin();

  if (all_asks.size() == 0 or all_bids.size() == 0) {
    return 0;
  }

  uint bid_qty = 0, ask_qty = 0, total_matching = 0, skipped_bid_qty = 0,
       skipped_ask_qty = 0;

  while (bids_companies != all_bids.cend() &&
         asks_companies != all_asks.crend()) {
    bid_qty = bid_qty == 0 ? bids_companies->second._qty : bid_qty;
    ask_qty = ask_qty == 0 ? asks_companies->second._qty : ask_qty;

    if (bids_companies->first == asks_companies->first) {
      skipped_bid_qty = bid_qty;
      skipped_ask_qty = ask_qty;
      asks_companies++;
      bids_companies++;
    } else {
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
  }
  if (asks_companies == all_asks.crend() and
      bids_companies == all_bids.cend()) {
    return total_matching;
  }
  if (asks_companies == all_asks.crend()) {
    ask_qty = skipped_ask_qty;
    while (ask_qty > 0 && bids_companies != all_bids.cend()) {
      bid_qty = bids_companies->second._qty;
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
  } else if (bids_companies == all_bids.cend()) {
    bid_qty = skipped_bid_qty;
    while (bid_qty > 0 && asks_companies != all_asks.crend()) {
      ask_qty = asks_companies->second._qty;
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
  }
  return total_matching;
}

std::vector<Order> OrderCache::getAllOrders() const {
  std::vector<Order> orders = {};
  // I would use a custom allocator here too had I had more time to
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