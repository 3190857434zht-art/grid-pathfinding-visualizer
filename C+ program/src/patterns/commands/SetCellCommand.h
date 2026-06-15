#pragma once

#include "patterns/commands/MapCommand.h"

namespace pathplanner {

class SetCellCommand : public MapCommand {
public:
    SetCellCommand(int row, int col, CellType oldType, CellType newType);

    void execute(Grid& grid) override;
    void undo(Grid& grid) override;
    std::string description() const override;

private:
    int row_;
    int col_;
    CellType oldType_;
    CellType newType_;
};

}  // namespace pathplanner
