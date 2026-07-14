#include "app.h"

#include <algorithm>
#include <iostream>
#include <vector>
#include <string>

#include "cli.h"
#include "../osm/OSMParser.h"
#include "../graph/graphBuilder.h"
#include "../routing/AStarRouter.h"
#include "../navigation/Directions.h"

CLI cliHandler;

void App::run() {
    loadMap("map.osm");

    int start = selectNode("Select starting point:");
    int goal  = selectNode("Select destination:");

    runRouting(start, goal);
}

void App::loadMap(const std::string& path) {
    std::cout << "Loading map: " << path << "\n";

    OSMParser parser;
    OSMData data = parser.parse(path);

    GraphBuilder builder;
    graph = builder.build(data);
}

int App::selectNode(const std::string& prompt) {
    std::vector<std::string> options;

    // For now: just list first N nodes
    int limit = std::min(20, (int)graph.nodes.size());

    for (int i = 0; i < limit; ++i) {
        const auto& node = graph.nodes[i];
        options.push_back(
            "Node " + std::to_string(node.id) +
            " (" + std::to_string(node.lat) +
            ", " + std::to_string(node.lon) + ")"
        );
    }

    std::string selected = cliHandler.MultipleChoice(options, prompt);

    // Map string back to index (temporary hack)
    for (int i = 0; i < options.size(); ++i) {
        if (options[i] == selected) {
            return i;
        }
    }

    return 0;
}

void App::runRouting(int start, int goal) {
    std::cout << "Routing from " << start << " to " << goal << "\n";

    AStarRouter router(graph);

    auto path = router.findPath(start, goal);

    if (path.empty()) {
        std::cout << "No path found\n";
        return;
    }

    DirectionsGenerator dirGen(graph);
    auto directions = dirGen.generate(path);

    std::cout << "\nDirections:\n";
    for (const auto& instr : directions) {
        std::cout << "- " << instr.text << "\n";
    }
}