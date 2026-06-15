#pragma once

#include "core/Grid.h"

#include <string>
#include <vector>

namespace pathplanner {

struct SearchStats {
    int expandedNodes = 0;
    int visitedNodes = 0;
    double elapsedMs = 0.0;
    bool found = false;
};

struct PathResult {
    std::vector<Point> path;
    std::vector<Point> visitedOrder;
    SearchStats stats;
    std::string algorithmName;
};

}  // namespace pathplanner
