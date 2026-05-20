#pragma once
#include <string>
#include "../core/TrafficEdgeData.h"

enum class WeatherState {
    CLEAR,      // Despejado   — factor 1.0
    RAIN,       // Lluvia      — factor 1.3
    STORM       // Tormenta    — factor 1.6
};

// El clima se configura manualmente (cambio explícito desde UI o sim).
// Opcionalmente puede programarse una transición automática
// con duración en minutos simulados.
class WeatherSystem {
private:
    WeatherState current;
    WeatherState target;         // Estado al que transiciona

    double transitionTimer;      // Minutos simulados restantes
    bool   transitioning;

public:
    WeatherSystem();

    // Avanza el sistema con el delta de tiempo simulado
    void update(double simDeltaMinutes);

    // ── Control manual ────────────────────────────────────────

    // Cambia el clima inmediatamente
    void setWeather(WeatherState state);

    // Programa una transición al estado dado en N minutos simulados
    void scheduleTransition(WeatherState to, double inMinutes);

    // ── Getters ───────────────────────────────────────────────
    WeatherState getState()       const;
    double       getFactor()      const; // Factor de peso para TrafficEdgeData
    bool         isTransitioning() const;
    double       transitionTimeLeft() const;

    std::string  stateName()      const;
};