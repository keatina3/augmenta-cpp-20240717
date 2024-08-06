// Todo: your implementation of the OrderCache...
#include <BookSide.h>
#include <OrderCache.h>
#include <algorithm>
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
  auto& book =
      _orderBooks.try_emplace(order.securityId(), _alloc).first->second;
  Order& added_order = book.emplace(std::forward<Order>(order));
  // getting the side so we can map back to the order from the sec id
  auto& side = book.get_side(order.side());

  _ordIdMapToBookSideAndUser.try_emplace(added_order.orderId(), side,
                                         added_order.user());

  // TODO: fix the user mappings as they are broken
  _userOrders[order.user()].emplace(added_order.orderId());
  // _companies.emplace(order.company());
  _total_orders++;
}

void OrderCache::cancelOrder(const std::string& orderId) {
  auto iter = _ordIdMapToBookSideAndUser.find(orderId);
  if (iter == _ordIdMapToBookSideAndUser.end()) {
    // I would in normal circumstances make this throw an exception
    return;
  }

  auto& [side, user] = iter->second;

  std::set<std::string>& ords = _userOrders.at(user);
  ords.erase(orderId);

  _ordIdMapToBookSideAndUser.erase(orderId);
  side.erase(orderId);

  _total_orders--;
}

void OrderCache::cancelOrdersForUser(const std::string& user) {
  auto iter = _userOrders.find(user);
  if (iter == _userOrders.end()) {
    return;
  }
  std::set<std::string>& orders = iter->second;

  while (orders.size() > 0) {
    cancelOrder(*orders.begin());
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
          [minQty](Order& order) { return order.qty() >= minQty; });

      orders.erase(iter, orders.end());
      _total_orders -= (before - orders.size());
    }
  }
}

unsigned int OrderCache::getMatchingSizeForSecurity(
    const std::string& securityId) {
  // the test supported providing requests for empty books
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