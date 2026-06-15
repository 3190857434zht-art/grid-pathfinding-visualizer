#pragma once

#include "algorithms/GraphPathFinder.h"

#include <unordered_set>

namespace pathplanner {

class IDDFSPathFinder : public GraphPathFinder {
public:
    std::string name() const override { return "IDDFS 迭代加深DFS"; }
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

    bool depthLimitedSearch(int currentKey, int depth, int limit, std::unordered_set<int>& onPath);
};

}  // namespace pathplanner
