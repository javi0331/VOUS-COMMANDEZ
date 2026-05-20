#include "../simulation/EventSystem.h"
#include <algorithm>

EventSystem::EventSystem(
    TrafficSystem& traffic,
    WeatherSystem& weather
)
    : trafficSystem(traffic),
      weatherSystem(weather) {}

void EventSystem::update(double currentSimTime) {
    for (auto& event : events) {
        if (!event.fired && currentSimTime >= event.triggerTime) {
            event.action(trafficSystem, weatherSystem);
            event.fired = true;
        }
    }
}

// ── Programación de eventos ───────────────────────────────────

void EventSystem::schedule(
    const std::string& id,
    double triggerTime,
    std::function<void(TrafficSystem&, WeatherSystem&)> action
) {
    events.emplace_back(id, triggerTime, std::move(action));
}

void EventSystem::scheduleWeather(
    const std::string& id,
    double triggerTime,
    WeatherState state
) {
    schedule(id, triggerTime,
        [state](TrafficSystem&, WeatherSystem& w) {
            w.setWeather(state);
        }
    );
}

void EventSystem::scheduleTrafficEvent(
    const std::string& id,
    double triggerTime,
    EventState event,
    double durationMinutes
) {
    schedule(id, triggerTime,
        [event, durationMinutes](TrafficSystem& t, WeatherSystem&) {
            t.setEvent(event, durationMinutes);
        }
    );
}

// ── Gestión ───────────────────────────────────────────────────

void EventSystem::cancelEvent(const std::string& id) {
    events.erase(
        std::remove_if(events.begin(), events.end(),
            [&id](const SimEvent& e) {
                return e.id == id && !e.fired;
            }
        ),
        events.end()
    );
}

void EventSystem::clearAll() {
    events.clear();
}

void EventSystem::clearFired() {
    events.erase(
        std::remove_if(events.begin(), events.end(),
            [](const SimEvent& e) { return e.fired; }
        ),
        events.end()
    );
}

int EventSystem::pendingCount() const {
    int count = 0;
    for (const auto& e : events) {
        if (!e.fired) count++;
    }
    return count;
}

int EventSystem::totalCount() const {
    return static_cast<int>(events.size());
}

bool EventSystem::hasPending() const {
    return pendingCount() > 0;
}