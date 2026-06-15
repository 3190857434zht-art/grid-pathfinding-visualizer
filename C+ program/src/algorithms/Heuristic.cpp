#include "algorithms/Heuristic.h"

#include <cmath>

namespace pathplanner {

double manhattanHeuristic(const Point& a, const Point& b) {
    return static_cast<double>(std::abs(a.row - b.row) + std::abs(a.col - b.col));
}

double euclideanHeuristic(const Point& a, const Point& b) {
    const double dr = static_cast<double>(a.row - b.row);
    const double dc = static_cast<double>(a.col - b.col);
    return std::sqrt(dr * dr + dc * dc);
}

}  // namespace pathplanner
