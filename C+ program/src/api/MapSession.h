#pragma once

#include "api/GridSerializer.h"
#include "core/Grid.h"
#include "patterns/CommandManager.h"

#include <json.hpp>
#include <memory>
#include <vector>

namespace pathplanner {

class MapSession {
public:
    static MapSession& instance();

    nlohmann::json loadGrid(const Grid& grid);
    nlohmann::json applyMacro(const nlohmann::json& commandsJson);
    nlohmann::json undo();
    nlohmann::json redo();
    nlohmann::json stateJson() const;

    const Grid& grid() const { return grid_; }

private:
    MapSession() = default;
    nlohmann::json buildResponse() const;
    std::unique_ptr<MapCommand> parseCommand(const nlohmann::json& json) const;

    Grid grid_{15, 21};
    CommandManager commands_;
};

}  // namespace pathplanner
