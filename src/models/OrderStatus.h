#pragma once

enum class OrderStatus {
    PENDING,       // Generado, esperando repartidor
    ASSIGNED,      // Repartidor asignado, yendo al restaurante
    PICKING_UP,    // Repartidor en el restaurante recogiendo
    IN_TRANSIT,    // En camino al cliente
    DELIVERED,     // Entregado exitosamente
    CANCELLED      // Cancelado (timeout o sin repartidor disponible)
};