#include "core/MazeGenerator.h"

#include <algorithm>
#include <cmath>
#include <queue>
#include <random>
#include <stack>
#include <unordered_set>

namespace pathplanner {

namespace {

unsigned resolveSeed(unsigned seed) {
    if (seed == 0) {
        std::random_device rd;
        return rd();
    }
    return seed;
}

Point findFarthestWalkable(const Grid& grid, const Point& origin) {
    std::queue<std::pair<Point, int>> frontier;
    std::unordered_set<int> visited;
    const int cols = grid.cols();

    frontier.push({origin, 0});
    visited.insert(origin.row * cols + origin.col);

    Point farthest = origin;
    int maxDist = 0;

    while (!frontier.empty()) {
        const auto [current, dist] = frontier.front();
        frontier.pop();

        if (dist >= maxDist) {
            maxDist = dist;
            farthest = current;
        }

        for (const Point& next : grid.neighbors4(current.row, current.col)) {
            const int key = next.row * cols + next.col;
            if (visited.insert(key).second) {
                frontier.push({next, dist + 1});
            }
        }
    }
    return farthest;
}

void placeStartGoal(Grid& grid) {
    Point start{1, 1};
    if (!grid.isWalkable(start.row, start.col)) {
        for (int r = 1; r < grid.rows() - 1; ++r) {
            for (int c = 1; c < grid.cols() - 1; ++c) {
                if (grid.isWalkable(r, c)) {
                    start = {r, c};
                    goto found_start;
                }
            }
        }
    }
found_start:
    const Point goal = findFarthestWalkable(grid, start);

    if (start.row != goal.row || start.col != goal.col) {
        grid.set(start.row, start.col, CellType::Empty);
    }
    grid.set(start.row, start.col, CellType::Start);

    if (goal.row != start.row || goal.col != start.col) {
        grid.set(goal.row, goal.col, CellType::Empty);
    }
    grid.set(goal.row, goal.col, CellType::Goal);
}

}  // namespace

bool MazeGenerator::isConnected(const Grid& grid) {
    const Point start = grid.start();
    const Point goal = grid.goal();
    if (!grid.isWalkable(start.row, start.col) || !grid.isWalkable(goal.row, goal.col)) {
        return false;
    }

    std::queue<Point> frontier;
    std::unordered_set<int> visited;
    const int cols = grid.cols();

    frontier.push(start);
    visited.insert(start.row * cols + start.col);

    while (!frontier.empty()) {
        const Point current = frontier.front();
        frontier.pop();
        if (current.row == goal.row && current.col == goal.col) {
            return true;
        }
        for (const Point& next : grid.neighbors4(current.row, current.col)) {
            const int key = next.row * cols + next.col;
            if (visited.insert(key).second) {
                frontier.push(next);
            }
        }
    }
    return false;
}

Grid MazeGenerator::generate(int rows, int cols, unsigned seed) {
    rows = std::max(rows, 5);
    cols = std::max(cols, 5);

    Grid grid(rows, cols);
    for (int r = 0; r < rows; ++r) {
        for (int c = 0; c < cols; ++c) {
            grid.set(r, c, CellType::Wall);
        }
    }

    std::mt19937 rng(resolveSeed(seed));
    std::stack<Point> stack;
    const int startR = 1;
    const int startC = 1;
    grid.set(startR, startC, CellType::Empty);
    stack.push({startR, startC});

    static constexpr int kDr[] = {-2, 2, 0, 0};
    static constexpr int kDc[] = {0, 0, -2, 2};

    while (!stack.empty()) {
        const Point current = stack.top();
        std::vector<int> directions = {0, 1, 2, 3};
        std::shuffle(directions.begin(), directions.end(), rng);

        bool carved = false;
        for (int dir : directions) {
            const int nr = current.row + kDr[dir];
            const int nc = current.col + kDc[dir];
            if (nr <= 0 || nr >= rows - 1 || nc <= 0 || nc >= cols - 1) {
                continue;
            }
            if (grid.at(nr, nc) == CellType::Wall) {
                grid.set(current.row + kDr[dir] / 2, current.col + kDc[dir] / 2, CellType::Empty);
                grid.set(nr, nc, CellType::Empty);
                stack.push({nr, nc});
                carved = true;
                break;
            }
        }
        if (!carved) {
            stack.pop();
        }
    }

    placeStartGoal(grid);
    return grid;
}

Grid MazeGenerator::addRandomObstacles(const Grid& base, double density, unsigned seed) {
    Grid grid = base;
    density = std::clamp(density, 0.0, 0.45);

    std::mt19937 rng(resolveSeed(seed));
    std::uniform_real_distribution<double> dist(0.0, 1.0);

    const Point start = grid.start();
    const Point goal = grid.goal();

    for (int r = 0; r < grid.rows(); ++r) {
        for (int c = 0; c < grid.cols(); ++c) {
            if ((r == start.row && c == start.col) || (r == goal.row && c == goal.col)) {
                continue;
            }
            if (grid.at(r, c) != CellType::Wall && dist(rng) < density) {
                grid.set(r, c, CellType::Wall);
            }
        }
    }

    if (!MazeGenerator::isConnected(grid)) {
        return base;
    }
    return grid;
}

}  // namespace pathplanner
