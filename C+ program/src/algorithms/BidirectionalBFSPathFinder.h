#pragma once

#include "algorithms/GraphPathFinder.h"

namespace pathplanner {

class BidirectionalBFSPathFinder : public GraphPathFinder {
public:
    std::string name() const override { return "Bidirectional BFS 双向BFS"; }
    PathResult findPath(const Grid& grid) override;

private:
    std::vector<Point> mergePaths(int meetKey, const ParentMap& forward, const ParentMap& backward, int startKey,
                                  int goalKey, int cols) const;
};

}  // namespace pathplanner
