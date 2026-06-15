#include "algorithms/AStarPathFinder.h"

#include <cmath>
#include <limits>
#include <queue>
#include <unordered_map>
#include <unordered_set>

namespace pathplanner {

namespace {

struct AStarNode {
    int key = 0;
    double fScore = 0.0;

    bool operator>(const AStarNode& other) const {
        return fScore > other.fScore;
    }
};

}  // namespace

double AStarPathFinder::heuristic(const Point& a, const Point& b) {
    return static_cast<double>(std::abs(a.row - b.row) + std::abs(a.col - b.col));
}

PathResult AStarPathFinder::findPath(const Grid& grid) {
    const auto startTime = std::chrono::steady_clock::now();

    PathResult result;
    result.algorithmName = name();

    const Point start = grid.start();
    const Point goal = grid.goal();
    const int cols = grid.cols();

    const int startKey = encodePoint(start.row, start.col, cols);
    const int goalKey = encodePoint(goal.row, goal.col, cols);

    std::priority_queue<AStarNode, std::vector<AStarNode>, std::greater<AStarNode>> frontier;
    ParentMap parent;
    std::unordered_map<int, double> gScore;
    std::unordered_set<int> closed;

    gScore[startKey] = 0.0;
    frontier.push({startKey, heuristic(start, goal)});

    int step = 0;
    while (!frontier.empty()) {
        const AStarNode currentNode = frontier.top();
        frontier.pop();

        if (closed.count(currentNode.key) > 0) {
            continue;
        }

        closed.insert(currentNode.key);
        ++result.stats.expandedNodes;

        const Point current = decodePoint(currentNode.key, cols);
        result.visitedOrder.push_back(current);
        notify({SearchEventType::NodeExpanded, current, step++, "A* expanded node"});

        if (currentNode.key == goalKey) {
            result.path = reconstructPath(parent, startKey, goalKey, cols);
            result.stats.found = true;
            notify({SearchEventType::PathFound, goal, step, "Path found"});
            break;
        }

        const double currentG = gScore[currentNode.key];
        for (const Point& next : grid.neighbors4(current.row, current.col)) {
            const int nextKey = encodePoint(next.row, next.col, cols);
            if (closed.count(nextKey) > 0) {
                continue;
            }

            const double tentativeG = currentG + 1.0;
            const auto it = gScore.find(nextKey);
            if (it == gScore.end() || tentativeG < it->second) {
                gScore[nextKey] = tentativeG;
                parent[nextKey] = currentNode.key;
                const double fScore = tentativeG + heuristic(next, goal);
                frontier.push({nextKey, fScore});
                ++result.stats.visitedNodes;
                notify({SearchEventType::NodeVisited, next, step, "A* queued node"});
            }
        }
    }

    result.stats.elapsedMs = elapsedMs(startTime);
    notify({SearchEventType::SearchFinished, goal, step, result.stats.found ? "Done" : "No path"});
    return result;
}

}  // namespace pathplanner
