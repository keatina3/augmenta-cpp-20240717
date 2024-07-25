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
  Order& added_order = book.emplace(std::forward<Order>(order));
  // getting the side so we can map back to the order from the sec id
  auto& side = book.getSide(order.side());
  _ordIdMapToBookSideAndUser.emplace(
      std::piecewise_construct, std::forward_as_tuple(added_order.orderId()),
      std::forward_as_tuple(side, added_order.user()));

  // TODO: fix the user mappings as they are broken
  _userOrders[order.user()].emplace(added_order.orderId());
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
    auto& orders = side.orders();
    auto iter = std::remove_if(
        orders.begin(), orders.end(),
        [minQty](Order& order) { return order.qty() >= minQty; });
    if (iter == orders.end()) {
      // TODO: exception for not finding any perhaps
    }
  }
}

unsigned int OrderCache::getMatchingSizeForSecurity(
    const std::string& securityId) {
  // Todo...
}

std::vector<Order> OrderCache::getAllOrders() const {
  // Todo...
}
