#pragma once

#include <unordered_map>
#include <vector>
#include <string>

#include "../core/Node.h"
#include "../core/Edge.h"

class Graph {
private:
    std::unordered_map<std::string, Node> nodes;

    std::unordered_map<
        std::string,
        std::vector<Edge>
    > adjacencyList;

public:
    Graph();

    void addNode(const Node& node);
    void addEdge(const Edge& edge);

    bool hasNode(const std::string& id)   const;
    bool edgeExists(
        const std::string& from,
        const std::string& to
    ) const;

    const Node& getNode(const std::string& id) const;

    // Expone todos los nodos — necesario para Bellman-Ford
    const std::unordered_map<std::string, Node>&
        getNodes() const;

    const std::unordered_map<
        std::string,
        std::vector<Edge>
    >& getAdjacencyList() const;

    // Devuelve vector vacío si el nodo no tiene aristas salientes.
    // No lanza excepción — un nodo sin vecinos es válido.
    const std::vector<Edge>& getNeighbors(
        const std::string& id
    ) const;

    size_t nodeCount() const;
    size_t edgeCount() const;
};