#pragma once

#include <string>
#include <vector>

struct Edge {
    int to;
    double distance;
    double speed;
    std::string name;
};

struct Node {
    int id;
    double lat, lon;
};

class Graph {
public:
    std::vector<Node> nodes;
    std::vector<std::vector<Edge>> adj;
};