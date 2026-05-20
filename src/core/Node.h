#pragma once

#include <string>
#include "../core/Coords.h"
#include "../core/NodeType.h"
#include "../core/Zone.h"

class Node {
private:
    std::string id;
    std::string name;

    NodeType type;
    Zone zone;

    Coords position;

public:
    Node();

    Node(
        const std::string& id,
        const std::string& name,
        NodeType type,
        Zone zone,
        const Coords& position
    );

    const std::string& getId() const;

    const std::string& getName() const;

    NodeType getType() const;

    Zone getZone() const;

    const Coords& getPosition() const;
};