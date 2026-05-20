#include "../models/Client.h"

Client::Client()
    : zone(Zone::CENTRO),
      totalSpent(0.0),
      averageRating(5.0) {}

Client::Client(
    const std::string& id,
    const std::string& name,
    const std::string& nodeId,
    Zone zone
)
    : id(id),
      name(name),
      nodeId(nodeId),
      zone(zone),
      totalSpent(0.0),
      averageRating(5.0) {}

// ── Getters ───────────────────────────────────────────────────

const std::string& Client::getId() const {
    return id;
}

const std::string& Client::getName() const {
    return name;
}

const std::string& Client::getNodeId() const {
    return nodeId;
}

Zone Client::getZone() const {
    return zone;
}

int Client::getOrderCount() const {
    return static_cast<int>(orderHistory.size());
}

double Client::getTotalSpent() const {
    return totalSpent;
}

double Client::getAverageRating() const {
    return averageRating;
}

const std::vector<std::string>& Client::getOrderHistory() const {
    return orderHistory;
}

// ── Actualización ─────────────────────────────────────────────

void Client::addOrder(
    const std::string& orderId,
    double amount
) {
    orderHistory.push_back(orderId);
    totalSpent += amount;
}

void Client::addRating(double score) {
    int n = getOrderCount();
    if (n > 1) {
        averageRating = (averageRating * (n - 1) + score) / n;
    } else {
        averageRating = score;
    }
}

// ── Helpers ───────────────────────────────────────────────────

bool Client::isFrequent() const {
    // Umbral: cliente frecuente si tiene 5+ pedidos
    // Mover a Config.h si se quiere configurable
    return getOrderCount() >= 5;
}