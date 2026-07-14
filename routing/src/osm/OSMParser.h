#pragma once

#include "OSMTypes.h"
#include <string>

class OSMParser {
public:
    OSMData parse(const std::string& filepath);
};