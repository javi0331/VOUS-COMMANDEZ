#include "../utils/Random.h"
#include <numeric>

// ── Inicialización de estáticos ───────────────────────────────
std::mt19937 Random::engine;
bool         Random::seeded = false;

void Random::ensureSeeded() {
    if (!seeded) seedRandom();
}

// ── Seed ──────────────────────────────────────────────────────

void Random::seed(unsigned int s) {
    engine.seed(s);
    seeded = true;
}

void Random::seedRandom() {
    std::random_device rd;
    engine.seed(rd());
    seeded = true;
}

// ── Distribución uniforme ─────────────────────────────────────

int Random::uniformInt(int min, int max) {
    ensureSeeded();
    std::uniform_int_distribution<int> dist(min, max);
    return dist(engine);
}

double Random::uniformReal(double min, double max) {
    ensureSeeded();
    std::uniform_real_distribution<double> dist(min, max);
    return dist(engine);
}

// ── Distribución normal ───────────────────────────────────────

double Random::normal(double mean, double stddev) {
    ensureSeeded();
    std::normal_distribution<double> dist(mean, stddev);
    return dist(engine);
}

double Random::normalClamped(
    double mean,
    double stddev,
    double min,
    double max
) {
    ensureSeeded();
    std::normal_distribution<double> dist(mean, stddev);

    for (int i = 0; i < 100; i++) {
        double val = dist(engine);
        if (val >= min && val <= max) return val;
    }
    return mean; // fallback si no converge
}

// ── Distribución ponderada ────────────────────────────────────

int Random::weighted(const std::vector<double>& weights) {
    if (weights.empty()) {
        throw std::runtime_error("Random::weighted — vector vacío");
    }

    ensureSeeded();
    std::discrete_distribution<int> dist(
        weights.begin(),
        weights.end()
    );
    return dist(engine);
}

const std::string& Random::weightedPick(
    const std::vector<std::string>& items,
    const std::vector<double>&      weights
) {
    if (items.empty() || items.size() != weights.size()) {
        throw std::runtime_error(
            "Random::weightedPick — items y weights deben tener igual tamaño"
        );
    }
    int idx = weighted(weights);
    return items[idx];
}

// ── Bernoulli ─────────────────────────────────────────────────

bool Random::chance(double probability) {
    ensureSeeded();
    std::bernoulli_distribution dist(probability);
    return dist(engine);
}