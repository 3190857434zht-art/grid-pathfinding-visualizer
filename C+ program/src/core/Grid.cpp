#include "core/Grid.h"

#include <fstream>
#include <sstream>
#include <stdexcept>

namespace pathplanner {

Grid::Grid(int rows, int cols)
    : rows_(rows), cols_(cols), cells_(static_cast<std::size_t>(rows * cols), CellType::Empty) {
    if (rows <= 0 || cols <= 0) {
        throw std::invalid_argument("Grid size must be positive.");
    }
    setStart(0, 0);
    setGoal(rows - 1, cols - 1);
}

int Grid::index(int row, int col) const {
    return row * cols_ + col;
}

CellType Grid::at(int row, int col) const {
    if (!inBounds(row, col)) {
        throw std::out_of_range("Grid coordinate out of bounds.");
    }
    return cells_[static_cast<std::size_t>(index(row, col))];
}

void Grid::set(int row, int col, CellType type) {
    if (!inBounds(row, col)) {
        throw std::out_of_range("Grid coordinate out of bounds.");
    }
    cells_[static_cast<std::size_t>(index(row, col))] = type;
    if (type == CellType::Start) {
        start_ = {row, col};
    } else if (type == CellType::Goal) {
        goal_ = {row, col};
    }
}

void Grid::setStart(int row, int col) {
    if (!inBounds(row, col) || !isWalkable(row, col)) {
        throw std::invalid_argument("Invalid start position.");
    }
    if (start_ != goal_) {
        set(start_.row, start_.col, CellType::Empty);
    }
    set(row, col, CellType::Start);
}

void Grid::setGoal(int row, int col) {
    if (!inBounds(row, col) || !isWalkable(row, col)) {
        throw std::invalid_argument("Invalid goal position.");
    }
    if (goal_ != start_) {
        set(goal_.row, goal_.col, CellType::Empty);
    }
    set(row, col, CellType::Goal);
}

bool Grid::inBounds(int row, int col) const {
    return row >= 0 && row < rows_ && col >= 0 && col < cols_;
}

bool Grid::isWalkable(int row, int col) const {
    if (!inBounds(row, col)) {
        return false;
    }
    const CellType type = at(row, col);
    return type != CellType::Wall;
}

Grid Grid::fromFile(const std::string& path) {
    std::ifstream in(path);
    if (!in) {
        throw std::runtime_error("Cannot open map file: " + path);
    }

    int rows = 0;
    int cols = 0;
    in >> rows >> cols;
    if (rows <= 0 || cols <= 0) {
        throw std::runtime_error("Invalid map dimensions in file: " + path);
    }

    Grid grid(rows, cols);
    for (int r = 0; r < rows; ++r) {
        for (int c = 0; c < cols; ++c) {
            int value = 0;
            in >> value;
            grid.set(r, c, static_cast<CellType>(value));
        }
    }
    return grid;
}

bool Grid::saveToFile(const std::string& path) const {
    std::ofstream out(path);
    if (!out) {
        return false;
    }

    out << rows_ << ' ' << cols_ << '\n';
    for (int r = 0; r < rows_; ++r) {
        for (int c = 0; c < cols_; ++c) {
            out << static_cast<int>(at(r, c));
            if (c + 1 < cols_) {
                out << ' ';
            }
        }
        out << '\n';
    }
    return true;
}

std::vector<Point> Grid::neighbors4(int row, int col) const {
    static constexpr int kDr[] = {-1, 1, 0, 0};
    static constexpr int kDc[] = {0, 0, -1, 1};

    std::vector<Point> result;
    result.reserve(4);
    for (int i = 0; i < 4; ++i) {
        const int nr = row + kDr[i];
        const int nc = col + kDc[i];
        if (isWalkable(nr, nc)) {
            result.push_back({nr, nc});
        }
    }
    return result;
}

std::vector<Point> Grid::neighbors8(int row, int col) const {
    std::vector<Point> result;
    result.reserve(8);
    for (int dr = -1; dr <= 1; ++dr) {
        for (int dc = -1; dc <= 1; ++dc) {
            if (dr == 0 && dc == 0) {
                continue;
            }
            const int nr = row + dr;
            const int nc = col + dc;
            if (isWalkable(nr, nc)) {
                result.push_back({nr, nc});
            }
        }
    }
    return result;
}

}  // namespace pathplanner
