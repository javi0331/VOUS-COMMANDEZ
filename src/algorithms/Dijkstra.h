#pragma once

#include <string>
#include "../core/Graph.h"
#include "../core/TrafficEdgeData.h"
#include "../algorithms/RouteResult.h"

class Dijkstra {
public:
    // Calcula la ruta óptima entre dos nodos del grafo
    // aplicando los multiplicadores de tráfico en tiempo real.
    // Devuelve RouteResult con success=false si no hay ruta.
    static RouteResult compute(
        const Graph&          graph,
        const std::string&    originId,
        const std::string&    destinationId,
        const TrafficEdgeData& traffic
    );
};