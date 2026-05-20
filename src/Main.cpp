#include <iostream>

// ── Utils ─────────────────────────────────────────────────────
#include "utils/Logger.h"
#include "utils/Config.h"
#include "utils/Random.h"

// ── Core ──────────────────────────────────────────────────────
#include "core/GraphLoader.h"
#include "core/Graph.h"

// ── Models ────────────────────────────────────────────────────
#include "models/Dealer.h"
#include "models/Restaurant.h"
#include "models/Client.h"

// ── Simulation ────────────────────────────────────────────────
#include "simulation/Simulator.h"

// ── Visual ────────────────────────────────────────────────────
#include "visual/RendererSFML.h"

int main() {

    // ── Logger ────────────────────────────────────────────────
    Logger::init(
        Config::LOG_TO_CONSOLE,
        Config::LOG_TO_FILE,
        Config::LOG_FILENAME,
        LogLevel::DEBUG
    );
    Logger::info("Iniciando VousCommandez...");

    // ── Semilla aleatoria ─────────────────────────────────────
    Random::seedRandom();

    // ── Carga del grafo desde CSV ─────────────────────────────
    // cwd = build/, así que las rutas son relativas a build/
    Graph graph;

    try {
        graph = GraphLoader::loadFromCSV(
            "data/nodes.csv",
            "data/edges.csv"
        );
    }
    catch (const std::exception& e) {
        std::cerr << "ERROR cargando CSV: " << e.what() << std::endl;
        Logger::error(std::string("Fallo GraphLoader: ") + e.what());
        return -1;
    }

    Logger::info("Grafo cargado: "
        + std::to_string(graph.nodeCount()) + " nodos, "
        + std::to_string(graph.edgeCount()) + " aristas");

    // ── Verificación de nodos de dealers ─────────────────────
    Logger::info("Verificando nodos de dealers...");
    for (const auto& nid : {"C2","C1","N1","O4","S1","M1","SU1","O1","C10","S7"}) {
        if (!graph.hasNode(nid))
            Logger::error("Nodo de dealer NO encontrado en grafo: " + std::string(nid));
    }

    // ── Simulator ─────────────────────────────────────────────
    Simulator simulator(graph);

    // ── Restaurantes ──────────────────────────────────────────
    simulator.addRestaurant(Restaurant("R01", "McDonald's Zona T",    "R01", Zone::CENTRO_NORTE));
    simulator.addRestaurant(Restaurant("R02", "Crepes & Waffles ZT",  "R02", Zone::CENTRO_NORTE));
    simulator.addRestaurant(Restaurant("R03", "Subway Chapinero",     "R03", Zone::CENTRO_NORTE));
    simulator.addRestaurant(Restaurant("R04", "El Corral Rosales",    "R04", Zone::CENTRO_NORTE));
    simulator.addRestaurant(Restaurant("R05", "Andrés CDR Chía",      "R05", Zone::NORTE));
    simulator.addRestaurant(Restaurant("R06", "Harry Sasson",         "R06", Zone::CENTRO_NORTE));
    simulator.addRestaurant(Restaurant("R07", "Criterion",            "R07", Zone::CENTRO_NORTE));
    simulator.addRestaurant(Restaurant("R08", "Crepes Unicentro",     "R08", Zone::NORTE));
    simulator.addRestaurant(Restaurant("R09", "McDonald's Calle 72",  "R09", Zone::CENTRO_NORTE));
    simulator.addRestaurant(Restaurant("R10", "Pizza Hut Cedritos",   "R10", Zone::NORTE));
    simulator.addRestaurant(Restaurant("R11", "Burger Master Salitre","R11", Zone::OCCIDENTE));
    simulator.addRestaurant(Restaurant("R12", "KFC Salitre Plaza",    "R12", Zone::OCCIDENTE));
    simulator.addRestaurant(Restaurant("R13", "McDonald's Parkway",   "R13", Zone::CENTRO_NORTE));
    simulator.addRestaurant(Restaurant("R14", "La Hamburguesería",    "R14", Zone::CENTRO_NORTE));
    simulator.addRestaurant(Restaurant("R15", "Wok Galerías",         "R15", Zone::CENTRO_NORTE));
    simulator.addRestaurant(Restaurant("R16", "KFC Kennedy",          "R16", Zone::SUR_OCCIDENTE));
    simulator.addRestaurant(Restaurant("R17", "Burger King Kennedy",  "R17", Zone::SUR_OCCIDENTE));
    simulator.addRestaurant(Restaurant("R18", "McDonald's Américas",  "R18", Zone::SUR_OCCIDENTE));
    simulator.addRestaurant(Restaurant("R19", "El Corral Corabastos", "R19", Zone::SUR_OCCIDENTE));
    simulator.addRestaurant(Restaurant("R20", "Subway Fontibón",      "R20", Zone::OCCIDENTE));
    simulator.addRestaurant(Restaurant("R21", "La Puerta Falsa",      "R21", Zone::CENTRO));
    simulator.addRestaurant(Restaurant("R22", "Rest. La Macarena",    "R22", Zone::CENTRO));
    simulator.addRestaurant(Restaurant("R23", "El Boliche Candelaria","R23", Zone::CENTRO));
    simulator.addRestaurant(Restaurant("R24", "Frisby Restrepo",      "R24", Zone::SUR));
    simulator.addRestaurant(Restaurant("R25", "McDonald's San Crist.","R25", Zone::SUR));

    // ── Repartidores ──────────────────────────────────────────
    simulator.addDealer(Dealer("D01", "Carlos M.",  "C2",  Config::DEALER_MAX_ORDERS));
    simulator.addDealer(Dealer("D02", "Andrés P.",  "C1",  Config::DEALER_MAX_ORDERS));
    simulator.addDealer(Dealer("D03", "Luis G.",    "N1",  Config::DEALER_MAX_ORDERS));
    simulator.addDealer(Dealer("D04", "María F.",   "O4",  Config::DEALER_MAX_ORDERS));
    simulator.addDealer(Dealer("D05", "Jorge R.",   "S1",  Config::DEALER_MAX_ORDERS));
    simulator.addDealer(Dealer("D06", "Sandra V.",  "M1",  Config::DEALER_MAX_ORDERS));
    simulator.addDealer(Dealer("D07", "Felipe T.",  "SU1", Config::DEALER_MAX_ORDERS));
    simulator.addDealer(Dealer("D08", "Diana L.",   "O1",  Config::DEALER_MAX_ORDERS));
    simulator.addDealer(Dealer("D09", "Pablo N.",   "C10", Config::DEALER_MAX_ORDERS));
    simulator.addDealer(Dealer("D10", "Camila Z.",  "S7",  Config::DEALER_MAX_ORDERS));

    // ── Clientes ──────────────────────────────────────────────
    const std::vector<std::string> clientNodes = {
        "N1","N2","N3","N5","N6",
        "C1","C2","C3","C4","C5","C8","C10","C12",
        "O1","O2","O4","O5",
        "S1","S2","S4","S5","S7",
        "M1","M2","M3","M4","M6",
        "SU1","SU2","SU4","SU5"
    };

    int clientIdx = 1;
    for (const auto& nodeId : clientNodes) {
        simulator.addClient(Client(
            "CLI_" + std::to_string(clientIdx++),
            "Cliente " + nodeId,
            nodeId,
            Zone::CENTRO
        ));
    }

    // ── Eventos programados ────────────────────────────────────
    simulator.getEventSystem().scheduleTrafficEvent(
        "partido_tarde", 180.0,
        EventState::MATCH, 120.0
    );
    simulator.getEventSystem().scheduleWeather(
        "lluvia_noche", 300.0,
        WeatherState::RAIN
    );

    Logger::info("Simulator configurado. Iniciando render...");

    // ── Render ────────────────────────────────────────────────
    // cwd = build/, la fuente está en build/assets/fonts/
    RendererSFML renderer(simulator);

    if (!renderer.init("assets/fonts/Roboto-Regular.ttf")) {
        Logger::warn("Fuente no encontrada, continuando sin texto");
    }

    renderer.run();

    Logger::info("VousCommandez cerrado.");
    Logger::flush();

    return 0;
}