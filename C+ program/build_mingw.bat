@echo off
chcp 65001 >nul
setlocal

cd /d "%~dp0"

if not exist output mkdir output
if not exist build\bin mkdir build\bin

set CXX=g++
where g++ >nul 2>&1
if errorlevel 1 (
    if exist "D:\mingw\MinGW\bin\g++.exe" (
        set "CXX=D:\mingw\MinGW\bin\g++.exe"
    ) else (
        echo 未找到 g++，请安装 MinGW 或 Visual Studio。
        exit /b 1
    )
)

echo 使用 MinGW 编译...
"%CXX%" -std=c++17 -Wall -Wextra ^
    -I src -I third_party/httplib -I third_party/json ^
    src/main.cpp ^
    src/core/Grid.cpp ^
    src/core/MazeGenerator.cpp ^
    src/algorithms/Heuristic.cpp ^
    src/algorithms/GraphPathFinder.cpp ^
    src/algorithms/BFSPathFinder.cpp ^
    src/algorithms/DFSPathFinder.cpp ^
    src/algorithms/DijkstraPathFinder.cpp ^
    src/algorithms/AStarPathFinder.cpp ^
    src/algorithms/GreedyBestFirstPathFinder.cpp ^
    src/algorithms/WeightedAStarPathFinder.cpp ^
    src/algorithms/BidirectionalBFSPathFinder.cpp ^
    src/algorithms/BidirectionalAStarPathFinder.cpp ^
    src/algorithms/IDAStarPathFinder.cpp ^
    src/algorithms/IDDFSPathFinder.cpp ^
    src/patterns/Observer.cpp ^
    src/patterns/PathfindingContext.cpp ^
    src/patterns/AlgorithmFactory.cpp ^
    src/patterns/CommandManager.cpp ^
    src/patterns/commands/SetCellCommand.cpp ^
    src/patterns/commands/MoveMarkerCommand.cpp ^
    src/patterns/commands/MacroCommand.cpp ^
    src/api/PathfindingService.cpp ^
    src/api/GridSerializer.cpp ^
    src/api/MapSession.cpp ^
    src/api/ApiServer.cpp ^
    src/ui/ConsoleRenderer.cpp ^
    src/ui/HtmlExporter.cpp ^
    src/app/Application.cpp ^
    -o build/bin/path_planner.exe -lws2_32

if errorlevel 1 (
    echo 编译失败。
    exit /b 1
)

echo.
echo 编译成功！运行 run.bat 启动 Web 图形界面。
exit /b 0
