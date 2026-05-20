#include "../structures/DeliveryHistory.h"
#include <stdexcept>
#include <numeric>

DeliveryHistory::DeliveryHistory(int maxCapacity)
    : maxCapacity(maxCapacity) {}

// ── Inserción ─────────────────────────────────────────────────

void DeliveryHistory::record(const DeliveryRecord& delivery) {
    records.push_back(delivery);

    if (maxCapacity > 0 &&
        static_cast<int>(records.size()) > maxCapacity) {
        records.pop_front();
    }
}

// ── Acceso ────────────────────────────────────────────────────

const DeliveryRecord& DeliveryHistory::latest() const {
    if (isEmpty()) {
        throw std::runtime_error(
            "DeliveryHistory::latest — historial vacío"
        );
    }
    return records.back();
}

const DeliveryRecord& DeliveryHistory::oldest() const {
    if (isEmpty()) {
        throw std::runtime_error(
            "DeliveryHistory::oldest — historial vacío"
        );
    }
    return records.front();
}

std::vector<DeliveryRecord> DeliveryHistory::getLastN(int n) const {
    std::vector<DeliveryRecord> result;
    int count = static_cast<int>(records.size());
    int start = std::max(0, count - n);

    for (int i = count - 1; i >= start; i--) {
        result.push_back(records[i]);
    }
    return result;
}

std::vector<DeliveryRecord> DeliveryHistory::getByDealer(
    const std::string& dealerId
) const {
    std::vector<DeliveryRecord> result;
    for (const auto& r : records) {
        if (r.dealerId == dealerId) result.push_back(r);
    }
    return result;
}

std::vector<DeliveryRecord> DeliveryHistory::getByRestaurant(
    const std::string& restaurantId
) const {
    std::vector<DeliveryRecord> result;
    for (const auto& r : records) {
        if (r.restaurantId == restaurantId) result.push_back(r);
    }
    return result;
}

// ── Estadísticas ──────────────────────────────────────────────

double DeliveryHistory::averageDeliveryTime() const {
    if (isEmpty()) return 0.0;
    double sum = 0.0;
    for (const auto& r : records) sum += r.deliveryTime;
    return sum / records.size();
}

double DeliveryHistory::averageWaitTime() const {
    if (isEmpty()) return 0.0;
    double sum = 0.0;
    for (const auto& r : records) sum += r.waitTime;
    return sum / records.size();
}

double DeliveryHistory::averageDistance() const {
    if (isEmpty()) return 0.0;
    double sum = 0.0;
    for (const auto& r : records) sum += r.totalDistance;
    return sum / records.size();
}

int DeliveryHistory::totalDeliveries() const {
    return static_cast<int>(records.size());
}

// ── Utilidades ────────────────────────────────────────────────

bool DeliveryHistory::isEmpty() const {
    return records.empty();
}

void DeliveryHistory::clear() {
    records.clear();
}