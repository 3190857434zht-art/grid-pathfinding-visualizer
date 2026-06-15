#pragma once

#include "algorithms/GraphPathFinder.h"

namespace pathplanner {

class BFSPathFinder : public GraphPathFinder {
public:
    std::string name() const override { return "BFS (Breadth-First Search)"; }
    PathResult findPath(const Grid& grid) override;
};

}  // namespace pathplanner
