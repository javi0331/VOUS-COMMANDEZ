#include "../simulation/OrderGenerator.h"
#include "../utils/Random.h"
#include <cmath>
#include <sstream>
#include <iomanip>
#include <Config.h>

OrderGenerator::OrderGenerator(
    const TimeSystem& time,
    double baseSpawnRate
)
    : timeSystem(time),
      spawnAccumulator(0.0),
      baseSpawnRate(baseSpawnRate),
      nextOrderIndex(1) {}

void OrderGenerator::update(double simDeltaMinutes) {
    // No generar órdenes mientras el tiempo esté pausado
    if (timeSystem.isPaused()) return;

    if (restaurants.empty() || clientNodeIds.empty()) return;

    double effectiveRate = baseSpawnRate * demandFactor();
    double interval = (effectiveRate > 0.0) ? 1.0 / effectiveRate : 999.0;

    spawnAccumulator += simDeltaMinutes;

    while (spawnAccumulator >= interval) {
        spawnAccumulator -= interval;
        pendingOrders.push_back(generateOrder());
    }
}

// ── Configuración ─────────────────────────────────────────────

void OrderGenerator::addClientNode(const std::string& nodeId) {
    clientNodeIds.push_back(nodeId);
}

void OrderGenerator::addRestaurant(
    const std::string& restaurantId,
    const std::string& nodeId,
    double weight
) {
    restaurants.push_back({restaurantId, nodeId, weight});
}

void OrderGenerator::updateRestaurantWeight(
    const std::string& restaurantId,
    double newWeight
) {
    for (auto& r : restaurants) {
        if (r.restaurantId == restaurantId) {
            r.weight = newWeight;
            return;
        }
    }
}

void OrderGenerator::setBaseSpawnRate(double rate) {
    if (rate > 0.0) baseSpawnRate = rate;
}

// ── Recogida de órdenes ───────────────────────────────────────

std::vector<Order> OrderGenerator::takeGenerated() {
    std::vector<Order> result = std::move(pendingOrders);
    pendingOrders.clear();
    return result;
}

bool OrderGenerator::hasGenerated() const {
    return !pendingOrders.empty();
}

// ── Privados ──────────────────────────────────────────────────

double OrderGenerator::demandFactor() const {
    int hour = timeSystem.getHour();

    if (hour >= 0  && hour < 6)  return Config::DEMAND_DAWN;
    if (hour >= 9  && hour < 11) return Config::DEMAND_MORNING;
    if (hour >= 12 && hour < 14) return Config::DEMAND_LUNCH;
    if (hour >= 14 && hour < 18) return Config::DEMAND_AFTERNOON;
    if (hour >= 19 && hour < 22) return Config::DEMAND_DINNER;
    if (hour >= 22)              return Config::DEMAND_NIGHT;

    return 1.0;
}

const OrderGenerator::RestaurantWeight*
OrderGenerator::pickRestaurant() {
    if (restaurants.empty()) return nullptr;

    double total = 0.0;
    for (const auto& r : restaurants) total += r.weight;

    double roll = Random::uniformReal(0.0, total);
    double cumulative = 0.0;

    for (const auto& r : restaurants) {
        cumulative += r.weight;
        if (roll <= cumulative) return &r;
    }

    return &restaurants.back();
}

std::string OrderGenerator::pickClientNode() {
    if (clientNodeIds.empty()) return "";
    int idx = Random::uniformInt(0, static_cast<int>(clientNodeIds.size()) - 1);
    return clientNodeIds[idx];
}

Order OrderGenerator::generateOrder() {
    const RestaurantWeight* rest = pickRestaurant();
    std::string clientNode       = pickClientNode();

    int priority = Random::uniformInt(
        Config::ORDER_PRIORITY_MIN,
        Config::ORDER_PRIORITY_MAX
    );

    std::string clientId = "CLIENT_" + clientNode;

    return Order(
        makeOrderId(),
        rest ? rest->restaurantId : "",
        clientId,
        clientNode,
        priority,
        timeSystem.getSimulatedTime()
    );
}

std::string OrderGenerator::makeOrderId() {
    std::ostringstream ss;
    ss << "ORD_" << std::setw(5) << std::setfill('0') << nextOrderIndex++;
    return ss.str();
}