// Todo: your implementation of the OrderCache...
#include <OrderCache.h>
#include <tuple>
#include <utility>

void OrderCache::addOrder(Order order) {
  // TODO: see below
  // should create in place if doesn't exist already
  // maybe we don't want that
  auto& book = _orderBooks[order.securityId()];
  auto idx = book.emplace(std::forward<Order>(order));
  auto& side = book.getSide(order.side());
  _ordIdMapToBookSide.emplace(std::piecewise_construct, order.orderId(),
                              std::forward_as_tuple(side, idx));

  auto& userSet = _userOrders[order.user()];
  userSet.emplace(side.orders[idx]);
}

void OrderCache::cancelOrder(const std::string& orderId) {
  auto& mapping = _ordIdMapToBookSide.at(orderId);

  // TODO fix this not permititng me to specify by class
  BookSide& side = std::get<0>(mapping);
  size_t idx = std::get<1>(mapping);

  // TODO: need to have an Order getter method
  Order& order = side.orders[idx];
  side.erase(idx);

  _ordIdMapToBookSide.erase(orderId);
  _userOrders.at(order.user()).erase(order);
}

void OrderCache::cancelOrdersForUser(const std::string& user) {
  std::set<Order>& orders = _userOrders.at(user);

  // TODO: check this is okay because the set is getting edited as we remove
  for (auto& order : orders) {
    cancelOrder(order.orderId());
  }
}

void OrderCache::cancelOrdersForSecIdWithMinimumQty(
    const std::string& securityId, unsigned int minQty) {
  // Todo...
}

unsigned int OrderCache::getMatchingSizeForSecurity(
    const std::string& securityId) {
  // Todo...
}

std::vector<Order> OrderCache::getAllOrders() const {
  // Todo...
}
