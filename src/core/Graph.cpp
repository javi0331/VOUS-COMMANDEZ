#include "../core/Graph.h"
#include <stdexcept>

Graph::Graph() {}

void Graph::addNode(const Node& node) {
    nodes[node.getId()] = node;
}

void Graph::addEdge(const Edge& edge) {
    adjacencyList[edge.getFrom()].push_back(edge);
}

bool Graph::hasNode(const std::string& id) const {
    return nodes.find(id) != nodes.end();
}

bool Graph::edgeExists(
    const std::string& from,
    const std::string& to
) const {
    auto it = adjacencyList.find(from);
    if (it == adjacencyList.end()) return false;

    for (const Edge& edge : it->second) {
        if (edge.getTo() == to) return true;
    }
    return false;
}

const Node& Graph::getNode(const std::string& id) const {
    auto it = nodes.find(id);
    if (it == nodes.end()) {
        throw std::runtime_error(
            "Graph::getNode — nodo no encontrado: " + id
        );
    }
    return it->second;
}

// Expone el mapa completo de nodos para Bellman-Ford
const std::unordered_map<std::string, Node>&
Graph::getNodes() const {
    return nodes;
}

const std::unordered_map<
    std::string,
    std::vector<Edge>
>& Graph::getAdjacencyList() const {
    return adjacencyList;
}

// Devuelve vector vacío si el nodo no tiene aristas salientes.
// No lanza excepción — un nodo aislado o restaurante sin salidas
// es un estado válido en la simulación.
const std::vector<Edge>& Graph::getNeighbors(
    const std::string& id
) const {
    static const std::vector<Edge> empty;
    auto it = adjacencyList.find(id);
    return it != adjacencyList.end() ? it->second : empty;
}

size_t Graph::nodeCount() const {
    return nodes.size();
}

size_t Graph::edgeCount() const {
    size_t total = 0;
    for (const auto& pair : adjacencyList) {
        total += pair.second.size();
    }
    return total;
}