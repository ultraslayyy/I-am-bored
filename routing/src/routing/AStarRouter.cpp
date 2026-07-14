#include "AStarRouter.h"
#include "../geo/geo.h"
#include <queue>
#include <limits>
#include <algorithm>

struct PQNode {
    int node;
    double fScore;

    bool operator>(const PQNode& other) const {
        return fScore > other.fScore;
    }
};

double travelTime(double distance, double speedKmh) {
    double speedMs = speedKmh * (1000.0 / 3600.0);
    return distance / speedMs;
}

AStarRouter::AStarRouter(const Graph& graph) : graph(graph) {}

double AStarRouter::heuristic(int a, int b) const {
    const auto& n1 = graph.nodes[a];
    const auto& n2 = graph.nodes[b];

    double dist = haversine(n1.lat, n1.lon, n2.lat, n2.lon);

    double maxSpeed = 130.0;

    double speedMs = maxSpeed * (1000.0 / 3600.0);
    return dist / speedMs;
}

std::vector<int> AStarRouter::findPath(int start, int goal) {
    int n = graph.nodes.size();

    std::vector<double> gScore(n, std::numeric_limits<double>::infinity());
    std::vector<double> fScore(n, std::numeric_limits<double>::infinity());
    std::vector<int> cameFrom(n, -1);

    std::priority_queue<PQNode, std::vector<PQNode>, std::greater<>> openSet;

    gScore[start] = 0.0;
    fScore[start] = heuristic(start, goal);

    openSet.push({start, fScore[start]});

    std::vector<bool> visited(n, false);

    while (!openSet.empty()) {
        int current = openSet.top().node;
        openSet.pop();

        if (visited[current]) continue;
        visited[current] = true;

        if (current == goal) {
            std::vector<int> path;
            for (int at = goal; at != -1; at = cameFrom[at]) {
                path.push_back(at);
            }
            std::reverse(path.begin(), path.end());
            return path;
        }

        for (const auto& edge : graph.adj[current]) {
            int neighbor = edge.to;

            double time = edge.distance / (edge.speed * (1000.0 / 3600.0));
            double tentativeG = gScore[current] + time;

            if (tentativeG < gScore[neighbor]) {
                cameFrom[neighbor] = current;
                gScore[neighbor] = tentativeG;
                fScore[neighbor] = tentativeG + heuristic(neighbor, goal);

                openSet.push({neighbor, fScore[neighbor]});
            }
        }
    }

    return {};
}