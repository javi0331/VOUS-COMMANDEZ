#include "../simulation/Simulator.h"
#include "../algorithms/Dijkstra.h"
#include "../utils/Logger.h"
#include "../utils/Random.h"
#include "../utils/Config.h"
#include <cmath>
#include <algorithm>

Simulator::Simulator(const Graph& graph)
    : graph(graph),
      timeSystem(Config::SIM_SPEED_DEFAULT),  // antes hardcodeado 60.0 — causaba x60 al arranque
      trafficSystem(timeSystem, weatherSystem),
      eventSystem(trafficSystem, weatherSystem),
      orderGenerator(timeSystem),
      running(false),
      boostCheckTimer(0.0) {}

// ── Setup ─────────────────────────────────────────────────────

void Simulator::addDealer(const Dealer& dealer) {
    dealers[dealer.getId()] = dealer;
    // No se inserta en el heap todavía — se hace en start()
}

void Simulator::addRestaurant(const Restaurant& restaurant) {
    restaurants[restaurant.getId()] = restaurant;
    avlRestaurants.insert(
        restaurant.getId(),
        restaurant.getPopularityScore()
    );
    orderGenerator.addRestaurant(
        restaurant.getId(),
        restaurant.getNodeId(),
        1.0   // Peso inicial igual para todos; se actualiza con la sim
    );
}

void Simulator::addClient(const Client& client) {
    clients[client.getId()] = client;
    orderGenerator.addClientNode(client.getNodeId());
}

// ── Control ───────────────────────────────────────────────────

void Simulator::start() {
    running = true;

    // Insertar todos los dealers disponibles en el heap con score 0
    for (const auto& [id, dealer] : dealers) {
        if (dealer.isAvailable()) {
            heapDealers.push(id, 0.0);
        }
    }
}

void Simulator::pause() {
    timeSystem.pause();
    running = false;
}

void Simulator::resume() {
    timeSystem.resume();
    running = true;
}

void Simulator::reset() {
    timeSystem.reset();
    weatherSystem.setWeather(WeatherState::CLEAR);
    trafficSystem.clearEvent();
    eventSystem.clearAll();
    orders.clear();
    while (!heapDealers.isEmpty()) heapDealers.pop();
    deliveryHistory.clear();
    stats = SimStats{};
    running = false;
}

// ── Loop principal ────────────────────────────────────────────

void Simulator::tick(double realDeltaSeconds) {
    if (!running) return;

    // 1. Avanzar tiempo
    timeSystem.update(realDeltaSeconds);
    double simDelta = realDeltaSeconds * timeSystem.getSimulationSpeed();

    // 2. Avanzar sistemas
    weatherSystem.update(simDelta);
    trafficSystem.update(simDelta);
    eventSystem.update(timeSystem.getSimulatedTime());

    // 3. Generar nuevas órdenes
    orderGenerator.update(simDelta);
    processNewOrders();

    // 4. Asignar órdenes a dealers
    assignOrders();

    // 5. Avanzar dealers en tránsito
    updateDealers(simDelta);

    // 6. Boost de órdenes viejas
    boostStaleOrders(simDelta);

    // 7. Actualizar stats para la UI
    updateStats();
}

// ── Pasos internos ────────────────────────────────────────────

void Simulator::processNewOrders() {
    auto newOrders = orderGenerator.takeGenerated();

    for (auto& order : newOrders) {
        orders[order.getId()] = order;
        pendingOrders.push(
            order.getId(),
            order.getPriority(),
            order.getCreatedAt()
        );

        // Notificar al restaurante
        auto it = restaurants.find(order.getRestaurantId());
        if (it != restaurants.end()) {
            it->second.orderReceived();
            avlRestaurants.updateScore(
                it->second.getId(),
                it->second.getPopularityScore()
            );
        }

        stats.totalOrdersGenerated++;
        Logger::info("Nueva orden " + order.getId() + " desde " + order.getRestaurantId());
    }
}

void Simulator::assignOrders() {
    while (!pendingOrders.isEmpty() && !heapDealers.isEmpty()) {
        // Tomar la orden más prioritaria
        OrderEntry oe = pendingOrders.pop();

        auto orderIt = orders.find(oe.orderId);
        if (orderIt == orders.end()) continue;

        Order& order = orderIt->second;
        if (!order.isPending()) continue;

        // Reconstruir heap con scores frescos para este restaurante
        rebuildDealerHeap(
            restaurants.count(order.getRestaurantId())
            ? restaurants.at(order.getRestaurantId()).getNodeId()
            : ""
        );

        if (heapDealers.isEmpty()) {
            // Devolver orden al heap — no hay dealers
            pendingOrders.push(
                order.getId(),
                order.getPriority(),
                order.getCreatedAt()
            );
            break;
        }

        DealerEntry de = heapDealers.pop();
        auto dealerIt = dealers.find(de.dealerId);
        if (dealerIt == dealers.end()) continue;

        Dealer& dealer = dealerIt->second;

        // Asignar
        order.setDealerId(dealer.getId());
        order.setStatus(OrderStatus::ASSIGNED);
        order.setAssignedAt(timeSystem.getSimulatedTime());

        dealer.assignOrder(order.getId());
        dealer.setStatus(DealerStatus::HEADING_TO_RESTAURANT);

        // Calcular ruta con Dijkstra
        TrafficEdgeData traffic = trafficSystem.getCurrentTraffic();
        RouteResult route = Dijkstra::compute(
            graph,
            dealer.getCurrentNodeId(),
            restaurants.at(order.getRestaurantId()).getNodeId(),
            traffic
        );

        if (route.success) {
            order.setEstimatedTime(route.path.estimatedTime);
            order.setTotalDistance(route.path.totalDistance);
        }

        // Si el dealer aún tiene capacidad, vuelve al heap
        if (!dealer.isFull()) {
            double score = computeDealerScore(
                dealer,
                restaurants.count(order.getRestaurantId())
                ? restaurants.at(order.getRestaurantId()).getNodeId()
                : ""
            );
            heapDealers.push(dealer.getId(), score);
        }
    }
}

void Simulator::updateDealers(double simDeltaMinutes) {
    TrafficEdgeData traffic = trafficSystem.getCurrentTraffic();

    for (auto& [id, dealer] : dealers) {
        if (dealer.getStatus() == DealerStatus::IDLE) continue;
        if (dealer.getActiveOrderIds().empty()) {
            dealer.setStatus(DealerStatus::IDLE);
            if (!heapDealers.contains(dealer.getId())) {
                heapDealers.push(dealer.getId(), 0.0);
            }
            continue;
        }

        // Avanzar la primera orden activa del dealer
        const std::string& orderId = dealer.getActiveOrderIds().front();
        auto orderIt = orders.find(orderId);
        if (orderIt == orders.end()) continue;

        Order& order = orderIt->second;
        double remaining = order.getEstimatedTime() - simDeltaMinutes;

        if (remaining <= 0.0) {
            // Entrega completada
            order.setStatus(OrderStatus::DELIVERED);
            order.setDeliveredAt(timeSystem.getSimulatedTime());

            dealer.completeDelivery(order.getTotalDistance());
            dealer.removeOrder(orderId);

            // Notificar restaurante
            auto restIt = restaurants.find(order.getRestaurantId());
            if (restIt != restaurants.end()) {
                restIt->second.orderDispatched();
                avlRestaurants.updateScore(
                    restIt->first,
                    restIt->second.getPopularityScore()
                );
            }

            // Registrar en historial
            deliveryHistory.record(DeliveryRecord(
                order.getId(),
                order.getRestaurantId(),
                dealer.getId(),
                order.getClientNodeId(),
                timeSystem.getSimulatedTime(),
                order.waitTime(),
                order.deliveryTime(),
                order.getTotalDistance(),
                order.getPriority()
            ));

            stats.totalDelivered++;

            if (dealer.getActiveOrderIds().empty()) {
                dealer.setStatus(DealerStatus::IDLE);
                if (!heapDealers.contains(dealer.getId())) {
                    heapDealers.push(dealer.getId(), 0.0);
                }
            }
        } else {
            order.setEstimatedTime(remaining);
        }
    }
}

void Simulator::boostStaleOrders(double simDeltaMinutes) {
    boostCheckTimer += simDeltaMinutes;
    if (boostCheckTimer >= BOOST_INTERVAL) {
        boostCheckTimer = 0.0;
        pendingOrders.boostStaleOrders(
            timeSystem.getSimulatedTime(),
            STALE_THRESHOLD,
            BOOST_AMOUNT
        );
    }
}

// ── Helpers ───────────────────────────────────────────────────

double Simulator::computeDealerScore(
    const Dealer& dealer,
    const std::string& restaurantNodeId
) const {
    constexpr double ALPHA = 0.6; // peso distancia
    constexpr double BETA  = 0.4; // peso carga

    double availabilityScore = 1.0 - dealer.loadFactor();

    // Distancia estimada con Dijkstra
    double distScore = 1.0;
    if (!restaurantNodeId.empty() &&
        graph.hasNode(dealer.getCurrentNodeId()) &&
        graph.hasNode(restaurantNodeId))
    {
        TrafficEdgeData traffic = trafficSystem.getCurrentTraffic();
        RouteResult r = Dijkstra::compute(
            graph,
            dealer.getCurrentNodeId(),
            restaurantNodeId,
            traffic
        );
        if (r.success && r.path.totalCost > 0.0) {
            distScore = 1.0 / r.path.totalCost;
        }
    }

    return ALPHA * distScore + BETA * availabilityScore;
}

void Simulator::rebuildDealerHeap(
    const std::string& restaurantNodeId
) {
    // Vaciar y reconstruir con scores frescos
    while (!heapDealers.isEmpty()) heapDealers.pop();

    for (const auto& [id, dealer] : dealers) {
        if (dealer.isAvailable()) {
            double score = computeDealerScore(dealer, restaurantNodeId);
            heapDealers.push(id, score);
        }
    }
}

void Simulator::updateStats() {
    stats.activeOrders = 0;
    for (const auto& [id, o] : orders) {
        if (o.isActive()) stats.activeOrders++;
    }

    stats.idleDealers = 0;
    for (const auto& [id, d] : dealers) {
        if (d.isAvailable()) stats.idleDealers++;
    }

    stats.avgDeliveryTime = deliveryHistory.averageDeliveryTime();
    stats.avgWaitTime     = deliveryHistory.averageWaitTime();
}

// ── Acceso al estado ──────────────────────────────────────────

const TimeSystem& Simulator::getTimeSystem() const {
    return timeSystem;
}

const WeatherSystem& Simulator::getWeatherSystem() const {
    return weatherSystem;
}

const TrafficSystem& Simulator::getTrafficSystem() const {
    return trafficSystem;
}

EventSystem& Simulator::getEventSystem() {
    return eventSystem;
}

const std::unordered_map<std::string, Order>&
Simulator::getOrders() const { return orders; }

const std::unordered_map<std::string, Dealer>&
Simulator::getDealers() const { return dealers; }

const std::unordered_map<std::string, Restaurant>&
Simulator::getRestaurants() const { return restaurants; }

const DeliveryHistory& Simulator::getHistory() const {
    return deliveryHistory;
}

const SimStats& Simulator::getStats() const {
    return stats;
}

RouteResult Simulator::getRouteFor(
    const std::string& dealerId
) const {
    auto it = dealers.find(dealerId);
    if (it == dealers.end()) return RouteResult{};

    const Dealer& dealer = it->second;
    if (dealer.getActiveOrderIds().empty()) return RouteResult{};

    const std::string& orderId = dealer.getActiveOrderIds().front();
    auto orderIt = orders.find(orderId);
    if (orderIt == orders.end()) return RouteResult{};

    const Order& order = orderIt->second;
    auto restIt = restaurants.find(order.getRestaurantId());
    if (restIt == restaurants.end()) return RouteResult{};

    return Dijkstra::compute(
        graph,
        dealer.getCurrentNodeId(),
        order.getClientNodeId(),
        trafficSystem.getCurrentTraffic()
    );
}

void Simulator::setWeather(WeatherState state) {
    weatherSystem.setWeather(state);
}

void Simulator::triggerEvent(
    EventState event,
    double durationMinutes
) {
    trafficSystem.setEvent(event, durationMinutes);
}