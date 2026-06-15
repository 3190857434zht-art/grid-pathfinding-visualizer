#include "patterns/PathfindingContext.h"

#include <stdexcept>

namespace pathplanner {

void PathfindingContext::setStrategy(std::unique_ptr<PathFinder> strategy) {
    strategy_ = std::move(strategy);
}

PathResult PathfindingContext::run(const Grid& grid) {
    if (!strategy_) {
        throw std::runtime_error("No pathfinding strategy selected.");
    }
    return strategy_->findPath(grid);
}

}  // namespace pathplanner
