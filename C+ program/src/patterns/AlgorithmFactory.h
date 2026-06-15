#pragma once

#include "core/PathFinder.h"

#include <memory>
#include <string>
#include <vector>

namespace pathplanner {

enum class AlgorithmType {
    BFS = 1,
    DFS = 2,
    Dijkstra = 3,
    AStar = 4,
    GreedyBestFirst = 5,
    WeightedAStar = 6,
    BidirectionalBFS = 7,
    BidirectionalAStar = 8,
    IDAStar = 9,
    IDDFS = 10
};

class AlgorithmFactory {
public:
    static std::unique_ptr<PathFinder> create(AlgorithmType type);
    static std::string nameOf(AlgorithmType type);
    static std::vector<AlgorithmType> allTypes();
    static bool isValid(int id);
};

}  // namespace pathplanner
