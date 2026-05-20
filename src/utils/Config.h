#pragma once

// ═══════════════════════════════════════════════════════════════
//  CONFIG — fuente unica de verdad para todos los parametros
//  Toda constante numerica del proyecto debe vivir aqui.
//  Simulacion, trafico, estructuras, visual y UI dependen de esto.
// ═══════════════════════════════════════════════════════════════

class Config {
public:
    Config() = delete;

    // ── Simulacion ────────────────────────────────────────────
    // min simulados / seg real
    //   Lenta  = 0.75x (casi tiempo real, muy observable)
    //   Normal = 2x    (por defecto)
    //   Rapida = 5x    (acelerado)
    static constexpr double SIM_SPEED_DEFAULT   = 2.0;
    static constexpr double SIM_SPEED_FAST      = 5.0;
    static constexpr double SIM_SPEED_SLOW      = 0.75;

    static constexpr int    SIM_START_HOUR      = 8;

    // ── Trafico — multiplicadores de peso ─────────────────────
    static constexpr double WEATHER_CLEAR       = 1.0;
    static constexpr double WEATHER_RAIN        = 1.3;
    static constexpr double WEATHER_STORM       = 1.6;

    static constexpr double HOUR_NORMAL         = 1.0;
    static constexpr double HOUR_PEAK           = 1.5;
    static constexpr double HOUR_DAWN           = 0.8;

    static constexpr double EVENT_NONE          = 1.0;
    static constexpr double EVENT_MARCH         = 1.2;
    static constexpr double EVENT_CONCERT       = 1.4;
    static constexpr double EVENT_MATCH         = 1.4;

    // ── Repartidores ──────────────────────────────────────────
    static constexpr int    DEALER_MAX_ORDERS   = 3;
    static constexpr double DEALER_SPEED_KMH    = 35.0;
    static constexpr double DEALER_SCORE_ALPHA  = 0.6;
    static constexpr double DEALER_SCORE_BETA   = 0.4;

    // Tiempo que el dealer espera en el restaurante recogiendo
    // (en minutos simulados). Visible en pantalla como PICKING_UP.
    static constexpr double DEALER_PICKUP_TIME  = 3.0;

    // ── Ordenes ───────────────────────────────────────────────
    static constexpr int    ORDER_MAX_ACTIVE    = 200;
    static constexpr int    ORDER_PRIORITY_MIN  = 1;
    static constexpr int    ORDER_PRIORITY_MAX  = 5;
    static constexpr double ORDER_STALE_MINUTES = 10.0;
    static constexpr int    ORDER_BOOST_AMOUNT  = 2;
    static constexpr double ORDER_BOOST_INTERVAL= 5.0;

    // ── Generador de ordenes ──────────────────────────────────
    static constexpr double SPAWN_RATE_BASE     = 0.5;
    static constexpr double DEMAND_DAWN         = 0.2;
    static constexpr double DEMAND_MORNING      = 0.8;
    static constexpr double DEMAND_LUNCH        = 2.0;
    static constexpr double DEMAND_AFTERNOON    = 0.7;
    static constexpr double DEMAND_DINNER       = 1.8;
    static constexpr double DEMAND_NIGHT        = 0.5;

    // ── Restaurantes ──────────────────────────────────────────
    static constexpr int    RESTAURANT_BUSY_THRESHOLD = 5;
    static constexpr double REST_SCORE_VOLUME   = 0.7;
    static constexpr double REST_SCORE_RATING   = 0.3;
    static constexpr double REST_SCORE_BUSY_PEN = 0.05;

    // ── Historial de entregas ─────────────────────────────────
    static constexpr int    HISTORY_MAX_RECORDS = 1000;

    // ── Visual / SFML ─────────────────────────────────────────
    static constexpr int    WINDOW_WIDTH        = 1280;
    static constexpr int    WINDOW_HEIGHT       = 720;
    static constexpr int    TARGET_FPS          = 60;
    static constexpr float  CAMERA_ZOOM_MIN     = 0.3f;
    static constexpr float  CAMERA_ZOOM_MAX     = 5.0f;
    static constexpr float  CAMERA_ZOOM_DEFAULT = 1.0f;
    static constexpr float  CAMERA_PAN_SPEED    = 300.0f;

    // ── Nodos en pantalla ─────────────────────────────────────
    static constexpr float  NODE_RADIUS_BARRIO  = 8.0f;
    static constexpr float  NODE_RADIUS_REST    = 7.0f;
    static constexpr float  EDGE_THICKNESS      = 1.5f;
    static constexpr float  ROUTE_THICKNESS     = 3.5f;
    static constexpr float  DEALER_ICON_SIZE    = 13.0f;

    // ── UI ────────────────────────────────────────────────────
    static constexpr float  UI_PANEL_WIDTH      = 280.0f;
    static constexpr float  UI_HUD_HEIGHT       = 28.0f;
    static constexpr float  UI_FONT_SIZE_SMALL  = 12.0f;
    static constexpr float  UI_FONT_SIZE_NORMAL = 14.0f;
    static constexpr float  UI_FONT_SIZE_TITLE  = 18.0f;

    // ── Logger ────────────────────────────────────────────────
    static constexpr bool   LOG_TO_CONSOLE      = true;
    static constexpr bool   LOG_TO_FILE         = true;
    static constexpr char   LOG_FILENAME[]      = "vous_commandez.log";
};