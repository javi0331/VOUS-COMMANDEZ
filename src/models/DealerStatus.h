#pragma once

enum class DealerStatus {
    IDLE,           // Libre, esperando pedidos
    HEADING_TO_RESTAURANT, // Yendo a recoger
    PICKING_UP,     // En el restaurante
    DELIVERING,     // Entregando al cliente
    RETURNING       // Volviendo a zona base (opcional)
};