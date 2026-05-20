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

// Estadísticas globales de la simulación (para la UI)
struct SimStats {
    int    totalOrdersGenerated = 0;
    int    totalDelivered       = 0;
    int    totalCancelled       = 0;
    int    activeOrders         = 0;
    int    idleDealers          = 0;
    double avgDeliveryTime      = 0.0;
    double avgWaitTime          = 0.0;
};

// Orquesta todos los sistemas. El render loop llama a tick()
// con el delta real de SFML, y consulta el estado para dibujar.
//
// Loop externo (RendererSFML):
//   while (window.isOpen()) {
//       float dt = clock.restart().asSeconds();
//       simulator.tick(dt);
//       renderer.draw(simulator);
//   }

class Simulator {
private:
    // ── Core ──────────────────────────────────────────────────
    const Graph& graph;

    // ── Sistemas de simulación ────────────────────────────────
    TimeSystem    timeSystem;
    WeatherSystem weatherSystem;
    TrafficSystem trafficSystem;
    EventSystem   eventSystem;
    OrderGenerator orderGenerator;

    // ── Estructuras de datos ──────────────────────────────────
    HeapDealers          heapDealers;
    PriorityQueueOrders  pendingOrders;
    AVLRestaurants       avlRestaurants;
    DeliveryHistory      deliveryHistory;

    // ── Registros en memoria ──────────────────────────────────
    std::unordered_map<std::string, Order>      orders;
    std::unordered_map<std::string, Dealer>     dealers;
    std::unordered_map<std::string, Restaurant> restaurants;
    std::unordered_map<std::string, Client>     clients;

    // ── Estado ────────────────────────────────────────────────
    bool    running;
    SimStats stats;

    // Intervalo en minutos simulados para boost de órdenes viejas
    double boostCheckTimer;
    static constexpr double BOOST_INTERVAL   = 5.0;  // cada 5 min sim
    static constexpr double STALE_THRESHOLD  = 10.0; // orden espera 10 min
    static constexpr int    BOOST_AMOUNT     = 2;    // +2 prioridad

public:
    explicit Simulator(const Graph& graph);

    // ── Setup (llamar antes de start()) ───────────────────────
    void addDealer(const Dealer& dealer);
    void addRestaurant(const Restaurant& restaurant);
    void addClient(const Client& client);

    void start();
    void pause();
    void resume();
    void reset();

    // ── Loop principal ────────────────────────────────────────
    // realDeltaSeconds: tiempo real desde el último frame (SFML dt)
    void tick(double realDeltaSeconds);

    // ── Acceso al estado (para RendererSFML / UI) ─────────────
    const TimeSystem&    getTimeSystem()    const;
    const WeatherSystem& getWeatherSystem() const;
    const TrafficSystem& getTrafficSystem() const;
    EventSystem&         getEventSystem();

    const std::unordered_map<std::string, Order>&      getOrders()      const;
    const std::unordered_map<std::string, Dealer>&     getDealers()     const;
    const std::unordered_map<std::string, Restaurant>& getRestaurants() const;
    const DeliveryHistory& getHistory() const;

    const SimStats& getStats() const;

    // Calcula la ruta actual de un dealer (para visualización)
    RouteResult getRouteFor(const std::string& dealerId) const;

    // Control manual de clima y eventos desde la UI
    void setWeather(WeatherState state);
    void triggerEvent(EventState event, double durationMinutes);
    const Graph& getGraph() const { return graph; }
    void setSimulationSpeed(double speed) {
    timeSystem.setSimulationSpeed(speed);
    
}

private:
    // ── Pasos internos del tick ───────────────────────────────

    // 1. Genera nuevas órdenes y las ingresa al heap + registro
    void processNewOrders();

    // 2. Asigna órdenes pendientes a dealers disponibles
    void assignOrders();

    // 3. Avanza el estado de cada dealer en tránsito
    void updateDealers(double simDeltaMinutes);

    // 4. Boost de órdenes que llevan mucho esperando
    void boostStaleOrders(double simDeltaMinutes);

    // ── Helpers ───────────────────────────────────────────────

    // Calcula el score logístico para un dealer frente a una orden
    // Score = α·(1/dist) + β·(1-loadFactor)
    double computeDealerScore(
        const Dealer& dealer,
        const std::string& restaurantNodeId
    ) const;

    // Reconstruye el heap de dealers con scores frescos
    // para la orden actual
    void rebuildDealerHeap(const std::string& restaurantNodeId);

    void updateStats();
};