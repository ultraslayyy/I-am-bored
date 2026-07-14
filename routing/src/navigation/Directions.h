#pragma once

#include <vector>
#include <string>
#include "../graph/graph.h"

struct Instruction {
    std::string text;
    double distance;
};

class DirectionsGenerator {
public:
    explicit DirectionsGenerator(const Graph& graph);

    std::vector<Instruction> generate(const std::vector<int>& path);

private:
    const Graph& graph;

    double bearing(int a, int b) const;
    double angleBetween(int a, int b, int c) const;

    std::string turnType(double angle) const;
};