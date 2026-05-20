#include "../simulation/TimeSystem.h"
#include <cmath>

TimeSystem::TimeSystem(double simulationSpeed)
    : simulatedTime(0.0),
      simulationSpeed(simulationSpeed),
      paused(false),
      startOffsetMinutes(0.0) {}

void TimeSystem::update(double realDeltaSeconds) {
    if (paused) return;
    simulatedTime += realDeltaSeconds * simulationSpeed;
}

// ── Getters ───────────────────────────────────────────────────

double TimeSystem::getSimulatedTime() const {
    return simulatedTime;
}

double TimeSystem::getSimulationSpeed() const {
    return simulationSpeed;
}

bool TimeSystem::isPaused() const {
    return paused;
}

double TimeSystem::getTimeOfDay() const {
    // Suma el offset de hora de inicio y vuelve al rango 0-1439
    double t = std::fmod(
        simulatedTime + startOffsetMinutes,
        1440.0
    );
    if (t < 0.0) t += 1440.0;
    return t;
}

int TimeSystem::getHour() const {
    return static_cast<int>(getTimeOfDay()) / 60;
}

int TimeSystem::getMinute() const {
    return static_cast<int>(getTimeOfDay()) % 60;
}

// ── Control ───────────────────────────────────────────────────

void TimeSystem::setSimulationSpeed(double speed) {
    if (speed > 0.0) simulationSpeed = speed;
}

void TimeSystem::setStartHour(int hour, int minute) {
    startOffsetMinutes = static_cast<double>(hour * 60 + minute);
}

void TimeSystem::pause() {
    paused = true;
}

void TimeSystem::resume() {
    paused = false;
}

void TimeSystem::reset() {
    simulatedTime = 0.0;
    paused        = false;
}