#include "../models/Order.h"

Order::Order()
    : status(OrderStatus::PENDING),
      priority(0),
      estimatedTime(0.0),
      totalDistance(0.0),
      createdAt(0.0),
      assignedAt(-1.0),
      deliveredAt(-1.0) {}

Order::Order(
    const std::string& id,
    const std::string& restaurantId,
    const std::string& clientId,
    const std::string& clientNodeId,
    int priority,
    double createdAt
)
    : id(id),
      restaurantId(restaurantId),
      clientId(clientId),
      clientNodeId(clientNodeId),
      status(OrderStatus::PENDING),
      priority(priority),
      estimatedTime(0.0),
      totalDistance(0.0),
      createdAt(createdAt),
      assignedAt(-1.0),
      deliveredAt(-1.0) {}

// ── Getters ───────────────────────────────────────────────────

const std::string& Order::getId() const {
    return id;
}

const std::string& Order::getRestaurantId() const {
    return restaurantId;
}

const std::string& Order::getClientId() const {
    return clientId;
}

const std::string& Order::getDealerId() const {
    return dealerId;
}

const std::string& Order::getClientNodeId() const {
    return clientNodeId;
}

OrderStatus Order::getStatus() const {
    return status;
}

int Order::getPriority() const {
    return priority;
}

double Order::getEstimatedTime() const {
    return estimatedTime;
}

double Order::getTotalDistance() const {
    return totalDistance;
}

double Order::getCreatedAt() const {
    return createdAt;
}

double Order::getAssignedAt() const {
    return assignedAt;
}

double Order::getDeliveredAt() const {
    return deliveredAt;
}

// ── Setters ───────────────────────────────────────────────────

void Order::setDealerId(const std::string& id) {
    dealerId = id;
}

void Order::setStatus(OrderStatus s) {
    status = s;
}

void Order::setEstimatedTime(double minutes) {
    estimatedTime = minutes;
}

void Order::setTotalDistance(double km) {
    totalDistance = km;
}

void Order::setAssignedAt(double time) {
    assignedAt = time;
}

void Order::setDeliveredAt(double time) {
    deliveredAt = time;
}

// ── Helpers ───────────────────────────────────────────────────

bool Order::isPending() const {
    return status == OrderStatus::PENDING;
}

bool Order::isActive() const {
    return status == OrderStatus::ASSIGNED
        || status == OrderStatus::PICKING_UP
        || status == OrderStatus::IN_TRANSIT;
}

bool Order::isCompleted() const {
    return status == OrderStatus::DELIVERED
        || status == OrderStatus::CANCELLED;
}

double Order::waitTime() const {
    if (assignedAt < 0.0) return -1.0;
    return assignedAt - createdAt;
}

double Order::deliveryTime() const {
    if (assignedAt < 0.0 || deliveredAt < 0.0) return -1.0;
    return deliveredAt - assignedAt;
}