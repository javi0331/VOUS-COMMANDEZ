#pragma once

#include <string>
#include "../models/OrderStatus.h"

class Order {
private:
    std::string id;
    std::string restaurantId;   // Nodo origen (restaurante)
    std::string clientId;       // Cliente que hizo el pedido
    std::string dealerId;       // Repartidor asignado (vacío si PENDING)
    std::string clientNodeId;   // Nodo destino en el grafo

    OrderStatus status;

    int priority;               // Mayor número = mayor prioridad (para el heap)

    double estimatedTime;       // Minutos estimados de entrega (calculado por Dijkstra)
    double totalDistance;       // Km totales de la ruta
    double createdAt;           // Tiempo de simulación al crear la orden
    double assignedAt;          // Tiempo al asignar repartidor
    double deliveredAt;         // Tiempo al entregar

public:
    Order();

    Order(
        const std::string& id,
        const std::string& restaurantId,
        const std::string& clientId,
        const std::string& clientNodeId,
        int priority,
        double createdAt
    );

    // ── Getters ───────────────────────────────────────────────
    const std::string& getId()          const;
    const std::string& getRestaurantId() const;
    const std::string& getClientId()    const;
    const std::string& getDealerId()    const;
    const std::string& getClientNodeId() const;

    OrderStatus getStatus()         const;
    int         getPriority()       const;
    double      getEstimatedTime()  const;
    double      getTotalDistance()  const;
    double      getCreatedAt()      const;
    double      getAssignedAt()     const;
    double      getDeliveredAt()    const;

    // ── Setters (solo los que cambian durante la simulación) ──
    void setDealerId(const std::string& dealerId);
    void setStatus(OrderStatus status);
    void setEstimatedTime(double minutes);
    void setTotalDistance(double km);
    void setAssignedAt(double time);
    void setDeliveredAt(double time);

    // ── Helpers ───────────────────────────────────────────────
    bool isPending()    const;
    bool isActive()     const;  // ASSIGNED, PICKING_UP o IN_TRANSIT
    bool isCompleted()  const;  // DELIVERED o CANCELLED
    double waitTime()   const;  // assignedAt - createdAt
    double deliveryTime() const; // deliveredAt - assignedAt
};