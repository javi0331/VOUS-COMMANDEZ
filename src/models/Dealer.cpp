#include "../models/Dealer.h"
#include <algorithm>

Dealer::Dealer()
    : status(DealerStatus::IDLE),
      maxOrders(3),
      totalDeliveries(0.0),
      totalDistanceKm(0.0),
      rating(5.0),
      logisticScore(0.0) {}

Dealer::Dealer(
    const std::string& id,
    const std::string& name,
    const std::string& startNodeId,
    int maxOrders
)
    : id(id),
      name(name),
      currentNodeId(startNodeId),
      status(DealerStatus::IDLE),
      maxOrders(maxOrders),
      totalDeliveries(0.0),
      totalDistanceKm(0.0),
      rating(5.0),
      logisticScore(0.0) {}

// ── Getters ───────────────────────────────────────────────────

const std::string& Dealer::getId() const {
    return id;
}

const std::string& Dealer::getName() const {
    return name;
}

const std::string& Dealer::getCurrentNodeId() const {
    return currentNodeId;
}

DealerStatus Dealer::getStatus() const {
    return status;
}

int Dealer::getActiveCount() const {
    return static_cast<int>(activeOrderIds.size());
}

int Dealer::getMaxOrders() const {
    return maxOrders;
}

double Dealer::getTotalDeliveries() const {
    return totalDeliveries;
}

double Dealer::getTotalDistanceKm() const {
    return totalDistanceKm;
}

double Dealer::getRating() const {
    return rating;
}

double Dealer::getLogisticScore() const {
    return logisticScore;
}

const std::vector<std::string>& Dealer::getActiveOrderIds() const {
    return activeOrderIds;
}

// ── Setters ───────────────────────────────────────────────────

void Dealer::setCurrentNodeId(const std::string& nodeId) {
    currentNodeId = nodeId;
}

void Dealer::setStatus(DealerStatus s) {
    status = s;
}

void Dealer::setLogisticScore(double score) {
    logisticScore = score;
}

// ── Gestión de pedidos ────────────────────────────────────────

bool Dealer::assignOrder(const std::string& orderId) {
    if (isFull()) return false;
    activeOrderIds.push_back(orderId);
    return true;
}

void Dealer::removeOrder(const std::string& orderId) {
    activeOrderIds.erase(
        std::remove(
            activeOrderIds.begin(),
            activeOrderIds.end(),
            orderId
        ),
        activeOrderIds.end()
    );
}

bool Dealer::hasOrder(const std::string& orderId) const {
    return std::find(
        activeOrderIds.begin(),
        activeOrderIds.end(),
        orderId
    ) != activeOrderIds.end();
}

// ── Actualización de historial ────────────────────────────────

void Dealer::completeDelivery(double distanceKm) {
    totalDeliveries += 1.0;
    totalDistanceKm += distanceKm;
}

void Dealer::addRating(double score) {
    // Promedio acumulado sin guardar historial completo
    // rating = (rating * (n-1) + score) / n
    if (totalDeliveries > 0.0) {
        rating = (rating * (totalDeliveries - 1.0) + score)
                 / totalDeliveries;
    } else {
        rating = score;
    }
}

// ── Helpers ───────────────────────────────────────────────────

bool Dealer::isAvailable() const {
    return status == DealerStatus::IDLE && !isFull();
}

bool Dealer::isFull() const {
    return static_cast<int>(activeOrderIds.size()) >= maxOrders;
}

bool Dealer::isActive() const {
    return !id.empty();
}

double Dealer::loadFactor() const {
    if (maxOrders == 0) return 1.0;
    return static_cast<double>(activeOrderIds.size())
           / static_cast<double>(maxOrders);
}