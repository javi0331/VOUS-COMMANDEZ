#include "../utils/Logger.h"
#include "../utils/Config.h"

#include <iostream>
#include <chrono>
#include <ctime>
#include <iomanip>
#include <sstream>

Logger* Logger::instance = nullptr;

// ── Constructor / Destructor ──────────────────────────────────

Logger::Logger(
    bool toConsole,
    bool toFile,
    const std::string& filename,
    LogLevel minLevel
)
    : minLevel(minLevel),
      toConsole(toConsole),
      toFile(toFile)
{
    if (toFile) {
        fileStream.open(filename, std::ios::app);
    }
}

Logger::~Logger() {
    if (fileStream.is_open()) fileStream.close();
    delete instance;
    instance = nullptr;
}

// ── Init ──────────────────────────────────────────────────────

void Logger::init(
    bool toConsole,
    bool toFile,
    const std::string& filename,
    LogLevel minLevel
) {
    if (instance) return; // Ya inicializado
    instance = new Logger(toConsole, toFile, filename, minLevel);
    instance->writeEntry(LogLevel::INFO, "Logger inicializado — VousCommandez");
}

// ── API pública ───────────────────────────────────────────────

void Logger::debug(const std::string& msg) {
    log(LogLevel::DEBUG, msg);
}

void Logger::info(const std::string& msg) {
    log(LogLevel::INFO, msg);
}

void Logger::warn(const std::string& msg) {
    log(LogLevel::WARNING, msg);
}

void Logger::error(const std::string& msg) {
    log(LogLevel::ERROR, msg);
}

void Logger::log(LogLevel level, const std::string& msg) {
    if (!instance) {
        // Auto-init con defaults de Config si no se llamó init()
        init(
            Config::LOG_TO_CONSOLE,
            Config::LOG_TO_FILE,
            Config::LOG_FILENAME
        );
    }
    if (level >= instance->minLevel) {
        instance->writeEntry(level, msg);
    }
}

void Logger::setMinLevel(LogLevel level) {
    if (instance) instance->minLevel = level;
}

void Logger::flush() {
    if (instance && instance->fileStream.is_open()) {
        instance->fileStream.flush();
    }
}

// ── Internos ──────────────────────────────────────────────────

void Logger::writeEntry(
    LogLevel level,
    const std::string& message
) {
    std::string entry = "[" + timestamp() + "] "
                      + levelTag(level)  + " "
                      + message;

    if (toConsole) {
        // Color por nivel en consola (ANSI)
        switch (level) {
            case LogLevel::DEBUG:   std::cout << "\033[37m"; break; // gris
            case LogLevel::INFO:    std::cout << "\033[32m"; break; // verde
            case LogLevel::WARNING: std::cout << "\033[33m"; break; // amarillo
            case LogLevel::ERROR:   std::cout << "\033[31m"; break; // rojo
        }
        std::cout << entry << "\033[0m" << "\n";
    }

    if (toFile && fileStream.is_open()) {
        fileStream << entry << "\n";
    }
}

std::string Logger::levelTag(LogLevel level) {
    switch (level) {
        case LogLevel::DEBUG:   return "[DEBUG  ]";
        case LogLevel::INFO:    return "[INFO   ]";
        case LogLevel::WARNING: return "[WARNING]";
        case LogLevel::ERROR:   return "[ERROR  ]";
        default:                return "[?      ]";
    }
}

std::string Logger::timestamp() {
    auto now  = std::chrono::system_clock::now();
    auto time = std::chrono::system_clock::to_time_t(now);
    std::ostringstream ss;
    ss << std::put_time(std::localtime(&time), "%H:%M:%S");
    return ss.str();
}