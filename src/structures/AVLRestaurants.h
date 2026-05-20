#pragma once

#include <string>
#include <unordered_map>
#include <vector>

// Nodo interno del AVL.
// Solo guarda restaurantId + score para mantener el árbol liviano.
// Los datos completos del restaurante viven en el Simulator/Registry.
struct AVLNode {
    std::string restaurantId;
    double      score;      // popularityScore de Restaurant

    int height;

    AVLNode* left;
    AVLNode* right;

    AVLNode(const std::string& id, double score)
        : restaurantId(id),
          score(score),
          height(1),
          left(nullptr),
          right(nullptr) {}
};

// AVL ordenado por popularityScore (clave de comparación).
// Permite operaciones O(log n):
//   - Insertar restaurante
//   - Actualizar score (remove + insert)
//   - Buscar por id
//   - Recorrido inorder (restaurantes ordenados por popularidad)
//
// Justificación académica: mantiene restaurantes ordenados
// dinámicamente por métricas de demanda, garantizando
// búsqueda y actualización en O(log n) frente a cambios
// continuos durante la simulación.

class AVLRestaurants {
private:
    AVLNode* root;

    // ── Utilidades internas ───────────────────────────────────
    int      height(AVLNode* n)      const;
    int      balanceFactor(AVLNode* n) const;
    void     updateHeight(AVLNode* n);

    AVLNode* rotateRight(AVLNode* y);
    AVLNode* rotateLeft(AVLNode* x);
    AVLNode* balance(AVLNode* n);

    AVLNode* insert(AVLNode* node,
                    const std::string& id,
                    double score);

    AVLNode* remove(AVLNode* node,
                    const std::string& id,
                    double score);

    AVLNode* minNode(AVLNode* node) const;

    AVLNode* findById(AVLNode* node,
                      const std::string& id) const;

    void inorder(AVLNode* node,
                 std::vector<std::string>& out) const;

    void destroy(AVLNode* node);

public:
    AVLRestaurants();
    ~AVLRestaurants();

    // No permitir copia (árbol con punteros propios)
    AVLRestaurants(const AVLRestaurants&)            = delete;
    AVLRestaurants& operator=(const AVLRestaurants&) = delete;

    // ── API pública ───────────────────────────────────────────

    // Inserta un restaurante con su score inicial.
    void insert(const std::string& restaurantId, double score);

    // Actualiza el score de un restaurante ya insertado.
    // Internamente hace remove + insert para mantener el orden.
    void updateScore(const std::string& restaurantId,
                     double newScore);

    // Elimina un restaurante del árbol.
    void remove(const std::string& restaurantId, double score);

    // Devuelve true si el restaurante está en el árbol.
    bool contains(const std::string& restaurantId) const;

    // Devuelve los IDs ordenados de menor a mayor score.
    // Inorder traversal O(n).
    std::vector<std::string> getSortedByScore() const;

    // Devuelve los N restaurantes con mayor score.
    std::vector<std::string> getTopN(int n) const;

    int  size()    const;
    bool isEmpty() const;

private:
    int nodeCount; // contador manual para size() O(1)

    // Score actual por id — necesario para updateScore y remove por id
    // sin tener que buscarlo en el árbol
    std::unordered_map<std::string, double> scoreIndex;
};