#pragma once

#include <SFML/Graphics.hpp>
#include <string>
#include "../simulation/Simulator.h"
#include "../utils/Config.h"

// Dibuja todo lo que NO es el grafo:
//   - Panel lateral derecho: stats, controles de clima/evento, historial
//   - HUD superior: hora simulada, multiplicador de trafico, FPS
//   - Tooltip al hacer hover sobre un nodo
//   - Leyenda de colores de dealers y marcadores
//
// UI siempre se dibuja en coordenadas de pantalla (sin Camera).
// Recibe el estado del Simulator por referencia — solo lee, nunca escribe.

class UI {
private:
    sf::Font font;
    bool     fontLoaded;

    sf::RectangleShape panelBg;
    float panelX;

    sf::RectangleShape tooltipBg;
    std::string        tooltipText;
    sf::Vector2f       tooltipPos;
    bool               tooltipVisible;

    std::function<void()>             onPause;
    std::function<void()>             onResume;
    std::function<void(WeatherState)> onWeatherChange;
    std::function<void(EventState, double)> onEventTrigger;
    std::function<void(double)>       onSpeedChange;

public:
    UI();

    bool loadFont(const std::string& fontPath);

    // ── Callbacks ─────────────────────────────────────────────
    void setOnPause(std::function<void()> cb);
    void setOnResume(std::function<void()> cb);
    void setOnWeatherChange(std::function<void(WeatherState)> cb);
    void setOnEventTrigger(std::function<void(EventState, double)> cb);
    void setOnSpeedChange(std::function<void(double)> cb);

    // ── Eventos de entrada ────────────────────────────────────
    void handleMouseClick(float mx, float my, const Simulator& sim);
    void handleMouseMove(float mx, float my);

    // ── Tooltip ───────────────────────────────────────────────
    void showTooltip(const std::string& text, sf::Vector2f screenPos);
    void hideTooltip();

    // ── Dibujo ────────────────────────────────────────────────
    void draw(sf::RenderWindow& window, const Simulator& sim, float fps);

private:
    void drawPanel     (sf::RenderWindow& w, const Simulator& sim);
    void drawHUD       (sf::RenderWindow& w, const Simulator& sim, float fps);
    void drawTooltip   (sf::RenderWindow& w);

    void drawStats     (sf::RenderWindow& w, const SimStats& stats, float y);
    void drawTimeInfo  (sf::RenderWindow& w, const Simulator& sim,  float y);
    void drawTraffic   (sf::RenderWindow& w, const Simulator& sim,  float y);
    void drawControls  (sf::RenderWindow& w, const Simulator& sim,  float y);
    void drawHistory   (sf::RenderWindow& w, const DeliveryHistory& h, float y);
    void drawLegend    (sf::RenderWindow& w, float y);  // nueva

    sf::Text makeText(
        const std::string& str,
        float x, float y,
        unsigned int size,
        sf::Color color = sf::Color::White
    ) const;

    sf::RectangleShape makeButton(
        float x, float y,
        float w, float h,
        sf::Color color
    ) const;

    bool isInside(float mx, float my,
                  float rx, float ry,
                  float rw, float rh) const;
};