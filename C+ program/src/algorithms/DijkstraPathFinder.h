#pragma once

#include "algorithms/GraphPathFinder.h"

namespace pathplanner {

class DijkstraPathFinder : public GraphPathFinder {
public:
    std::string name() const override { return "Dijkstra"; }
    PathResult findPath(const Grid& grid) override;
};

}  // namespace pathplanner
