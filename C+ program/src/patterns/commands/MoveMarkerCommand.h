#pragma once

#include "patterns/commands/MapCommand.h"

namespace pathplanner {

enum class MarkerKind { Start, Goal };

class MoveMarkerCommand : public MapCommand {
public:
    MoveMarkerCommand(MarkerKind kind, Point oldPos, Point newPos, CellType oldAtNew);

    void execute(Grid& grid) override;
    void undo(Grid& grid) override;
    std::string description() const override;

private:
    MarkerKind kind_;
    Point oldPos_;
    Point newPos_;
    CellType oldAtNew_;
};

}  // namespace pathplanner
