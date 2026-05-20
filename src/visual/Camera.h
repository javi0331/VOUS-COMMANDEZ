#pragma once

#include <SFML/Graphics.hpp>
#include "../utils/Config.h"

// Maneja zoom, paneo y la transformación mundo↔pantalla.
// RendererSFML aplica la vista de Camera antes de dibujar el grafo.
// UI nunca pasa por Camera — se dibuja en coordenadas de pantalla fijas.

class Camera {
private:
    sf::View     view;
    sf::Vector2f center;
    float        zoom;

    float panSpeed;
    float zoomMin;
    float zoomMax;

    sf::FloatRect worldBounds;

    // Dimensiones del área del mapa (ventana - panel - HUD)
    // Se calculan una vez en init() y se reusan en applyZoom/screenToWorld
    float mapW;
    float mapH;

public:
    Camera();

    void init(
        const sf::FloatRect& worldBounds,
        unsigned int windowWidth,
        unsigned int windowHeight
    );

    // ── Actualización ─────────────────────────────────────────
    void update(float dt);

    // ── Zoom ──────────────────────────────────────────────────
    void zoomIn (float factor = 1.1f);
    void zoomOut(float factor = 1.1f);
    void setZoom(float z);
    float getZoom() const;
    void resetZoom();

    // ── Paneo ─────────────────────────────────────────────────
    void move(float dx, float dy);
    void centerOn(sf::Vector2f worldPos);
    void resetCenter();

    // ── Transformaciones ──────────────────────────────────────
    sf::Vector2f worldToScreen(float wx, float wy) const;
    sf::Vector2f screenToWorld(float sx, float sy) const;

    // ── Acceso ────────────────────────────────────────────────
    const sf::View& getView() const;
    sf::View getUIView(unsigned int w, unsigned int h) const;

private:
    void clampCenter();
    void applyZoom();
};