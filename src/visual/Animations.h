#pragma once

#include <SFML/Graphics.hpp>
#include <unordered_map>
#include <vector>
#include <string>
#include "../core/Graph.h"

// Estado de animación de un dealer en movimiento.
// Interpola su posición entre nodos de la ruta calculada.
struct DealerAnim {
    std::string dealerId;

    std::vector<sf::Vector2f> waypoints; // posiciones mundo de cada nodo
    int    currentWaypoint;  // índice del nodo actual
    float  progress;         // 0.0 → 1.0 entre waypoint actual y siguiente
    float  speed;            // unidades mundo / segundo simulado
    bool   active;

    sf::Vector2f currentPos; // posición interpolada actual (para dibujar)

    DealerAnim()
        : currentWaypoint(0),
          progress(0.f),
          speed(80.f),
          active(false) {}
};

// Gestiona todas las animaciones de dealers.
// RendererSFML llama a update() cada frame y lee currentPos para dibujar.
//
// Cuando el Simulator asigna una nueva ruta a un dealer,
// RendererSFML llama a setRoute() con los nodos de la ruta.

class Animations {
private:
    std::unordered_map<std::string, DealerAnim> anims;
    const Graph& graph;

public:
    explicit Animations(const Graph& graph);

    // Asigna o reemplaza la ruta animada de un dealer.
    // nodeIds: lista de IDs de nodos en orden (del RouteResult)
    // simDurationSeconds: tiempo simulado esperado de la ruta en segundos.
    //   Si > 0, la velocidad se calibra automáticamente para que la
    //   animación dure exactamente ese tiempo. Si es 0 usa speed por defecto.
    void setRoute(
        const std::string& dealerId,
        const std::vector<std::string>& nodeIds,
        float simDurationSeconds = 0.f
    );

    // Detiene y limpia la animación de un dealer (entrega completada)
    void clearRoute(const std::string& dealerId);

    // Avanza todas las animaciones activas.
    // simDeltaSeconds: delta de tiempo simulado en segundos
    void update(float simDeltaSeconds);

    // Posición actual interpolada del dealer en coordenadas mundo.
    // Devuelve {-1,-1} si el dealer no tiene animación activa.
    sf::Vector2f getPosition(const std::string& dealerId) const;

    bool isActive(const std::string& dealerId) const;

private:
    // Convierte las coordenadas geoX/geoY del nodo a sf::Vector2f
    sf::Vector2f nodeToWorld(const std::string& nodeId) const;
};