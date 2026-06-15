#include "algorithms/IDAStarPathFinder.h"

#include "algorithms/Heuristic.h"

#include <limits>
#include <unordered_set>

namespace pathplanner {

double IDAStarPathFinder::heuristic(const Point& a, const Point& b) {
    return manhattanHeuristic(a, b);
}

bool IDAStarPathFinder::search(
    int currentKey, double g, double bound, double& nextBound, std::unordered_set<int>& onPath) {
    if (onPath.count(currentKey) > 0) {
        return false;
    }
    onPath.insert(currentKey);

    const Point current = decodePoint(currentKey, cols_);
    const double f = g + heuristic(current, goal_);
    if (f > bound) {
        nextBound = std::min(nextBound, f);
        onPath.erase(currentKey);
        return false;
    }

    ++result_->stats.expandedNodes;
    result_->visitedOrder.push_back(current);
    notify({SearchEventType::NodeExpanded, current, step_++, "IDA* expanded"});

    if (currentKey == goalKey_) {
        onPath.erase(currentKey);
        return true;
    }

    for (const Point& next : grid_->neighbors4(current.row, current.col)) {
        const int nextKey = encodePoint(next.row, next.col, cols_);
        if (onPath.count(nextKey) > 0) {
            continue;
        }
        (*parent_)[nextKey] = currentKey;
        ++result_->stats.visitedNodes;
        notify({SearchEventType::NodeVisited, next, step_, "IDA* visit"});
        if (search(nextKey, g + 1.0, bound, nextBound, onPath)) {
            onPath.erase(currentKey);
            return true;
        }
    }

    onPath.erase(currentKey);
    return false;
}

PathResult IDAStarPathFinder::findPath(const Grid& grid) {
    const auto startTime = std::chrono::steady_clock::now();
    PathResult result;
    result.algorithmName = name();

    grid_ = &grid;
    goal_ = grid.goal();
    cols_ = grid.cols();
    const Point start = grid.start();
    startKey_ = encodePoint(start.row, start.col, cols_);
    goalKey_ = encodePoint(goal_.row, goal_.col, cols_);
    result_ = &result;
    step_ = 0;

    ParentMap parent;
    parent_ = &parent;

    double bound = heuristic(start, goal_);
    const int maxIterations = grid.rows() * grid.cols() * 2;
    int iteration = 0;

    while (iteration++ < maxIterations) {
        double nextBound = std::numeric_limits<double>::infinity();
        parent.clear();
        std::unordered_set<int> onPath;
        if (search(startKey_, 0.0, bound, nextBound, onPath)) {
            result.path = reconstructPath(parent, startKey_, goalKey_, cols_);
            result.stats.found = true;
            notify({SearchEventType::PathFound, goal_, step_, "Path found"});
            break;
        }
        if (nextBound == std::numeric_limits<double>::infinity()) {
            break;
        }
        if (nextBound <= bound) {
            break;
        }
        bound = nextBound;
    }

    result.stats.elapsedMs = elapsedMs(startTime);
    notify({SearchEventType::SearchFinished, goal_, step_, result.stats.found ? "Done" : "No path"});
    grid_ = nullptr;
    parent_ = nullptr;
    result_ = nullptr;
    return result;
}

}  // namespace pathplanner
