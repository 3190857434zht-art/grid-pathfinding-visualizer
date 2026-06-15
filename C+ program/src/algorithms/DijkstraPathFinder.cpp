#include "algorithms/DijkstraPathFinder.h"

#include <limits>
#include <queue>
#include <unordered_map>
#include <unordered_set>

namespace pathplanner {

namespace {

struct DijkstraNode {
    int key = 0;
    double cost = 0.0;

    bool operator>(const DijkstraNode& other) const {
        return cost > other.cost;
    }
};

}  // namespace

PathResult DijkstraPathFinder::findPath(const Grid& grid) {
    const auto startTime = std::chrono::steady_clock::now();

    PathResult result;
    result.algorithmName = name();

    const Point start = grid.start();
    const Point goal = grid.goal();
    const int cols = grid.cols();

    const int startKey = encodePoint(start.row, start.col, cols);
    const int goalKey = encodePoint(goal.row, goal.col, cols);

    std::priority_queue<DijkstraNode, std::vector<DijkstraNode>, std::greater<DijkstraNode>> frontier;
    ParentMap parent;
    std::unordered_map<int, double> bestCost;
    std::unordered_set<int> finalized;

    frontier.push({startKey, 0.0});
    bestCost[startKey] = 0.0;

    int step = 0;
    while (!frontier.empty()) {
        const DijkstraNode currentNode = frontier.top();
        frontier.pop();

        if (finalized.count(currentNode.key) > 0) {
            continue;
        }

        finalized.insert(currentNode.key);
        ++result.stats.expandedNodes;

        const Point current = decodePoint(currentNode.key, cols);
        result.visitedOrder.push_back(current);
        notify({SearchEventType::NodeExpanded, current, step++, "Dijkstra expanded node"});

        if (currentNode.key == goalKey) {
            result.path = reconstructPath(parent, startKey, goalKey, cols);
            result.stats.found = true;
            notify({SearchEventType::PathFound, goal, step, "Path found"});
            break;
        }

        for (const Point& next : grid.neighbors4(current.row, current.col)) {
            const int nextKey = encodePoint(next.row, next.col, cols);
            if (finalized.count(nextKey) > 0) {
                continue;
            }

            const double nextCost = currentNode.cost + 1.0;
            const auto it = bestCost.find(nextKey);
            if (it == bestCost.end() || nextCost < it->second) {
                bestCost[nextKey] = nextCost;
                parent[nextKey] = currentNode.key;
                frontier.push({nextKey, nextCost});
                ++result.stats.visitedNodes;
                notify({SearchEventType::NodeVisited, next, step, "Dijkstra relaxed node"});
            }
        }
    }

    result.stats.elapsedMs = elapsedMs(startTime);
    notify({SearchEventType::SearchFinished, goal, step, result.stats.found ? "Done" : "No path"});
    return result;
}

}  // namespace pathplanner
