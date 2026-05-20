#include "../visual/RendererSFML.h"
#include "../algorithms/Dijkstra.h"
#include "../utils/Logger.h"
#include <cmath>

// ── Constructor ───────────────────────────────────────────────

RendererSFML::RendererSFML(Simulator& sim)
    : fontLoaded(false),
      simulator(sim),
      animations(sim.getGraph()),
      fpsAccum(0.f),
      fpsCount(0),
      currentFps(0.f)
{
    zoneColors["NORTE"]         = sf::Color(59,  139, 212);
    zoneColors["CENTRO_NORTE"]  = sf::Color(127, 119, 221);
    zoneColors["OCCIDENTE"]     = sf::Color(239, 159,  39);
    zoneColors["SUR_OCCIDENTE"] = sf::Color(216,  90,  48);
    zoneColors["CENTRO"]        = sf::Color( 29, 158, 117);
    zoneColors["SUR"]           = sf::Color(226,  75,  74);
}

// ── Init ──────────────────────────────────────────────────────

bool RendererSFML::init(const std::string& fontPath) {
    window.create(
        sf::VideoMode({
            static_cast<unsigned int>(Config::WINDOW_WIDTH),
            static_cast<unsigned int>(Config::WINDOW_HEIGHT)
        }),
        "VousCommandez - Bogota",
        sf::Style::Titlebar | sf::Style::Close
    );
    window.setFramerateLimit(Config::TARGET_FPS);

    fontLoaded = font.openFromFile(fontPath);
    if (!fontLoaded) {
        Logger::warn("RendererSFML: fuente no encontrada en " + fontPath);
    }

    ui.loadFont(fontPath);

    ui.setOnPause ([this]()       { simulator.pause();  });
    ui.setOnResume([this]()       { simulator.resume(); });
    ui.setOnWeatherChange([this](WeatherState w) {
        simulator.setWeather(w);
    });
    ui.setOnEventTrigger([this](EventState e, double dur) {
        simulator.triggerEvent(e, dur);
    });
    ui.setOnSpeedChange([this](double s) {
        simulator.setSimulationSpeed(s);
    });

    sf::FloatRect bounds = computeWorldBounds();
    camera.init(bounds,
                Config::WINDOW_WIDTH,
                Config::WINDOW_HEIGHT);

    nodeShape.setRadius(Config::NODE_RADIUS_BARRIO);
    nodeShape.setOutlineThickness(1.5f);
    dealerShape.setRadius(Config::DEALER_ICON_SIZE / 2.f);
    dealerShape.setOutlineThickness(2.f);
    restaurantShape.setRadius(Config::NODE_RADIUS_REST);
    restaurantShape.setOutlineThickness(1.5f);
    restaurantShape.setPointCount(4);

    Logger::info("RendererSFML inicializado - ventana "
        + std::to_string(Config::WINDOW_WIDTH) + "x"
        + std::to_string(Config::WINDOW_HEIGHT));

    return true;
}

// ── Loop principal ────────────────────────────────────────────

void RendererSFML::run() {
    simulator.start();

    while (window.isOpen()) {
        float dt = clock.restart().asSeconds();
        updateFps(dt);

        handleEvents();
        update(dt);
        render();
    }
}

// ── Eventos ───────────────────────────────────────────────────

void RendererSFML::handleEvents() {
    while (const std::optional<sf::Event> event = window.pollEvent()) {

        if (event->is<sf::Event::Closed>()) {
            window.close();
        }

        if (const auto* scroll = event->getIf<sf::Event::MouseWheelScrolled>()) {
            if (scroll->delta > 0)
                camera.zoomIn();
            else
                camera.zoomOut();
        }

        if (const auto* click = event->getIf<sf::Event::MouseButtonPressed>()) {
            if (click->button == sf::Mouse::Button::Left) {
                float mx = static_cast<float>(click->position.x);
                float my = static_cast<float>(click->position.y);

                ui.handleMouseClick(mx, my, simulator);

                sf::Vector2f world = camera.screenToWorld(mx, my);
                float bestDist = 15.f / camera.getZoom();
                selectedDealerId = "";

                for (const auto& [id, dealer] : simulator.getDealers()) {
                    sf::Vector2f pos = getDealerScreenPos(id);
                    if (pos.x < 0.f) continue;
                    float dx = pos.x - world.x;
                    float dy = pos.y - world.y;
                    float d  = std::sqrt(dx * dx + dy * dy);
                    if (d < bestDist) {
                        bestDist         = d;
                        selectedDealerId = id;
                    }
                }
            }
        }

        if (const auto* moved = event->getIf<sf::Event::MouseMoved>()) {
            ui.handleMouseMove(
                static_cast<float>(moved->position.x),
                static_cast<float>(moved->position.y)
            );
        }

        if (const auto* key = event->getIf<sf::Event::KeyPressed>()) {
            if (key->code == sf::Keyboard::Key::R) {
                camera.resetCenter();
                camera.resetZoom();
            }
            if (key->code == sf::Keyboard::Key::Escape) {
                window.close();
            }
        }
    }
}

// ── Update ────────────────────────────────────────────────────

void RendererSFML::update(float realDt) {
    camera.update(realDt);
    simulator.tick(realDt);

    syncAnimations();
    

    if (!simulator.getTimeSystem().isPaused()) {
        float simDelta = realDt
            * static_cast<float>(
                simulator.getTimeSystem().getSimulationSpeed()
              );
        animations.update(simDelta);
    }
}

// ── Render ────────────────────────────────────────────────────

void RendererSFML::render() {
    window.clear(sf::Color(15, 15, 25));

    window.setView(camera.getView());

    drawGraph();
    drawOrderMarkers();  // <-- restaurante (rojo) y cliente (verde) de cada orden activa
    drawRoutes();
    drawDealers();

    window.setView(window.getDefaultView());

    ui.draw(window, simulator, currentFps);

    window.display();
}

// ── Dibujo del mapa ───────────────────────────────────────────

void RendererSFML::drawGraph() {
    drawEdges();
    drawNodes();
}

void RendererSFML::drawEdges() {
    for (const auto& [fromId, edges]
         : simulator.getGraph().getAdjacencyList())
    {
        const auto& fromNode = simulator.getGraph().getNode(fromId);
        sf::Vector2f fromPos = {
            static_cast<float>(fromNode.getPosition().x),
            static_cast<float>(fromNode.getPosition().y)
        };

        for (const auto& edge : edges) {
            const auto& toNode =
                simulator.getGraph().getNode(edge.getTo());
            sf::Vector2f toPos = {
                static_cast<float>(toNode.getPosition().x),
                static_cast<float>(toNode.getPosition().y)
            };

            sf::Color col = edgeColor(edge.getRoadType());
            col.a = 100;

            drawEdge(fromPos, toPos, col, Config::EDGE_THICKNESS);
        }
    }
}

void RendererSFML::drawNodes() {
    for (const auto& [id, node]
         : simulator.getGraph().getNodes())
    {
        sf::Vector2f pos = {
            static_cast<float>(node.getPosition().x),
            static_cast<float>(node.getPosition().y)
        };

        bool isRestaurant = (node.getType() == NodeType::RESTAURANTE);
        sf::Color fill    = zoneColor(node.getZone());
        sf::Color outline = sf::Color::White;
        float radius      = isRestaurant
                            ? Config::NODE_RADIUS_REST
                            : Config::NODE_RADIUS_BARRIO;

        drawNode(pos, radius, fill, outline);

        if (fontLoaded && camera.getZoom() > 1.8f) {
            sf::Text label(font, node.getName(), 8);
            label.setFillColor(sf::Color(220, 220, 220));
            label.setPosition({pos.x + radius + 2.f, pos.y - 6.f});
            window.draw(label);
        }
    }
}

// ── Marcadores de orden activa (restaurante + cliente) ────────
// Rojo  = restaurante donde el dealer va a recoger
// Verde = nodo del cliente donde se entrega
void RendererSFML::drawOrderMarkers() {
    const float MARKER_R = 7.f;

    sf::CircleShape marker(MARKER_R);
    marker.setOrigin({MARKER_R, MARKER_R});
    marker.setOutlineThickness(2.f);

    for (const auto& [dealerId, dealer] : simulator.getDealers()) {
        if (dealer.getActiveOrderIds().empty()) continue;

        const std::string& orderId = dealer.getActiveOrderIds().front();
        auto orderIt = simulator.getOrders().find(orderId);
        if (orderIt == simulator.getOrders().end()) continue;

        const Order& order = orderIt->second;

        // ── Restaurante (rojo) ────────────────────────────────
        auto restIt = simulator.getRestaurants().find(order.getRestaurantId());
        if (restIt != simulator.getRestaurants().end()) {
            const std::string& restNodeId = restIt->second.getNodeId();
            if (simulator.getGraph().hasNode(restNodeId)) {
                const auto& rn = simulator.getGraph().getNode(restNodeId);
                sf::Vector2f rpos = {
                    static_cast<float>(rn.getPosition().x),
                    static_cast<float>(rn.getPosition().y)
                };
                marker.setPosition(rpos);
                marker.setFillColor(sf::Color(220, 50, 50, 200));
                marker.setOutlineColor(sf::Color(255, 150, 150));
                window.draw(marker);
            }
        }

        // ── Cliente destino (verde) ───────────────────────────
        const std::string& clientNodeId = order.getClientNodeId();
        if (simulator.getGraph().hasNode(clientNodeId)) {
            const auto& cn = simulator.getGraph().getNode(clientNodeId);
            sf::Vector2f cpos = {
                static_cast<float>(cn.getPosition().x),
                static_cast<float>(cn.getPosition().y)
            };
            marker.setPosition(cpos);
            marker.setFillColor(sf::Color(50, 220, 50, 200));
            marker.setOutlineColor(sf::Color(150, 255, 150));
            window.draw(marker);
        }
    }
}

void RendererSFML::drawRoutes() {
    for (const auto& [id, dealer] : simulator.getDealers()) {
        if (dealer.getActiveOrderIds().empty()) continue;

        bool highlight = (id == selectedDealerId);

        // Usar la ruta guardada en dealerRouteCache, no recalcular Dijkstra cada frame
        auto cacheIt = dealerRouteCache.find(id);
        if (cacheIt == dealerRouteCache.end() || cacheIt->second.path.nodes.empty()) continue;

        const RouteResult& route = cacheIt->second;
        if (!route.success) continue;

        sf::Color routeCol = highlight
            ? sf::Color(255, 200, 0, 220)
            : sf::Color(100, 180, 255, 140);

        float thickness = highlight
            ? Config::ROUTE_THICKNESS + 1.f
            : Config::ROUTE_THICKNESS;

        const auto& nodes = route.path.nodes;
        for (size_t i = 0; i + 1 < nodes.size(); i++) {
            if (!simulator.getGraph().hasNode(nodes[i]) ||
                !simulator.getGraph().hasNode(nodes[i + 1])) continue;

            const auto& a = simulator.getGraph().getNode(nodes[i]);
            const auto& b = simulator.getGraph().getNode(nodes[i + 1]);

            drawEdge(
                {static_cast<float>(a.getPosition().x),
                 static_cast<float>(a.getPosition().y)},
                {static_cast<float>(b.getPosition().x),
                 static_cast<float>(b.getPosition().y)},
                routeCol,
                thickness
            );
        }
    }
}

void RendererSFML::drawDealers() {
    for (const auto& [id, dealer] : simulator.getDealers()) {
        sf::Vector2f pos = getDealerScreenPos(id);
        if (pos.x < 0.f) continue;
        drawDealer(pos, id, id == selectedDealerId);
    }
}

// ── Helpers de dibujo ─────────────────────────────────────────

sf::Vector2f RendererSFML::getDealerScreenPos(const std::string& dealerId) const {
    // Primero intentar posición animada
    sf::Vector2f pos = animations.getPosition(dealerId);
    if (pos.x >= 0.f) return pos;

    // Si no hay animación activa, usar nodo actual del dealer (posición fija)
    auto it = simulator.getDealers().find(dealerId);
    if (it == simulator.getDealers().end()) return {-1.f, -1.f};

    const std::string& nodeId = it->second.getCurrentNodeId();
    if (!simulator.getGraph().hasNode(nodeId)) return {-1.f, -1.f};

    const auto& n = simulator.getGraph().getNode(nodeId);
    return {
        static_cast<float>(n.getPosition().x),
        static_cast<float>(n.getPosition().y)
    };
}

void RendererSFML::drawEdge(
    sf::Vector2f from,
    sf::Vector2f to,
    sf::Color color,
    float thickness
) {
    sf::Vector2f dir   = to - from;
    float length       = std::sqrt(dir.x * dir.x + dir.y * dir.y);
    if (length < 0.001f) return;

    float angle = std::atan2(dir.y, dir.x) * 180.f / 3.14159f;

    sf::RectangleShape line({length, thickness});
    line.setPosition(from);
    line.setRotation(sf::degrees(angle));
    line.setFillColor(color);
    window.draw(line);
}

void RendererSFML::drawNode(
    sf::Vector2f pos,
    float radius,
    sf::Color fill,
    sf::Color outline
) {
    nodeShape.setRadius(radius);
    nodeShape.setFillColor(fill);
    nodeShape.setOutlineColor(outline);
    nodeShape.setOrigin({radius, radius});
    nodeShape.setPosition(pos);
    window.draw(nodeShape);
}

void RendererSFML::drawDealer(
    sf::Vector2f pos,
    const std::string& dealerId,
    bool isSelected
) {
    float r = Config::DEALER_ICON_SIZE / 2.f;
    dealerShape.setRadius(r);
    dealerShape.setOrigin({r, r});
    dealerShape.setPosition(pos);

    auto dealerIt = simulator.getDealers().find(dealerId);
    if (dealerIt == simulator.getDealers().end()) return;
    const auto& dealer = dealerIt->second;

    sf::Color fill;
    switch (dealer.getStatus()) {
    case DealerStatus::IDLE:
        fill = sf::Color(100, 255, 100); break;        // verde = libre
    case DealerStatus::HEADING_TO_RESTAURANT:
        fill = sf::Color(255, 200, 50);  break;        // amarillo = yendo a restaurante
    case DealerStatus::PICKING_UP:
        fill = sf::Color(255, 160, 30);  break;        // naranja cálido = recogiendo
    case DealerStatus::DELIVERING:
        fill = sf::Color(255, 100, 50);  break;        // naranja rojo = entregando
    case DealerStatus::RETURNING:
        fill = sf::Color(100, 200, 255); break;        // azul claro = regresando
    default:
        fill = sf::Color(200, 200, 200); break;
}

    dealerShape.setFillColor(fill);
    dealerShape.setOutlineColor(isSelected
        ? sf::Color::White
        : sf::Color(50, 50, 50));
    dealerShape.setOutlineThickness(isSelected ? 2.5f : 1.5f);
    window.draw(dealerShape);

    if (fontLoaded && camera.getZoom() > 1.2f) {
        sf::Text label(font, dealerId.substr(0, 4), 9);
        label.setFillColor(sf::Color::White);
        label.setPosition({pos.x + r + 2.f, pos.y - 6.f});
        window.draw(label);
    }
}

// ── Sincronización de animaciones ─────────────────────────────
//
// CORRECCIONES respecto a la versión original:
//
// 1. La animación ya NO se limpia cuando activeOrderIds está vacía.
//    La animación es puramente visual y corre hasta terminar por sí sola.
//    El dealer en IDLE sin animación activa simplemente muestra su nodo fijo.
//
// 2. La ruta se calcula UNA SOLA VEZ al asignar la orden (cuando cambia
//    el orderId) y se guarda en dealerRouteCache. Tanto la animación como
//    drawRoutes usan esa ruta cacheada, sin recalcular Dijkstra cada frame.
//
// 3. La ruta incluye restaurante->cliente: dealer_pos -> restaurante -> cliente
//    usando dos tramos de Dijkstra concatenados.

void RendererSFML::syncAnimations() {
    const auto& dealers = simulator.getDealers();
    const auto& orders  = simulator.getOrders();
    const auto& rests   = simulator.getRestaurants();

    for (const auto& [id, dealer] : dealers) {

        // Si no tiene orden activa, no iniciar nueva animación.
        // Si ya hay una animación corriendo, dejarla terminar sola.
        if (dealer.getActiveOrderIds().empty()) {
            // Limpiar cache de ruta cuando el dealer queda libre
            // (la animación puede seguir hasta terminar)
            dealerRouteCache.erase(id);
            dealerCurrentOrder.erase(id);
            continue;
        }

        const std::string& currentOrderId = dealer.getActiveOrderIds().front();

        // Si es la misma orden que ya estamos animando, no hacer nada
        auto it = dealerCurrentOrder.find(id);
        if (it != dealerCurrentOrder.end() &&
            it->second == currentOrderId)
        {
            continue;
        }

        // Nueva orden → calcular ruta completa y arrancar animación
        dealerCurrentOrder[id] = currentOrderId;

        auto orderIt = orders.find(currentOrderId);
        if (orderIt == orders.end()) continue;

        const Order& order = orderIt->second;

        auto restIt = rests.find(order.getRestaurantId());
        if (restIt == rests.end()) continue;

        const std::string& restNodeId    = restIt->second.getNodeId();
        const std::string& clientNodeId  = order.getClientNodeId();
        const std::string& dealerNodeId  = dealer.getCurrentNodeId();

        TrafficEdgeData traffic = simulator.getTrafficSystem().getCurrentTraffic();

        // Tramo 1: dealer → restaurante
        RouteResult leg1 = Dijkstra::compute(
            simulator.getGraph(), dealerNodeId, restNodeId, traffic);

        // Tramo 2: restaurante → cliente
        RouteResult leg2 = Dijkstra::compute(
            simulator.getGraph(), restNodeId, clientNodeId, traffic);

        if (!leg1.success && !leg2.success) continue;

        // Concatenar los dos tramos en una sola ruta para la animación
        RouteResult fullRoute;
        fullRoute.success       = true;
        fullRoute.algorithmName = "Dijkstra";

        if (leg1.success) {
            fullRoute.path.nodes = leg1.path.nodes;
            for (const auto& seg : leg1.path.segments)
                fullRoute.path.segments.push_back(seg);
            fullRoute.path.totalCost     = leg1.path.totalCost;
            fullRoute.path.totalDistance = leg1.path.totalDistance;
            fullRoute.path.estimatedTime = leg1.path.estimatedTime;
        }

        if (leg2.success && !leg2.path.nodes.empty()) {
            // Evitar duplicar el nodo de conexión (restaurante)
            for (size_t i = 1; i < leg2.path.nodes.size(); i++)
                fullRoute.path.nodes.push_back(leg2.path.nodes[i]);
            for (const auto& seg : leg2.path.segments)
                fullRoute.path.segments.push_back(seg);
            fullRoute.path.totalCost     += leg2.path.totalCost;
            fullRoute.path.totalDistance += leg2.path.totalDistance;
            fullRoute.path.estimatedTime += leg2.path.estimatedTime;
        }

        fullRoute.path.valid = true;
        dealerRouteCache[id] = fullRoute;

        // Duración = tiempo estimado total de la orden (minutos simulados)
        // La animación correrá exactamente ese tiempo antes de terminar
        float simDuration = static_cast<float>(order.getEstimatedTime());
        if (simDuration <= 0.f)
            simDuration = static_cast<float>(fullRoute.path.estimatedTime);

        animations.setRoute(id, fullRoute.path.nodes, simDuration);
    }
}

// ── Bounding box del grafo ────────────────────────────────────

sf::FloatRect RendererSFML::computeWorldBounds() const {

    const auto& nodes = simulator.getGraph().getNodes();

    if (nodes.empty()) {
        return sf::FloatRect(
            sf::Vector2f(0.f, 0.f),
            sf::Vector2f(1000.f, 1000.f)
        );
    }

    float minX = 999999.f;
    float minY = 999999.f;
    float maxX = -999999.f;
    float maxY = -999999.f;

    for (const auto& [id, node] : nodes) {
        float x = static_cast<float>(node.getPosition().x);
        float y = static_cast<float>(node.getPosition().y);
        minX = std::min(minX, x);
        minY = std::min(minY, y);
        maxX = std::max(maxX, x);
        maxY = std::max(maxY, y);
    }

    float padding = 100.f;

    return sf::FloatRect(
        sf::Vector2f(minX - padding, minY - padding),
        sf::Vector2f(
            (maxX - minX) + padding * 2.f,
            (maxY - minY) + padding * 2.f
        )
    );
}

// ── Color por zona ────────────────────────────────────────────

sf::Color RendererSFML::zoneColor(Zone zone) const {
    switch (zone) {
        case Zone::NORTE:         return sf::Color( 59, 139, 212);
        case Zone::CENTRO_NORTE:  return sf::Color(127, 119, 221);
        case Zone::OCCIDENTE:     return sf::Color(239, 159,  39);
        case Zone::SUR_OCCIDENTE: return sf::Color(216,  90,  48);
        case Zone::CENTRO:        return sf::Color( 29, 158, 117);
        case Zone::SUR:           return sf::Color(226,  75,  74);
        default:                  return sf::Color(150, 150, 150);
    }
}

// ── Color por tipo de vía ─────────────────────────────────────

sf::Color RendererSFML::edgeColor(RoadType road) const {
    switch (road) {
        case RoadType::AUTOPISTA: return sf::Color(220, 180,  60);
        case RoadType::AVENIDA:   return sf::Color(180, 180, 180);
        case RoadType::CARRERA:   return sf::Color(140, 140, 160);
        case RoadType::CALLE:     return sf::Color(100, 100, 120);
        default:                  return sf::Color( 80,  80, 100);
    }
}

// ── FPS ───────────────────────────────────────────────────────

void RendererSFML::updateFps(float dt) {
    fpsAccum += dt;
    fpsCount++;
    if (fpsAccum >= 0.5f) {
        currentFps = static_cast<float>(fpsCount) / fpsAccum;
        fpsAccum   = 0.f;
        fpsCount   = 0;
    }
}