#pragma once

#include <string>
#include <vector>
#include <unordered_map>
#include "../models/Order.h"
#include "../simulation/TimeSystem.h"

// Genera órdenes de forma procedural basándose en:
//   - Hora del día (más pedidos en almuerzo y noche)
//   - Popularidad del restaurante (restaurantes activos generan más)
//   - Tasa base configurable (pedidos por minuto simulado)
//
// Simulator llama a update() cada tick y recoge las nuevas órdenes
// con takeGenerated().

class OrderGenerator {
private:
    const TimeSystem& timeSystem;

    double spawnAccumulator;    // Tiempo acumulado para el próximo spawn
    double baseSpawnRate;       // Pedidos por minuto simulado (base)
    int    nextOrderIndex;      // Contador para generar IDs únicos

    // Nodos cliente disponibles por zona (cargados al inicio)
    std::vector<std::string> clientNodeIds;

    // Restaurantes disponibles con sus pesos de popularidad
    struct RestaurantWeight {
        std::string restaurantId;
        std::string nodeId;
        double      weight;     // Probabilidad relativa de ser elegido
    };
    std::vector<RestaurantWeight> restaurants;

    // Órdenes generadas pendientes de ser recogidas por Simulator
    std::vector<Order> pendingOrders;

public:
    explicit OrderGenerator(
        const TimeSystem& time,
        double baseSpawnRate = 0.5   // 0.5 pedidos/min = 1 cada 2 min base
    );

    // Avanza el generador y acumula tiempo simulado.
    // Genera órdenes cuando el acumulador supera el intervalo.
    void update(double simDeltaMinutes);

    // ── Configuración (llamar antes de la sim) ────────────────
    void addClientNode(const std::string& nodeId);
    void addRestaurant(
        const std::string& restaurantId,
        const std::string& nodeId,
        double weight = 1.0
    );
    void updateRestaurantWeight(
        const std::string& restaurantId,
        double newWeight
    );

    void setBaseSpawnRate(double rate);

    // ── Recogida de órdenes ───────────────────────────────────
    // Devuelve y vacía la lista de órdenes generadas desde
    // el último tick. Simulator las procesa e ingresa al heap.
    std::vector<Order> takeGenerated();

    bool hasGenerated() const;

private:
    // Factor de demanda según hora del día
    // Almuerzo (12-14h): ×2.0
    // Cena (19-22h):     ×1.8
    // Madrugada (0-6h):  ×0.2
    // Resto:             ×1.0
    double demandFactor() const;

    // Selecciona un restaurante al azar ponderado por weight
    const RestaurantWeight* pickRestaurant();

    // Selecciona un nodo cliente al azar
    std::string pickClientNode();

    // Genera una orden completa
    Order generateOrder();

    std::string makeOrderId();
};