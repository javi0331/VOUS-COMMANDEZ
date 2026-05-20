#include "../simulation/WeatherSystem.h"

WeatherSystem::WeatherSystem()
    : current(WeatherState::CLEAR),
      target(WeatherState::CLEAR),
      transitionTimer(0.0),
      transitioning(false) {}

void WeatherSystem::update(double simDeltaMinutes) {
    if (!transitioning) return;

    transitionTimer -= simDeltaMinutes;

    if (transitionTimer <= 0.0) {
        current      = target;
        transitioning = false;
        transitionTimer = 0.0;
    }
}

// ── Control manual ────────────────────────────────────────────

void WeatherSystem::setWeather(WeatherState state) {
    current       = state;
    transitioning = false;
    transitionTimer = 0.0;
}

void WeatherSystem::scheduleTransition(
    WeatherState to,
    double inMinutes
) {
    target          = to;
    transitionTimer = inMinutes;
    transitioning   = (inMinutes > 0.0);

    // Si el tiempo es 0 o negativo, aplica inmediatamente
    if (!transitioning) current = to;
}

// ── Getters ───────────────────────────────────────────────────

WeatherState WeatherSystem::getState() const {
    return current;
}

double WeatherSystem::getFactor() const {
    switch (current) {
        case WeatherState::CLEAR: return 1.0;
        case WeatherState::RAIN:  return 1.3;
        case WeatherState::STORM: return 1.6;
        default:                  return 1.0;
    }
}

bool WeatherSystem::isTransitioning() const {
    return transitioning;
}

double WeatherSystem::transitionTimeLeft() const {
    return transitionTimer;
}

std::string WeatherSystem::stateName() const {
    switch (current) {
        case WeatherState::CLEAR: return "Despejado";
        case WeatherState::RAIN:  return "Lluvia";
        case WeatherState::STORM: return "Tormenta";
        default:                  return "Desconocido";
    }
}