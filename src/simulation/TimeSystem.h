#pragma once
#include <string>
#include "../core/TrafficEdgeData.h"

// Gestiona el tiempo simulado acelerado.
//
// Relación:
//   simulatedTime += realDeltaTime * simulationSpeed
//
// Ejemplo: simulationSpeed = 60.0
//   → 1 segundo real = 1 minuto simulado
//   → una jornada de 8h se simula en ~8 minutos reales

class TimeSystem {
private:
    double simulatedTime;   // Minutos simulados transcurridos
    double simulationSpeed; // Factor de aceleración (minutos sim / seg real)
    bool   paused;

public:
    explicit TimeSystem(double simulationSpeed = 60.0);

    // Avanza el tiempo con el delta real del render loop (segundos)
    void update(double realDeltaSeconds);

    // ── Getters ───────────────────────────────────────────────
    double getSimulatedTime()   const; // minutos totales transcurridos
    double getSimulationSpeed() const;
    bool   isPaused()           const;

    // Hora del día en minutos (0–1439)
    // Permite iniciar la sim a cualquier hora del día
    double getTimeOfDay()       const;

    // Hora del día en formato legible (0–23)
    int    getHour()            const;
    int    getMinute()          const;

    // ── Control ───────────────────────────────────────────────
    void   setSimulationSpeed(double speed);
    void   setStartHour(int hour, int minute = 0);
    void   pause();
    void   resume();
    void   reset();

private:
    double startOffsetMinutes; // Offset para hora de inicio configurable
};