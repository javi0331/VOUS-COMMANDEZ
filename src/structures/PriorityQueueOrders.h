#pragma once

#include <vector>
#include <string>
#include "../models/Order.h"

// Max-heap de órdenes pendientes ordenado por prioridad.
// La orden con mayor priority siempre está en la cima.
//
// Separado de HeapDealers intencionalmente:
// - Tipo de entrada diferente (OrderEntry vs DealerEntry)
// - Lógica de prioridad diferente (puede incluir tiempo de espera)
// - Facilita debug y mantenimiento independiente

struct OrderEntry {
    std::string orderId;
    int         priority;
    double      createdAt;  // Para desempate: más antigua primero

    bool operator<(const OrderEntry& other) const {
        if (priority != other.priority) {
            return priority < other.priority; // mayor prioridad gana
        }
        return createdAt > other.createdAt;   // más antigua gana en empate
    }
};

class PriorityQueueOrders {
private:
    std::vector<OrderEntry> heap;

    void siftUp(int index);
    void siftDown(int index);

    int parent(int i) const { return (i - 1) / 2; }
    int left(int i)   const { return 2 * i + 1; }
    int right(int i)  const { return 2 * i + 2; }

public:
    PriorityQueueOrders() = default;

    // ── Inserción / Extracción ────────────────────────────────

    void push(
        const std::string& orderId,
        int priority,
        double createdAt
    );

    // Devuelve y elimina la orden más prioritaria.
    // Precondición: !isEmpty()
    OrderEntry pop();

    // Devuelve la orden más prioritaria sin eliminarla.
    // Precondición: !isEmpty()
    const OrderEntry& top() const;

    // ── Actualización ─────────────────────────────────────────

    // Actualiza la prioridad de una orden (ej: boost por tiempo de espera).
    void updatePriority(
        const std::string& orderId,
        int newPriority
    );

    // Elimina una orden del heap (ej: cancelación).
    void remove(const std::string& orderId);

    // ── Consultas ─────────────────────────────────────────────
    bool isEmpty()  const;
    int  size()     const;
    bool contains(const std::string& orderId) const;

    // Boost automático de prioridad para órdenes que llevan
    // más de `thresholdMinutes` esperando. Llamar periódicamente
    // desde Simulator::tick(). O(n).
    void boostStaleOrders(
        double currentTime,
        double thresholdMinutes,
        int    boostAmount
    );
};