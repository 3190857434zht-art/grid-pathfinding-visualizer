#include "algorithms/WeightedAStarPathFinder.h"

#include "algorithms/Heuristic.h"

#include <queue>
#include <unordered_map>
#include <unordered_set>

namespace pathplanner {

namespace {

struct WANode {
    int key = 0;
    double fScore = 0.0;
    bool operator>(const WANode& other) const { return fScore > other.fScore; }
};

}  // namespace

WeightedAStarPathFinder::WeightedAStarPathFinder(double weight) : weight_(weight) {}

double WeightedAStarPathFinder::heuristic(const Point& a, const Point& b) {
    return manhattanHeuristic(a, b);
}

PathResult WeightedAStarPathFinder::findPath(const Grid& grid) {
    const auto startTime = std::chrono::steady_clock::now();
    PathResult result;
    result.algorithmName = name();

    const Point start = grid.start();
    const Point goal = grid.goal();
    const int cols = grid.cols();
    const int startKey = encodePoint(start.row, start.col, cols);
    const int goalKey = encodePoint(goal.row, goal.col, cols);

    std::priority_queue<WANode, std::vector<WANode>, std::greater<WANode>> frontier;
    ParentMap parent;
    std::unordered_map<int, double> gScore;
    std::unordered_set<int> closed;

    gScore[startKey] = 0.0;
    frontier.push({startKey, weight_ * heuristic(start, goal)});

    int step = 0;
    while (!frontier.empty()) {
        const WANode node = frontier.top();
        frontier.pop();
        if (closed.count(node.key) > 0) {
            continue;
        }
        closed.insert(node.key);
        ++result.stats.expandedNodes;

        const Point current = decodePoint(node.key, cols);
        result.visitedOrder.push_back(current);
        notify({SearchEventType::NodeExpanded, current, step++, "WA* expanded"});

        if (node.key == goalKey) {
            result.path = reconstructPath(parent, startKey, goalKey, cols);
            result.stats.found = true;
            notify({SearchEventType::PathFound, goal, step, "Path found"});
            break;
        }

        const double currentG = gScore[node.key];
        for (const Point& next : grid.neighbors4(current.row, current.col)) {
            const int nextKey = encodePoint(next.row, next.col, cols);
            if (closed.count(nextKey) > 0) {
                continue;
            }
            const double tentativeG = currentG + 1.0;
            const auto it = gScore.find(nextKey);
            if (it == gScore.end() || tentativeG < it->second) {
                gScore[nextKey] = tentativeG;
                parent[nextKey] = node.key;
                frontier.push({nextKey, tentativeG + weight_ * heuristic(next, goal)});
                ++result.stats.visitedNodes;
                notify({SearchEventType::NodeVisited, next, step, "WA* queued"});
            }
        }
    }

    result.stats.elapsedMs = elapsedMs(startTime);
    notify({SearchEventType::SearchFinished, goal, step, result.stats.found ? "Done" : "No path"});
    return result;
}

}  // namespace pathplanner
