#pragma once

#include "algorithms/GraphPathFinder.h"

#include <unordered_set>

namespace pathplanner {

class IDAStarPathFinder : public GraphPathFinder {
public:
    std::string name() const override { return "IDA* 迭代加深A*"; }
    PathResult findPath(const Grid& grid) override;

private:
    const Grid* grid_ = nullptr;
    Point goal_{};
    int cols_ = 0;
    int startKey_ = 0;
    int goalKey_ = 0;
    ParentMap* parent_ = nullptr;
    PathResult* result_ = nullptr;
    int step_ = 0;

    static double heuristic(const Point& a, const Point& b);
    bool search(int currentKey, double g, double bound, double& nextBound, std::unordered_set<int>& onPath);
};

}  // namespace pathplanner
