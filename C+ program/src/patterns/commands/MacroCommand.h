#pragma once

#include "patterns/commands/MapCommand.h"

#include <memory>
#include <string>
#include <vector>

namespace pathplanner {

class MacroCommand : public MapCommand {
public:
    explicit MacroCommand(std::vector<std::unique_ptr<MapCommand>> commands);

    void execute(Grid& grid) override;
    void undo(Grid& grid) override;
    std::string description() const override;

private:
    std::vector<std::unique_ptr<MapCommand>> commands_;
};

}  // namespace pathplanner
