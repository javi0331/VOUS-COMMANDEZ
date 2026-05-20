#include "../algorithms/Dijkstra.h"

#include <chrono>
#include <limits>
#include <queue>
#include <unordered_map>
#include <vector>
#include <stdexcept>
#include <algorithm>

RouteResult Dijkstra::compute(
    const Graph&           graph,
    const std::string&     originId,
    const std::string&     destinationId,
    const TrafficEdgeData& traffic
) {
    RouteResult result;
    result.algorithmName = "Dijkstra";

    // Validar que ambos nodos existen
    if (!graph.hasNode(originId) || !graph.hasNode(destinationId)) {
        return result; // success = false
    }

    auto startTime = std::chrono::high_resolution_clock::now();

    // ── Estructuras internas ──────────────────────────────────
    // {costo acumulado, nodeId}
    using PQEntry = std::pair<double, std::string>;

    std::unordered_map<std::string, double>      dist;
    std::unordered_map<std::string, std::string> prev;
    std::unordered_map<std::string, double>      distKm;

    const auto& allNodes = graph.getNodes();
    for (const auto& [id, _] : allNodes) {
        dist[id]   = std::numeric_limits<double>::max();
        distKm[id] = 0.0;
    }
    dist[originId] = 0.0;

    std::priority_queue<
        PQEntry,
        std::vector<PQEntry>,
        std::greater<PQEntry>
    > pq;
    pq.push({0.0, originId});

    int visited = 0;

    // ── Bucle principal ───────────────────────────────────────
    while (!pq.empty()) {
        auto [d, u] = pq.top();
        pq.pop();

        // Entrada obsoleta en la cola
        if (d > dist[u]) continue;

        visited++;

        // Destino alcanzado — corte temprano
        if (u == destinationId) break;

        // Obtener vecinos (devuelve vector vacío si no hay aristas)
        const std::vector<Edge>* neighbors = nullptr;
        try {
            neighbors = &graph.getNeighbors(u);
        } catch (...) {
            continue;
        }

        for (const Edge& edge : *neighbors) {
            if (edge.isBlocked()) continue;

            const std::string& v    = edge.getTo();
            double             w    = edge.getBaseTime() * traffic.finalMultiplier();
            double             newD = dist[u] + w;

            if (newD < dist[v]) {
                dist[v]   = newD;
                prev[v]   = u;
                distKm[v] = distKm[u] + edge.getDistanceKm();
                pq.push({newD, v});
            }
        }
    }

    auto endTime = std::chrono::high_resolution_clock::now();
    result.executionTimeMs = std::chrono::duration<double, std::milli>(
        endTime - startTime
    ).count();
    result.visitedNodes = visited;

    // ── Sin ruta ──────────────────────────────────────────────
    if (dist[destinationId] == std::numeric_limits<double>::max()) {
        return result; // success = false
    }

    // ── Reconstruir camino ────────────────────────────────────
    result.success           = true;
    result.path.valid        = true;
    result.path.totalCost    = dist[destinationId];
    result.path.totalDistance = distKm[destinationId];
    result.path.estimatedTime = dist[destinationId]; // baseTime ya en minutos

    // Recorrer prev[] hacia atrás
    std::vector<std::string> nodes;
    std::string cur = destinationId;
    while (cur != originId) {
        nodes.push_back(cur);
        cur = prev[cur];
    }
    nodes.push_back(originId);
    std::reverse(nodes.begin(), nodes.end());
    result.path.nodes = nodes;

    // Construir segmentos
    for (size_t i = 0; i + 1 < nodes.size(); i++) {
        const std::string& from = nodes[i];
        const std::string& to   = nodes[i + 1];
        double segTime = 0.0;
        double segDist = 0.0;

        const std::vector<Edge>* nbrs = nullptr;
        try {
            nbrs = &graph.getNeighbors(from);
        } catch (...) {}

        if (nbrs) {
            for (const Edge& e : *nbrs) {
                if (e.getTo() == to) {
                    segTime = e.getBaseTime() * traffic.finalMultiplier();
                    segDist = e.getDistanceKm();
                    break;
                }
            }
        }

        result.path.segments.emplace_back(
            from, to,
            segDist,
            segTime,
            traffic.finalMultiplier()
        );
    }

    return result;
}