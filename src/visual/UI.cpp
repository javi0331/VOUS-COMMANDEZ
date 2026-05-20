#include "../visual/UI.h"
#include <sstream>
#include <iomanip>

UI::UI()
    : fontLoaded(false),
      tooltipVisible(false)
{
    panelX = static_cast<float>(Config::WINDOW_WIDTH)
            - Config::UI_PANEL_WIDTH;

    panelBg.setSize({Config::UI_PANEL_WIDTH,
                     static_cast<float>(Config::WINDOW_HEIGHT)});
    panelBg.setPosition({panelX, 0.f});
    panelBg.setFillColor(sf::Color(20, 20, 30, 220));

    tooltipBg.setFillColor(sf::Color(40, 40, 50, 230));
    tooltipBg.setOutlineColor(sf::Color(100, 100, 120));
    tooltipBg.setOutlineThickness(1.f);
}

bool UI::loadFont(const std::string& fontPath) {
    fontLoaded = font.openFromFile(fontPath);
    return fontLoaded;
}

// ── Callbacks ─────────────────────────────────────────────────

void UI::setOnPause(std::function<void()> cb)          { onPause = cb; }
void UI::setOnResume(std::function<void()> cb)         { onResume = cb; }
void UI::setOnWeatherChange(std::function<void(WeatherState)> cb) { onWeatherChange = cb; }
void UI::setOnEventTrigger(std::function<void(EventState, double)> cb) { onEventTrigger = cb; }
void UI::setOnSpeedChange(std::function<void(double)> cb) { onSpeedChange = cb; }

// ── Tooltip ───────────────────────────────────────────────────

void UI::showTooltip(const std::string& text, sf::Vector2f pos) {
    tooltipText    = text;
    tooltipPos     = pos;
    tooltipVisible = true;
}

void UI::hideTooltip() {
    tooltipVisible = false;
}

// ── Eventos de entrada ────────────────────────────────────────

void UI::handleMouseClick(float mx, float my, const Simulator& sim) {
    if (!isInside(mx, my, panelX, 0.f,
                  Config::UI_PANEL_WIDTH,
                  static_cast<float>(Config::WINDOW_HEIGHT))) return;

    float bx = panelX + 10.f;
    float bw = (Config::UI_PANEL_WIDTH - 25.f) / 3.f;

    float by = 190.f;

    by += 18.f;  // label "CONTROLES"

    // Boton Pausa / Resume
    if (isInside(mx, my, bx, by, bw * 2.f, 22.f)) {
        if (sim.getTimeSystem().isPaused()) { if (onResume) onResume(); }
        else                               { if (onPause)  onPause();  }
    }
    by += 28.f;

    by += 16.f;  // label "Velocidad:"

    // Botones de velocidad
    if (isInside(mx, my, bx,               by, bw, 20.f))
        { if (onSpeedChange) onSpeedChange(Config::SIM_SPEED_SLOW); }
    if (isInside(mx, my, bx + bw + 2.f,   by, bw, 20.f))
        { if (onSpeedChange) onSpeedChange(Config::SIM_SPEED_DEFAULT); }
    if (isInside(mx, my, bx + 2*(bw+2.f), by, bw, 20.f))
        { if (onSpeedChange) onSpeedChange(Config::SIM_SPEED_FAST); }
    by += 26.f;

    by += 16.f;  // label "Clima:"

    // Botones de clima
    if (isInside(mx, my, bx,               by, bw, 20.f))
        { if (onWeatherChange) onWeatherChange(WeatherState::CLEAR); }
    if (isInside(mx, my, bx + bw + 2.f,   by, bw, 20.f))
        { if (onWeatherChange) onWeatherChange(WeatherState::RAIN);  }
    if (isInside(mx, my, bx + 2*(bw+2.f), by, bw, 20.f))
        { if (onWeatherChange) onWeatherChange(WeatherState::STORM); }
    by += 26.f;

    by += 16.f;  // label "Evento:"

    // Botones de evento
    if (isInside(mx, my, bx,               by, bw, 20.f))
        { if (onEventTrigger) onEventTrigger(EventState::MATCH,   120.0); }
    if (isInside(mx, my, bx + bw + 2.f,   by, bw, 20.f))
        { if (onEventTrigger) onEventTrigger(EventState::CONCERT, 120.0); }
    if (isInside(mx, my, bx + 2*(bw+2.f), by, bw, 20.f))
        { if (onEventTrigger) onEventTrigger(EventState::MARCH,    60.0); }
}

void UI::handleMouseMove(float mx, float my) {
    if (!isInside(mx, my, panelX, 0.f,
                  Config::UI_PANEL_WIDTH,
                  static_cast<float>(Config::WINDOW_HEIGHT))) {
        hideTooltip();
    }
}

// ── Dibujo principal ──────────────────────────────────────────

void UI::draw(
    sf::RenderWindow& window,
    const Simulator& sim,
    float fps
) {
    drawPanel(window, sim);
    drawHUD  (window, sim, fps);
    if (tooltipVisible) drawTooltip(window);
}

// ── Panel lateral ─────────────────────────────────────────────

void UI::drawPanel(sf::RenderWindow& w, const Simulator& sim) {
    w.draw(panelBg);

    float y = 12.f;
    float x = panelX + 10.f;

    w.draw(makeText("Vous Commandez", x, y, 18,
                    sf::Color(255, 200, 50)));
    y += 30.f;

    sf::RectangleShape sep({Config::UI_PANEL_WIDTH - 20.f, 1.f});
    sep.setPosition({x, y});
    sep.setFillColor(sf::Color(80, 80, 100));
    w.draw(sep);
    y += 8.f;

    drawTimeInfo (w, sim, y); y += 60.f;
    drawTraffic  (w, sim, y); y += 80.f;
    drawControls (w, sim, y); y += 174.f;
    drawStats    (w, sim.getStats(), y); y += 130.f;
    drawHistory  (w, sim.getHistory(), y);
}

void UI::drawTimeInfo(sf::RenderWindow& w,
                      const Simulator& sim, float y) {
    float x = panelX + 10.f;
    const auto& ts = sim.getTimeSystem();

    std::ostringstream ss;
    ss << "Hora: "
       << std::setw(2) << std::setfill('0') << ts.getHour()
       << ":"
       << std::setw(2) << std::setfill('0') << ts.getMinute();

    w.draw(makeText(ss.str(), x, y, 14));
    y += 20.f;

    std::ostringstream ss2;
    ss2 << "Velocidad: x"
        << std::fixed << std::setprecision(0)
        << ts.getSimulationSpeed();
    w.draw(makeText(ss2.str(), x, y, 12, sf::Color(180, 180, 180)));
    y += 18.f;

    if (ts.isPaused()) {
        w.draw(makeText("[ PAUSADO ]", x, y, 13,
                        sf::Color(255, 100, 100)));
    }
}

void UI::drawTraffic(sf::RenderWindow& w,
                     const Simulator& sim, float y) {
    float x = panelX + 10.f;
    const auto& ts = sim.getTrafficSystem();
    const auto& ws = sim.getWeatherSystem();

    w.draw(makeText("TRAFICO", x, y, 13, sf::Color(150, 200, 255)));
    y += 18.f;

    std::ostringstream ss;
    ss << "Clima: " << ws.stateName()
       << "  x" << std::fixed << std::setprecision(1)
       << ws.getFactor();
    w.draw(makeText(ss.str(), x, y, 12)); y += 16.f;

    std::ostringstream ss2;
    ss2 << "Evento: " << ts.eventName();
    w.draw(makeText(ss2.str(), x, y, 12)); y += 16.f;

    std::ostringstream ss3;
    ss3 << "Factor total: x"
        << std::fixed << std::setprecision(2)
        << ts.getFinalMultiplier();

    sf::Color factorCol = sf::Color::Green;
    if (ts.getFinalMultiplier() > 2.0) factorCol = sf::Color::Red;
    else if (ts.getFinalMultiplier() > 1.3) factorCol = sf::Color::Yellow;

    w.draw(makeText(ss3.str(), x, y, 13, factorCol));
}

void UI::drawControls(sf::RenderWindow& w,
                      const Simulator& sim, float y) {
    float x    = panelX + 10.f;
    float bw   = (Config::UI_PANEL_WIDTH - 25.f) / 3.f;
    bool paused = sim.getTimeSystem().isPaused();

    w.draw(makeText("CONTROLES", x, y, 13,
                    sf::Color(150, 200, 255)));
    y += 18.f;

    // ── Pausa / Resume ────────────────────────────────────────
    auto pauseBtn = makeButton(x, y, bw * 2.f, 22.f,
        paused ? sf::Color(50, 150, 50) : sf::Color(150, 50, 50));
    w.draw(pauseBtn);
    w.draw(makeText(paused ? "Resume" : "Pausa",
                    x + 6.f, y + 4.f, 12));
    y += 28.f;

    // ── Velocidad ─────────────────────────────────────────────
    w.draw(makeText("Velocidad:", x, y, 12,
                    sf::Color(180, 180, 180)));
    y += 16.f;

    const char* speeds[] = {"Lenta", "Normal", "Rapida"};
    for (int i = 0; i < 3; i++) {
        auto btn = makeButton(x + i * (bw + 2.f), y,
                              bw, 20.f, sf::Color(50, 70, 100));
        w.draw(btn);
        w.draw(makeText(speeds[i], x + i * (bw + 2.f) + 4.f,
                        y + 4.f, 11));
    }
    y += 26.f;

    // ── Clima ─────────────────────────────────────────────────
    w.draw(makeText("Clima:", x, y, 12, sf::Color(180, 180, 180)));
    y += 16.f;

    const char* weathers[] = {"Claro", "Lluvia", "Tormenta"};
    sf::Color wColors[] = {
        sf::Color(50, 100, 50),
        sf::Color(50, 80, 150),
        sf::Color(80, 50, 130)
    };
    for (int i = 0; i < 3; i++) {
        auto btn = makeButton(x + i * (bw + 2.f), y,
                              bw, 20.f, wColors[i]);
        w.draw(btn);
        w.draw(makeText(weathers[i], x + i * (bw + 2.f) + 3.f,
                        y + 4.f, 11));
    }
    y += 26.f;

    // ── Evento ────────────────────────────────────────────────
    w.draw(makeText("Evento:", x, y, 12, sf::Color(180, 180, 180)));
    y += 16.f;

    const char* evts[] = {"Partido", "Concierto", "Marcha"};
    for (int i = 0; i < 3; i++) {
        auto btn = makeButton(x + i * (bw + 2.f), y,
                              bw, 20.f, sf::Color(100, 60, 30));
        w.draw(btn);
        w.draw(makeText(evts[i], x + i * (bw + 2.f) + 3.f,
                        y + 4.f, 11));
    }
}

void UI::drawStats(sf::RenderWindow& w,
                   const SimStats& s, float y) {
    float x = panelX + 10.f;
    w.draw(makeText("ESTADISTICAS", x, y, 13,
                    sf::Color(150, 200, 255)));
    y += 18.f;

    auto line = [&](const std::string& label,
                    const std::string& val,
                    sf::Color vc = sf::Color::White) {
        w.draw(makeText(label, x, y, 12,
                        sf::Color(160, 160, 160)));
        w.draw(makeText(val, x + 115.f, y, 12, vc));
        y += 16.f;
    };

    line("Generadas:",      std::to_string(s.totalOrdersGenerated));
    line("Entregadas:",     std::to_string(s.totalDelivered),
                            sf::Color(100, 255, 100));
    line("Canceladas:",     std::to_string(s.totalCancelled),
                            sf::Color(255, 100, 100));
    line("Activas:",        std::to_string(s.activeOrders),
                            sf::Color(255, 200, 50));
    line("Dealers libres:", std::to_string(s.idleDealers));

    std::ostringstream td, tw;
    td << std::fixed << std::setprecision(1) << s.avgDeliveryTime << " min";
    tw << std::fixed << std::setprecision(1) << s.avgWaitTime     << " min";
    line("T. entrega avg:", td.str());
    line("T. espera avg:",  tw.str());
}

void UI::drawHistory(sf::RenderWindow& w,
                     const DeliveryHistory& h, float y) {
    float x = panelX + 10.f;
    w.draw(makeText("ULTIMAS ENTREGAS", x, y, 13,
                    sf::Color(150, 200, 255)));
    y += 18.f;

    if (h.isEmpty()) {
        w.draw(makeText("Sin entregas aun", x, y, 12,
                        sf::Color(120, 120, 120)));
        return;
    }

    auto last = h.getLastN(5);
    for (const auto& rec : last) {
        std::ostringstream ss;
        ss << rec.orderId
           << "  "
           << std::fixed << std::setprecision(1)
           << rec.deliveryTime << "m";
        w.draw(makeText(ss.str(), x, y, 11,
                        sf::Color(180, 220, 180)));
        y += 14.f;
        if (y > static_cast<float>(Config::WINDOW_HEIGHT) - 20.f) break;
    }
}

// ── Leyenda de colores ────────────────────────────────────────

void UI::drawLegend(sf::RenderWindow& w, float y) {
    float x = panelX + 10.f;
    float bw = (Config::UI_PANEL_WIDTH - 25.f) / 3.f;

    w.draw(makeText("LEYENDA", x, y, 13, sf::Color(150, 200, 255)));
    y += 16.f;

    struct LegendEntry { sf::Color color; const char* label; };
    LegendEntry entries[] = {
        { sf::Color(100, 255, 100), "Dealer libre"     },
        { sf::Color(255, 200,  50), "Yendo a recog."   },
        { sf::Color(255, 100,  50), "Entregando"       },
        { sf::Color(220,  50,  50), "Restaurante"      },
        { sf::Color( 50, 220,  50), "Destino cliente"  },
    };

    sf::CircleShape dot(5.f);
    dot.setOrigin({5.f, 5.f});

    for (const auto& e : entries) {
        dot.setFillColor(e.color);
        dot.setPosition({x + 5.f, y + 6.f});
        w.draw(dot);
        w.draw(makeText(e.label, x + 16.f, y, 11, sf::Color(200, 200, 200)));
        y += 14.f;
        if (y > static_cast<float>(Config::WINDOW_HEIGHT) - 10.f) break;
    }
}

// ── HUD ───────────────────────────────────────────────────────

void UI::drawHUD(sf::RenderWindow& w,
                 const Simulator& sim, float fps) {
    sf::RectangleShape hudBar(
        {static_cast<float>(Config::WINDOW_WIDTH)
         - Config::UI_PANEL_WIDTH, 28.f}
    );
    hudBar.setFillColor(sf::Color(10, 10, 20, 180));
    hudBar.setPosition({0.f, 0.f});
    w.draw(hudBar);

    const auto& ts = sim.getTimeSystem();
    const auto& tr = sim.getTrafficSystem();

    std::ostringstream ss;
    ss << "  "
       << std::setw(2) << std::setfill('0') << ts.getHour()
       << ":"
       << std::setw(2) << std::setfill('0') << ts.getMinute()
       << "   x" << std::fixed << std::setprecision(2)
       << tr.getFinalMultiplier()
       << "   " << sim.getWeatherSystem().stateName()
       << "   " << tr.eventName()
       << "   FPS: " << static_cast<int>(fps);

    w.draw(makeText(ss.str(), 4.f, 6.f, 13,
                    sf::Color(220, 220, 220)));
}

// ── Tooltip ───────────────────────────────────────────────────

void UI::drawTooltip(sf::RenderWindow& w) {
    if (!fontLoaded) return;

    sf::Text t = makeText(tooltipText,
                          tooltipPos.x + 10.f,
                          tooltipPos.y - 20.f, 12);

    float tw = t.getLocalBounds().size.x + 10.f;
    float th = t.getLocalBounds().size.y + 10.f;

    tooltipBg.setSize({tw, th});
    tooltipBg.setPosition({tooltipPos.x + 8.f, tooltipPos.y - 24.f});
    w.draw(tooltipBg);
    w.draw(t);
}

// ── Helpers ───────────────────────────────────────────────────

sf::Text UI::makeText(
    const std::string& str,
    float x, float y,
    unsigned int size,
    sf::Color color
) const {
    sf::Text t(font, str, size);
    t.setFillColor(color);
    t.setPosition({x, y});
    return t;
}

sf::RectangleShape UI::makeButton(
    float x, float y,
    float w, float h,
    sf::Color color
) const {
    sf::RectangleShape btn({w, h});
    btn.setPosition({x, y});
    btn.setFillColor(color);
    btn.setOutlineColor(sf::Color(120, 120, 140));
    btn.setOutlineThickness(1.f);
    return btn;
}

bool UI::isInside(
    float mx, float my,
    float rx, float ry,
    float rw, float rh
) const {
    return mx >= rx && mx <= rx + rw
        && my >= ry && my <= ry + rh;
}