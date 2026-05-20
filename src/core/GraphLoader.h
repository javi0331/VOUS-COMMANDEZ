#pragma once

#include <string>
#include "../core/Graph.h"

class GraphLoader {
public:
    static Graph loadFromCSV(
        const std::string& nodesFile,
        const std::string& edgesFile
    );
};