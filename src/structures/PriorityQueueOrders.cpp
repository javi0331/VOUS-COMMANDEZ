#include "../structures/PriorityQueueOrders.h"
#include <stdexcept>
#include <algorithm>

// ── Operaciones internas ──────────────────────────────────────

void PriorityQueueOrders::siftUp(int i) {
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

void PriorityQueueOrders::siftDown(int i) {
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

void PriorityQueueOrders::push(
    const std::string& orderId,
    int priority,
    double createdAt
) {
    heap.push_back({orderId, priority, createdAt});
    siftUp(static_cast<int>(heap.size()) - 1);
}

OrderEntry PriorityQueueOrders::pop() {
    if (isEmpty()) {
        throw std::runtime_error(
            "PriorityQueueOrders::pop — cola vacía"
        );
    }

    OrderEntry best = heap[0];
    heap[0] = heap.back();
    heap.pop_back();

    if (!isEmpty()) siftDown(0);

    return best;
}

const OrderEntry& PriorityQueueOrders::top() const {
    if (isEmpty()) {
        throw std::runtime_error(
            "PriorityQueueOrders::top — cola vacía"
        );
    }
    return heap[0];
}

// ── Actualización ─────────────────────────────────────────────

void PriorityQueueOrders::updatePriority(
    const std::string& orderId,
    int newPriority
) {
    for (int i = 0; i < static_cast<int>(heap.size()); i++) {
        if (heap[i].orderId == orderId) {
            int old = heap[i].priority;
            heap[i].priority = newPriority;

            if (newPriority > old) {
                siftUp(i);
            } else {
                siftDown(i);
            }
            return;
        }
    }
}

void PriorityQueueOrders::remove(const std::string& orderId) {
    for (int i = 0; i < static_cast<int>(heap.size()); i++) {
        if (heap[i].orderId == orderId) {
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

bool PriorityQueueOrders::isEmpty() const {
    return heap.empty();
}

int PriorityQueueOrders::size() const {
    return static_cast<int>(heap.size());
}

bool PriorityQueueOrders::contains(
    const std::string& orderId
) const {
    for (const auto& entry : heap) {
        if (entry.orderId == orderId) return true;
    }
    return false;
}

// Boost automático — llamar desde Simulator::tick()
void PriorityQueueOrders::boostStaleOrders(
    double currentTime,
    double thresholdMinutes,
    int    boostAmount
) {
    bool anyChanged = false;

    for (auto& entry : heap) {
        double waiting = currentTime - entry.createdAt;
        if (waiting >= thresholdMinutes) {
            entry.priority += boostAmount;
            anyChanged = true;
        }
    }

    // Reconstruye el heap si algo cambió — O(n)
    if (anyChanged) {
        int n = static_cast<int>(heap.size());
        for (int i = n / 2 - 1; i >= 0; i--) {
            siftDown(i);
        }
    }
}