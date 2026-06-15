#pragma once

#include "core/Grid.h"
#include "patterns/commands/MapCommand.h"

#include <memory>
#include <string>
#include <vector>

namespace pathplanner {

class CommandManager {
public:
    void execute(std::unique_ptr<MapCommand> command, Grid& grid);
    void undo(Grid& grid);
    void redo(Grid& grid);
    void clear();

    bool canUndo() const { return !undoStack_.empty(); }
    bool canRedo() const { return !redoStack_.empty(); }
    std::string undoDescription() const;
    std::string redoDescription() const;

private:
    static constexpr std::size_t kMaxHistory = 120;
    std::vector<std::unique_ptr<MapCommand>> undoStack_;
    std::vector<std::unique_ptr<MapCommand>> redoStack_;
};

}  // namespace pathplanner
