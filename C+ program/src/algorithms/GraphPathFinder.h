#pragma once

#include "core/Grid.h"
#include "core/PathFinder.h"
#include "patterns/Observer.h"

#include <chrono>
#include <unordered_map>
#include <vector>

namespace pathplanner {

class GraphPathFinder : public PathFinder, protected SearchSubject {
public:
    void addObserver(SearchObserver* observer) { SearchSubject::addObserver(observer); }
    void removeObserver(SearchObserver* observer) { SearchSubject::removeObserver(observer); }

protected:
    using ParentMap = std::unordered_map<int, int>;

    static int encodePoint(int row, int col, int cols) {
        return row * cols + col;
    }

    static Point decodePoint(int encoded, int cols) {
        return {encoded / cols, encoded % cols};
    }

    std::vector<Point> reconstructPath(const ParentMap& parent, int startKey, int goalKey, int cols) const;

    template <typename Clock = std::chrono::steady_clock>
    double elapsedMs(const typename Clock::time_point& start) const {
        const auto end = Clock::now();
        return std::chrono::duration<double, std::milli>(end - start).count();
    }
};

}  // namespace pathplanner
