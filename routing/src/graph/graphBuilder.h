#pragma once

#include "../osm/OSMTypes.h"
#include "graph.h"

class GraphBuilder {
public:
    Graph build(const OSMData& data);

private:
    bool isDrivable(const OSMWay& way);
    double defaultSpeed(const OSMWay& way);
    double getSpeed(const OSMWay& way);
    bool getOneWay(const OSMWay& way);
    std::string getName(const OSMWay& way);
};