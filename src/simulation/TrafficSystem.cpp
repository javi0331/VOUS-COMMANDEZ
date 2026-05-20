#include "../simulation/TrafficSystem.h"
#include <string>

TrafficSystem::TrafficSystem(
    const TimeSystem&    time,
    const WeatherSystem& weather
)
    : timeSystem(time),
      weatherSystem(weather),
      currentEvent(EventState::NONE),
      eventTimer(0.0) {}

void TrafficSystem::update(double simDeltaMinutes) {
    if (currentEvent == EventState::NONE) return;

    eventTimer -= simDeltaMinutes;
    if (eventTimer <= 0.0) {
        currentEvent = EventState::NONE;
        eventTimer   = 0.0;
    }
}

// ── Control de eventos ────────────────────────────────────────

void TrafficSystem::setEvent(
    EventState event,
    double durationMinutes
) {
    currentEvent = event;
    eventTimer   = durationMinutes;
}

void TrafficSystem::clearEvent() {
    currentEvent = EventState::NONE;
    eventTimer   = 0.0;
}

// ── Producto principal ────────────────────────────────────────

TrafficEdgeData TrafficSystem::getCurrentTraffic() const {
    return TrafficEdgeData(
        getWeatherFactor(),
        getHourFactor(),
        getEventFactor()
    );
}

// ── Getters individuales ──────────────────────────────────────

double TrafficSystem::getWeatherFactor() const {
    return weatherSystem.getFactor();
}

double TrafficSystem::getHourFactor() const {
    return computeHourFactor(timeSystem.getHour());
}

double TrafficSystem::getEventFactor() const {
    switch (currentEvent) {
        case EventState::NONE:    return 1.0;
        case EventState::MARCH:   return 1.2;
        case EventState::CONCERT: return 1.4;
        case EventState::MATCH:   return 1.4;
        default:                  return 1.0;
    }
}

double TrafficSystem::getFinalMultiplier() const {
    return getWeatherFactor()
         * getHourFactor()
         * getEventFactor();
}

EventState TrafficSystem::getEvent() const {
    return currentEvent;
}

std::string TrafficSystem::eventName() const {
    switch (currentEvent) {
        case EventState::NONE:    return "Sin evento";
        case EventState::MARCH:   return "Marcha";
        case EventState::CONCERT: return "Concierto";
        case EventState::MATCH:   return "Partido";
        default:                  return "Desconocido";
    }
}

// ── Cálculo de factor por hora ────────────────────────────────

double TrafficSystem::computeHourFactor(int hour) const {
    // Madrugada (0–5h): tráfico mínimo
    if (hour >= 0 && hour < 5)  return 0.8;

    // Hora pico mañana (7–9h)
    if (hour >= 7 && hour < 9)  return 1.5;

    // Hora pico tarde (17–20h)
    if (hour >= 17 && hour < 20) return 1.5;

    // Resto del día: normal
    return 1.0;
}