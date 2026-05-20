#pragma once

#include <SFML/Graphics.hpp>
#include <string>
#include <functional>
#include "../simulation/Simulator.h"
#include "../utils/Config.h"

// Dibuja todo lo que NO es el grafo:
//   - Panel lateral derecho: stats, controles, historial, leyenda
//   - HUD superior: hora, multiplicador de trafico, FPS
//   - Boton [?] esquina inferior izquierda -> abre assets/info.html
//   - Tooltip al hacer hover

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

    // Boton de info — esquina inferior izquierda del mapa
    sf::RectangleShape infoBtnBg;
    static constexpr float INFO_BTN_SIZE = 32.f;
    static constexpr float INFO_BTN_MARGIN = 10.f;

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
    void drawInfoButton(sf::RenderWindow& w);

    void drawStats     (sf::RenderWindow& w, const SimStats& stats, float y);
    void drawTimeInfo  (sf::RenderWindow& w, const Simulator& sim,  float y);
    void drawTraffic   (sf::RenderWindow& w, const Simulator& sim,  float y);
    void drawControls  (sf::RenderWindow& w, const Simulator& sim,  float y);
    void drawHistory   (sf::RenderWindow& w, const DeliveryHistory& h, float y);
    void drawLegend    (sf::RenderWindow& w, float y);

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

    // Abre assets/info.html en el navegador del sistema
    void openInfoHTML() const;
};