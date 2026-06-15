#include "algorithms/GreedyBestFirstPathFinder.h"

#include "algorithms/Heuristic.h"

#include <queue>
#include <unordered_set>

namespace pathplanner {

namespace {

struct GreedyNode {
    int key = 0;
    double h = 0.0;
    bool operator>(const GreedyNode& other) const { return h > other.h; }
};

}  // namespace

PathResult GreedyBestFirstPathFinder::findPath(const Grid& grid) {
    const auto startTime = std::chrono::steady_clock::now();
    PathResult result;
    result.algorithmName = name();

    const Point start = grid.start();
    const Point goal = grid.goal();
    const int cols = grid.cols();
    const int startKey = encodePoint(start.row, start.col, cols);
    const int goalKey = encodePoint(goal.row, goal.col, cols);

    std::priority_queue<GreedyNode, std::vector<GreedyNode>, std::greater<GreedyNode>> frontier;
    ParentMap parent;
    std::unordered_set<int> closed;

    frontier.push({startKey, manhattanHeuristic(start, goal)});

    int step = 0;
    while (!frontier.empty()) {
        const GreedyNode node = frontier.top();
        frontier.pop();
        if (closed.count(node.key) > 0) {
            continue;
        }
        closed.insert(node.key);
        ++result.stats.expandedNodes;

        const Point current = decodePoint(node.key, cols);
        result.visitedOrder.push_back(current);
        notify({SearchEventType::NodeExpanded, current, step++, "Greedy expanded"});

        if (node.key == goalKey) {
            result.path = reconstructPath(parent, startKey, goalKey, cols);
            result.stats.found = true;
            notify({SearchEventType::PathFound, goal, step, "Path found"});
            break;
        }

        for (const Point& next : grid.neighbors4(current.row, current.col)) {
            const int nextKey = encodePoint(next.row, next.col, cols);
            if (closed.count(nextKey) > 0) {
                continue;
            }
            parent[nextKey] = node.key;
            frontier.push({nextKey, manhattanHeuristic(next, goal)});
            ++result.stats.visitedNodes;
            notify({SearchEventType::NodeVisited, next, step, "Greedy queued"});
        }
    }

    result.stats.elapsedMs = elapsedMs(startTime);
    notify({SearchEventType::SearchFinished, goal, step, result.stats.found ? "Done" : "No path"});
    return result;
}

}  // namespace pathplanner
