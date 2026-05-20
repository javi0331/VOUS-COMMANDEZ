#include "../core/Node.h"

Node::Node() {}

Node::Node(
    const std::string& id,
    const std::string& name,
    NodeType type,
    Zone zone,
    const Coords& position
)
    : id(id),
      name(name),
      type(type),
      zone(zone),
      position(position) {}

const std::string& Node::getId() const {
    return id;
}

const std::string& Node::getName() const {
    return name;
}

NodeType Node::getType() const {
    return type;
}

Zone Node::getZone() const {
    return zone;
}

const Coords& Node::getPosition() const {
    return position;
}