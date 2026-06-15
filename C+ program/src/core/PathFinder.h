#pragma once

#include "core/Grid.h"
#include "core/PathResult.h"

namespace pathplanner {

class PathFinder {
public:
    virtual ~PathFinder() = default;

    virtual std::string name() const = 0;
    virtual PathResult findPath(const Grid& grid) = 0;
};

}  // namespace pathplanner
