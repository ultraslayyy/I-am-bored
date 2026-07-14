#include "graphBuilder.h"
#include <unordered_map>
#include <cmath>
#include <iostream>
#include "../geo/geo.h"

Graph GraphBuilder::build(const OSMData& data) {
    Graph graph;

    std::unordered_map<long, int> idMap;

    int index = 0;
    for (const auto& [osmId, osmNode] : data.nodes) {
        Node node;
        node.id  = index;
        node.lat = osmNode.lat;
        node.lon = osmNode.lon;

        graph.nodes.push_back(node);
        graph.adj.emplace_back();

        idMap[osmId] = index;
        index++;
    }

    std::cout << "Parsed graph nodes: " << graph.nodes.size() << "\n";

    for (const auto& way : data.ways) {
        if (!isDrivable(way)) continue;

        double speed = getSpeed(way);
        bool oneway  = getOneWay(way);
        std::string name = getName(way);

        for (size_t i = 0; i + 1 < way.nodeRefs.size(); ++i) {
            long fromId = way.nodeRefs[i];
            long toId   = way.nodeRefs[i + 1];

            if (!idMap.count(fromId) || !idMap.count(toId)) continue;

            int u = idMap[fromId];
            int v = idMap[toId];

            const auto& n1 = graph.nodes[u];
            const auto& n2 = graph.nodes[v];

            double dist = haversine(n1.lat, n1.lon, n2.lat, n2.lon);

            graph.adj[u].push_back({v, dist, speed, name});

            if (!oneway) {
                graph.adj[v].push_back({u, dist, speed, name});
            }
        }
    }

    int edgeCount = 0;
    for (const auto& list : graph.adj) {
        edgeCount += list.size();
    }

    std::cout << "Parsed graph edges: " << edgeCount << "\n";

    return graph;
}

bool GraphBuilder::isDrivable(const OSMWay& way) {
    auto it = way.tags.find("highway");
    if (it == way.tags.end()) return false;

    return true;
}

double GraphBuilder::defaultSpeed(const OSMWay& way) {
    auto it = way.tags.find("highway");
    if (it == way.tags.end()) return 50.0;

    const std::string& type = it->second;

    if (type == "motorway") return 100.0;
    if (type == "primary") return 80.0;
    if (type == "secondary") return 60.0;
    if (type == "residential") return 50.0;

    return 50.0;
}

double GraphBuilder::getSpeed(const OSMWay& way) {
    auto it = way.tags.find("maxspeed");
    if (it != way.tags.end()) {
        try {
            return std::stod(it->second);
        } catch (...) {}
    }

    auto hw = way.tags.find("highway");
    if (hw != way.tags.end()) {
        const std::string& type = hw->second;

        if (type == "motorway") return 100;
        if (type == "primary") return 80;
        if (type == "secondary") return 60;
        if (type == "residential") return 50;
    }

    return 50;
}

bool GraphBuilder::getOneWay(const OSMWay& way) {
    auto it = way.tags.find("oneway");
    if (it != way.tags.end() && it->second == "yes") {
        return true;
    }

    return false;
}

std::string GraphBuilder::getName(const OSMWay& way) {
    auto it = way.tags.find("name");
    if (it != way.tags.end()) {
        return it->second;
    }

    return "";
}