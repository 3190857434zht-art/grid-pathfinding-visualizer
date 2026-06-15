#include "ui/ConsoleRenderer.h"

#include <iostream>
#include <unordered_set>

namespace pathplanner {

namespace {

bool isOnPath(const PathResult* result, int row, int col) {
    if (result == nullptr) {
        return false;
    }
    for (const Point& p : result->path) {
        if (p.row == row && p.col == col) {
            return true;
        }
    }
    return false;
}

bool isVisited(const PathResult* result, int row, int col) {
    if (result == nullptr) {
        return false;
    }
    for (const Point& p : result->visitedOrder) {
        if (p.row == row && p.col == col) {
            return true;
        }
    }
    return false;
}

}  // namespace

void ConsoleRenderer::onSearchEvent(const SearchEvent& event) {
    if (!verbose_) {
        return;
    }

    switch (event.type) {
        case SearchEventType::NodeVisited:
            std::cout << "[Visited] (" << event.point.row << ", " << event.point.col << ")\n";
            break;
        case SearchEventType::NodeExpanded:
            std::cout << "[Expanded] (" << event.point.row << ", " << event.point.col << ")\n";
            break;
        case SearchEventType::PathFound:
            std::cout << "[Path Found] goal reached at step " << event.step << "\n";
            break;
        case SearchEventType::SearchFinished:
            std::cout << "[Finished] " << event.message << "\n";
            break;
    }
}

char ConsoleRenderer::cellChar(const Grid& grid, int row, int col, const PathResult* result) const {
    const CellType type = grid.at(row, col);
    if (type == CellType::Wall) {
        return '#';
    }
    if (type == CellType::Start) {
        return 'S';
    }
    if (type == CellType::Goal) {
        return 'G';
    }
    if (isOnPath(result, row, col)) {
        return '*';
    }
    if (isVisited(result, row, col)) {
        return '.';
    }
    return ' ';
}

void ConsoleRenderer::printGrid(const Grid& grid) const {
    printGridWithSearch(grid, PathResult{});
}

void ConsoleRenderer::printGridWithSearch(const Grid& grid, const PathResult& result) const {
    const PathResult* ptr = result.stats.found || !result.visitedOrder.empty() ? &result : nullptr;

    std::cout << "\n";
    for (int r = 0; r < grid.rows(); ++r) {
        for (int c = 0; c < grid.cols(); ++c) {
            std::cout << cellChar(grid, r, c, ptr);
        }
        std::cout << '\n';
    }
    std::cout << "图例: S=起点 G=终点 #=障碍 *=路径 .=已访问\n\n";
}

void ConsoleRenderer::printStats(const PathResult& result) const {
    std::cout << "算法: " << result.algorithmName << '\n';
    std::cout << "结果: " << (result.stats.found ? "找到路径" : "未找到路径") << '\n';
    std::cout << "路径长度: " << result.path.size() << '\n';
    std::cout << "扩展节点: " << result.stats.expandedNodes << '\n';
    std::cout << "访问节点: " << result.stats.visitedNodes << '\n';
    std::cout << "耗时(ms): " << result.stats.elapsedMs << "\n\n";
}

void ConsoleRenderer::printComparison(const std::vector<PathResult>& results) const {
    std::cout << "\n========== 算法对比 ==========\n";
    std::cout << "算法\t\t\t找到\t路径\t扩展\t访问\t耗时(ms)\n";
    for (const PathResult& result : results) {
        std::cout << result.algorithmName << '\t'
                  << (result.stats.found ? "是" : "否") << '\t'
                  << result.path.size() << '\t'
                  << result.stats.expandedNodes << '\t'
                  << result.stats.visitedNodes << '\t'
                  << result.stats.elapsedMs << '\n';
    }
    std::cout << "==============================\n\n";
}

}  // namespace pathplanner
