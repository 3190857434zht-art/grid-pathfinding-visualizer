#include "api/GridSerializer.h"

#include "patterns/AlgorithmFactory.h"

#include <algorithm>
#include <stdexcept>
#include <vector>

namespace pathplanner {

namespace {

nlohmann::json pointToJson(const Point& point) {
    return nlohmann::json{{"row", point.row}, {"col", point.col}};
}

std::vector<Point> subsamplePoints(const std::vector<Point>& points, std::size_t maxPoints) {
    if (points.size() <= maxPoints) {
        return points;
    }
    std::vector<Point> result;
    result.reserve(maxPoints);
    const double step = static_cast<double>(points.size() - 1) / static_cast<double>(maxPoints - 1);
    for (std::size_t i = 0; i < maxPoints; ++i) {
        const std::size_t index = static_cast<std::size_t>(step * static_cast<double>(i) + 0.5);
        result.push_back(points[std::min(index, points.size() - 1)]);
    }
    return result;
}

Point pointFromJson(const nlohmann::json& json) {
    return {json.at("row").get<int>(), json.at("col").get<int>()};
}

}  // namespace

nlohmann::json GridSerializer::gridToJson(const Grid& grid) {
    nlohmann::json cells = nlohmann::json::array();
    for (int r = 0; r < grid.rows(); ++r) {
        nlohmann::json row = nlohmann::json::array();
        for (int c = 0; c < grid.cols(); ++c) {
            row.push_back(static_cast<int>(grid.at(r, c)));
        }
        cells.push_back(row);
    }

    return nlohmann::json{
        {"rows", grid.rows()},
        {"cols", grid.cols()},
        {"cells", cells},
        {"start", pointToJson(grid.start())},
        {"goal", pointToJson(grid.goal())},
    };
}

Grid GridSerializer::gridFromJson(const nlohmann::json& json) {
    const int rows = json.at("rows").get<int>();
    const int cols = json.at("cols").get<int>();
    if (rows < 3 || cols < 3 || rows > 80 || cols > 80) {
        throw std::invalid_argument("Map size must be between 3x3 and 80x80.");
    }

    Grid grid(rows, cols);
    const auto& cells = json.at("cells");
    if (!cells.is_array() || static_cast<int>(cells.size()) != rows) {
        throw std::invalid_argument("Invalid cells data.");
    }

    for (int r = 0; r < rows; ++r) {
        const auto& row = cells.at(r);
        if (!row.is_array() || static_cast<int>(row.size()) != cols) {
            throw std::invalid_argument("Invalid row in cells data.");
        }
        for (int c = 0; c < cols; ++c) {
            const int value = row.at(c).get<int>();
            if (value < 0 || value > 3) {
                throw std::invalid_argument("Invalid cell value.");
            }
            grid.set(r, c, static_cast<CellType>(value));
        }
    }

    if (json.contains("start")) {
        const Point start = pointFromJson(json.at("start"));
        if (grid.isWalkable(start.row, start.col)) {
            if (grid.start() != grid.goal()) {
                grid.set(grid.start().row, grid.start().col, CellType::Empty);
            }
            grid.set(start.row, start.col, CellType::Start);
        }
    }
    if (json.contains("goal")) {
        const Point goal = pointFromJson(json.at("goal"));
        if (grid.isWalkable(goal.row, goal.col)) {
            if (grid.goal() != grid.start()) {
                grid.set(grid.goal().row, grid.goal().col, CellType::Empty);
            }
            grid.set(goal.row, goal.col, CellType::Goal);
        }
    }

    return grid;
}

nlohmann::json GridSerializer::pathResultToJson(const PathResult& result, int algorithmId) {
    static constexpr std::size_t kMaxAnimPoints = 320;
    const std::vector<Point> visitedSample = subsamplePoints(result.visitedOrder, kMaxAnimPoints);

    nlohmann::json visited = nlohmann::json::array();
    for (const Point& p : visitedSample) {
        visited.push_back(pointToJson(p));
    }

    nlohmann::json path = nlohmann::json::array();
    for (const Point& p : result.path) {
        path.push_back(pointToJson(p));
    }

    return nlohmann::json{
        {"algorithmId", algorithmId},
        {"algorithmName", result.algorithmName},
        {"found", result.stats.found},
        {"path", path},
        {"visitedOrder", visited},
        {"stats",
         {{"expandedNodes", result.stats.expandedNodes},
          {"visitedNodes", result.stats.visitedNodes},
          {"elapsedMs", result.stats.elapsedMs},
          {"pathLength", static_cast<int>(result.path.size())},
          {"visitedTotal", static_cast<int>(result.visitedOrder.size())}}},
    };
}

nlohmann::json GridSerializer::algorithmsToJson() {
    nlohmann::json list = nlohmann::json::array();
    const char* descriptions[] = {
        "按层扩展，保证无权图最短步数",
        "深度优先，不保证最优，用于对比",
        "经典单源最短路径，等权网格稳定",
        "启发式 f=g+h，通常扩展节点最少",
        "仅按 h 值扩展，速度快但不保证最优",
        "加权 f=g+w·h，平衡速度与最优性",
        "起点终点同时 BFS，相遇即停",
        "双向 A* 搜索，大图效率更高",
        "迭代加深 + 启发式，省内存",
        "迭代加深 DFS，逐步增加深度限制",
    };
    int i = 0;
    for (AlgorithmType type : AlgorithmFactory::allTypes()) {
        list.push_back(nlohmann::json{
            {"id", static_cast<int>(type)},
            {"name", AlgorithmFactory::nameOf(type)},
            {"description", descriptions[i++]},
        });
    }
    return list;
}

}  // namespace pathplanner
