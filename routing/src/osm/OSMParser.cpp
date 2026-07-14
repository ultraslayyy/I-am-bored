#include "OSMParser.h"
#include "../../vendor/tinyxml2/tinyxml2.h"
#include <iostream>

OSMData OSMParser::parse(const std::string& filepath) {
    OSMData data;

    tinyxml2::XMLDocument doc;
    if (doc.LoadFile(filepath.c_str()) != tinyxml2::XML_SUCCESS) {
        std::cerr << "Failed to load OSM file\n";
        return data;
    }

    tinyxml2::XMLElement* root = doc.FirstChildElement("osm");
    if (!root) {
        std::cerr << "Invalid OSM file\n";
        return data;
    }

    for (tinyxml2::XMLElement* elem = root->FirstChildElement("node"); elem != nullptr; elem = elem->NextSiblingElement("node")) {
        OSMNode node;

        elem->QueryInt64Attribute("id", &node.id);
        elem->QueryDoubleAttribute("lat", &node.lat);
        elem->QueryDoubleAttribute("lon", &node.lon);

        data.nodes[node.id] = node;
    }

    std::cout << "Parsed " << data.nodes.size() << " nodes\n";

    for (tinyxml2::XMLElement* elem = root->FirstChildElement("way"); elem != nullptr; elem = elem->NextSiblingElement("way")) {
        OSMWay way;
        elem->QueryInt64Attribute("id", &way.id);

        for (tinyxml2::XMLElement* nd = elem->FirstChildElement("nd"); nd != nullptr; nd = nd->NextSiblingElement("nd")) {
            std::int64_t ref;
            nd->QueryInt64Attribute("ref", &ref);
            way.nodeRefs.push_back(ref);
        }

        for (tinyxml2::XMLElement* tag = elem->FirstChildElement("tag"); tag != nullptr; tag = tag->NextSiblingElement("tag")) {
            const char* k = tag->Attribute("k");
            const char* v = tag->Attribute("v");

            if (k && v) {
                way.tags[k] = v;
            }
        }

        data.ways.push_back(std::move(way));
    }

    std::cout << "Parsed " << data.ways.size() << " ways\n";

    return data;
}