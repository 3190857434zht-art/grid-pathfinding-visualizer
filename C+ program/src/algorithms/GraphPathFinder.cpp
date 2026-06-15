#include "algorithms/GraphPathFinder.h"

#include <algorithm>

namespace pathplanner {

std::vector<Point> GraphPathFinder::reconstructPath(
    const ParentMap& parent, int startKey, int goalKey, int cols) const {
    std::vector<Point> path;
    int current = goalKey;
    path.push_back(decodePoint(current, cols));

    while (current != startKey) {
        const auto it = parent.find(current);
        if (it == parent.end()) {
            return {};
        }
        current = it->second;
        path.push_back(decodePoint(current, cols));
    }

    std::reverse(path.begin(), path.end());
    return path;
}

}  // namespace pathplanner
