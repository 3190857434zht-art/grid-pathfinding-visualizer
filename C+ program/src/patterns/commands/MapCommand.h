#pragma once

#include "core/Grid.h"

namespace pathplanner {

class MapCommand {
public:
    virtual ~MapCommand() = default;
    virtual void execute(Grid& grid) = 0;
    virtual void undo(Grid& grid) = 0;
    virtual std::string description() const = 0;
};

}  // namespace pathplanner
