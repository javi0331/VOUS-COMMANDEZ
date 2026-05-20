#include "../visual/Camera.h"
#include <algorithm>

Camera::Camera()
    : center(0.f, 0.f),
      zoom(Config::CAMERA_ZOOM_DEFAULT),
      panSpeed(Config::CAMERA_PAN_SPEED),
      zoomMin(Config::CAMERA_ZOOM_MIN),
      zoomMax(Config::CAMERA_ZOOM_MAX),
      worldBounds({0.f, 0.f}, {800.f, 600.f}),
      mapW(0.f),
      mapH(0.f) {}

void Camera::init(
    const sf::FloatRect& bounds,
    unsigned int winW,
    unsigned int winH
) {
    worldBounds = bounds;

    // Área del mapa = ventana - panel lateral - HUD superior
    mapW = static_cast<float>(winW) - Config::UI_PANEL_WIDTH;
    mapH = static_cast<float>(winH) - Config::UI_HUD_HEIGHT;

    // Viewport normalizado — establecer ANTES de setSize/setCenter
    // para que SFML no lo resetee al perder o recuperar foco de ventana
    float vpX1 = 0.f;
    float vpY1 = Config::UI_HUD_HEIGHT / static_cast<float>(winH);
    float vpW  = mapW / static_cast<float>(winW);
    float vpH  = mapH / static_cast<float>(winH);

    view.setViewport(sf::FloatRect(
        sf::Vector2f(vpX1, vpY1),
        sf::Vector2f(vpW,  vpH)
    ));

    // Centro en el centroide del grafo
    center = sf::Vector2f(
        bounds.position.x + bounds.size.x / 2.f,
        bounds.position.y + bounds.size.y / 2.f
    );

    // Zoom inicial para que el grafo quepa completo con padding del 5%
    float scaleX = mapW / std::max(bounds.size.x, 1.f);
    float scaleY = mapH / std::max(bounds.size.y, 1.f);
    zoom = std::max(zoomMin, std::min(zoomMax,
                    std::min(scaleX, scaleY) * 0.95f));

    view.setSize({mapW / zoom, mapH / zoom});
    view.setCenter(center);
}

// ── Actualización ─────────────────────────────────────────────

void Camera::update(float dt) {
    float speed = panSpeed / zoom;

    if (sf::Keyboard::isKeyPressed(sf::Keyboard::Key::Left)  ||
        sf::Keyboard::isKeyPressed(sf::Keyboard::Key::A))
        move(-speed * dt, 0.f);

    if (sf::Keyboard::isKeyPressed(sf::Keyboard::Key::Right) ||
        sf::Keyboard::isKeyPressed(sf::Keyboard::Key::D))
        move( speed * dt, 0.f);

    if (sf::Keyboard::isKeyPressed(sf::Keyboard::Key::Up)    ||
        sf::Keyboard::isKeyPressed(sf::Keyboard::Key::W))
        move(0.f, -speed * dt);

    if (sf::Keyboard::isKeyPressed(sf::Keyboard::Key::Down)  ||
        sf::Keyboard::isKeyPressed(sf::Keyboard::Key::S))
        move(0.f,  speed * dt);
}

// ── Zoom ──────────────────────────────────────────────────────

void Camera::zoomIn(float factor) {
    zoom = std::min(zoomMax, zoom * factor);
    applyZoom();
}

void Camera::zoomOut(float factor) {
    zoom = std::max(zoomMin, zoom / factor);
    applyZoom();
}

void Camera::setZoom(float z) {
    zoom = std::max(zoomMin, std::min(zoomMax, z));
    applyZoom();
}

float Camera::getZoom() const { return zoom; }

void Camera::resetZoom() {
    float scaleX = mapW / std::max(worldBounds.size.x, 1.f);
    float scaleY = mapH / std::max(worldBounds.size.y, 1.f);
    zoom = std::max(zoomMin, std::min(zoomMax,
                    std::min(scaleX, scaleY) * 0.95f));
    applyZoom();
}

// ── Paneo ─────────────────────────────────────────────────────

void Camera::move(float dx, float dy) {
    center.x += dx;
    center.y += dy;
    clampCenter();
    view.setCenter(center);
}

void Camera::centerOn(sf::Vector2f worldPos) {
    center = worldPos;
    clampCenter();
    view.setCenter(center);
}

void Camera::resetCenter() {
    center = sf::Vector2f(
        worldBounds.position.x + worldBounds.size.x / 2.f,
        worldBounds.position.y + worldBounds.size.y / 2.f
    );
    view.setCenter(center);
}

// ── Transformaciones ──────────────────────────────────────────

sf::Vector2f Camera::worldToScreen(float wx, float wy) const {
    sf::Vector2f vSize   = view.getSize();
    sf::Vector2f vCenter = view.getCenter();

    float sx = (wx - vCenter.x) / vSize.x * mapW + mapW / 2.f;
    float sy = (wy - vCenter.y) / vSize.y * mapH + mapH / 2.f
               + Config::UI_HUD_HEIGHT;
    return {sx, sy};
}

sf::Vector2f Camera::screenToWorld(float sx, float sy) const {
    sf::Vector2f vSize   = view.getSize();
    sf::Vector2f vCenter = view.getCenter();

    float wx = (sx - mapW / 2.f) / mapW * vSize.x + vCenter.x;
    float wy = ((sy - Config::UI_HUD_HEIGHT) - mapH / 2.f)
               / mapH * vSize.y + vCenter.y;
    return {wx, wy};
}

// ── Acceso ────────────────────────────────────────────────────

const sf::View& Camera::getView() const { return view; }

sf::View Camera::getUIView(unsigned int w, unsigned int h) const {
    sf::View ui;
    ui.setSize({static_cast<float>(w), static_cast<float>(h)});
    ui.setCenter({static_cast<float>(w) / 2.f,
                  static_cast<float>(h) / 2.f});
    return ui;
}

// ── Privados ──────────────────────────────────────────────────

void Camera::clampCenter() {
    float hw = view.getSize().x / 2.f;
    float hh = view.getSize().y / 2.f;

    center.x = std::max(worldBounds.position.x + hw,
               std::min(worldBounds.position.x + worldBounds.size.x - hw,
                        center.x));
    center.y = std::max(worldBounds.position.y + hh,
               std::min(worldBounds.position.y + worldBounds.size.y - hh,
                        center.y));
}

void Camera::applyZoom() {
    view.setSize({mapW / zoom, mapH / zoom});
    view.setCenter(center);
}