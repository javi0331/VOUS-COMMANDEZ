#pragma once

#include <SFML/Graphics.hpp>
#include <unordered_map>
#include <string>

#include "../simulation/Simulator.h"
#include "../visual/Camera.h"
#include "../visual/UI.h"
#include "../visual/Animations.h"
#include "../algorithms/RouteResult.h"
#include "../utils/Config.h"

// Orquesta el render loop completo.
// Separa responsabilidades:
//   - drawGraph()        → nodos y aristas del grafo
//   - drawOrderMarkers() → marcador rojo (restaurante) y verde (cliente)
//   - drawRoutes()       → rutas activas de cada dealer (desde cache)
//   - drawDealers()      → iconos animados de repartidores
//   - UI::draw()         → panel lateral + HUD (sin camera)
//
// Leyenda de colores dealers:
//   Verde  = IDLE (libre, esperando pedido)
//   Amarillo = HEADING_TO_RESTAURANT (yendo a recoger)
//   Naranja  = DELIVERING (en camino al cliente)
//
// Leyenda de marcadores de orden:
//   Rojo  = restaurante de la orden activa
//   Verde = nodo destino del cliente

class RendererSFML {
private:
    sf::RenderWindow window;
    sf::Clock        clock;
    sf::Font         font;
    bool             fontLoaded;

    Simulator&  simulator;
    Camera      camera;
    UI          ui;
    Animations  animations;

    // Shapes reutilizables (evita allocar cada frame)
    sf::CircleShape    nodeShape;
    sf::RectangleShape edgeShape;
    sf::CircleShape    dealerShape;
    sf::CircleShape    restaurantShape;

    // Cache de colores por zona
    std::unordered_map<std::string, sf::Color> zoneColors;

    // Ruta seleccionada para highlight (click en dealer)
    std::string selectedDealerId;

    // Trackea qué orden tiene cada dealer animado.
    // syncAnimations solo reinicia la animación si cambia el orderId.
    std::unordered_map<std::string, std::string> dealerCurrentOrder;

    // Cache de rutas calculadas por dealer (dealer → restaurante → cliente).
    // Se recalcula solo cuando cambia la orden asignada.
    // drawRoutes lee de aquí — NO recalcula Dijkstra cada frame.
    std::unordered_map<std::string, RouteResult> dealerRouteCache;

    // FPS suavizado
    float fpsAccum;
    int   fpsCount;
    float currentFps;

public:
    explicit RendererSFML(Simulator& simulator);

    // Crea la ventana, carga fuente, inicializa Camera y UI
    bool init(const std::string& fontPath = "assets/fonts/Roboto-Regular.ttf");

    // Loop principal — bloquea hasta cerrar ventana
    void run();

private:
    // ── Loop ──────────────────────────────────────────────────
    void handleEvents();
    void update(float realDt);
    void render();

    // ── Dibujo del mapa (con camera transform) ────────────────
    void drawGraph();
    void drawEdges();
    void drawNodes();
    void drawOrderMarkers(); // Marcador rojo=restaurante, verde=cliente
    void drawRoutes();       // Rutas activas de dealers (desde cache)
    void drawDealers();      // Iconos animados

    // ── Helpers de dibujo ─────────────────────────────────────
    void drawEdge(
        sf::Vector2f from,
        sf::Vector2f to,
        sf::Color color,
        float thickness
    );

    void drawNode(
        sf::Vector2f pos,
        float radius,
        sf::Color fill,
        sf::Color outline
    );

    void drawDealer(
        sf::Vector2f pos,
        const std::string& dealerId,
        bool isSelected
    );

    // Devuelve la posicion actual del dealer:
    //   - Si tiene animacion activa: posicion interpolada
    //   - Si no: posicion fija del nodo currentNodeId
    //   - Si nada: {-1, -1}
    sf::Vector2f getDealerScreenPos(const std::string& dealerId) const;

    // Bounding box del grafo para inicializar la Camera
    sf::FloatRect computeWorldBounds() const;

    // Color del nodo segun zona
    sf::Color zoneColor(Zone zone) const;

    // Color de la arista segun tipo de via
    sf::Color edgeColor(RoadType road) const;

    // Sincroniza animaciones con el estado del Simulator.
    // Calcula la ruta completa (dealer->restaurante->cliente) UNA SOLA VEZ
    // por orden y la guarda en dealerRouteCache.
    void syncAnimations();

    void updateFps(float dt);
};