#include "../algorithms/BellmanFord.h"

#include <chrono>
#include <limits>
#include <unordered_map>
#include <vector>
#include <stdexcept>
#include <algorithm>

RouteResult BellmanFord::compute(
    const Graph&           graph,
    const std::string&     originId,
    const std::string&     destinationId,
    const TrafficEdgeData& traffic
) {
    RouteResult result;
    result.algorithmName = "Bellman-Ford";

    if (!graph.hasNode(originId) || !graph.hasNode(destinationId)) {
        return result;
    }

    auto startTime = std::chrono::high_resolution_clock::now();

    const auto& allNodes = graph.getNodes();
    int n = static_cast<int>(allNodes.size());

    // ── Inicialización ────────────────────────────────────────
    std::unordered_map<std::string, double>      dist;
    std::unordered_map<std::string, std::string> prev;
    std::unordered_map<std::string, double>      distKm;

    for (const auto& [id, _] : allNodes) {
        dist[id]   = std::numeric_limits<double>::max();
        distKm[id] = 0.0;
    }
    dist[originId] = 0.0;

    int visited  = 0;
    const auto& adjList = graph.getAdjacencyList();

    // ── n-1 relajaciones ─────────────────────────────────────
    for (int i = 0; i < n - 1; i++) {
        bool changed = false;

        for (const auto& [u, edges] : adjList) {
            if (dist[u] == std::numeric_limits<double>::max()) continue;

            for (const Edge& edge : edges) {
                if (edge.isBlocked()) continue;

                const std::string& v = edge.getTo();
                double w    = edge.getBaseTime() * traffic.finalMultiplier();
                double newD = dist[u] + w;

                if (newD < dist[v]) {
                    dist[v]   = newD;
                    prev[v]   = u;
                    distKm[v] = distKm[u] + edge.getDistanceKm();
                    changed   = true;
                    visited++;
                }
            }
        }

        // Early exit si no hubo cambios
        if (!changed) break;
    }

    auto endTime = std::chrono::high_resolution_clock::now();
    result.executionTimeMs = std::chrono::duration<double, std::milli>(
        endTime - startTime
    ).count();
    result.visitedNodes = visited;

    // ── Detección de ciclo negativo ───────────────────────────
    if (detectNegativeCycle(graph, traffic, dist)) {
        // No debería ocurrir con multiplicadores >= 1.0,
        // pero si ocurre, devolvemos fallo con nombre especial
        result.algorithmName = "Bellman-Ford [CICLO NEGATIVO]";
        return result;
    }

    // ── Sin ruta ──────────────────────────────────────────────
    if (dist[destinationId] == std::numeric_limits<double>::max()) {
        return result;
    }

    // ── Reconstruir camino ────────────────────────────────────
    result.success            = true;
    result.path.valid         = true;
    result.path.totalCost     = dist[destinationId];
    result.path.totalDistance = distKm[destinationId];
    result.path.estimatedTime = dist[destinationId];

    std::vector<std::string> nodes;
    std::string cur = destinationId;
    while (cur != originId) {
        nodes.push_back(cur);
        auto it = prev.find(cur);
        if (it == prev.end()) {
            // Ruta rota — no debería ocurrir
            result.success    = false;
            result.path.valid = false;
            return result;
        }
        cur = it->second;
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

        auto it = adjList.find(from);
        if (it != adjList.end()) {
            for (const Edge& e : it->second) {
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

// ── Detección de ciclo negativo ───────────────────────────────

bool BellmanFord::detectNegativeCycle(
    const Graph&           graph,
    const TrafficEdgeData& traffic,
    const std::unordered_map<std::string, double>& dist
) {
    for (const auto& [u, edges] : graph.getAdjacencyList()) {
        if (dist.at(u) == std::numeric_limits<double>::max()) continue;

        for (const Edge& edge : edges) {
            if (edge.isBlocked()) continue;

            double w = edge.getBaseTime() * traffic.finalMultiplier();

            if (dist.at(u) + w < dist.at(edge.getTo())) {
                return true;
            }
        }
    }
    return false;
}