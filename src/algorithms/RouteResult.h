#pragma once

#include <string>
#include "../core/Path.h"

class RouteResult {
public:
    Path   path;               // Ruta matemática (nodos + segmentos)

    bool   success;            // false si no existe ruta
    int    visitedNodes;       // Nodos procesados durante la búsqueda
    double executionTimeMs;    // Tiempo de cómputo real en ms
    std::string algorithmName; // "Dijkstra" o "Bellman-Ford"

    // Comparación entre algoritmos
    // Útil para mostrar en UI cuál fue más eficiente
    bool fasterThan(const RouteResult& other) const {
        return executionTimeMs < other.executionTimeMs;
    }

    bool cheaperThan(const RouteResult& other) const {
        return path.totalCost < other.path.totalCost;
    }

    RouteResult()
        : success(false),
          visitedNodes(0),
          executionTimeMs(0.0) {}
};