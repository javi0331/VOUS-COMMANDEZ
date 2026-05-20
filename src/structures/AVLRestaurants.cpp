#include "../structures/AVLRestaurants.h"
#include <vector>
#include <unordered_map>
#include <algorithm>

// ── Constructor / Destructor ──────────────────────────────────

AVLRestaurants::AVLRestaurants()
    : root(nullptr), nodeCount(0) {}

AVLRestaurants::~AVLRestaurants() {
    destroy(root);
}

void AVLRestaurants::destroy(AVLNode* node) {
    if (!node) return;
    destroy(node->left);
    destroy(node->right);
    delete node;
}

// ── Utilidades internas ───────────────────────────────────────

int AVLRestaurants::height(AVLNode* n) const {
    return n ? n->height : 0;
}

int AVLRestaurants::balanceFactor(AVLNode* n) const {
    return n ? height(n->left) - height(n->right) : 0;
}

void AVLRestaurants::updateHeight(AVLNode* n) {
    if (n) {
        n->height = 1 + std::max(
            height(n->left),
            height(n->right)
        );
    }
}

AVLNode* AVLRestaurants::rotateRight(AVLNode* y) {
    AVLNode* x  = y->left;
    AVLNode* T2 = x->right;

    x->right = y;
    y->left  = T2;

    updateHeight(y);
    updateHeight(x);

    return x;
}

AVLNode* AVLRestaurants::rotateLeft(AVLNode* x) {
    AVLNode* y  = x->right;
    AVLNode* T2 = y->left;

    y->left  = x;
    x->right = T2;

    updateHeight(x);
    updateHeight(y);

    return y;
}

AVLNode* AVLRestaurants::balance(AVLNode* n) {
    updateHeight(n);
    int bf = balanceFactor(n);

    // Izquierda-Izquierda
    if (bf > 1 && balanceFactor(n->left) >= 0) {
        return rotateRight(n);
    }
    // Izquierda-Derecha
    if (bf > 1 && balanceFactor(n->left) < 0) {
        n->left = rotateLeft(n->left);
        return rotateRight(n);
    }
    // Derecha-Derecha
    if (bf < -1 && balanceFactor(n->right) <= 0) {
        return rotateLeft(n);
    }
    // Derecha-Izquierda
    if (bf < -1 && balanceFactor(n->right) > 0) {
        n->right = rotateRight(n->right);
        return rotateLeft(n);
    }

    return n;
}

// Inserción por score. Desempate por id (lexicográfico)
// para que el árbol sea determinista con scores iguales.
AVLNode* AVLRestaurants::insert(
    AVLNode* node,
    const std::string& id,
    double score
) {
    if (!node) return new AVLNode(id, score);

    if (score < node->score ||
       (score == node->score && id < node->restaurantId)) {
        node->left  = insert(node->left,  id, score);
    } else {
        node->right = insert(node->right, id, score);
    }

    return balance(node);
}

AVLNode* AVLRestaurants::minNode(AVLNode* node) const {
    while (node->left) node = node->left;
    return node;
}

AVLNode* AVLRestaurants::remove(
    AVLNode* node,
    const std::string& id,
    double score
) {
    if (!node) return nullptr;

    if (score < node->score ||
       (score == node->score && id < node->restaurantId)) {
        node->left  = remove(node->left,  id, score);
    } else if (score > node->score ||
              (score == node->score && id > node->restaurantId)) {
        node->right = remove(node->right, id, score);
    } else {
        // Nodo encontrado
        if (!node->left || !node->right) {
            AVLNode* child = node->left ? node->left : node->right;
            delete node;
            return child;
        }
        // Dos hijos: reemplaza con el mínimo del subárbol derecho
        AVLNode* successor = minNode(node->right);
        node->restaurantId = successor->restaurantId;
        node->score        = successor->score;
        node->right = remove(
            node->right,
            successor->restaurantId,
            successor->score
        );
    }

    return balance(node);
}

AVLNode* AVLRestaurants::findById(
    AVLNode* node,
    const std::string& id
) const {
    if (!node) return nullptr;
    if (node->restaurantId == id) return node;

    AVLNode* left  = findById(node->left,  id);
    if (left) return left;

    return findById(node->right, id);
}

void AVLRestaurants::inorder(
    AVLNode* node,
    std::vector<std::string>& out
) const {
    if (!node) return;
    inorder(node->left,  out);
    out.push_back(node->restaurantId);
    inorder(node->right, out);
}

// ── API pública ───────────────────────────────────────────────

void AVLRestaurants::insert(
    const std::string& restaurantId,
    double score
) {
    root = insert(root, restaurantId, score);
    scoreIndex[restaurantId] = score;
    nodeCount++;
}

void AVLRestaurants::updateScore(
    const std::string& restaurantId,
    double newScore
) {
    auto it = scoreIndex.find(restaurantId);
    if (it == scoreIndex.end()) return;

    double oldScore = it->second;
    root = remove(root, restaurantId, oldScore);
    root = insert(root, restaurantId, newScore);
    scoreIndex[restaurantId] = newScore;
    // nodeCount no cambia
}

void AVLRestaurants::remove(
    const std::string& restaurantId,
    double score
) {
    if (!contains(restaurantId)) return;
    root = remove(root, restaurantId, score);
    scoreIndex.erase(restaurantId);
    nodeCount--;
}

bool AVLRestaurants::contains(
    const std::string& restaurantId
) const {
    return scoreIndex.find(restaurantId) != scoreIndex.end();
}

std::vector<std::string> AVLRestaurants::getSortedByScore() const {
    std::vector<std::string> result;
    inorder(root, result);
    return result;
}

std::vector<std::string> AVLRestaurants::getTopN(int n) const {
    std::vector<std::string> all = getSortedByScore();
    // Inorder da menor→mayor, invertimos para mayor→menor
    std::reverse(all.begin(), all.end());
    if (n < static_cast<int>(all.size())) {
        all.resize(n);
    }
    return all;
}

int AVLRestaurants::size() const {
    return nodeCount;
}

bool AVLRestaurants::isEmpty() const {
    return nodeCount == 0;
}