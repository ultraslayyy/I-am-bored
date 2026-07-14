#pragma once

#include <vector>

class IRouter {
public:
    virtual ~IRouter() = default;
    virtual std::vector<int> findPath(int start, int goal) = 0;
};