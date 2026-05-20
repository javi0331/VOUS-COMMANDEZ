# 🛵 Vous Commandez — Simulador de Domicilios en Bogotá

> Proyecto de Ciencias de la Computación I  
> Universidad Distrital Francisco José de Caldas  
> Ingeniería de Sistemas — Grupo 81

---

## Descripción general

**Vous Commandez** es una simulación de un servicio de domicilios estilo Rappi sobre un grafo real de Bogotá. El programa modela repartidores moviéndose por la ciudad, recibiendo pedidos de restaurantes reales y entregándolos a clientes, todo en tiempo simulado con visualización en tiempo real.

El grafo representa **83 nodos** (barrios y restaurantes) y **162 aristas** distribuidas en 7 zonas de Bogotá: Norte, Centro-Norte, Centro, Occidente, Sur-Occidente y Sur. Cada arista tiene un peso que representa el costo de desplazamiento, afectado dinámicamente por clima, hora del día y eventos de tráfico.

---

## Características principales

- **Pathfinding con Dijkstra y Bellman-Ford** — los repartidores calculan la ruta óptima en tiempo real considerando el tráfico actual.
- **Sistema de tráfico dinámico** — el peso de las aristas cambia según clima (lluvia, tormenta), hora del día (horas pico) y eventos (partidos, conciertos, marchas).
- **Generador de órdenes procedural** — la tasa de pedidos varía según la hora simulada: máximo al mediodía y en la noche, mínimo en madrugada.
- **Estructuras de datos propias** — el proyecto implementa desde cero un Heap de repartidores (HeapDealers), cola de prioridad de órdenes (PriorityQueueOrders) y un árbol AVL de restaurantes (AVLRestaurants).
- **Historial de entregas** — registro con tiempo de espera, tiempo de entrega y distancia por pedido.
- **Visualización SFML** — renderizado en tiempo real con cámara con zoom/paneo, animaciones interpoladas de repartidores y panel de estadísticas en vivo.

---

## Estructura del proyecto

```
C1_VOUSCOMMANDEZ/
├── src/
│   ├── algorithms/         # Dijkstra, Bellman-Ford, RouteResult, Path
│   ├── core/               # Graph, Node, Edge, GraphLoader, coordenadas
│   ├── models/             # Dealer, Order, Restaurant, Client, estados
│   ├── simulation/         # Simulator, TimeSystem, TrafficSystem,
│   │                       # WeatherSystem, EventSystem, OrderGenerator
│   ├── structures/         # HeapDealers, PriorityQueueOrders,
│   │                       # AVLRestaurants, DeliveryHistory
│   ├── utils/              # Config, Logger, Random
│   ├── visual/             # RendererSFML, Camera, UI, Animations
│   └── Main.cpp
├── build/
│   ├── VousCommandez.exe
│   ├── assets/fonts/Roboto-Regular.ttf
│   └── data/
│       ├── nodes.csv
│       └── edges.csv
└── .vscode/
    └── tasks.json
```

---

## Grafo de Bogotá

El grafo se carga desde dos archivos CSV en `build/data/`.

### `nodes.csv`

Cada nodo representa un barrio o restaurante con su zona geográfica y coordenadas de visualización.

| Campo | Descripción |
|-------|-------------|
| `id` | Identificador único (ej: `C2`, `N1`, `R01`) |
| `name` | Nombre del lugar |
| `zone` | Zona: `NORTE`, `CENTRO_NORTE`, `CENTRO`, `OCCIDENTE`, `SUR_OCCIDENTE`, `SUR` |
| `type` | `BARRIO` o `RESTAURANT` |
| `x`, `y` | Coordenadas de visualización en pantalla |

**Zonas y colores en pantalla:**

| Zona | Color |
|------|-------|
| Norte | Azul |
| Centro-Norte | Morado |
| Centro | Verde |
| Occidente | Naranja |
| Sur-Occidente | Rojo |
| Sur | Rojo oscuro |
| Restaurante | Amarillo/verde |

### `edges.csv`

| Campo | Descripción |
|-------|-------------|
| `source` | Nodo origen |
| `target` | Nodo destino |
| `weight` | Costo base de la arista (minutos) |
| `bidirectional` | `1` = doble sentido, `0` = unidireccional |

---

## Arquitectura del sistema

### Flujo principal

```
Main.cpp
  └── Simulator (tick cada frame)
        ├── TimeSystem       → avanza el reloj simulado
        ├── WeatherSystem    → actualiza el clima
        ├── TrafficSystem    → recalcula multiplicadores de peso
        ├── EventSystem      → dispara eventos programados
        ├── OrderGenerator   → genera pedidos según hora del día
        ├── PriorityQueueOrders → encola pedidos por prioridad
        ├── HeapDealers      → selecciona el repartidor óptimo
        ├── Dijkstra         → calcula ruta dealer → restaurante → cliente
        └── DeliveryHistory  → registra entregas completadas

RendererSFML (loop SFML)
  ├── Camera       → zoom, paneo, transformación mundo↔pantalla
  ├── Animations   → interpolación de posición de repartidores
  └── UI           → HUD, panel lateral, estadísticas, controles
```

### Módulos principales

#### `Simulator`
Orquesta todos los sistemas. Cada llamada a `tick(dt)` avanza el tiempo, genera órdenes, las asigna a repartidores usando Dijkstra, y actualiza el estado de las entregas en curso.

#### `TrafficSystem`
Calcula el multiplicador de tráfico total como producto de tres factores:

```
factor_total = factor_clima × factor_hora × factor_evento
```

| Condición | Multiplicador |
|-----------|---------------|
| Clima despejado | ×1.0 |
| Lluvia | ×1.3 |
| Tormenta | ×1.6 |
| Hora normal | ×1.0 |
| Hora pico | ×1.5 |
| Madrugada | ×0.8 |
| Sin evento | ×1.0 |
| Marcha | ×1.2 |
| Concierto / Partido | ×1.4 |

#### `OrderGenerator`
Genera pedidos proceduralmente. La tasa base se multiplica por un factor según la hora simulada:

| Hora | Factor de demanda |
|------|-------------------|
| 00–06h (madrugada) | ×0.2 |
| 09–11h (mañana) | ×0.8 |
| 12–14h (almuerzo) | ×2.0 |
| 14–18h (tarde) | ×0.7 |
| 19–22h (noche) | ×1.8 |
| 22h+ | ×0.5 |

#### `HeapDealers`
Max-heap que prioriza repartidores con mejor combinación de proximidad al restaurante y carga libre. El score se calcula como:

```
score = 0.6 × (1 / distancia_al_restaurante) + 0.4 × (1 − carga_actual)
```

#### `AVLRestaurants`
Árbol AVL que mantiene los restaurantes ordenados por popularidad (score basado en volumen de pedidos y rating). Permite actualización eficiente O(log n) cuando un restaurante recibe o despacha un pedido.

#### `PriorityQueueOrders`
Cola de prioridad implementada con cuatro colas internas. Las órdenes que llevan más de 10 minutos esperando reciben un boost de +2 de prioridad automáticamente cada 5 minutos simulados.

#### `Animations`
Interpola linealmente la posición de cada repartidor entre los waypoints de su ruta. La velocidad de animación se calibra automáticamente para que el punto en pantalla llegue al destino exactamente cuando la simulación marca la entrega como completada — garantizando sincronización visual con la lógica.

---

## Algoritmos de pathfinding

### Dijkstra
Algoritmo principal usado en tiempo real para asignar rutas. Implementado con cola de prioridad (min-heap). El peso de cada arista en el cálculo es:

```
peso_efectivo = peso_base × factor_trafico_actual
```

### Bellman-Ford
Implementación alternativa disponible para comparación. Soporta aristas con peso negativo (no se usan en el grafo actual, pero el algoritmo está disponible como demostración).

`RouteResult` expone `executionTimeMs` y `visitedNodes` para comparar la eficiencia real de ambos algoritmos sobre el mismo grafo.

---

## Visualización

### Controles de teclado y mouse

| Acción | Control |
|--------|---------|
| Mover cámara | `W A S D` o flechas |
| Zoom in | Scroll ↑ |
| Zoom out | Scroll ↓ |
| Click en dealer | Resalta su ruta activa |

### Panel lateral (UI)

- **HUD superior:** hora simulada, multiplicador de velocidad, FPS.
- **Tráfico:** clima actual, evento activo, factor total.
- **Controles:** pausa/reanuda, velocidad (Lenta ×2 / Normal ×5 / Rápida ×20), clima manual (Claro / Lluvia / Tormenta), eventos manuales (Partido / Concierto / Marcha).
- **Estadísticas:** órdenes generadas, entregadas, canceladas, activas, dealers libres, tiempos promedio.
- **Últimas entregas:** historial de los últimos pedidos con ID y tiempo de entrega.

---

## Compilación y ejecución

### Requisitos

- Compilador C++17 (MinGW-w64 / GCC recomendado)
- [SFML 3.x](https://www.sfml-dev.org/) enlazado estáticamente
- VS Code con extensión C/C++ (o cualquier entorno con g++)

### Compilar (VS Code)

```
Ctrl + Shift + B
```

Genera `build/VousCommandez.exe`.

### Ejecutar

```bash
cd build
./VousCommandez.exe
```

El ejecutable debe correrse desde `build/` para que las rutas relativas a `data/` y `assets/` resuelvan correctamente.

---

## Parámetros configurables

Todos los parámetros numéricos del proyecto están centralizados en `src/utils/Config.h`. Los más relevantes:

| Constante | Valor por defecto | Descripción |
|-----------|-------------------|-------------|
| `SIM_SPEED_DEFAULT` | `5.0` | Velocidad inicial (×5 real) |
| `SIM_SPEED_SLOW` | `2.0` | Velocidad lenta |
| `SIM_SPEED_FAST` | `20.0` | Velocidad rápida |
| `DEALER_MAX_ORDERS` | `3` | Pedidos simultáneos por repartidor |
| `DEALER_SPEED_KMH` | `35.0` | Velocidad del repartidor en km/h |
| `SPAWN_RATE_BASE` | `0.5` | Pedidos/min simulado base |
| `ORDER_STALE_MINUTES` | `10.0` | Minutos para boost de prioridad |
| `HISTORY_MAX_RECORDS` | `1000` | Registros máximos en historial |
| `WINDOW_WIDTH/HEIGHT` | `1280×720` | Resolución de ventana |

---

## Estructuras de datos implementadas

| Estructura | Archivo | Descripción |
|------------|---------|-------------|
| Max-Heap | `HeapDealers` | Selección óptima de repartidor en O(log n) |
| Cola de prioridad | `PriorityQueueOrders` | Gestión de pedidos pendientes con boost anti-starvation |
| Árbol AVL | `AVLRestaurants` | Ranking de restaurantes por popularidad en O(log n) |
| Lista enlazada implícita | `DeliveryHistory` | Registro histórico de entregas con estadísticas |
| Grafo dirigido/no-dirigido | `Graph` | Representación con lista de adyacencia |

---

> Universidad Distrital Francisco José de Caldas  
> Facultad de Ingeniería — Ingeniería de Sistemas  
> Ciencias de la Computación I — 2026-1