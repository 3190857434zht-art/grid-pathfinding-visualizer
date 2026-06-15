#include "ui/HtmlExporter.h"

#include <fstream>
#include <sstream>
#include <unordered_set>

namespace pathplanner {

namespace {

std::unordered_set<int> encodeSet(const Grid& grid, const std::vector<Point>& points) {
    std::unordered_set<int> set;
    for (const Point& p : points) {
        set.insert(p.row * grid.cols() + p.col);
    }
    return set;
}

std::string cellClass(
    const Grid& grid,
    int row,
    int col,
    const std::unordered_set<int>& visited,
    const std::unordered_set<int>& path) {
    const int key = row * grid.cols() + col;
    const CellType type = grid.at(row, col);

    if (type == CellType::Wall) {
        return "wall";
    }
    if (type == CellType::Start) {
        return "start";
    }
    if (type == CellType::Goal) {
        return "goal";
    }
    if (path.count(key) > 0) {
        return "path";
    }
    if (visited.count(key) > 0) {
        return "visited";
    }
    return "empty";
}

void writeGridHtml(
    std::ostringstream& html,
    const Grid& grid,
    const std::unordered_set<int>& visited,
    const std::unordered_set<int>& path,
    int cellSize) {
    html << "<div class=\"grid\" style=\"grid-template-columns: repeat(" << grid.cols()
         << ", " << cellSize << "px);\">\n";

    for (int r = 0; r < grid.rows(); ++r) {
        for (int c = 0; c < grid.cols(); ++c) {
            html << "  <div class=\"cell " << cellClass(grid, r, c, visited, path)
                 << "\" title=\"(" << r << ", " << c << ")\"></div>\n";
        }
    }
    html << "</div>\n";
}

std::string baseStyles() {
    return R"(
body { font-family: "Segoe UI", Arial, sans-serif; background: #0f172a; color: #e2e8f0; margin: 24px; }
h1, h2 { color: #38bdf8; }
.card { background: #1e293b; border-radius: 12px; padding: 20px; margin-bottom: 20px; box-shadow: 0 8px 24px rgba(0,0,0,.25); }
.grid { display: grid; gap: 2px; width: fit-content; margin: 12px 0; }
.cell { width: 24px; height: 24px; border-radius: 4px; }
.empty { background: #334155; }
.wall { background: #0b1220; border: 1px solid #475569; }
.start { background: #22c55e; }
.goal { background: #ef4444; }
.visited { background: #fde047; opacity: 0.85; }
.path { background: #3b82f6; }
.legend span { display: inline-block; margin-right: 16px; }
.legend i { display: inline-block; width: 14px; height: 14px; border-radius: 3px; margin-right: 6px; vertical-align: middle; }
.stats { line-height: 1.8; }
table { border-collapse: collapse; width: 100%; }
th, td { border: 1px solid #475569; padding: 8px 12px; text-align: left; }
th { background: #334155; }
)";
}

}  // namespace

bool HtmlExporter::exportVisualization(
    const Grid& grid,
    const PathResult& result,
    const std::string& outputPath) const {
    std::ofstream out(outputPath);
    if (!out) {
        return false;
    }

    const auto visited = encodeSet(grid, result.visitedOrder);
    const auto path = encodeSet(grid, result.path);

    std::ostringstream body;
    body << "<div class=\"card\">\n";
    body << "<h2>" << result.algorithmName << "</h2>\n";
    body << "<div class=\"stats\">\n";
    body << "结果: " << (result.stats.found ? "找到路径" : "未找到路径") << "<br>\n";
    body << "路径长度: " << result.path.size() << "<br>\n";
    body << "扩展节点: " << result.stats.expandedNodes << "<br>\n";
    body << "访问节点: " << result.stats.visitedNodes << "<br>\n";
    body << "耗时(ms): " << result.stats.elapsedMs << "<br>\n";
    body << "</div>\n";
    writeGridHtml(body, grid, visited, path, 24);
    body << "</div>\n";

    out << "<!DOCTYPE html>\n<html lang=\"zh-CN\">\n<head>\n";
    out << "<meta charset=\"UTF-8\">\n";
    out << "<title>Path Planner - " << result.algorithmName << "</title>\n";
    out << "<style>" << baseStyles() << "</style>\n";
    out << "</head>\n<body>\n";
    out << "<h1>多策略路径规划可视化</h1>\n";
    out << "<div class=\"card legend\">\n";
    out << "<span><i class=\"start\"></i>起点</span>\n";
    out << "<span><i class=\"goal\"></i>终点</span>\n";
    out << "<span><i class=\"wall\"></i>障碍</span>\n";
    out << "<span><i class=\"visited\"></i>已访问</span>\n";
    out << "<span><i class=\"path\"></i>最终路径</span>\n";
    out << "</div>\n";
    out << body.str();
    out << "</body>\n</html>\n";
    return true;
}

bool HtmlExporter::exportComparison(
    const Grid& grid,
    const std::vector<PathResult>& results,
    const std::string& outputPath) const {
    std::ofstream out(outputPath);
    if (!out) {
        return false;
    }

    std::ostringstream body;
    body << "<div class=\"card\">\n<h2>算法性能对比</h2>\n<table>\n";
    body << "<tr><th>算法</th><th>找到路径</th><th>路径长度</th><th>扩展节点</th><th>访问节点</th><th>耗时(ms)</th></tr>\n";
    for (const PathResult& result : results) {
        body << "<tr><td>" << result.algorithmName << "</td><td>"
             << (result.stats.found ? "是" : "否") << "</td><td>"
             << result.path.size() << "</td><td>"
             << result.stats.expandedNodes << "</td><td>"
             << result.stats.visitedNodes << "</td><td>"
             << result.stats.elapsedMs << "</td></tr>\n";
    }
    body << "</table>\n</div>\n";

    for (const PathResult& result : results) {
        const auto visited = encodeSet(grid, result.visitedOrder);
        const auto path = encodeSet(grid, result.path);
        body << "<div class=\"card\">\n<h2>" << result.algorithmName << "</h2>\n";
        writeGridHtml(body, grid, visited, path, 20);
        body << "</div>\n";
    }

    out << "<!DOCTYPE html>\n<html lang=\"zh-CN\">\n<head>\n";
    out << "<meta charset=\"UTF-8\">\n";
    out << "<title>Path Planner - Algorithm Comparison</title>\n";
    out << "<style>" << baseStyles() << "</style>\n";
    out << "</head>\n<body>\n";
    out << "<h1>多策略路径规划 - 算法对比</h1>\n";
    out << body.str();
    out << "</body>\n</html>\n";
    return true;
}

}  // namespace pathplanner
