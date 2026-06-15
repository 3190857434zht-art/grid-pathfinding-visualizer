#include "algorithms/BFSPathFinder.h"

#include <queue>
#include <unordered_set>

namespace pathplanner {

PathResult BFSPathFinder::findPath(const Grid& grid) {
    const auto startTime = std::chrono::steady_clock::now();

    PathResult result;
    result.algorithmName = name();

    const Point start = grid.start();
    const Point goal = grid.goal();
    const int cols = grid.cols();

    const int startKey = encodePoint(start.row, start.col, cols);
    const int goalKey = encodePoint(goal.row, goal.col, cols);

    std::queue<int> frontier;
    ParentMap parent;
    std::unordered_set<int> visited;

    frontier.push(startKey);
    visited.insert(startKey);

    int step = 0;
    while (!frontier.empty()) {
        const int currentKey = frontier.front();
        frontier.pop();
        ++result.stats.expandedNodes;

        const Point current = decodePoint(currentKey, cols);
        result.visitedOrder.push_back(current);
        notify({SearchEventType::NodeExpanded, current, step++, "BFS expanded node"});

        if (currentKey == goalKey) {
            result.path = reconstructPath(parent, startKey, goalKey, cols);
            result.stats.found = true;
            notify({SearchEventType::PathFound, goal, step, "Path found"});
            break;
        }

        for (const Point& next : grid.neighbors4(current.row, current.col)) {
            const int nextKey = encodePoint(next.row, next.col, cols);
            if (visited.insert(nextKey).second) {
                parent[nextKey] = currentKey;
                frontier.push(nextKey);
                ++result.stats.visitedNodes;
                notify({SearchEventType::NodeVisited, next, step, "BFS queued node"});
            }
        }
    }

    result.stats.elapsedMs = elapsedMs(startTime);
    notify({SearchEventType::SearchFinished, goal, step, result.stats.found ? "Done" : "No path"});
    return result;
}

}  // namespace pathplanner
