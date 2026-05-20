#pragma once

#include <vector>
#include <string>
#include "../core/RouteSegment.h"

class Path {
public:
    std::vector<std::string> nodes;
    std::vector<RouteSegment> segments;

    double totalCost;
    double totalDistance;
    double estimatedTime;

    bool valid;

    Path()
        : totalCost(0.0),
          totalDistance(0.0),
          estimatedTime(0.0),
          valid(false) {}
};