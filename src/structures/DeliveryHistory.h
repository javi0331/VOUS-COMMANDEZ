#pragma once

#include <deque>
#include <string>
#include <vector>

// Registro de una entrega completada — snapshot inmutable
struct DeliveryRecord {
    std::string orderId;
    std::string restaurantId;
    std::string dealerId;
    std::string clientNodeId;

    double completedAt;     // Tiempo de simulación al entregar
    double waitTime;        // Tiempo desde creación hasta asignación (min)
    double deliveryTime;    // Tiempo desde asignación hasta entrega (min)
    double totalDistance;   // Km recorridos
    int    priority;        // Prioridad original de la orden

    DeliveryRecord() = default;

    DeliveryRecord(
        const std::string& orderId,
        const std::string& restaurantId,
        const std::string& dealerId,
        const std::string& clientNodeId,
        double completedAt,
        double waitTime,
        double deliveryTime,
        double totalDistance,
        int    priority
    )
        : orderId(orderId),
          restaurantId(restaurantId),
          dealerId(dealerId),
          clientNodeId(clientNodeId),
          completedAt(completedAt),
          waitTime(waitTime),
          deliveryTime(deliveryTime),
          totalDistance(totalDistance),
          priority(priority) {}
};

// Historial cronológico de entregas usando std::deque.
// - push_back O(1): nueva entrega al final
// - front / back O(1): primera y última entrega
// - Capacidad máxima configurable (descarta las más antiguas)
// - Estadísticas calculadas bajo demanda

class DeliveryHistory {
private:
    std::deque<DeliveryRecord> records;
    int maxCapacity; // 0 = sin límite

public:
    explicit DeliveryHistory(int maxCapacity = 0);

    // ── Inserción ─────────────────────────────────────────────

    // Registra una nueva entrega. Si se supera maxCapacity,
    // descarta la más antigua (pop_front).
    void record(const DeliveryRecord& delivery);

    // ── Acceso ────────────────────────────────────────────────
    const DeliveryRecord& latest()  const; // más reciente
    const DeliveryRecord& oldest()  const; // más antigua

    // Devuelve los últimos N registros (más recientes primero)
    std::vector<DeliveryRecord> getLastN(int n) const;

    // Devuelve todos los registros de un repartidor específico
    std::vector<DeliveryRecord> getByDealer(
        const std::string& dealerId
    ) const;

    // Devuelve todos los registros de un restaurante específico
    std::vector<DeliveryRecord> getByRestaurant(
        const std::string& restaurantId
    ) const;

    // ── Estadísticas ──────────────────────────────────────────
    double averageDeliveryTime()  const;
    double averageWaitTime()      const;
    double averageDistance()      const;
    int    totalDeliveries()      const;

    // ── Utilidades ────────────────────────────────────────────
    bool isEmpty() const;
    void clear();
};