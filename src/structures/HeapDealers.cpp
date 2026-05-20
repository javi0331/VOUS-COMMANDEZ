#include "../structures/HeapDealers.h"
#include <stdexcept>
#include <algorithm>

// ── Operaciones internas ──────────────────────────────────────

void HeapDealers::siftUp(int i) {
    while (i > 0) {
        int p = parent(i);
        if (heap[p] < heap[i]) {
            std::swap(heap[p], heap[i]);
            i = p;
        } else {
            break;
        }
    }
}

void HeapDealers::siftDown(int i) {
    int n = static_cast<int>(heap.size());

    while (true) {
        int largest = i;
        int l = left(i);
        int r = right(i);

        if (l < n && heap[largest] < heap[l]) largest = l;
        if (r < n && heap[largest] < heap[r]) largest = r;

        if (largest == i) break;

        std::swap(heap[i], heap[largest]);
        i = largest;
    }
}

// ── Inserción / Extracción ────────────────────────────────────

void HeapDealers::push(
    const std::string& dealerId,
    double score
) {
    heap.push_back({dealerId, score});
    siftUp(static_cast<int>(heap.size()) - 1);
}

DealerEntry HeapDealers::pop() {
    if (isEmpty()) {
        throw std::runtime_error(
            "HeapDealers::pop — heap vacío"
        );
    }

    DealerEntry best = heap[0];

    // Mueve el último al tope y reordena
    heap[0] = heap.back();
    heap.pop_back();

    if (!isEmpty()) siftDown(0);

    return best;
}

const DealerEntry& HeapDealers::top() const {
    if (isEmpty()) {
        throw std::runtime_error(
            "HeapDealers::top — heap vacío"
        );
    }
    return heap[0];
}

// ── Actualización de score ────────────────────────────────────

void HeapDealers::updateScore(
    const std::string& dealerId,
    double newScore
) {
    for (int i = 0; i < static_cast<int>(heap.size()); i++) {
        if (heap[i].dealerId == dealerId) {
            double old = heap[i].score;
            heap[i].score = newScore;

            // Si subió el score sube en el heap, si bajó baja
            if (newScore > old) {
                siftUp(i);
            } else {
                siftDown(i);
            }
            return;
        }
    }
}

void HeapDealers::remove(const std::string& dealerId) {
    for (int i = 0; i < static_cast<int>(heap.size()); i++) {
        if (heap[i].dealerId == dealerId) {
            // Reemplaza con el último y reordena
            heap[i] = heap.back();
            heap.pop_back();

            if (i < static_cast<int>(heap.size())) {
                siftUp(i);
                siftDown(i);
            }
            return;
        }
    }
}

// ── Consultas ─────────────────────────────────────────────────

bool HeapDealers::isEmpty() const {
    return heap.empty();
}

int HeapDealers::size() const {
    return static_cast<int>(heap.size());
}

bool HeapDealers::contains(const std::string& dealerId) const {
    for (const auto& entry : heap) {
        if (entry.dealerId == dealerId) return true;
    }
    return false;
}

double HeapDealers::getScore(const std::string& dealerId) const {
    for (const auto& entry : heap) {
        if (entry.dealerId == dealerId) return entry.score;
    }
    return -1.0;
}

void HeapDealers::rebuild() {
    int n = static_cast<int>(heap.size());
    // Heapify desde abajo — O(n)
    for (int i = n / 2 - 1; i >= 0; i--) {
        siftDown(i);
    }
}