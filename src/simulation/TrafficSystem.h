#pragma once
#include <string>
#include "../core/TrafficEdgeData.h"
#include "../simulation/TimeSystem.h"
#include "../simulation/WeatherSystem.h"

enum class EventState {
    NONE,       // Sin evento — factor 1.0
    MARCH,      // Marcha     — factor 1.2
    CONCERT,    // Concierto  — factor 1.4
    MATCH       // Partido    — factor 1.4
};

// Ensambla el TrafficEdgeData final combinando:
//   - weatherFactor  (WeatherSystem)
//   - hourFactor     (TimeSystem → hora del día)
//   - eventFactor    (EventState manual)
//
// Es la única clase que produce TrafficEdgeData para los algoritmos.
// Dijkstra y Bellman-Ford solo necesitan llamar getCurrentTraffic().

class TrafficSystem {
private:
    const TimeSystem&    timeSystem;
    const WeatherSystem& weatherSystem;

    EventState currentEvent;
    double     eventTimer;   // Minutos simulados que resta el evento

public:
    TrafficSystem(
        const TimeSystem&    time,
        const WeatherSystem& weather
    );

    // Avanza el sistema — descuenta el timer del evento activo
    void update(double simDeltaMinutes);

    // ── Control de eventos (manual desde UI o EventSystem) ────
    void setEvent(EventState event, double durationMinutes);
    void clearEvent();

    // ── Producto principal ────────────────────────────────────
    // Devuelve el TrafficEdgeData listo para pasar a Dijkstra/Bellman
    TrafficEdgeData getCurrentTraffic() const;

    // ── Getters individuales ──────────────────────────────────
    double getWeatherFactor() const;
    double getHourFactor()    const;  // Calculado desde TimeSystem
    double getEventFactor()   const;
    double getFinalMultiplier() const;

    EventState  getEvent()    const;
    std::string eventName()   const;

private:
    // Calcula el factor de hora a partir de la hora del día
    // Hora pico mañana (7-9h): ×1.5
    // Hora pico tarde (17-20h): ×1.5
    // Madrugada (0-5h):  ×0.8
    // Resto: ×1.0
    double computeHourFactor(int hour) const;
};