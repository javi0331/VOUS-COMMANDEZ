# VousCommandez — Documentación Técnica Completa

> Proyecto de Ciencias de la Computación I  
> Universidad Distrital Francisco José de Caldas  
> Ingeniería de Sistemas — Grupo 81  
> Semestre 2026-1

---

## Tabla de contenidos

1. [Descripción general](#1-descripción-general)
2. [Arquitectura del sistema](#2-arquitectura-del-sistema)
3. [Estructura del proyecto](#3-estructura-del-proyecto)
4. [Grafo de Bogotá](#4-grafo-de-bogotá)
5. [Flujo completo de una orden](#5-flujo-completo-de-una-orden)
6. [Estructuras de datos implementadas](#6-estructuras-de-datos-implementadas)
7. [Algoritmos de pathfinding](#7-algoritmos-de-pathfinding)
8. [Módulos de simulación](#8-módulos-de-simulación)
9. [Módulo visual (SFML)](#9-módulo-visual-sfml)
10. [Modelos de datos](#10-modelos-de-datos)
11. [Utilidades](#11-utilidades)
12. [Parámetros de configuración (Config.h)](#12-parámetros-de-configuración-configh)
13. [Compilación y ejecución](#13-compilación-y-ejecución)
14. [Glosario](#14-glosario)

---

## 1. Descripción general

**VousCommandez** es una simulación de un servicio de domicilios estilo Rappi sobre un grafo real de Bogotá. El programa modela repartidores moviéndose por la ciudad, recibiendo pedidos de restaurantes y entregándolos a clientes, todo en **tiempo simulado acelerado** con visualización en tiempo real mediante SFML.

### Características principales

| Característica | Descripción |
|---|---|
| Pathfinding en tiempo real | Dijkstra con multiplicadores de tráfico dinámico por arista |
| Bellman-Ford alternativo | Para comparación académica y verificación de resultados |
| Tráfico dinámico | Factor = clima × hora del día × evento de tráfico |
| Generador procedural | Tasa de pedidos varía por hora simulada (almuerzo, noche, madrugada) |
| Estructuras propias | Heap, Cola de prioridad, AVL y deque implementados desde cero |
| Historial de entregas | Registro con tiempos de espera, entrega y distancia |
| Visualización SFML | Render en tiempo real, animaciones interpoladas, panel de estadísticas |

### Grafo del mapa

- **83 nodos**: barrios y restaurantes de Bogotá agrupados en 7 zonas
- **~162 aristas**: conexiones bidireccionales con peso en minutos
- **25 restaurantes** distribuidos por toda la ciudad
- **3 repartidores** iniciales, uno por zona (Norte, Centro, Sur)

---

## 2. Arquitectura del sistema

```
Main.cpp
  ├── Logger::init()          — inicializa el sistema de logging
  ├── Random::seedRandom()    — semilla aleatoria para reproducibilidad
  ├── GraphLoader::loadFromCSV() — carga el grafo desde nodes.csv y edges.csv
  ├── Simulator               — orquesta todos los subsistemas
  │     ├── TimeSystem        — reloj simulado acelerado
  │     ├── WeatherSystem     — estado del clima (Clear/Rain/Storm)
  │     ├── TrafficSystem     — multiplica factores: clima × hora × evento
  │     ├── EventSystem       — dispara eventos programados (partidos, lluvia)
  │     ├── OrderGenerator    — genera órdenes proceduralmente por hora del día
  │     ├── PriorityQueueOrders — cola de prioridad de órdenes pendientes
  │     ├── HeapDealers       — max-heap de repartidores por score logístico
  │     ├── AVLRestaurants    — árbol AVL de popularidad de restaurantes
  │     ├── Dijkstra          — calcula rutas óptimas dealer→restaurante→cliente
  │     └── DeliveryHistory   — registra entregas completadas en deque
  └── RendererSFML            — loop de render SFML a 60 FPS
        ├── Camera            — zoom, paneo, transformaciones mundo↔pantalla
        ├── Animations        — interpola posición de dealers entre waypoints
        └── UI                — panel lateral, HUD, botones, historial
```

### Flujo por frame (60 FPS)

```
handleEvents()  → procesa input del usuario (clicks, scroll, teclado)
update(dt)      → simulator.tick(dt) + syncAnimations() + animations.update()
render()        → drawGraph() + drawOrderMarkers() + drawRoutes() + drawDealers() + UI.draw()
```

---

## 3. Estructura del proyecto

```
C1_VOUSCOMMANDEZ/
├── src/
│   ├── algorithms/
│   │   ├── Dijkstra.h / Dijkstra.cpp          — pathfinding principal O((V+E)logV)
│   │   ├── BellmanFord.h / BellmanFord.cpp    — pathfinding alternativo O(V·E)
│   │   └── RouteResult.h                      — resultado de ambos algoritmos
│   │
│   ├── core/
│   │   ├── Graph.h / Graph.cpp                — grafo con lista de adyacencia
│   │   ├── Node.h / Node.cpp                  — nodo del grafo (barrio o restaurante)
│   │   ├── Edge.h / Edge.cpp                  — arista con distancia y tiempo base
│   │   ├── GraphLoader.h / GraphLoader.cpp    — parseo de CSV a objetos Graph
│   │   ├── Path.h                             — lista de nodos + segmentos de ruta
│   │   ├── RouteSegment.h                     — segmento individual (from→to)
│   │   ├── TrafficEdgeData.h                  — multiplicadores de tráfico por arista
│   │   ├── Coords.h                           — coordenadas x/y para visualización
│   │   ├── NodeType.h                         — enum: BARRIO, RESTAURANTE, etc.
│   │   ├── Zone.h                             — enum: NORTE, CENTRO, SUR, etc.
│   │   └── Types.h                            — enum RoadType: CALLE, AVENIDA, etc.
│   │
│   ├── models/
│   │   ├── Order.h / Order.cpp                — orden/pedido con ciclo de vida
│   │   ├── Dealer.h / Dealer.cpp              — repartidor con estado y métricas
│   │   ├── Restaurant.h / Restaurant.cpp      — restaurante con score dinámico
│   │   ├── Client.h / Client.cpp              — cliente con historial de pedidos
│   │   ├── OrderStatus.h                      — enum de estados de una orden
│   │   └── DealerStatus.h                     — enum de estados de un dealer
│   │
│   ├── simulation/
│   │   ├── Simulator.h / Simulator.cpp        — orquestador principal (tick por frame)
│   │   ├── TimeSystem.h / TimeSystem.cpp      — reloj simulado acelerado
│   │   ├── WeatherSystem.h / WeatherSystem.cpp — clima con transiciones
│   │   ├── TrafficSystem.h / TrafficSystem.cpp — multiplicador total de tráfico
│   │   ├── EventSystem.h / EventSystem.cpp    — eventos programados (partidos, lluvia)
│   │   └── OrderGenerator.h / OrderGenerator.cpp — spawn procedural de órdenes
│   │
│   ├── structures/
│   │   ├── HeapDealers.h / HeapDealers.cpp    — max-heap de repartidores
│   │   ├── PriorityQueueOrders.h / .cpp       — max-heap de órdenes pendientes
│   │   ├── AVLRestaurants.h / AVLRestaurants.cpp — árbol AVL por popularidad
│   │   └── DeliveryHistory.h / DeliveryHistory.cpp — deque de entregas completadas
│   │
│   ├── utils/
│   │   ├── Config.h                           — todas las constantes del proyecto
│   │   ├── Logger.h / Logger.cpp              — logger singleton con colores ANSI
│   │   └── Random.h / Random.cpp              — wrapper de <random> (Mersenne Twister)
│   │
│   ├── visual/
│   │   ├── RendererSFML.h / RendererSFML.cpp  — loop principal de render
│   │   ├── Camera.h / Camera.cpp              — zoom, paneo, transformaciones
│   │   ├── Animations.h / Animations.cpp      — interpolación de posición de dealers
│   │   └── UI.h / UI.cpp                      — HUD, panel lateral, botones
│   │
│   └── Main.cpp                               — punto de entrada
│
├── build/
│   ├── VousCommandez.exe                      — ejecutable generado
│   ├── assets/
│   │   ├── fonts/Roboto-Regular.ttf
│   │   └── info.html                          — documentación visual interactiva
│   └── data/
│       ├── nodes.csv                          — 83 nodos del grafo
│       └── edges.csv                          — ~162 aristas del grafo
│
└── .vscode/
    ├── tasks.json                             — tarea de build con g++
    └── launch.json                            — configuración de debug con gdb
```

---

## 4. Grafo de Bogotá

### Zonas y nodos

| Zona | Prefijo | Color en pantalla | Barrios representados |
|------|---------|-------------------|-----------------------|
| Norte | N1–N10 | Azul | Usaquén, Cedritos, Santa Bárbara, Suba, etc. |
| Centro-Norte | C1–C13 | Morado | Chapinero, Zona T, Chicó, Rosales, etc. |
| Occidente | O1–O8 | Naranja | Engativá, Fontibón, Salitre, etc. |
| Sur-Occidente | S1–S8 | Rojo | Kennedy, Bosa, Corabastos, etc. |
| Centro | M1–M7 | Verde | Santa Fe, La Candelaria, San Victorino, etc. |
| Sur | SU1–SU7 | Rojo oscuro | Restrepo, San Cristóbal, Ciudad Bolívar, etc. |
| Restaurantes | R01–R25 | Amarillo | Distribuidos por todas las zonas |

### Archivos CSV

**`build/data/nodes.csv`** — formato:
```
id,name,zone,type,x,y
N1,Usaquén,NORTE,BARRIO,390,55
R01,McDonald's Zona T,CENTRO_NORTE,RESTAURANT,333,128
```

**`build/data/edges.csv`** — formato:
```
source,target,weight,bidirectional
N1,N2,10,1        ← peso = minutos de viaje, 1 = doble sentido
```

### Cálculo de tiempos de arista

En `GraphLoader.cpp`:
```cpp
double baseTime   = weight;                              // ya en minutos
double distanceKm = (weight / 60.0) * DEALER_SPEED_KMH; // km derivados
```

El peso del edge es el **tiempo base en minutos** bajo condiciones normales (sin tráfico).

### Factores de tráfico

El peso efectivo que Dijkstra usa por arista es:
```
peso_efectivo = baseTime × factor_clima × factor_hora × factor_evento
```

| Condición | Factor |
|-----------|--------|
| Despejado | ×1.0 |
| Lluvia | ×1.3 |
| Tormenta | ×1.6 |
| Madrugada (0–5h) | ×0.8 |
| Hora normal | ×1.0 |
| Hora pico (7–9h, 17–20h) | ×1.5 |
| Sin evento | ×1.0 |
| Marcha | ×1.2 |
| Concierto / Partido | ×1.4 |

---

## 5. Flujo completo de una orden

### Diagrama de estados

```
[GENERADA]
    │
    ▼
[PENDING] ──── OrderGenerator genera la orden
    │           PriorityQueueOrders.push()
    ▼
[ASSIGNED] ─── Dijkstra calcula ruta dealer → restaurante
    │           HeapDealers.pop() selecciona el mejor dealer
    ▼
[PICKING_UP] ─ Dealer llega al restaurante
    │           Espera DEALER_PICKUP_TIME (3 min simulados)
    ▼
[IN_TRANSIT] ─ Dijkstra calcula ruta restaurante → cliente
    │           Dealer se mueve al nodo del cliente
    ▼
[DELIVERED] ── DeliveryHistory.record() guarda el registro
               AVLRestaurants actualiza popularityScore
               Stats actualizadas
```

### Máquina de estados del dealer

Cada dealer tiene una `DealerPhaseState` con un `phaseTimer`:

```
DeliveryPhase::HEADING_TO_RESTAURANT
    phaseTimer = tiempo Dijkstra(dealerNode → restaurantNode)
    Cuando timer llega a 0 → transición a PICKING_UP

DeliveryPhase::PICKING_UP
    phaseTimer = Config::DEALER_PICKUP_TIME (3 min)
    Dealer estático en nodo del restaurante
    Cuando timer llega a 0 → transición a DELIVERING

DeliveryPhase::DELIVERING
    phaseTimer = tiempo Dijkstra(restaurantNode → clientNode)
    Cuando timer llega a 0 → DELIVERED
```

### Score logístico para asignación de dealers

```
score = 0.6 × (1 / distancia_al_restaurante) + 0.4 × (1 − carga_actual)
```

- El dealer **más cercano al restaurante** y con **menos carga** obtiene mayor score.
- Se recalcula el heap completo para cada restaurante antes de hacer `pop()`.

### Anti-starvation de órdenes

Cada `Config::ORDER_BOOST_INTERVAL` (5 min simulados), las órdenes que llevan más de `Config::ORDER_STALE_MINUTES` (10 min) esperando reciben +`Config::ORDER_BOOST_AMOUNT` (2) de prioridad automáticamente.

---

## 6. Estructuras de datos implementadas

### 6.1 HeapDealers — Max-heap de repartidores

**Archivo:** `src/structures/HeapDealers.h/.cpp`

**Propósito:** Mantener al repartidor con mejor score logístico siempre accesible en O(1), con inserción/extracción en O(log n).

**Implementación:** Vector con heapify manual (`siftUp` + `siftDown`). No usa `std::priority_queue`.

```cpp
struct DealerEntry {
    std::string dealerId;
    double      score;        // mayor score = mejor candidato
    bool operator<(const DealerEntry& other) const {
        return score < other.score;  // max-heap
    }
};
```

| Operación | Complejidad |
|-----------|-------------|
| `push(id, score)` | O(log n) |
| `pop()` | O(log n) |
| `top()` | O(1) |
| `updateScore(id, newScore)` | O(n) búsqueda + O(log n) reorder |
| `remove(id)` | O(n) |
| `rebuild()` | O(n) — heapify de abajo hacia arriba |

**Uso en Simulator:** `rebuildDealerHeap(restaurantNode)` reconstruye el heap completo antes de cada asignación con scores frescos.

---

### 6.2 PriorityQueueOrders — Cola de prioridad de órdenes

**Archivo:** `src/structures/PriorityQueueOrders.h/.cpp`

**Propósito:** Encolar pedidos pendientes ordenados por prioridad (1–5). Las órdenes más urgentes se atienden primero; en empate gana la más antigua.

**Implementación:** Vector con heapify manual, mismo patrón que HeapDealers.

```cpp
struct OrderEntry {
    std::string orderId;
    int         priority;     // 1 (baja) a 5 (alta)
    double      createdAt;    // para desempate: más antigua primero

    bool operator<(const OrderEntry& other) const {
        if (priority != other.priority)
            return priority < other.priority;  // mayor prioridad gana
        return createdAt > other.createdAt;    // más antigua gana en empate
    }
};
```

**Boost anti-starvation:**
```cpp
void boostStaleOrders(double currentTime, double threshold, int boost);
```
Recorre el heap en O(n) y aplica +boost a las órdenes que llevan más de `threshold` minutos sin ser atendidas. Luego reconstruye el heap.

| Operación | Complejidad |
|-----------|-------------|
| `push()` | O(log n) |
| `pop()` | O(log n) |
| `boostStaleOrders()` | O(n) |
| `updatePriority()` | O(n) búsqueda + O(log n) |

---

### 6.3 AVLRestaurants — Árbol AVL de popularidad

**Archivo:** `src/structures/AVLRestaurants.h/.cpp`

**Propósito:** Mantener los restaurantes ordenados dinámicamente por su `popularityScore`, permitiendo actualización eficiente cuando llega o se despacha un pedido.

**Clave de ordenamiento:**
```
popularityScore = 0.7 × (totalOrders / 1000)
                + 0.3 × (averageRating / 5.0)
                - 0.05 × activeOrders
```

**Rotaciones implementadas:** LL, LR, RL, RR — todas en O(1).

**Actualización de score:**
```cpp
void updateScore(const std::string& id, double newScore);
// Internamente: remove(id, oldScore) → insert(id, newScore)
```

| Operación | Complejidad |
|-----------|-------------|
| `insert()` | O(log n) |
| `updateScore()` | O(log n) |
| `remove()` | O(log n) |
| `getSortedByScore()` | O(n) — inorder traversal |
| `getTopN(n)` | O(n) — inorder + reverse + resize |

**Invariante AVL:** Para todo nodo, `|height(izq) - height(der)| ≤ 1`. Se mantiene mediante `balance()` después de cada inserción o eliminación.

**Índice auxiliar:** `scoreIndex` (`unordered_map<string, double>`) permite buscar el score actual de un restaurante en O(1) sin recorrer el árbol — necesario para `updateScore` y `remove`.

---

### 6.4 DeliveryHistory — Historial de entregas

**Archivo:** `src/structures/DeliveryHistory.h/.cpp`

**Propósito:** Registro cronológico de entregas completadas con capacidad máxima configurable.

**Implementación:** `std::deque<DeliveryRecord>` — permite `push_back` y `pop_front` en O(1).

**Cada registro contiene:**
```cpp
struct DeliveryRecord {
    std::string orderId, restaurantId, dealerId, clientNodeId;
    double completedAt;    // tiempo simulado de entrega
    double waitTime;       // createdAt → assignedAt (min simulados)
    double deliveryTime;   // assignedAt → deliveredAt (min simulados)
    double totalDistance;  // km recorridos
    int    priority;
};
```

**Capacidad máxima:** Si `maxCapacity > 0`, al insertar se descarta el registro más antiguo (`pop_front()`). Por defecto `Config::HISTORY_MAX_RECORDS = 1000`.

| Operación | Complejidad |
|-----------|-------------|
| `record()` | O(1) amortizado |
| `latest()` / `oldest()` | O(1) |
| `getLastN(n)` | O(n) |
| `getByDealer(id)` | O(n) |
| `averageDeliveryTime()` | O(n) |

---

## 7. Algoritmos de pathfinding

### 7.1 Dijkstra

**Archivo:** `src/algorithms/Dijkstra.h/.cpp`

**Complejidad:** O((V + E) log V)

**Implementación:**
1. Inicializa todas las distancias en `INF`, origen en 0.
2. Usa `std::priority_queue<pair<double, string>, ..., greater<>>` (min-heap de STL).
3. **Early exit:** cuando se extrae el nodo destino, termina inmediatamente.
4. Ignora aristas bloqueadas (`edge.isBlocked()`).
5. El peso efectivo de cada arista es `baseTime × traffic.finalMultiplier()`.

```cpp
// Peso efectivo usado en Dijkstra y Bellman-Ford
double w = edge.getBaseTime() * traffic.finalMultiplier();
```

**Resultado:** `RouteResult` con `path.nodes[]`, `path.segments[]`, `totalCost`, `totalDistance`, `executionTimeMs`, `visitedNodes`.

**Usos en el sistema:**
- Asignación de órdenes: dealer → restaurante → cliente
- Cálculo del score de dealers (`computeDealerScore`)
- Cache de rutas en `RendererSFML` para visualización

---

### 7.2 Bellman-Ford

**Archivo:** `src/algorithms/BellmanFord.h/.cpp`

**Complejidad:** O(V × E) peor caso, con **early exit** cuando no hay cambios.

**Implementación:**
1. Inicializa todas las distancias en `INF`, origen en 0.
2. Itera V−1 veces relajando todas las aristas del grafo.
3. **Early exit:** si en una iteración no hay cambios, termina antes.
4. **Detección de ciclos negativos:** pasa adicional sobre todas las aristas.
5. Retrocede con `prev[]` para reconstruir el camino.

**Justificación académica:** Soporta pesos negativos (aunque no ocurren en esta simulación con multiplicadores ≥ 0.8). Se usa para comparación de eficiencia vs Dijkstra. `RouteResult` expone `executionTimeMs` y `visitedNodes` para comparar.

---

### 7.3 RouteResult

**Archivo:** `src/algorithms/RouteResult.h`

Estructura de retorno compartida por ambos algoritmos:

```cpp
class RouteResult {
public:
    Path   path;             // nodos + segmentos de la ruta
    bool   success;          // false si no hay camino
    int    visitedNodes;     // nodos procesados (métrica de eficiencia)
    double executionTimeMs;  // tiempo real de cómputo en ms
    std::string algorithmName; // "Dijkstra" o "Bellman-Ford"
};
```

---

## 8. Módulos de simulación

### 8.1 Simulator

**Archivo:** `src/simulation/Simulator.h/.cpp`

El orquestador principal. Su método `tick(realDeltaSeconds)` es llamado cada frame:

```cpp
void Simulator::tick(double realDeltaSeconds) {
    if (!running) return;

    timeSystem.update(realDeltaSeconds);
    double simDelta = realDeltaSeconds * timeSystem.getSimulationSpeed();

    weatherSystem.update(simDelta);
    trafficSystem.update(simDelta);
    eventSystem.update(timeSystem.getSimulatedTime());

    orderGenerator.update(simDelta);
    processNewOrders();   // toma órdenes generadas → pendingOrders heap
    assignOrders();       // asigna órdenes a dealers disponibles
    updateDealers(simDelta); // avanza la máquina de estados de cada dealer
    boostStaleOrders(simDelta); // anti-starvation cada 5 min simulados
    updateStats();        // actualiza SimStats para la UI
}
```

**Estado por dealer:** `DealerPhaseState` almacena la fase actual y el timer restante **sin contaminar el modelo Dealer**. Esto separa la lógica de simulación del modelo de datos.

---

### 8.2 TimeSystem

**Archivo:** `src/simulation/TimeSystem.h/.cpp`

Gestiona el reloj simulado acelerado:

```
simulatedTime += realDeltaSeconds × simulationSpeed
```

Con `simulationSpeed = 2.0`, 1 segundo real = 2 minutos simulados.

El método `getTimeOfDay()` devuelve minutos dentro del día (0–1439), con soporte para `startOffsetMinutes` configurable desde `Config::SIM_START_HOUR`.

| Método | Descripción |
|--------|-------------|
| `getHour()` | Hora actual simulada (0–23) |
| `getMinute()` | Minuto actual simulado (0–59) |
| `getSimulatedTime()` | Minutos totales transcurridos desde el inicio |
| `isPaused()` | true si la simulación está en pausa |

---

### 8.3 WeatherSystem

**Archivo:** `src/simulation/WeatherSystem.h/.cpp`

Tres estados: `CLEAR` (×1.0), `RAIN` (×1.3), `STORM` (×1.6).

Soporta transiciones con timer: `scheduleTransition(to, inMinutes)` cambia el clima progresivamente.

---

### 8.4 TrafficSystem

**Archivo:** `src/simulation/TrafficSystem.h/.cpp`

Ensambla el `TrafficEdgeData` final:

```cpp
TrafficEdgeData TrafficSystem::getCurrentTraffic() const {
    return TrafficEdgeData(
        getWeatherFactor(),   // del WeatherSystem
        getHourFactor(),      // calculado desde TimeSystem.getHour()
        getEventFactor()      // del estado del evento activo
    );
}
```

`TrafficEdgeData.finalMultiplier()` = weatherFactor × hourFactor × eventFactor.

---

### 8.5 EventSystem

**Archivo:** `src/simulation/EventSystem.h/.cpp`

Permite programar eventos con callbacks:
```cpp
eventSystem.scheduleTrafficEvent("partido", 180.0, EventState::MATCH, 120.0);
eventSystem.scheduleWeather("lluvia", 300.0, WeatherState::RAIN);
```

Cada frame, `update(currentSimTime)` dispara los eventos cuyo `triggerTime <= currentSimTime`.

---

### 8.6 OrderGenerator

**Archivo:** `src/simulation/OrderGenerator.h/.cpp`

Genera órdenes proceduralmente según la hora simulada:

| Hora | Factor de demanda |
|------|-------------------|
| 00–06h (madrugada) | ×0.2 |
| 09–11h (mañana) | ×0.8 |
| 12–14h (almuerzo) | ×2.0 |
| 14–18h (tarde) | ×0.7 |
| 19–22h (noche) | ×1.8 |
| 22h+ | ×0.5 |

**Selección de restaurante:** ponderada por `weight`. Inicialmente todos tienen peso 1.0; puede modificarse con `updateRestaurantWeight()`.

**Selección de cliente:** aleatoria uniforme entre todos los `clientNodeIds` registrados.

---

## 9. Módulo visual (SFML)

### 9.1 RendererSFML

**Archivo:** `src/visual/RendererSFML.h/.cpp`

Loop principal de render. Responsabilidades separadas:

| Método | Qué dibuja |
|--------|-----------|
| `drawEdges()` | Aristas del grafo, color por tipo de vía |
| `drawNodes()` | Nodos (barrios y restaurantes), color por zona |
| `drawOrderMarkers()` | Círculo rojo = restaurante activo, verde = destino cliente |
| `drawRoutes()` | Ruta activa del dealer seleccionado (desde `dealerRouteCache`) |
| `drawDealers()` | Círculos animados con color según estado |

**Cache de rutas:** `dealerRouteCache` almacena la `RouteResult` de cada dealer. Solo se recalcula cuando cambia el `orderId` activo — **Dijkstra no se llama cada frame**.

**Sincronización visual:** `syncAnimations()` detecta cambios de orden y reinicia las animaciones con la duración correcta para que el dealer llegue al destino exactamente cuando la simulación marca la entrega.

---

### 9.2 Camera

**Archivo:** `src/visual/Camera.h/.cpp`

Gestiona la vista de SFML separando área del mapa (izquierda) del panel UI (derecha).

- **Viewport:** el área del mapa excluye el panel de 280px y el HUD de 28px
- **Zoom:** rango configurable `[CAMERA_ZOOM_MIN, CAMERA_ZOOM_MAX]`
- **Paneo:** teclas WASD/flechas, velocidad proporcional al zoom actual
- **Transformaciones:** `worldToScreen()` y `screenToWorld()` para clicks precisos

---

### 9.3 Animations

**Archivo:** `src/visual/Animations.h/.cpp`

Interpolación lineal de posición entre waypoints:

```cpp
void setRoute(const std::string& dealerId,
              const std::vector<std::string>& nodeIds,
              float simDurationSeconds);
```

Si se provee `simDurationSeconds`, la velocidad de animación se calibra automáticamente para que el dealer llegue al destino exactamente cuando el `phaseTimer` del simulador llega a cero.

**Posición actual:** `getPosition(dealerId)` retorna `{-1, -1}` si no hay animación activa.

---

### 9.4 UI

**Archivo:** `src/visual/UI.h/.cpp`

Panel lateral de 280px con:
- **HUD superior:** hora simulada, multiplicador de tráfico, FPS
- **Sección Tráfico:** clima, evento activo, factor total (coloreado rojo/amarillo/verde)
- **Sección Controles:** Pausa/Resume, velocidad (Lenta/Normal/Rápida), clima manual, eventos manuales
- **Sección Estadísticas:** órdenes generadas/entregadas/canceladas/activas, dealers libres, tiempos promedio
- **Últimas entregas:** historial de los últimos 4 pedidos
- **Leyenda:** colores de dealers y marcadores
- **Botón [?]:** abre `assets/info.html` en el navegador del sistema

**Callbacks:** la UI comunica eventos al `RendererSFML` mediante `std::function<>`:
- `setOnPause` / `setOnResume`
- `setOnWeatherChange`
- `setOnEventTrigger`
- `setOnSpeedChange`

---

## 10. Modelos de datos

### Order — ciclo de vida de un pedido

```
Estado:   PENDING → ASSIGNED → PICKING_UP → IN_TRANSIT → DELIVERED
                                                         → CANCELLED
```

Campos clave:
- `createdAt`, `assignedAt`, `deliveredAt` — timestamps en minutos simulados
- `waitTime()` = assignedAt − createdAt
- `deliveryTime()` = deliveredAt − assignedAt
- `priority` — de 1 (baja) a 5 (alta)

---

### Dealer — repartidor

Estados (`DealerStatus`): `IDLE`, `HEADING_TO_RESTAURANT`, `PICKING_UP`, `DELIVERING`, `RETURNING`

Campos clave:
- `currentNodeId` — nodo actual en el grafo (se actualiza al llegar al restaurante)
- `activeOrderIds` — lista de órdenes en curso (máximo `maxOrders`)
- `logisticScore` — calculado externamente por el `Simulator`
- `loadFactor()` — proporción de capacidad usada (0.0–1.0)

---

### Restaurant — restaurante

El `popularityScore` es la clave del AVL y se recalcula con `recalculateScore()` cada vez que llega o sale un pedido:

```
score = 0.7 × (totalOrders / 1000)
      + 0.3 × (averageRating / 5.0)
      - 0.05 × activeOrders
```

---

### Client — cliente

Almacena historial de pedidos (`orderHistory`), gasto total (`totalSpent`) y rating promedio. `isFrequent()` retorna true si tiene ≥ 5 pedidos.

---

## 11. Utilidades

### Config.h

Única fuente de verdad para todas las constantes del sistema. **Ningún número mágico debe existir fuera de este archivo.** Ver sección 12 para la tabla completa.

### Logger

Singleton con soporte para consola (colores ANSI) y archivo. Niveles: `DEBUG`, `INFO`, `WARNING`, `ERROR`. Auto-inicializa si no se llamó `init()`.

```cpp
Logger::info("Dealer D01 asignado a ORD_00123");
Logger::warn("Nodo N99 no encontrado en el grafo");
Logger::error("Fallo al abrir edges.csv");
```

### Random

Wrapper sobre `std::mt19937` (Mersenne Twister). Métodos útiles:
- `Random::uniformInt(min, max)` — entero uniforme inclusivo
- `Random::uniformReal(min, max)` — real uniforme
- `Random::normal(mean, stddev)` — distribución normal
- `Random::weighted(weights)` — índice según pesos (para selección de restaurante)
- `Random::chance(p)` — Bernoulli con probabilidad p
- `Random::seed(n)` — seed fija para reproducibilidad en debug

---

## 12. Parámetros de configuración (Config.h)

| Constante | Valor | Descripción |
|-----------|-------|-------------|
| `SIM_SPEED_DEFAULT` | 2.0 | Velocidad inicial (min simulados / seg real) |
| `SIM_SPEED_SLOW` | 0.75 | Velocidad lenta |
| `SIM_SPEED_FAST` | 5.0 | Velocidad rápida |
| `SIM_START_HOUR` | 8 | Hora de inicio de la simulación |
| `DEALER_MAX_ORDERS` | 3 | Pedidos simultáneos por repartidor |
| `DEALER_SPEED_KMH` | 35.0 | Velocidad del repartidor en km/h |
| `DEALER_PICKUP_TIME` | 3.0 | Minutos esperando en el restaurante |
| `DEALER_SCORE_ALPHA` | 0.6 | Peso de la distancia en el score logístico |
| `DEALER_SCORE_BETA` | 0.4 | Peso de la disponibilidad en el score logístico |
| `ORDER_PRIORITY_MIN` | 1 | Prioridad mínima de una orden |
| `ORDER_PRIORITY_MAX` | 5 | Prioridad máxima de una orden |
| `ORDER_STALE_MINUTES` | 10.0 | Minutos sin asignar para recibir boost |
| `ORDER_BOOST_AMOUNT` | 2 | Unidades de prioridad del boost |
| `ORDER_BOOST_INTERVAL` | 5.0 | Intervalo de chequeo de boost (min simulados) |
| `SPAWN_RATE_BASE` | 0.5 | Pedidos por minuto simulado (base) |
| `DEMAND_LUNCH` | 2.0 | Multiplicador de demanda al almuerzo |
| `DEMAND_DINNER` | 1.8 | Multiplicador de demanda en la noche |
| `DEMAND_DAWN` | 0.2 | Multiplicador de demanda en madrugada |
| `HISTORY_MAX_RECORDS` | 1000 | Registros máximos en DeliveryHistory |
| `WINDOW_WIDTH` | 1280 | Ancho de la ventana en píxeles |
| `WINDOW_HEIGHT` | 720 | Alto de la ventana en píxeles |
| `TARGET_FPS` | 60 | FPS objetivo |
| `CAMERA_ZOOM_MIN` | 0.3 | Zoom mínimo |
| `CAMERA_ZOOM_MAX` | 5.0 | Zoom máximo |
| `NODE_RADIUS_BARRIO` | 8.0 | Radio de nodo barrio en pantalla |
| `UI_PANEL_WIDTH` | 280.0 | Ancho del panel lateral en píxeles |

---

## 13. Compilación y ejecución

### Requisitos

| Herramienta | Versión recomendada |
|-------------|---------------------|
| Compilador | g++ (MinGW-w64 / GCC) con soporte C++17 |
| SFML | 3.x (enlazado estático) |
| Entorno | Windows + MSYS2/UCRT64 |
| IDE | VS Code con extensión C/C++ (opcional) |

### Compilar (VS Code)

```
Ctrl + Shift + B   →  ejecuta la tarea "Build VousCommandez"
```

O desde terminal:
```bash
cd C1_VOUSCOMMANDEZ
g++ -std=c++17 -g -Wall [todos los .cpp] -o build/VousCommandez.exe -lsfml-graphics -lsfml-window -lsfml-system
```

### Ejecutar

```bash
cd build
./VousCommandez.exe
```

> **IMPORTANTE:** El ejecutable debe correrse desde `build/` para que las rutas relativas a `data/` y `assets/` resuelvan correctamente.

### Controles

| Acción | Control |
|--------|---------|
| Mover cámara | WASD o flechas |
| Zoom in | Scroll ↑ |
| Zoom out | Scroll ↓ |
| Click en dealer | Resalta su ruta activa |
| Tecla R | Resetea cámara al centro |
| Escape | Cierra la aplicación |
| Botón [?] | Abre documentación interactiva en el navegador |

---

## 14. Glosario

| Término | Significado |
|---------|-------------|
| **Dealer** | Repartidor / domiciliario |
| **Order** | Pedido / domicilio |
| **Nodo** | Punto del grafo (barrio o restaurante) |
| **Arista** | Conexión entre dos nodos con peso (tiempo en minutos) |
| **Score logístico** | Métrica combinada de proximidad y disponibilidad del dealer |
| **Tick** | Un ciclo de actualización del simulador (por frame) |
| **simDelta** | Delta de tiempo en minutos simulados por frame |
| **baseTime** | Tiempo base de una arista sin multiplicadores de tráfico |
| **TrafficEdgeData** | Estructura con los tres factores de tráfico activos |
| **RouteResult** | Resultado de Dijkstra o Bellman-Ford con ruta y métricas |
| **DealerPhaseState** | Estado interno de la máquina de estados de un dealer |
| **Cache de rutas** | `dealerRouteCache` — evita recalcular Dijkstra cada frame |
| **Boost anti-starvation** | Aumento automático de prioridad de órdenes esperando mucho tiempo |
| **popularityScore** | Clave del AVL — combinación de volumen, rating y actividad del restaurante |
| **Inorder traversal** | Recorrido izq → raíz → der del AVL, produce restaurantes ordenados por score |

---

*Documentación generada para el proyecto VousCommandez — Ciencias de la Computación I, Universidad Distrital Francisco José de Caldas, 2026-1*