#pragma once

#include <unordered_map>
#include <string>

#include "../core/Graph.h"
#include "../models/Order.h"
#include "../models/Dealer.h"
#include "../models/Restaurant.h"
#include "../models/Client.h"

#include "../simulation/TimeSystem.h"
#include "../simulation/WeatherSystem.h"
#include "../simulation/TrafficSystem.h"
#include "../simulation/EventSystem.h"
#include "../simulation/OrderGenerator.h"

#include "../structures/HeapDealers.h"
#include "../structures/PriorityQueueOrders.h"
#include "../structures/AVLRestaurants.h"
#include "../structures/DeliveryHistory.h"

#include "../algorithms/RouteResult.h"

// Estadisticas globales de la simulacion (para la UI)
struct SimStats {
    int    totalOrdersGenerated = 0;
    int    totalDelivered       = 0;
    int    totalCancelled       = 0;
    int    activeOrders         = 0;
    int    idleDealers          = 0;
    double avgDeliveryTime      = 0.0;
    double avgWaitTime          = 0.0;
};

// ── Fase de entrega por dealer ────────────────────────────────
// El ciclo de vida de una orden tiene tres fases visuales:
//
//  HEADING_TO_RESTAURANT
//    Dealer se mueve desde su posicion hasta el restaurante.
//    Timer: tiempo de la ruta dealer→restaurante (Dijkstra).
//
//  PICKING_UP
//    Dealer llega al restaurante y espera DEALER_PICKUP_TIME minutos.
//    Durante este tiempo el punto rojo (restaurante) permanece visible.
//    El dealer esta estatico en el nodo del restaurante.
//
//  DELIVERING
//    Dealer se mueve desde el restaurante hasta el cliente.
//    Timer: tiempo de la ruta restaurante→cliente (Dijkstra).
//
enum class DeliveryPhase {
    HEADING_TO_RESTAURANT,
    PICKING_UP,
    DELIVERING
};

struct DealerPhaseState {
    DeliveryPhase phase     = DeliveryPhase::HEADING_TO_RESTAURANT;
    double        phaseTimer = 0.0;  // minutos simulados restantes en fase actual
    std::string   orderId;           // orden en curso
};

// Orquesta todos los sistemas. El render loop llama a tick()
// con el delta real de SFML, y consulta el estado para dibujar.

class Simulator {
private:
    const Graph& graph;

    TimeSystem    timeSystem;
    WeatherSystem weatherSystem;
    TrafficSystem trafficSystem;
    EventSystem   eventSystem;
    OrderGenerator orderGenerator;

    HeapDealers          heapDealers;
    PriorityQueueOrders  pendingOrders;
    AVLRestaurants       avlRestaurants;
    DeliveryHistory      deliveryHistory;

    std::unordered_map<std::string, Order>      orders;
    std::unordered_map<std::string, Dealer>     dealers;
    std::unordered_map<std::string, Restaurant> restaurants;
    std::unordered_map<std::string, Client>     clients;

    // Fase actual por dealer — separada del modelo Dealer
    // para no contaminar el modelo de datos con logica visual
    std::unordered_map<std::string, DealerPhaseState> dealerPhases;

    bool     running;
    SimStats stats;

    double boostCheckTimer;
    static constexpr double BOOST_INTERVAL   = 5.0;
    static constexpr double STALE_THRESHOLD  = 10.0;
    static constexpr int    BOOST_AMOUNT     = 2;

public:
    explicit Simulator(const Graph& graph);

    void addDealer(const Dealer& dealer);
    void addRestaurant(const Restaurant& restaurant);
    void addClient(const Client& client);

    void start();
    void pause();
    void resume();
    void reset();

    void tick(double realDeltaSeconds);

    const TimeSystem&    getTimeSystem()    const;
    const WeatherSystem& getWeatherSystem() const;
    const TrafficSystem& getTrafficSystem() const;
    EventSystem&         getEventSystem();

    const std::unordered_map<std::string, Order>&      getOrders()      const;
    const std::unordered_map<std::string, Dealer>&     getDealers()     const;
    const std::unordered_map<std::string, Restaurant>& getRestaurants() const;
    const DeliveryHistory& getHistory() const;

    const SimStats& getStats() const;

    // Fase actual de un dealer (para que RendererSFML sepa en que tramo esta)
    const DealerPhaseState* getPhase(const std::string& dealerId) const;

    RouteResult getRouteFor(const std::string& dealerId) const;

    void setWeather(WeatherState state);
    void triggerEvent(EventState event, double durationMinutes);
    const Graph& getGraph() const { return graph; }
    void setSimulationSpeed(double speed) {
        timeSystem.setSimulationSpeed(speed);
    }

private:
    void processNewOrders();
    void assignOrders();
    void updateDealers(double simDeltaMinutes);
    void boostStaleOrders(double simDeltaMinutes);

    double computeDealerScore(
        const Dealer& dealer,
        const std::string& restaurantNodeId
    ) const;

    void rebuildDealerHeap(const std::string& restaurantNodeId);
    void updateStats();
};