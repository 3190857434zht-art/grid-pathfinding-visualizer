#include "algorithms/BidirectionalAStarPathFinder.h"

#include "algorithms/Heuristic.h"

#include <queue>
#include <unordered_map>
#include <unordered_set>

namespace pathplanner {

namespace {

struct BiNode {
    int key = 0;
    double f = 0.0;
    bool operator>(const BiNode& other) const { return f > other.f; }
};

}  // namespace

double BidirectionalAStarPathFinder::heuristic(const Point& a, const Point& b) {
    return manhattanHeuristic(a, b);
}

std::vector<Point> BidirectionalAStarPathFinder::mergePaths(
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

PathResult BidirectionalAStarPathFinder::findPath(const Grid& grid) {
    const auto startTime = std::chrono::steady_clock::now();
    PathResult result;
    result.algorithmName = name();

    const Point start = grid.start();
    const Point goal = grid.goal();
    const int cols = grid.cols();
    const int startKey = encodePoint(start.row, start.col, cols);
    const int goalKey = encodePoint(goal.row, goal.col, cols);

    std::priority_queue<BiNode, std::vector<BiNode>, std::greater<BiNode>> forwardQ;
    std::priority_queue<BiNode, std::vector<BiNode>, std::greater<BiNode>> backwardQ;
    ParentMap forwardParent;
    ParentMap backwardParent;
    std::unordered_map<int, double> forwardG;
    std::unordered_map<int, double> backwardG;
    std::unordered_set<int> forwardClosed;
    std::unordered_set<int> backwardClosed;

    forwardG[startKey] = 0.0;
    backwardG[goalKey] = 0.0;
    forwardQ.push({startKey, heuristic(start, goal)});
    backwardQ.push({goalKey, heuristic(goal, start)});

    int step = 0;
    int meetKey = -1;

    while (!forwardQ.empty() && !backwardQ.empty()) {
        auto expandOne = [&](std::priority_queue<BiNode, std::vector<BiNode>, std::greater<BiNode>>& pq,
                             ParentMap& parent, std::unordered_map<int, double>& gScore,
                             std::unordered_set<int>& closed, const std::unordered_set<int>& otherClosed,
                             const Point& /*source*/, const Point& target, bool forward) {
            while (!pq.empty()) {
                const BiNode node = pq.top();
                pq.pop();
                if (closed.count(node.key) > 0) {
                    continue;
                }
                closed.insert(node.key);
                ++result.stats.expandedNodes;

                const Point current = decodePoint(node.key, cols);
                result.visitedOrder.push_back(current);
                notify({SearchEventType::NodeExpanded, current, step++, forward ? "BiA* forward" : "BiA* backward"});

                if (otherClosed.count(node.key) > 0) {
                    meetKey = node.key;
                    return true;
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
                        const double f = tentativeG + heuristic(next, target);
                        pq.push({nextKey, f});
                        ++result.stats.visitedNodes;
                        notify({SearchEventType::NodeVisited, next, step, "BiA* queued"});
                        if (otherClosed.count(nextKey) > 0) {
                            meetKey = nextKey;
                            return true;
                        }
                    }
                }
                return false;
            }
            return false;
        };

        if (forwardQ.size() <= backwardQ.size()) {
            if (expandOne(forwardQ, forwardParent, forwardG, forwardClosed, backwardClosed, start, goal, true)) {
                break;
            }
        } else {
            if (expandOne(backwardQ, backwardParent, backwardG, backwardClosed, forwardClosed, goal, start, false)) {
                break;
            }
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
