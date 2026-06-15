#pragma once

#include "core/Grid.h"

namespace pathplanner {

double manhattanHeuristic(const Point& a, const Point& b);
double euclideanHeuristic(const Point& a, const Point& b);

}  // namespace pathplanner
