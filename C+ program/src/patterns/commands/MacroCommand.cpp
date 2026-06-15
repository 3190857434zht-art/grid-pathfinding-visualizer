#include "patterns/commands/MacroCommand.h"

namespace pathplanner {

MacroCommand::MacroCommand(std::vector<std::unique_ptr<MapCommand>> commands)
    : commands_(std::move(commands)) {}

void MacroCommand::execute(Grid& grid) {
    for (const auto& command : commands_) {
        command->execute(grid);
    }
}

void MacroCommand::undo(Grid& grid) {
    for (auto it = commands_.rbegin(); it != commands_.rend(); ++it) {
        (*it)->undo(grid);
    }
}

std::string MacroCommand::description() const {
    if (commands_.size() == 1) {
        return commands_.front()->description();
    }
    return "批量编辑 (" + std::to_string(commands_.size()) + " 步)";
}

}  // namespace pathplanner
