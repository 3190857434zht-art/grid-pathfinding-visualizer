#pragma once

#include "core/Grid.h"

namespace pathplanner {

class MazeGenerator {
public:
    static Grid generate(int rows, int cols, unsigned seed = 0);
    static Grid addRandomObstacles(const Grid& base, double density, unsigned seed = 0);
    static bool isConnected(const Grid& grid);
};

}  // namespace pathplanner
