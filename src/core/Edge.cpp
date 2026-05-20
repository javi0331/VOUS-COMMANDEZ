#include "../core/Edge.h"

Edge::Edge()
    : distanceKm(0.0),
      baseTime(0.0),
      roadType(RoadType::CALLE),
      blocked(false) {}

Edge::Edge(
    const std::string& from,
    const std::string& to,
    double distanceKm,
    double baseTime,
    RoadType roadType
)
    : from(from),
      to(to),
      distanceKm(distanceKm),
      baseTime(baseTime),
      roadType(roadType),
      blocked(false) {}

const std::string& Edge::getFrom() const {
    return from;
}

const std::string& Edge::getTo() const {
    return to;
}

double Edge::getDistanceKm() const {
    return distanceKm;
}

double Edge::getBaseTime() const {
    return baseTime;
}

RoadType Edge::getRoadType() const {
    return roadType;
}

bool Edge::isBlocked() const {
    return blocked;
}

void Edge::setBlocked(bool blocked) {
    this->blocked = blocked;
}