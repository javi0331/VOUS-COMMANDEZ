#pragma once

#include <vector>
#include <string>
#include <random>
#include <stdexcept>
#include <algorithm>

// Wrapper sobre <random> de C++11.
// Usa Mersenne Twister (mt19937) — mucho mejor que rand().
//
// Seed fija → resultados reproducibles (útil para debug y demos).
// Seed aleatoria → comportamiento distinto cada ejecución.
//
// Uso:
//   Random::seed(42);
//   int n       = Random::uniformInt(1, 10);
//   double x    = Random::uniformReal(0.0, 1.0);
//   double g    = Random::normal(5.0, 1.5);
//   int idx     = Random::weighted({0.5, 0.3, 0.2});
//   bool evento = Random::chance(0.15);  // 15% de probabilidad

class Random {
private:
    static std::mt19937 engine;
    static bool seeded;

    // Inicializa con seed aleatoria si no se llamó seed() antes
    static void ensureSeeded();

public:
    Random() = delete;

    // ── Seed ──────────────────────────────────────────────────

    // Seed fija para reproducibilidad
    static void seed(unsigned int s);

    // Seed aleatoria basada en hardware
    static void seedRandom();

    // ── Distribución uniforme ─────────────────────────────────

    // Entero en [min, max] (ambos inclusive)
    static int    uniformInt(int min, int max);

    // Real en [min, max)
    static double uniformReal(double min, double max);

    // ── Distribución normal (Gaussiana) ───────────────────────
    // Útil para tiempos de preparación, ratings, etc.
    // Resultado puede ser negativo — clampear si hace falta.
    static double normal(double mean, double stddev);

    // Normal acotada: rechaza valores fuera de [min, max]
    // (máximo 100 intentos, luego devuelve mean)
    static double normalClamped(
        double mean,
        double stddev,
        double min,
        double max
    );

    // ── Distribución ponderada ────────────────────────────────
    // Recibe vector de pesos (no necesitan sumar 1).
    // Devuelve el índice del elemento seleccionado.
    // Ejemplo: weighted({3.0, 1.0, 1.0}) → 60% idx 0, 20% idx 1, 20% idx 2
    static int weighted(const std::vector<double>& weights);

    // Versión con IDs: devuelve el string correspondiente al índice
    static const std::string& weightedPick(
        const std::vector<std::string>& items,
        const std::vector<double>&      weights
    );

    // ── Bernoulli ─────────────────────────────────────────────
    // Devuelve true con probabilidad p (0.0–1.0)
    static bool chance(double probability);

    // ── Shuffle ───────────────────────────────────────────────
    template<typename T>
    static void shuffle(std::vector<T>& vec) {
        ensureSeeded();
        std::shuffle(vec.begin(), vec.end(), engine);
    }

    // ── Elemento aleatorio de un vector ──────────────────────
    template<typename T>
    static const T& pick(const std::vector<T>& vec) {
        if (vec.empty()) {
            throw std::runtime_error("Random::pick — vector vacío");
        }
        return vec[uniformInt(0, static_cast<int>(vec.size()) - 1)];
    }
};