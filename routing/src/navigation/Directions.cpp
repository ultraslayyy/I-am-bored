#define _USE_MATH_DEFINES

#include "Directions.h"
#include "../geo/geo.h"
#include <sstream>
#include <cmath>

DirectionsGenerator::DirectionsGenerator(const Graph& graph) : graph(graph) {}

double DirectionsGenerator::bearing(int a, int b) const {
    const auto& n1 = graph.nodes[a];
    const auto& n2 = graph.nodes[b];

    double lat1 = deg2rad(n1.lat);
    double lat2 = deg2rad(n2.lat);
    double dLon = deg2rad(n2.lon - n1.lon);

    double y = sin(dLon) * cos(lat2);
    double x = cos(lat1) * sin(lat2) -
               sin(lat1) * cos(lat2) * cos(dLon);

    return fmod((rad2deg(atan2(y, x)) + 360.0), 360.0);
}

double DirectionsGenerator::angleBetween(int a, int b, int c) const {
    double b1 = bearing(a, b);
    double b2 = bearing(b, c);

    double diff = b2 - b1;

    while (diff > 180) diff -= 360;
    while (diff < -180) diff += 360;

    return diff;
}

std::string DirectionsGenerator::turnType(double angle) const {
    if (std::abs(angle) < 15) return "Continue";

    if (angle > 0) {
        if (angle < 45) return "Slight right";
        if (angle < 135) return "Turn right";
        return "Sharp right";
    } else {
        if (angle > -45) return "Slight left";
        if (angle > -135) return "Turn left";
        return "Sharp left";
    }
}

std::vector<Instruction> DirectionsGenerator::generate(const std::vector<int>& path) {
    std::vector<Instruction> result;

    if (path.size() < 2) return result;

    double segmentDistance = 0.0;
    std::string currentRoad = "";

    for (size_t i = 0; i + 1 < path.size(); ++i) {
        int u = path[i];
        int v = path[i + 1];

        const Edge* edgePtr = nullptr;
        for (const auto& e : graph.adj[u]) {
            if (e.to == v) {
                edgePtr = &e;
                break;
            }
        }
        if (!edgePtr) continue;

        const Edge& edge = *edgePtr;

        if (i == 0) {
            currentRoad = edge.name;
            segmentDistance += edge.distance;
            continue;
        }

        int prev = path[i - 1];

        double angle = angleBetween(prev, u, v);

        bool roadChanged = edge.name != currentRoad;

        if (roadChanged || std::abs(angle) > 30) {
            std::ostringstream oss;

            oss << "Drive " << (int)segmentDistance << "m on "
                << (currentRoad.empty() ? "unnamed road" : currentRoad);

            result.push_back({oss.str(), segmentDistance});

            std::string turn = turnType(angle);

            std::ostringstream oss2;
            oss2 << turn << " onto "
                 << (edge.name.empty() ? "unnamed road" : edge.name);

            result.push_back({oss2.str(), 0});

            segmentDistance = 0;
            currentRoad = edge.name;
        }

        segmentDistance += edge.distance;
    }

    if (segmentDistance > 0) {
        std::ostringstream oss;
        oss << "Drive " << (int)segmentDistance << "m to destination";
        result.push_back({oss.str(), segmentDistance});
    }

    return result;
}