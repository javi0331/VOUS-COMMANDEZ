#pragma once

#include <string>

class RouteSegment {
public:
    std::string from;
    std::string to;

    double distance;
    double time;

    double trafficMultiplier;

    RouteSegment()
        : distance(0.0),
          time(0.0),
          trafficMultiplier(1.0) {}

    RouteSegment(
        const std::string& from,
        const std::string& to,
        double distance,
        double time,
        double multiplier
    )
        : from(from),
          to(to),
          distance(distance),
          time(time),
          trafficMultiplier(multiplier) {}
};