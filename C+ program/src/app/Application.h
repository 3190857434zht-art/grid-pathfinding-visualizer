#pragma once

#include "core/Grid.h"
#include "core/PathResult.h"
#include "patterns/AlgorithmFactory.h"

#include <string>
#include <vector>

namespace pathplanner {

class Application {
public:
    int run(int argc, char* argv[]);

private:
    void printBanner() const;
    void printHelp() const;
    int runInteractive();
    int runDemoAll();
    int runWeb(int port);
    int runSingleAlgorithm(AlgorithmType type, const std::string& mapPath, bool verbose);

    Grid loadGrid(const std::string& mapPath) const;
    PathResult executeAlgorithm(AlgorithmType type, const Grid& grid, bool verbose);
    void exportOutputs(const Grid& grid, const PathResult& result) const;
    void ensureOutputDir() const;
    std::string resolveWebRoot() const;
    std::string resolveMapsDir() const;

    std::string mapPath_ = "maps/sample_map.txt";
    std::string outputDir_ = "output";
    int webPort_ = 8765;
};

}  // namespace pathplanner
