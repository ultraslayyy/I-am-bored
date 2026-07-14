#pragma once

#include "IRouter.h"
#include "../graph/graph.h"

class AStarRouter : public IRouter {
public:
    explicit AStarRouter(const Graph& graph);

    std::vector<int> findPath(int start, int goal) override;

private:
    const Graph& graph;

    double heuristic(int a, int b) const;
};