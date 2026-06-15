#pragma once

#include "core/Grid.h"
#include "core/PathResult.h"
#include "patterns/Observer.h"

namespace pathplanner {

class ConsoleRenderer : public SearchObserver {
public:
    void onSearchEvent(const SearchEvent& event) override;

    void printGrid(const Grid& grid) const;
    void printGridWithSearch(const Grid& grid, const PathResult& result) const;
    void printStats(const PathResult& result) const;
    void printComparison(const std::vector<PathResult>& results) const;

    void setVerbose(bool verbose) { verbose_ = verbose; }

private:
    bool verbose_ = false;
    char cellChar(const Grid& grid, int row, int col, const PathResult* result) const;
};

}  // namespace pathplanner
