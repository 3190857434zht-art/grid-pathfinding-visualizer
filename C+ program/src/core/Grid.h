#pragma once

#include <cstddef>
#include <string>
#include <utility>
#include <vector>

namespace pathplanner {

enum class CellType {
    Empty = 0,
    Wall = 1,
    Start = 2,
    Goal = 3
};

struct Point {
    int row = 0;
    int col = 0;

    bool operator==(const Point& other) const {
        return row == other.row && col == other.col;
    }

    bool operator!=(const Point& other) const {
        return !(*this == other);
    }
};

class Grid {
public:
    Grid(int rows, int cols);

    int rows() const { return rows_; }
    int cols() const { return cols_; }

    CellType at(int row, int col) const;
    void set(int row, int col, CellType type);

    Point start() const { return start_; }
    Point goal() const { return goal_; }

    void setStart(int row, int col);
    void setGoal(int row, int col);

    bool inBounds(int row, int col) const;
    bool isWalkable(int row, int col) const;

    static Grid fromFile(const std::string& path);
    bool saveToFile(const std::string& path) const;

    std::vector<Point> neighbors4(int row, int col) const;
    std::vector<Point> neighbors8(int row, int col) const;

private:
    int rows_;
    int cols_;
    std::vector<CellType> cells_;
    Point start_{0, 0};
    Point goal_{0, 0};

    int index(int row, int col) const;
};

}  // namespace pathplanner
