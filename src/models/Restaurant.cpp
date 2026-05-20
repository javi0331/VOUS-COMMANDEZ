#include "../models/Restaurant.h"

Restaurant::Restaurant()
    : zone(Zone::CENTRO),
      activeOrders(0),
      totalOrders(0),
      averageRating(5.0),
      avgPrepTime(10.0),
      popularityScore(0.0) {}

Restaurant::Restaurant(
    const std::string& id,
    const std::string& name,
    const std::string& nodeId,
    Zone zone
)
    : id(id),
      name(name),
      nodeId(nodeId),
      zone(zone),
      activeOrders(0),
      totalOrders(0),
      averageRating(5.0),
      avgPrepTime(10.0),
      popularityScore(0.0) {}

// ── Getters estáticos ─────────────────────────────────────────

const std::string& Restaurant::getId() const {
    return id;
}

const std::string& Restaurant::getName() const {
    return name;
}

const std::string& Restaurant::getNodeId() const {
    return nodeId;
}

Zone Restaurant::getZone() const {
    return zone;
}

// ── Getters dinámicos ─────────────────────────────────────────

int Restaurant::getActiveOrders() const {
    return activeOrders;
}

int Restaurant::getTotalOrders() const {
    return totalOrders;
}

double Restaurant::getAverageRating() const {
    return averageRating;
}

double Restaurant::getAvgPrepTime() const {
    return avgPrepTime;
}

double Restaurant::getPopularityScore() const {
    return popularityScore;
}

// ── Actualización en tiempo de simulación ─────────────────────

void Restaurant::orderReceived() {
    activeOrders++;
    totalOrders++;
    recalculateScore();
}

void Restaurant::orderDispatched() {
    if (activeOrders > 0) activeOrders--;
    recalculateScore();
}

void Restaurant::addRating(double score) {
    if (totalOrders > 1) {
        averageRating = (averageRating * (totalOrders - 1) + score)
                        / totalOrders;
    } else {
        averageRating = score;
    }
    recalculateScore();
}

void Restaurant::updatePrepTime(double minutes) {
    if (totalOrders > 1) {
        avgPrepTime = (avgPrepTime * (totalOrders - 1) + minutes)
                      / totalOrders;
    } else {
        avgPrepTime = minutes;
    }
}

void Restaurant::recalculateScore() {
    // Score compuesto para el AVL:
    // Pondera volumen histórico (70%) y rating (30%)
    // El activeOrders reduce levemente el score (restaurante saturado)
    const double VOLUME_WEIGHT = 0.7;
    const double RATING_WEIGHT = 0.3;
    const double BUSY_PENALTY  = 0.05;

    double volumeNorm = static_cast<double>(totalOrders) / 1000.0;
    double ratingNorm = averageRating / 5.0;
    double busyness   = static_cast<double>(activeOrders) * BUSY_PENALTY;

    popularityScore = (VOLUME_WEIGHT * volumeNorm
                    +  RATING_WEIGHT * ratingNorm)
                    - busyness;

    if (popularityScore < 0.0) popularityScore = 0.0;
}

bool Restaurant::isBusy() const {
    // Umbral fijo aquí; se puede mover a Config.h
    return activeOrders >= 5;
}

std::string Restaurant::zoneName() const {
    switch (zone) {
        case Zone::NORTE:         return "Norte";
        case Zone::CENTRO_NORTE:  return "Centro-Norte";
        case Zone::OCCIDENTE:     return "Occidente";
        case Zone::SUR_OCCIDENTE: return "Sur-Occidente";
        case Zone::CENTRO:        return "Centro";
        case Zone::SUR:           return "Sur";
        default:                  return "Desconocida";
    }
}