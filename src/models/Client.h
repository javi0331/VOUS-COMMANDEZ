#pragma once

#include <string>
#include <vector>
#include "../core/Zone.h"

class Client {
private:
    std::string id;
    std::string name;
    std::string nodeId;     // Nodo del grafo donde está ubicado
    Zone zone;

    // ── Historial ─────────────────────────────────────────────
    std::vector<std::string> orderHistory;  // IDs de órdenes realizadas
    double totalSpent;                      // Gasto acumulado simulado
    double averageRating;                   // Rating promedio que da

public:
    Client();

    Client(
        const std::string& id,
        const std::string& name,
        const std::string& nodeId,
        Zone zone
    );

    // ── Getters ───────────────────────────────────────────────
    const std::string& getId()     const;
    const std::string& getName()   const;
    const std::string& getNodeId() const;
    Zone               getZone()   const;

    int    getOrderCount()    const;
    double getTotalSpent()    const;
    double getAverageRating() const;

    const std::vector<std::string>& getOrderHistory() const;

    // ── Actualización ─────────────────────────────────────────
    void addOrder(const std::string& orderId, double amount);
    void addRating(double score);

    // ── Helpers ───────────────────────────────────────────────
    bool isFrequent() const;    // orderCount >= umbral (Config)
};