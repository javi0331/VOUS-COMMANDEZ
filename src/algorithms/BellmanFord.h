#pragma once

#include <string>
#include "../core/Graph.h"
#include "../core/TrafficEdgeData.h"
#include "../algorithms/RouteResult.h"

class BellmanFord {
public:
    // Calcula la ruta óptima entre dos nodos del grafo.
    // A diferencia de Dijkstra, maneja pesos negativos
    // y detecta ciclos negativos (no deberían ocurrir con
    // los multiplicadores de tráfico, pero se valida igual).
    //
    // Más lento que Dijkstra O(V·E) vs O((V+E)logV),
    // pero sirve para comparar resultados y como verificación.
    static RouteResult compute(
        const Graph&           graph,
        const std::string&     originId,
        const std::string&     destinationId,
        const TrafficEdgeData& traffic
    );

private:
    // Devuelve true si encontró un ciclo de peso negativo
    static bool detectNegativeCycle(
        const Graph&           graph,
        const TrafficEdgeData& traffic,
        const std::unordered_map<std::string, double>& dist
    );
};