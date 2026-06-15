#include "patterns/CommandManager.h"

#include <algorithm>

namespace pathplanner {

void CommandManager::execute(std::unique_ptr<MapCommand> command, Grid& grid) {
    command->execute(grid);
    undoStack_.push_back(std::move(command));
    redoStack_.clear();
    if (undoStack_.size() > kMaxHistory) {
        undoStack_.erase(undoStack_.begin());
    }
}

void CommandManager::undo(Grid& grid) {
    if (undoStack_.empty()) {
        return;
    }
    std::unique_ptr<MapCommand> command = std::move(undoStack_.back());
    undoStack_.pop_back();
    command->undo(grid);
    redoStack_.push_back(std::move(command));
}

void CommandManager::redo(Grid& grid) {
    if (redoStack_.empty()) {
        return;
    }
    std::unique_ptr<MapCommand> command = std::move(redoStack_.back());
    redoStack_.pop_back();
    command->execute(grid);
    undoStack_.push_back(std::move(command));
}

void CommandManager::clear() {
    undoStack_.clear();
    redoStack_.clear();
}

std::string CommandManager::undoDescription() const {
    return undoStack_.empty() ? "" : undoStack_.back()->description();
}

std::string CommandManager::redoDescription() const {
    return redoStack_.empty() ? "" : redoStack_.back()->description();
}

}  // namespace pathplanner
