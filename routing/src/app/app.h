#pragma once

#include <string>
#include "../graph/graph.h"

class App {
public:
    void run();

private:
    void loadMap(const std::string& path);
    int selectNode(const std::string& prompt);
    void runRouting(int start, int goal);

    Graph graph;
};