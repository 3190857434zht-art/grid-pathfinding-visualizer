#include "algorithms/BidirectionalBFSPathFinder.h"

#include <algorithm>
#include <queue>
#include <unordered_set>

namespace pathplanner {

std::vector<Point> BidirectionalBFSPathFinder::mergePaths(
    int meetKey, const ParentMap& forward, const ParentMap& backward, int startKey, int goalKey, int cols) const {
    std::vector<Point> path;
    int current = meetKey;
    while (current != startKey) {
        path.push_back(decodePoint(current, cols));
        const auto it = forward.find(current);
        if (it == forward.end()) {
            return {};
        }
        current = it->second;
    }
    path.push_back(decodePoint(startKey, cols));
    std::reverse(path.begin(), path.end());

    current = meetKey;
    const auto backIt = backward.find(current);
    if (backIt == backward.end()) {
        return path;
    }
    current = backIt->second;
    while (current != goalKey) {
        path.push_back(decodePoint(current, cols));
        const auto it = backward.find(current);
        if (it == backward.end()) {
            return {};
        }
        current = it->second;
    }
    path.push_back(decodePoint(goalKey, cols));
    return path;
}

PathResult BidirectionalBFSPathFinder::findPath(const Grid& grid) {
    const auto startTime = std::chrono::steady_clock::now();
    PathResult result;
    result.algorithmName = name();

    const Point start = grid.start();
    const Point goal = grid.goal();
    const int cols = grid.cols();
    const int startKey = encodePoint(start.row, start.col, cols);
    const int goalKey = encodePoint(goal.row, goal.col, cols);

    std::queue<int> forwardQ;
    std::queue<int> backwardQ;
    ParentMap forwardParent;
    ParentMap backwardParent;
    std::unordered_set<int> forwardVisited;
    std::unordered_set<int> backwardVisited;

    forwardQ.push(startKey);
    backwardQ.push(goalKey);
    forwardVisited.insert(startKey);
    backwardVisited.insert(goalKey);

    int step = 0;
    int meetKey = -1;

    while (!forwardQ.empty() && !backwardQ.empty()) {
        auto expandLayer = [&](std::queue<int>& q, ParentMap& parent, std::unordered_set<int>& visited,
                               const std::unordered_set<int>& otherVisited, bool forward) {
            const std::size_t layerSize = q.size();
            for (std::size_t i = 0; i < layerSize; ++i) {
                const int currentKey = q.front();
                q.pop();
                ++result.stats.expandedNodes;
                const Point current = decodePoint(currentKey, cols);
                result.visitedOrder.push_back(current);
                notify({SearchEventType::NodeExpanded, current, step++, forward ? "BiBFS forward" : "BiBFS backward"});

                if (otherVisited.count(currentKey) > 0) {
                    meetKey = currentKey;
                    return true;
                }

                for (const Point& next : grid.neighbors4(current.row, current.col)) {
                    const int nextKey = encodePoint(next.row, next.col, cols);
                    if (visited.insert(nextKey).second) {
                        parent[nextKey] = currentKey;
                        q.push(nextKey);
                        ++result.stats.visitedNodes;
                        notify({SearchEventType::NodeVisited, next, step, "BiBFS queued"});
                        if (otherVisited.count(nextKey) > 0) {
                            meetKey = nextKey;
                            return true;
                        }
                    }
                }
            }
            return false;
        };

        if (expandLayer(forwardQ, forwardParent, forwardVisited, backwardVisited, true)) {
            break;
        }
        if (expandLayer(backwardQ, backwardParent, backwardVisited, forwardVisited, false)) {
            break;
        }
    }

    if (meetKey >= 0) {
        result.path = mergePaths(meetKey, forwardParent, backwardParent, startKey, goalKey, cols);
        result.stats.found = !result.path.empty();
        if (result.stats.found) {
            notify({SearchEventType::PathFound, goal, step, "Path found"});
        }
    }

    result.stats.elapsedMs = elapsedMs(startTime);
    notify({SearchEventType::SearchFinished, goal, step, result.stats.found ? "Done" : "No path"});
    return result;
}

}  // namespace pathplanner
