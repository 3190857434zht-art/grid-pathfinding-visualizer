#include "algorithms/IDDFSPathFinder.h"

#include <unordered_set>

namespace pathplanner {

bool IDDFSPathFinder::depthLimitedSearch(
    int currentKey, int depth, int limit, std::unordered_set<int>& onPath) {
    if (onPath.count(currentKey) > 0) {
        return false;
    }
    onPath.insert(currentKey);

    const Point current = decodePoint(currentKey, cols_);
    ++result_->stats.expandedNodes;
    result_->visitedOrder.push_back(current);
    notify({SearchEventType::NodeExpanded, current, step_++, "IDDFS expanded"});

    if (currentKey == goalKey_) {
        onPath.erase(currentKey);
        return true;
    }
    if (depth >= limit) {
        onPath.erase(currentKey);
        return false;
    }

    for (const Point& next : grid_->neighbors4(current.row, current.col)) {
        const int nextKey = encodePoint(next.row, next.col, cols_);
        if (onPath.count(nextKey) > 0) {
            continue;
        }
        (*parent_)[nextKey] = currentKey;
        ++result_->stats.visitedNodes;
        notify({SearchEventType::NodeVisited, next, step_, "IDDFS visit"});
        if (depthLimitedSearch(nextKey, depth + 1, limit, onPath)) {
            onPath.erase(currentKey);
            return true;
        }
        parent_->erase(nextKey);
    }

    onPath.erase(currentKey);
    return false;
}

PathResult IDDFSPathFinder::findPath(const Grid& grid) {
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

    const int maxLimit = grid.rows() + grid.cols();
    for (int limit = 0; limit <= maxLimit; ++limit) {
        parent.clear();
        std::unordered_set<int> onPath;
        if (depthLimitedSearch(startKey_, 0, limit, onPath)) {
            result.path = reconstructPath(parent, startKey_, goalKey_, cols_);
            result.stats.found = true;
            notify({SearchEventType::PathFound, goal_, step_, "Path found"});
            break;
        }
    }

    result.stats.elapsedMs = elapsedMs(startTime);
    notify({SearchEventType::SearchFinished, goal_, step_, result.stats.found ? "Done" : "No path"});
    grid_ = nullptr;
    parent_ = nullptr;
    result_ = nullptr;
    return result;
}

}  // namespace pathplanner
