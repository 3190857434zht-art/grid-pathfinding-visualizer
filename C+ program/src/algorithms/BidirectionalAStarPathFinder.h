#pragma once

#include "algorithms/GraphPathFinder.h"

namespace pathplanner {

class BidirectionalAStarPathFinder : public GraphPathFinder {
public:
    std::string name() const override { return "Bidirectional A* 双向A*"; }
    PathResult findPath(const Grid& grid) override;

private:
    static double heuristic(const Point& a, const Point& b);
    std::vector<Point> mergePaths(int meetKey, const ParentMap& forward, const ParentMap& backward, int startKey,
                                  int goalKey, int cols) const;
};

}  // namespace pathplanner
