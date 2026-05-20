#pragma once

#include <string>
#include "../core/Zone.h"

class Restaurant {
private:
    std::string id;
    std::string name;
    std::string nodeId;     // Nodo del grafo donde está ubicado
    Zone zone;

    // ── Métricas dinámicas (para el AVL) ──────────────────────
    int    activeOrders;    // Pedidos en curso ahora mismo
    int    totalOrders;     // Histórico acumulado
    double averageRating;   // Promedio de calificaciones
    double avgPrepTime;     // Tiempo promedio de preparación (minutos)

    // Score compuesto usado por AVLRestaurants como clave de ordenamiento
    // Score = f(totalOrders, averageRating, activeOrders)
    double popularityScore;

public:
    Restaurant();

    Restaurant(
        const std::string& id,
        const std::string& name,
        const std::string& nodeId,
        Zone zone
    );

    // ── Getters estáticos ─────────────────────────────────────
    const std::string& getId()     const;
    const std::string& getName()   const;
    const std::string& getNodeId() const;
    Zone               getZone()   const;

    // ── Getters dinámicos ─────────────────────────────────────
    int    getActiveOrders()   const;
    int    getTotalOrders()    const;
    double getAverageRating()  const;
    double getAvgPrepTime()    const;
    double getPopularityScore() const;

    // ── Actualización en tiempo de simulación ─────────────────
    void orderReceived();               // +1 activeOrders, +1 totalOrders
    void orderDispatched();             // -1 activeOrders
    void addRating(double score);       // Actualiza averageRating
    void updatePrepTime(double minutes); // Promedio acumulado

    // Recalcula popularityScore — llamar después de cambios
    void recalculateScore();

    // ── Helpers ───────────────────────────────────────────────
    bool isBusy() const;    // activeOrders >= umbral (definido en Config)
    std::string zoneName() const;
};