#include "api/PathfindingService.h"

#include "patterns/AlgorithmFactory.h"
#include "patterns/PathfindingContext.h"

namespace pathplanner {

PathResult PathfindingService::run(AlgorithmType type, const Grid& grid) {
    PathfindingContext context;
    context.setStrategy(AlgorithmFactory::create(type));
    return context.run(grid);
}

std::vector<PathResult> PathfindingService::compareAll(const Grid& grid) {
    return compareSelected(grid, AlgorithmFactory::allTypes());
}

std::vector<PathResult> PathfindingService::compareSelected(
    const Grid& grid, const std::vector<AlgorithmType>& types) {
    std::vector<PathResult> results;
    results.reserve(types.size());
    for (AlgorithmType type : types) {
        results.push_back(run(type, grid));
    }
    return results;
}

}  // namespace pathplanner
