#include "../core/GraphLoader.h"
#include <fstream>
#include <sstream>
#include <iostream>
#include <stdexcept>

// ── Helpers ───────────────────────────────────────────────────

static Zone parseZone(const std::string& s) {
    if (s == "NORTE")         return Zone::NORTE;
    if (s == "CENTRO_NORTE")  return Zone::CENTRO_NORTE;
    if (s == "OCCIDENTE")     return Zone::OCCIDENTE;
    if (s == "SUR_OCCIDENTE") return Zone::SUR_OCCIDENTE;
    if (s == "CENTRO")        return Zone::CENTRO;
    if (s == "SUR")           return Zone::SUR;
    return Zone::CENTRO; // fallback
}

static NodeType parseNodeType(const std::string& s) {
    if (s == "RESTAURANT" || s == "RESTAURANTE") return NodeType::RESTAURANTE;
    return NodeType::BARRIO;
}

// Deduce el tipo de vía según la distancia del segmento
static RoadType roadTypeFromWeight(double w) {
    if (w >= 18.0) return RoadType::AUTOPISTA;
    if (w >= 12.0) return RoadType::AVENIDA;
    if (w >= 7.0)  return RoadType::CARRERA;
    return RoadType::CALLE;
}

// ── Carga principal ───────────────────────────────────────────

Graph GraphLoader::loadFromCSV(
    const std::string& nodesPath,
    const std::string& edgesPath
) {
    Graph graph;

    // ── Nodos ─────────────────────────────────────────────────
    // Formato: id,name,zone,type,x,y
    {
        std::ifstream file(nodesPath);
        if (!file.is_open())
            throw std::runtime_error("No se pudo abrir: " + nodesPath);

        std::string line;
        std::getline(file, line); // saltar encabezado

        while (std::getline(file, line)) {
            if (line.empty()) continue;

            std::istringstream ss(line);
            std::string id, name, zoneStr, typeStr, xStr, yStr;

            std::getline(ss, id,      ',');
            std::getline(ss, name,    ',');
            std::getline(ss, zoneStr, ',');
            std::getline(ss, typeStr, ',');
            std::getline(ss, xStr,    ',');
            std::getline(ss, yStr,    ',');

            Zone     zone = parseZone(zoneStr);
            NodeType type = parseNodeType(typeStr);
            double   x    = std::stod(xStr);
            double   y    = std::stod(yStr);

            Node node(id, name, type, zone, {x, y});
            graph.addNode(node);
        }
    }

    // ── Aristas ───────────────────────────────────────────────
    // Formato: source,target,weight,bidirectional
    {
        std::ifstream file(edgesPath);
        if (!file.is_open())
            throw std::runtime_error("No se pudo abrir: " + edgesPath);

        std::string line;
        std::getline(file, line); // saltar encabezado

        while (std::getline(file, line)) {
            if (line.empty()) continue;

            std::istringstream ss(line);
            std::string from, to, weightStr, biStr;

            std::getline(ss, from,      ',');
            std::getline(ss, to,        ',');
            std::getline(ss, weightStr, ',');
            std::getline(ss, biStr,     ',');

            double   weight = std::stod(weightStr);
            bool     bi     = (biStr.find('1') != std::string::npos);
            RoadType road   = roadTypeFromWeight(weight);

            // baseTime en minutos: distancia / velocidad promedio (km/h → min)
            // Velocidad estimada 30 km/h en ciudad
            double baseTime = (weight / 30.0) * 60.0;

            graph.addEdge(Edge(from, to, weight, baseTime, road));
            if (bi) {
                graph.addEdge(Edge(to, from, weight, baseTime, road));
            }
        }
    }

    return graph;
}