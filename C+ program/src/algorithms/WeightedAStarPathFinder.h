#pragma once

#include "algorithms/AStarPathFinder.h"

namespace pathplanner {

class WeightedAStarPathFinder : public GraphPathFinder {
public:
    explicit WeightedAStarPathFinder(double weight = 1.8);
    std::string name() const override { return "Weighted A* 加权A*"; }
    PathResult findPath(const Grid& grid) override;

private:
    double weight_;
    static double heuristic(const Point& a, const Point& b);
};

}  // namespace pathplanner
