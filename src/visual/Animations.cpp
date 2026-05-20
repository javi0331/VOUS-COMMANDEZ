#include "../visual/Animations.h"
#include <cmath>

Animations::Animations(const Graph& graph)
    : graph(graph) {}

void Animations::setRoute(
    const std::string& dealerId,
    const std::vector<std::string>& nodeIds,
    float simDurationSeconds
) {
    if (nodeIds.empty()) return;

    DealerAnim anim;
    anim.dealerId        = dealerId;
    anim.currentWaypoint = 0;
    anim.progress        = 0.f;
    anim.active          = true;

    for (const auto& id : nodeIds) {
        anim.waypoints.push_back(nodeToWorld(id));
    }

    // Calibrar velocidad: si se conoce la duración simulada de la ruta,
    // calcular la velocidad para que la animación coincida exactamente.
    // Esto evita que la animación termine antes/después que la simulación.
    if (simDurationSeconds > 0.f && anim.waypoints.size() >= 2) {
        float totalDist = 0.f;
        for (size_t i = 0; i + 1 < anim.waypoints.size(); i++) {
            float dx = anim.waypoints[i + 1].x - anim.waypoints[i].x;
            float dy = anim.waypoints[i + 1].y - anim.waypoints[i].y;
            totalDist += std::sqrt(dx * dx + dy * dy);
        }
        if (totalDist > 0.f) {
            anim.speed = totalDist / simDurationSeconds;
        }
    }
    // Si simDurationSeconds == 0 se usa el speed por defecto del constructor (80)

    anim.currentPos = anim.waypoints.front();
    anims[dealerId] = anim;
}

void Animations::clearRoute(const std::string& dealerId) {
    auto it = anims.find(dealerId);
    if (it != anims.end()) {
        it->second.active = false;
        it->second.waypoints.clear();
    }
}

void Animations::update(float simDeltaSeconds) {
    for (auto& [id, anim] : anims) {
        if (!anim.active) continue;
        if (anim.waypoints.size() < 2) continue;

        int next = anim.currentWaypoint + 1;
        if (next >= static_cast<int>(anim.waypoints.size())) {
            // Llegó al final de la ruta
            anim.currentPos = anim.waypoints.back();
            anim.active     = false;
            continue;
        }

        sf::Vector2f from = anim.waypoints[anim.currentWaypoint];
        sf::Vector2f to   = anim.waypoints[next];

        // Distancia entre waypoints en unidades mundo
        float dx   = to.x - from.x;
        float dy   = to.y - from.y;
        float dist = std::sqrt(dx * dx + dy * dy);

        // Avance de progreso según velocidad
        float step = (dist > 0.f)
                     ? (anim.speed * simDeltaSeconds) / dist
                     : 1.f;

        anim.progress += step;

        if (anim.progress >= 1.f) {
            // Pasó al siguiente waypoint
            anim.progress -= 1.f;
            anim.currentWaypoint++;

            if (anim.currentWaypoint + 1 >=
                static_cast<int>(anim.waypoints.size())) {
                anim.currentPos = anim.waypoints.back();
                anim.active     = false;
                continue;
            }
        }

        // Interpolación lineal entre waypoints actuales
        from = anim.waypoints[anim.currentWaypoint];
        to   = anim.waypoints[anim.currentWaypoint + 1];

        anim.currentPos = {
            from.x + (to.x - from.x) * anim.progress,
            from.y + (to.y - from.y) * anim.progress
        };
    }
}

sf::Vector2f Animations::getPosition(
    const std::string& dealerId
) const {
    auto it = anims.find(dealerId);
    if (it == anims.end() || !it->second.active) {
        return {-1.f, -1.f};
    }
    return it->second.currentPos;
}

bool Animations::isActive(const std::string& dealerId) const {
    auto it = anims.find(dealerId);
    return it != anims.end() && it->second.active;
}

sf::Vector2f Animations::nodeToWorld(
    const std::string& nodeId
) const {
    if (!graph.hasNode(nodeId)) return {0.f, 0.f};
    const auto& pos = graph.getNode(nodeId).getPosition();
    return {static_cast<float>(pos.x),
            static_cast<float>(pos.y)};
}