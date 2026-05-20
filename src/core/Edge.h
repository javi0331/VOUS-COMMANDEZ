#pragma once

#include <string>
#include "../core/Types.h"

// Edge es un modelo de datos puro.
// El cálculo de peso vive en los algoritmos:
//   double w = edge.getBaseTime() * traffic.finalMultiplier();
class Edge {
private:
    std::string from;
    std::string to;

    double distanceKm;
    double baseTime;   // Tiempo base en minutos (condiciones normales)

    RoadType roadType;

    bool blocked;

public:
    Edge();

    Edge(
        const std::string& from,
        const std::string& to,
        double distanceKm,
        double baseTime,
        RoadType roadType
    );

    const std::string& getFrom()     const;
    const std::string& getTo()       const;
    double             getDistanceKm() const;
    double             getBaseTime() const;
    RoadType           getRoadType() const;
    bool               isBlocked()  const;

    void setBlocked(bool blocked);
};