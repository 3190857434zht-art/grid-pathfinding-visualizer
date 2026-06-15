#include "app/Application.h"

#include "algorithms/GraphPathFinder.h"
#include "api/ApiServer.h"
#include "api/PathfindingService.h"
#include "patterns/AlgorithmFactory.h"
#include "patterns/PathfindingContext.h"
#include "ui/ConsoleRenderer.h"
#include "ui/HtmlExporter.h"

#include <filesystem>
#include <iostream>
#include <limits>

namespace pathplanner {

namespace fs = std::filesystem;

void Application::printBanner() const {
    std::cout << "========================================\n";
    std::cout << "  多策略路径规划可视化系统 (C++)\n";
    std::cout << "  Strategy: 寻路算法  |  Observer: 搜索进度\n";
    std::cout << "========================================\n\n";
}

void Application::printHelp() const {
    std::cout << "用法:\n";
    std::cout << "  path_planner                 启动 Web 图形界面（默认）\n";
    std::cout << "  path_planner web [port]      指定端口启动 Web 界面\n";
    std::cout << "  path_planner console         控制台交互模式\n";
    std::cout << "  path_planner demo            运行三种算法并对比\n";
    std::cout << "  path_planner run <1|2|3>     运行指定算法 (1=BFS 2=Dijkstra 3=A*)\n";
    std::cout << "  path_planner map <path>      指定地图文件\n";
    std::cout << "  path_planner verbose         输出详细搜索日志\n";
    std::cout << "  path_planner help            显示帮助\n\n";
    std::cout << "地图格式: 第一行 rows cols，之后每格 0=空 1=墙 2=起点 3=终点\n\n";
}

void Application::ensureOutputDir() const {
    fs::create_directories(outputDir_);
}

Grid Application::loadGrid(const std::string& mapPath) const {
    if (fs::exists(mapPath)) {
        return Grid::fromFile(mapPath);
    }

    std::cout << "未找到地图文件 " << mapPath << "，使用内置默认地图。\n";
    Grid grid(12, 16);
    for (int r = 0; r < grid.rows(); ++r) {
        for (int c = 0; c < grid.cols(); ++c) {
            grid.set(r, c, CellType::Empty);
        }
    }

    const int walls[][2] = {
        {2, 3}, {2, 4}, {2, 5}, {2, 6}, {2, 7},
        {4, 8}, {5, 8}, {6, 8}, {7, 8}, {8, 8},
        {6, 2}, {6, 3}, {6, 4}, {6, 5},
        {9, 5}, {9, 6}, {9, 7}, {9, 8}, {9, 9}, {9, 10},
    };
    for (const auto& wall : walls) {
        grid.set(wall[0], wall[1], CellType::Wall);
    }

    grid.setStart(1, 1);
    grid.setGoal(10, 14);
    return grid;
}

PathResult Application::executeAlgorithm(AlgorithmType type, const Grid& grid, bool verbose) {
    PathfindingContext context;
    auto finder = AlgorithmFactory::create(type);
    auto* graphFinder = dynamic_cast<GraphPathFinder*>(finder.get());

    ConsoleRenderer renderer;
    renderer.setVerbose(verbose);
    if (graphFinder != nullptr) {
        graphFinder->addObserver(&renderer);
    }

    context.setStrategy(std::move(finder));
    PathResult result = context.run(grid);

    std::cout << "\n--- " << result.algorithmName << " ---\n";
    renderer.printStats(result);
    renderer.printGridWithSearch(grid, result);
    return result;
}

void Application::exportOutputs(const Grid& grid, const PathResult& result) const {
    ensureOutputDir();
    HtmlExporter exporter;

    const std::string htmlPath = outputDir_ + "/visualization.html";
    if (exporter.exportVisualization(grid, result, htmlPath)) {
        std::cout << "已导出 HTML 可视化: " << htmlPath << '\n';
        std::cout << "用浏览器打开该文件即可查看图形化结果。\n\n";
    }
}

int Application::runSingleAlgorithm(AlgorithmType type, const std::string& mapPath, bool verbose) {
    const Grid grid = loadGrid(mapPath);
    const PathResult result = executeAlgorithm(type, grid, verbose);
    exportOutputs(grid, result);
    return result.stats.found ? 0 : 1;
}

int Application::runDemoAll() {
    const Grid grid = loadGrid(mapPath_);
    std::vector<PathResult> results;

    std::cout << "正在对同一张地图运行全部 10 种算法...\n\n";
    for (AlgorithmType type : AlgorithmFactory::allTypes()) {
        results.push_back(executeAlgorithm(type, grid, false));
    }

    ConsoleRenderer renderer;
    renderer.printComparison(results);

    ensureOutputDir();
    HtmlExporter exporter;
    const std::string comparePath = outputDir_ + "/comparison.html";
    if (exporter.exportComparison(grid, results, comparePath)) {
        std::cout << "已导出对比页面: " << comparePath << '\n';
        std::cout << "答辩演示建议直接打开 comparison.html。\n\n";
    }

    return 0;
}

int Application::runInteractive() {
    const Grid grid = loadGrid(mapPath_);
    ConsoleRenderer renderer;
    renderer.printGrid(grid);

    while (true) {
        std::cout << "请选择算法:\n";
        std::cout << "  1. BFS  2. DFS  3. Dijkstra  4. A*  5. Greedy Best-First\n";
        std::cout << "  6. Weighted A*  7. 双向BFS  8. 双向A*  9. IDA*  10. IDDFS\n";
        std::cout << "  11. 十算法对比  0. 退出\n";
        std::cout << "> ";

        int choice = -1;
        if (!(std::cin >> choice)) {
            std::cin.clear();
            std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
            std::cout << "输入无效，请重试。\n";
            continue;
        }

        if (choice == 0) {
            break;
        }
        if (choice == 11) {
            runDemoAll();
            continue;
        }
        if (choice >= 1 && choice <= 10) {
            const auto type = static_cast<AlgorithmType>(choice);
            PathResult result = executeAlgorithm(type, grid, false);
            exportOutputs(grid, result);
            continue;
        }

        std::cout << "无效选项。\n";
    }

    return 0;
}

std::string Application::resolveWebRoot() const {
    const fs::path cwd = fs::current_path();
    const fs::path direct = cwd / "web";
    if (fs::exists(direct / "index.html")) {
        return direct.string();
    }
    const fs::path parent = cwd.parent_path() / "web";
    if (fs::exists(parent / "index.html")) {
        return parent.string();
    }
    return "web";
}

std::string Application::resolveMapsDir() const {
    const fs::path cwd = fs::current_path();
    if (fs::exists(cwd / "maps")) {
        return (cwd / "maps").string();
    }
    if (fs::exists(cwd.parent_path() / "maps")) {
        return (cwd.parent_path() / "maps").string();
    }
    return "maps";
}

int Application::runWeb(int port) {
    std::cout << "正在启动 Web 图形界面...\n";
    ApiServer server;
    return server.run(port, resolveWebRoot(), resolveMapsDir());
}

int Application::run(int argc, char* argv[]) {
    printBanner();

    AlgorithmType selected = AlgorithmType::AStar;
    bool hasRunCommand = false;
    bool verbose = false;
    bool demoMode = false;
    bool consoleMode = false;
    bool webMode = false;

    for (int i = 1; i < argc; ++i) {
        const std::string arg = argv[i];
        if (arg == "help" || arg == "--help" || arg == "-h") {
            printHelp();
            return 0;
        }
        if (arg == "demo") {
            demoMode = true;
        } else if (arg == "console") {
            consoleMode = true;
        } else if (arg == "web") {
            webMode = true;
            if (i + 1 < argc && argv[i + 1][0] >= '0' && argv[i + 1][0] <= '9') {
                webPort_ = std::stoi(argv[++i]);
            }
        } else if (arg == "verbose") {
            verbose = true;
        } else if (arg == "map" && i + 1 < argc) {
            mapPath_ = argv[++i];
        } else if (arg == "run" && i + 1 < argc) {
            hasRunCommand = true;
            const int id = std::stoi(argv[++i]);
            selected = static_cast<AlgorithmType>(id);
        }
    }

    if (demoMode) {
        return runDemoAll();
    }
    if (hasRunCommand) {
        return runSingleAlgorithm(selected, mapPath_, verbose);
    }
    if (consoleMode) {
        return runInteractive();
    }
    if (webMode || argc == 1) {
        return runWeb(webPort_);
    }

    printHelp();
    return 1;
}

}  // namespace pathplanner
