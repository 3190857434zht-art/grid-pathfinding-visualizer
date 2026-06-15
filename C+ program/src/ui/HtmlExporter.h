#pragma once

#include "core/Grid.h"
#include "core/PathResult.h"

#include <string>
#include <vector>

namespace pathplanner {

class HtmlExporter {
public:
    bool exportVisualization(
        const Grid& grid,
        const PathResult& result,
        const std::string& outputPath) const;

    bool exportComparison(
        const Grid& grid,
        const std::vector<PathResult>& results,
        const std::string& outputPath) const;
};

}  // namespace pathplanner
