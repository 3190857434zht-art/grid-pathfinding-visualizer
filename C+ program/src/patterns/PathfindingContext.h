#pragma once

#include "core/PathFinder.h"

#include <memory>

namespace pathplanner {

class PathfindingContext {
public:
    void setStrategy(std::unique_ptr<PathFinder> strategy);
    PathFinder* strategy() const { return strategy_.get(); }

    PathResult run(const Grid& grid);

private:
    std::unique_ptr<PathFinder> strategy_;
};

}  // namespace pathplanner
