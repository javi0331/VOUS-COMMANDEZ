#pragma once

class TrafficEdgeData {
public:
    double weatherFactor;
    double hourFactor;
    double eventFactor;

    TrafficEdgeData()
        : weatherFactor(1.0),
          hourFactor(1.0),
          eventFactor(1.0) {}

    TrafficEdgeData(
        double weather,
        double hour,
        double event
    )
        : weatherFactor(weather),
          hourFactor(hour),
          eventFactor(event) {}

    double finalMultiplier() const {
        return weatherFactor *
               hourFactor *
               eventFactor;
    }
};