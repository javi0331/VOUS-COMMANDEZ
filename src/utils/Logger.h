#pragma once

#include <string>
#include <fstream>
#include <sstream>

enum class LogLevel {
    DEBUG,    // Detalles internos (solo en desarrollo)
    INFO,     // Eventos normales de la sim
    WARNING,  // Situaciones inesperadas pero no fatales
    ERROR     // Errores que afectan la sim
};

// Logger singleton — escribe a consola y/o archivo según Config.
//
// Uso:
//   Logger::info("Orden ORD_001 asignada a dealer D3");
//   Logger::warn("Dealer D5 sin ruta al restaurante R12");
//   Logger::error("Nodo N99 no existe en el grafo");

class Logger {
private:
    static Logger* instance;

    std::ofstream fileStream;
    LogLevel      minLevel;
    bool          toConsole;
    bool          toFile;

    Logger(
        bool toConsole,
        bool toFile,
        const std::string& filename,
        LogLevel minLevel
    );

    void writeEntry(LogLevel level, const std::string& message);

    static std::string levelTag(LogLevel level);
    static std::string timestamp();

public:
    ~Logger();

    // Inicializar una sola vez al arrancar la app.
    // Llamar antes de cualquier log.
    static void init(
        bool toConsole        = true,
        bool toFile           = true,
        const std::string& filename = "vous_commandez.log",
        LogLevel minLevel     = LogLevel::DEBUG
    );

    static void debug(const std::string& msg);
    static void info (const std::string& msg);
    static void warn (const std::string& msg);
    static void error(const std::string& msg);

    // Versión con stream para componer mensajes fácilmente:
    //   Logger::log(LogLevel::INFO) << "Orden " << id << " entregada";
    // (helper interno — ver implementación)
    static void log(LogLevel level, const std::string& msg);

    static void setMinLevel(LogLevel level);
    static void flush();

    // No permitir copia
    Logger(const Logger&)            = delete;
    Logger& operator=(const Logger&) = delete;
};

// ── Macro helper para mensajes con contexto ───────────────────
// Logger::info("assignOrders") << "Dealer D1 -> Orden ORD_003";
// No necesita macro; se construye el string antes de llamar.
// Ejemplo limpio:
//   Logger::info("Dealer " + dealer.getId() + " asignado a " + orderId);