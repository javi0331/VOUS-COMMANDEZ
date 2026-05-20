#include "../simulation/Simulator.h"
#include "../algorithms/Dijkstra.h"
#include "../utils/Logger.h"
#include "../utils/Random.h"
#include "../utils/Config.h"
#include <cmath>
#include <algorithm>

Simulator::Simulator(const Graph& graph)
    : graph(graph),
      timeSystem(Config::SIM_SPEED_DEFAULT),
      trafficSystem(timeSystem, weatherSystem),
      eventSystem(trafficSystem, weatherSystem),
      orderGenerator(timeSystem),
      running(false),
      boostCheckTimer(0.0) {}

// ── Setup ─────────────────────────────────────────────────────

void Simulator::addDealer(const Dealer& dealer) {
    dealers[dealer.getId()] = dealer;
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
        1.0
    );
}

void Simulator::addClient(const Client& client) {
    clients[client.getId()] = client;
    orderGenerator.addClientNode(client.getNodeId());
}

// ── Control ───────────────────────────────────────────────────

void Simulator::start() {
    running = true;
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
    dealerPhases.clear();
    while (!heapDealers.isEmpty()) heapDealers.pop();
    deliveryHistory.clear();
    stats = SimStats{};
    running = false;
}

// ── Loop principal ────────────────────────────────────────────

void Simulator::tick(double realDeltaSeconds) {
    if (!running) return;

    timeSystem.update(realDeltaSeconds);
    double simDelta = realDeltaSeconds * timeSystem.getSimulationSpeed();

    weatherSystem.update(simDelta);
    trafficSystem.update(simDelta);
    eventSystem.update(timeSystem.getSimulatedTime());

    orderGenerator.update(simDelta);
    processNewOrders();

    assignOrders();
    updateDealers(simDelta);
    boostStaleOrders(simDelta);
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

        auto it = restaurants.find(order.getRestaurantId());
        if (it != restaurants.end()) {
            it->second.orderReceived();
            avlRestaurants.updateScore(
                it->second.getId(),
                it->second.getPopularityScore()
            );
        }

        stats.totalOrdersGenerated++;
        Logger::info("Nueva orden " + order.getId()
                     + " desde " + order.getRestaurantId());
    }
}

void Simulator::assignOrders() {
    while (!pendingOrders.isEmpty() && !heapDealers.isEmpty()) {

        OrderEntry oe = pendingOrders.pop();

        auto orderIt = orders.find(oe.orderId);
        if (orderIt == orders.end()) continue;

        Order& order = orderIt->second;
        if (!order.isPending()) continue;

        // Buscar el nodo del restaurante con guard
        auto restIt = restaurants.find(order.getRestaurantId());
        if (restIt == restaurants.end()) {
            Logger::warn("assignOrders: restaurante no encontrado "
                         + order.getRestaurantId());
            order.setStatus(OrderStatus::CANCELLED);
            stats.totalCancelled++;
            continue;
        }

        const std::string& restNodeId = restIt->second.getNodeId();

        // Verificar que el nodo existe en el grafo
        if (!graph.hasNode(restNodeId)) {
            Logger::warn("assignOrders: nodo restaurante no en grafo "
                         + restNodeId);
            order.setStatus(OrderStatus::CANCELLED);
            stats.totalCancelled++;
            continue;
        }

        if (!graph.hasNode(order.getClientNodeId())) {
            Logger::warn("assignOrders: nodo cliente no en grafo "
                         + order.getClientNodeId());
            order.setStatus(OrderStatus::CANCELLED);
            stats.totalCancelled++;
            continue;
        }

        // Reconstruir heap con scores frescos para este restaurante
        rebuildDealerHeap(restNodeId);

        if (heapDealers.isEmpty()) {
            // Devolver orden — no hay dealers disponibles ahora
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

        order.setDealerId(dealer.getId());
        order.setStatus(OrderStatus::ASSIGNED);
        order.setAssignedAt(timeSystem.getSimulatedTime());

        dealer.assignOrder(order.getId());
        dealer.setStatus(DealerStatus::HEADING_TO_RESTAURANT);

        TrafficEdgeData traffic = trafficSystem.getCurrentTraffic();

        // Tramo 1: dealer → restaurante
        RouteResult leg1;
        if (graph.hasNode(dealer.getCurrentNodeId())) {
            leg1 = Dijkstra::compute(
                graph,
                dealer.getCurrentNodeId(),
                restNodeId,
                traffic
            );
        }

        // Tramo 2: restaurante → cliente
        RouteResult leg2 = Dijkstra::compute(
            graph,
            restNodeId,
            order.getClientNodeId(),
            traffic
        );

        double totalTime = Config::DEALER_PICKUP_TIME;
        double totalDist = 0.0;

        if (leg1.success) {
            totalTime += leg1.path.estimatedTime;
            totalDist += leg1.path.totalDistance;
        }
        if (leg2.success) {
            totalTime += leg2.path.estimatedTime;
            totalDist += leg2.path.totalDistance;
        }

        order.setEstimatedTime(totalTime);
        order.setTotalDistance(totalDist);

        // Inicializar fase del dealer
        DealerPhaseState phase;
        phase.orderId    = order.getId();
        phase.phase      = DeliveryPhase::HEADING_TO_RESTAURANT;
        phase.phaseTimer = leg1.success ? leg1.path.estimatedTime
                                        : Config::DEALER_PICKUP_TIME;
        dealerPhases[dealer.getId()] = phase;

        Logger::info("Dealer " + dealer.getId()
                     + " asignado a " + order.getId());

        // Si el dealer aún tiene capacidad, vuelve al heap
        if (!dealer.isFull()) {
            double score = computeDealerScore(dealer, restNodeId);
            heapDealers.push(dealer.getId(), score);
        }
    }
}

// ── Maquina de estados: HEADING → PICKING_UP → DELIVERING ────

void Simulator::updateDealers(double simDeltaMinutes) {
    TrafficEdgeData traffic = trafficSystem.getCurrentTraffic();

    for (auto& [id, dealer] : dealers) {

        if (dealer.getStatus() == DealerStatus::IDLE) continue;

        if (dealer.getActiveOrderIds().empty()) {
            dealer.setStatus(DealerStatus::IDLE);
            dealerPhases.erase(id);
            if (!heapDealers.contains(id))
                heapDealers.push(id, 0.0);
            continue;
        }

        auto phaseIt = dealerPhases.find(id);
        if (phaseIt == dealerPhases.end()) continue;

        DealerPhaseState& ps = phaseIt->second;

        auto orderIt = orders.find(ps.orderId);
        if (orderIt == orders.end()) continue;

        Order& order = orderIt->second;

        ps.phaseTimer -= simDeltaMinutes;

        switch (ps.phase) {

        // ── Fase 1: yendo al restaurante ──────────────────────
        case DeliveryPhase::HEADING_TO_RESTAURANT:
            if (ps.phaseTimer <= 0.0) {
                // Llega al restaurante — actualizar posicion del dealer
                auto restIt = restaurants.find(order.getRestaurantId());
                if (restIt != restaurants.end()
                    && graph.hasNode(restIt->second.getNodeId()))
                {
                    dealer.setCurrentNodeId(restIt->second.getNodeId());
                }

                order.setStatus(OrderStatus::PICKING_UP);
                dealer.setStatus(DealerStatus::PICKING_UP);

                ps.phase      = DeliveryPhase::PICKING_UP;
                ps.phaseTimer = Config::DEALER_PICKUP_TIME;

                Logger::info("Dealer " + id
                             + " llego al restaurante, recogiendo "
                             + ps.orderId);
            }
            break;

        // ── Fase 2: esperando en el restaurante ───────────────
        case DeliveryPhase::PICKING_UP:
            if (ps.phaseTimer <= 0.0) {
                // Calcular ruta fresca restaurante → cliente
                double leg2Time = Config::DEALER_PICKUP_TIME; // fallback

                auto restIt = restaurants.find(order.getRestaurantId());
                if (restIt != restaurants.end()
                    && graph.hasNode(restIt->second.getNodeId())
                    && graph.hasNode(order.getClientNodeId()))
                {
                    RouteResult leg2 = Dijkstra::compute(
                        graph,
                        restIt->second.getNodeId(),
                        order.getClientNodeId(),
                        traffic
                    );
                    if (leg2.success)
                        leg2Time = leg2.path.estimatedTime;
                }

                order.setStatus(OrderStatus::IN_TRANSIT);
                dealer.setStatus(DealerStatus::DELIVERING);

                ps.phase      = DeliveryPhase::DELIVERING;
                ps.phaseTimer = leg2Time;

                Logger::info("Dealer " + id
                             + " sale del restaurante entregando "
                             + ps.orderId);
            }
            break;

        // ── Fase 3: entregando al cliente ─────────────────────
        case DeliveryPhase::DELIVERING:
            if (ps.phaseTimer <= 0.0) {
                order.setStatus(OrderStatus::DELIVERED);
                order.setDeliveredAt(timeSystem.getSimulatedTime());

                dealer.completeDelivery(order.getTotalDistance());
                dealer.removeOrder(ps.orderId);

                auto restIt = restaurants.find(order.getRestaurantId());
                if (restIt != restaurants.end()) {
                    restIt->second.orderDispatched();
                    avlRestaurants.updateScore(
                        restIt->first,
                        restIt->second.getPopularityScore()
                    );
                }

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

                Logger::info("Dealer " + id
                             + " entrego " + ps.orderId);

                dealerPhases.erase(id);

                if (dealer.getActiveOrderIds().empty()) {
                    dealer.setStatus(DealerStatus::IDLE);
                    if (!heapDealers.contains(id))
                        heapDealers.push(id, 0.0);
                }
            }
            break;
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
    constexpr double ALPHA = 0.6;
    constexpr double BETA  = 0.4;

    double availabilityScore = 1.0 - dealer.loadFactor();
    double distScore = 1.0;

    if (!restaurantNodeId.empty()
        && graph.hasNode(dealer.getCurrentNodeId())
        && graph.hasNode(restaurantNodeId))
    {
        TrafficEdgeData traffic = trafficSystem.getCurrentTraffic();
        RouteResult r = Dijkstra::compute(
            graph,
            dealer.getCurrentNodeId(),
            restaurantNodeId,
            traffic
        );
        if (r.success && r.path.totalCost > 0.0)
            distScore = 1.0 / r.path.totalCost;
    }

    return ALPHA * distScore + BETA * availabilityScore;
}

void Simulator::rebuildDealerHeap(
    const std::string& restaurantNodeId
) {
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
    for (const auto& [id, o] : orders)
        if (o.isActive()) stats.activeOrders++;

    stats.idleDealers = 0;
    for (const auto& [id, d] : dealers)
        if (d.isAvailable()) stats.idleDealers++;

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

const DealerPhaseState* Simulator::getPhase(
    const std::string& dealerId
) const {
    auto it = dealerPhases.find(dealerId);
    return it != dealerPhases.end() ? &it->second : nullptr;
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

    const std::string& restNodeId   = restIt->second.getNodeId();
    const std::string& clientNodeId = order.getClientNodeId();

    if (!graph.hasNode(restNodeId) || !graph.hasNode(clientNodeId))
        return RouteResult{};

    TrafficEdgeData traffic = trafficSystem.getCurrentTraffic();

    // Devuelve la ruta del tramo actual segun la fase
    auto phaseIt = dealerPhases.find(dealerId);
    if (phaseIt != dealerPhases.end()
        && phaseIt->second.phase == DeliveryPhase::DELIVERING)
    {
        // Tramo 2: restaurante → cliente
        return Dijkstra::compute(graph, restNodeId, clientNodeId, traffic);
    }

    // Tramo 1: dealer → restaurante
    if (!graph.hasNode(dealer.getCurrentNodeId()))
        return RouteResult{};

    return Dijkstra::compute(
        graph,
        dealer.getCurrentNodeId(),
        restNodeId,
        traffic
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