#pragma once

#include "algorithms/GraphPathFinder.h"

namespace pathplanner {

class AStarPathFinder : public GraphPathFinder {
public:
    std::string name() const override { return "A* (A-Star)"; }
    PathResult findPath(const Grid& grid) override;

private:
    static double heuristic(const Point& a, const Point& b);
};

}  // namespace pathplanner
