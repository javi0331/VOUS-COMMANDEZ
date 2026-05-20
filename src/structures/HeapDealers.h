#pragma once

#include <vector>
#include <string>
#include "../models/Dealer.h"

// Max-heap de repartidores ordenado por logisticScore.
// El repartidor con mayor score (mejor candidato logístico)
// siempre está en la cima.
//
// El Simulator calcula el score externamente:
//   score = α·(1/distancia) + β·(1/tiempoEstimado) + γ·(1 - loadFactor)
// y llama a updateScore() antes de pop().
//
// Internamente usa un vector con heapify manual para
// cumplir el requisito académico de estructura propia.

struct DealerEntry {
    std::string dealerId;
    double      score;

    // Comparador para el heap (mayor score = mayor prioridad)
    bool operator<(const DealerEntry& other) const {
        return score < other.score;
    }
};

class HeapDealers {
private:
    std::vector<DealerEntry> heap;

    // ── Operaciones internas ──────────────────────────────────
    void siftUp(int index);
    void siftDown(int index);

    int parent(int i) const { return (i - 1) / 2; }
    int left(int i)   const { return 2 * i + 1; }
    int right(int i)  const { return 2 * i + 2; }

public:
    HeapDealers() = default;

    // ── Inserción / Extracción ────────────────────────────────

    // Inserta un repartidor con su score inicial.
    void push(const std::string& dealerId, double score);

    // Devuelve y elimina el repartidor con mayor score.
    // Precondición: !isEmpty()
    DealerEntry pop();

    // Devuelve el mejor repartidor sin eliminarlo.
    // Precondición: !isEmpty()
    const DealerEntry& top() const;

    // ── Actualización de score ────────────────────────────────

    // Actualiza el score de un repartidor ya en el heap
    // y reordena. O(log n).
    // No hace nada si el id no existe.
    void updateScore(const std::string& dealerId, double newScore);

    // Elimina un repartidor del heap por id. O(n).
    // Usado cuando un repartidor se desconecta o queda fuera de zona.
    void remove(const std::string& dealerId);

    // ── Consultas ─────────────────────────────────────────────
    bool   isEmpty()              const;
    int    size()                 const;
    bool   contains(const std::string& dealerId) const;
    double getScore(const std::string& dealerId) const; // -1 si no existe

    // Reconstruye el heap completo desde cero. O(n).
    // Útil si se actualizan muchos scores a la vez.
    void rebuild();
};