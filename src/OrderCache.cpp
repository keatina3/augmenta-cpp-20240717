// Todo: your implementation of the OrderCache...
#include <BookSide.h>
#include <OrderCache.h>
#include <algorithm>
#include <tuple>
#include <utility>

void OrderCache::addOrder(Order order) {
  // TODO: see below
  // should create in place if doesn't exist already
  // maybe we don't want that
  // this will cause one large performance hit at the beginnning
  // due to the custom allocator's preallocation
  // In reality this would be loaded up from a config file
  // containing the security ids and not done at the first order seen
  auto& book = _orderBooks[order.securityId()];
  BookSide::order_set_t::iterator added_order =
      book.emplace(std::forward<Order>(order));
  // getting the side so we can map back to the order from the sec id
  auto& side = book.getSide(order.side());

  _ordIdMapToBookSideAndUser.emplace(
      std::piecewise_construct, std::forward_as_tuple(added_order->orderId()),
      std::forward_as_tuple(side, added_order->user()));

  // TODO: fix the user mappings as they are broken
  _userOrders[order.user()].emplace(added_order->orderId());
  // _companies.emplace(order.company());
}

void OrderCache::cancelOrder(const std::string& orderId) {
  auto& [side, user] = _ordIdMapToBookSideAndUser.at(orderId);

  std::set<std::string>& ords = _userOrders.at(user);
  ords.erase(orderId);

  _ordIdMapToBookSideAndUser.erase(orderId);
  side.erase(orderId);
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
      BookSide::order_set_t& orders = ord_set.second._orders;

      // TODO: fixme really
      // remove_if works for lists but not sets
      // do I need sets I don't know to be honest
      /*
      BookSide::order_set_t::iterator iter = std::remove_if(
          orders.begin(), orders.end(),
          [minQty](const Order& order) { return order.qty() >= minQty; });
      */
      /*
      if (iter == orders.end()) {
        // TODO: exception for not finding any perhaps
      }
      */
    }
  }
}

unsigned int OrderCache::getMatchingSizeForSecurity(
    const std::string& securityId) {
  OrderBook& book = _orderBooks.at(securityId);

  BookSide& bid = book.getSide("buy");
  BookSide& ask = book.getSide("sell");

  BookSide::order_heap_t all_bids = bid.orders();
  auto bids_companies = bid.orders().cbegin();
  auto asks_companies = ask.orders().crbegin();

  uint bid_qty = 0, ask_qty = 0, total_matching = 0;
  std::string overlap_company = "";

  while (true) {
    if (bids_companies->first == asks_companies->first) {
      // TODO: do something to handle when they overlap
    }
    bid_qty = bid_qty == 0 ? bids_companies->second._qty : bid_qty;
    ask_qty = ask_qty == 0 ? asks_companies->second._qty : ask_qty;

    if (bid_qty > ask_qty) {
      asks_companies++;
      bid_qty -= ask_qty;
      ask_qty = 0;
    } else if (ask_qty > bid_qty) {
      bids_companies++;
      ask_qty -= bid_qty;
      bid_qty = 0;
    } else {
      bids_companies++;
      asks_companies++;
      ask_qty = 0;
      bid_qty = 0;
    }
  }
  return 0;
}

/*
unsigned int OrderCache::getMatchingSizeForSecurity(
    const std::string& securityId) {
  OrderBook& book = _orderBooks.at(securityId);
  // TODO: maybe tidier to use the enums here
  BookSide& bid = book.getSide("Buy");
  BookSide& ask = book.getSide("Sell");

  BookSide::order_set_t::iterator starting_order = bid.orders().begin();
  BookSide::order_set_t::iterator starting_opposite_order =
      ask.orders().begin();

  uint32_t matching_size = 0;

  for (const auto& company : _companies) {
    while (starting_order != bid.orders().end()) {
      // TODO: make sure this is deterministic
      starting_order = std::find_if(
          starting_order, bid.orders().end(),
          [&company](Order& order) { return order.company() == company; });

      if (starting_order == bid.orders().end())
        break;

      while (true) {
        starting_opposite_order = std::find_if(
            starting_opposite_order, ask.orders().end(),
            [company](Order& order) { return order.company() != company; });
        // TODO: this isn't correct. I need to have some way to mark
        // matched orders on this side
        if (starting_opposite_order == ask.orders().end()) {
          break;
        }

        auto bid_size = starting_order->qty();
        auto ask_size = starting_opposite_order->qty();

        // TODO: need to track chipped orders too
        // TODO: check I'm incrementing the right value
        matching_size += (bid_size < ask_size) ? bid_size : ask_size;
      }
    }
    // so std::list is going to be O(n)
    // I _could_ change the class to a std::set which
    // would take me down to O(logn)
    // TODO: see above
    // TODO: I think in real-life situations it would need to be a
    // list to preserve priority but this project doesn't have that
    // OR you could have a priority value stored in each order that
    // would be used for sorting
    // std::set is going to be slower for adding/removing nodes...

    // std::reserve won't preallocate the nodes
    // the custom allocator should
    // also insert will allocate extra memory
    return 0;
  }
}
*/

std::vector<Order> OrderCache::getAllOrders() const {
  return std::vector<Order>();
}
