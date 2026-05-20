#pragma once

#include <vector>
#include <string>
#include <functional>
#include "../simulation/TrafficSystem.h"
#include "../simulation/WeatherSystem.h"

// Un evento programado en el tiempo simulado
struct SimEvent {
    std::string id;
    double      triggerTime;    // Minutos simulados en que dispara
    bool        fired;          // Ya fue ejecutado

    // Callback que ejecuta el evento.
    // Recibe referencias a los sistemas para poder modificarlos.
    std::function<void(TrafficSystem&, WeatherSystem&)> action;

    SimEvent(
        const std::string& id,
        double triggerTime,
        std::function<void(TrafficSystem&, WeatherSystem&)> action
    )
        : id(id),
          triggerTime(triggerTime),
          fired(false),
          action(std::move(action)) {}
};

// Gestiona eventos programados que se disparan en momentos
// específicos del tiempo simulado.
//
// Uso desde Simulator o UI:
//   eventSystem.schedule("lluvia_tarde", 1020.0, [](auto& t, auto& w){
//       w.setWeather(WeatherState::RAIN);
//   });
//
// Simulator llama a eventSystem.update(simulatedTime) cada tick.

class EventSystem {
private:
    std::vector<SimEvent> events;

    TrafficSystem& trafficSystem;
    WeatherSystem& weatherSystem;

public:
    EventSystem(
        TrafficSystem& traffic,
        WeatherSystem& weather
    );

    // Dispara todos los eventos cuyo triggerTime <= currentTime
    // y que no hayan sido ejecutados todavía.
    void update(double currentSimTime);

    // ── Programación de eventos ───────────────────────────────

    // Evento genérico con callback
    void schedule(
        const std::string& id,
        double triggerTime,
        std::function<void(TrafficSystem&, WeatherSystem&)> action
    );

    // Helpers para los eventos más comunes
    void scheduleWeather(
        const std::string& id,
        double triggerTime,
        WeatherState state
    );

    void scheduleTrafficEvent(
        const std::string& id,
        double triggerTime,
        EventState event,
        double durationMinutes
    );

    // ── Gestión ───────────────────────────────────────────────
    void cancelEvent(const std::string& id);
    void clearAll();
    void clearFired();  // Limpia solo los ya ejecutados

    int  pendingCount() const;
    int  totalCount()   const;
    bool hasPending()   const;
};