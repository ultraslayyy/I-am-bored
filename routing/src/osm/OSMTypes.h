#pragma once

#include <vector>
#include <unordered_map>
#include <string>
#include <cstdint>

struct OSMNode {
    std::int64_t id;
    double lat;
    double lon;
};

struct OSMWay {
    std::int64_t id;
    std::vector<long> nodeRefs;
    std::unordered_map<std::string, std::string> tags;
};

struct OSMData {
    std::unordered_map<long, OSMNode> nodes;
    std::vector<OSMWay> ways;
};