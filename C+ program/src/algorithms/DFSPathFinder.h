#pragma once

#include "algorithms/GraphPathFinder.h"

namespace pathplanner {

class DFSPathFinder : public GraphPathFinder {
public:
    std::string name() const override { return "DFS 深度优先"; }
    PathResult findPath(const Grid& grid) override;
};

}  // namespace pathplanner
