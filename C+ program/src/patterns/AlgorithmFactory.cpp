#include "patterns/AlgorithmFactory.h"

#include "algorithms/AStarPathFinder.h"
#include "algorithms/BFSPathFinder.h"
#include "algorithms/BidirectionalAStarPathFinder.h"
#include "algorithms/BidirectionalBFSPathFinder.h"
#include "algorithms/DFSPathFinder.h"
#include "algorithms/DijkstraPathFinder.h"
#include "algorithms/GreedyBestFirstPathFinder.h"
#include "algorithms/IDAStarPathFinder.h"
#include "algorithms/IDDFSPathFinder.h"
#include "algorithms/WeightedAStarPathFinder.h"

#include <stdexcept>

namespace pathplanner {

std::unique_ptr<PathFinder> AlgorithmFactory::create(AlgorithmType type) {
    switch (type) {
        case AlgorithmType::BFS:
            return std::make_unique<BFSPathFinder>();
        case AlgorithmType::DFS:
            return std::make_unique<DFSPathFinder>();
        case AlgorithmType::Dijkstra:
            return std::make_unique<DijkstraPathFinder>();
        case AlgorithmType::AStar:
            return std::make_unique<AStarPathFinder>();
        case AlgorithmType::GreedyBestFirst:
            return std::make_unique<GreedyBestFirstPathFinder>();
        case AlgorithmType::WeightedAStar:
            return std::make_unique<WeightedAStarPathFinder>();
        case AlgorithmType::BidirectionalBFS:
            return std::make_unique<BidirectionalBFSPathFinder>();
        case AlgorithmType::BidirectionalAStar:
            return std::make_unique<BidirectionalAStarPathFinder>();
        case AlgorithmType::IDAStar:
            return std::make_unique<IDAStarPathFinder>();
        case AlgorithmType::IDDFS:
            return std::make_unique<IDDFSPathFinder>();
    }
    throw std::invalid_argument("Unknown algorithm type.");
}

std::string AlgorithmFactory::nameOf(AlgorithmType type) {
    return create(type)->name();
}

std::vector<AlgorithmType> AlgorithmFactory::allTypes() {
    return {
        AlgorithmType::BFS,
        AlgorithmType::DFS,
        AlgorithmType::Dijkstra,
        AlgorithmType::AStar,
        AlgorithmType::GreedyBestFirst,
        AlgorithmType::WeightedAStar,
        AlgorithmType::BidirectionalBFS,
        AlgorithmType::BidirectionalAStar,
        AlgorithmType::IDAStar,
        AlgorithmType::IDDFS,
    };
}

bool AlgorithmFactory::isValid(int id) {
    return id >= static_cast<int>(AlgorithmType::BFS) && id <= static_cast<int>(AlgorithmType::IDDFS);
}

}  // namespace pathplanner
