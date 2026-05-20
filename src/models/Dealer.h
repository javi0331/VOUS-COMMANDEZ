#pragma once

#include <string>
#include <vector>
#include "../models/DealerStatus.h"

class Dealer {
private:
    std::string id;
    std::string name;
    std::string currentNodeId;  // Nodo actual en el grafo

    DealerStatus status;

    std::vector<std::string> activeOrderIds;  // Pedidos en curso
    int maxOrders;                            // Capacidad máxima simultánea

    double totalDeliveries;     // Histórico: entregas completadas
    double totalDistanceKm;     // Histórico: km recorridos
    double rating;              // Score promedio (0.0 - 5.0)

    // Score logístico para el HeapDealers
    // Score = α·distancia + β·tiempoEstimado + γ·cargaActual
    // Calculado externamente por HeapDealers según pesos configurables
    double logisticScore;

public:
    Dealer();

    Dealer(
        const std::string& id,
        const std::string& name,
        const std::string& startNodeId,
        int maxOrders = 3
    );

    // ── Getters ───────────────────────────────────────────────
    const std::string& getId()          const;
    const std::string& getName()        const;
    const std::string& getCurrentNodeId() const;

    DealerStatus getStatus()        const;
    int          getActiveCount()   const;
    int          getMaxOrders()     const;
    double       getTotalDeliveries() const;
    double       getTotalDistanceKm() const;
    double       getRating()        const;
    double       getLogisticScore() const;

    const std::vector<std::string>& getActiveOrderIds() const;

    // ── Setters ───────────────────────────────────────────────
    void setCurrentNodeId(const std::string& nodeId);
    void setStatus(DealerStatus status);
    void setLogisticScore(double score);

    // ── Gestión de pedidos ────────────────────────────────────
    bool assignOrder(const std::string& orderId);   // false si está lleno
    void removeOrder(const std::string& orderId);
    bool hasOrder(const std::string& orderId) const;

    // ── Actualización de historial ────────────────────────────
    void completeDelivery(double distanceKm);
    void addRating(double score);  // Promedio acumulado

    // ── Helpers ───────────────────────────────────────────────
    bool isAvailable() const;   // IDLE y con capacidad libre
    bool isFull()      const;   // activeOrderIds.size() >= maxOrders
    bool isActive()    const;
    double loadFactor() const;  // activeCount / maxOrders (0.0 - 1.0)
};