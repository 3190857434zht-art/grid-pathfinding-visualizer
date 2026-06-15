#pragma once

#include "algorithms/GraphPathFinder.h"

namespace pathplanner {

class GreedyBestFirstPathFinder : public GraphPathFinder {
public:
    std::string name() const override { return "Greedy Best-First 贪婪最佳优先"; }
    PathResult findPath(const Grid& grid) override;
};

}  // namespace pathplanner
