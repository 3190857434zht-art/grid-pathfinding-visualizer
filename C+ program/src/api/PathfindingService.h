#pragma once

#include "core/Grid.h"
#include "core/PathResult.h"
#include "patterns/AlgorithmFactory.h"

#include <vector>

namespace pathplanner {

class PathfindingService {
public:
    static PathResult run(AlgorithmType type, const Grid& grid);
    static std::vector<PathResult> compareAll(const Grid& grid);
    static std::vector<PathResult> compareSelected(const Grid& grid, const std::vector<AlgorithmType>& types);
};

}  // namespace pathplanner
