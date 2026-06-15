#include "patterns/commands/MoveMarkerCommand.h"

#include <sstream>

namespace pathplanner {

MoveMarkerCommand::MoveMarkerCommand(MarkerKind kind, Point oldPos, Point newPos, CellType oldAtNew)
    : kind_(kind), oldPos_(oldPos), newPos_(newPos), oldAtNew_(oldAtNew) {}

void MoveMarkerCommand::execute(Grid& grid) {
    if (kind_ == MarkerKind::Start) {
        if (grid.start() != grid.goal()) {
            grid.set(grid.start().row, grid.start().col, CellType::Empty);
        }
        grid.setStart(newPos_.row, newPos_.col);
    } else {
        if (grid.goal() != grid.start()) {
            grid.set(grid.goal().row, grid.goal().col, CellType::Empty);
        }
        grid.setGoal(newPos_.row, newPos_.col);
    }
}

void MoveMarkerCommand::undo(Grid& grid) {
    if (kind_ == MarkerKind::Start) {
        grid.set(newPos_.row, newPos_.col, oldAtNew_);
        grid.setStart(oldPos_.row, oldPos_.col);
    } else {
        grid.set(newPos_.row, newPos_.col, oldAtNew_);
        grid.setGoal(oldPos_.row, oldPos_.col);
    }
}

std::string MoveMarkerCommand::description() const {
    return kind_ == MarkerKind::Start ? "移动起点" : "移动终点";
}

}  // namespace pathplanner
