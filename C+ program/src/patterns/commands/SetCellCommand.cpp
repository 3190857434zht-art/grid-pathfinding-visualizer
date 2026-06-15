#include "patterns/commands/SetCellCommand.h"

#include <sstream>

namespace pathplanner {

SetCellCommand::SetCellCommand(int row, int col, CellType oldType, CellType newType)
    : row_(row), col_(col), oldType_(oldType), newType_(newType) {}

void SetCellCommand::execute(Grid& grid) {
    grid.set(row_, col_, newType_);
}

void SetCellCommand::undo(Grid& grid) {
    grid.set(row_, col_, oldType_);
}

std::string SetCellCommand::description() const {
    std::ostringstream oss;
    oss << "编辑单元 (" << row_ << "," << col_ << ")";
    return oss.str();
}

}  // namespace pathplanner
