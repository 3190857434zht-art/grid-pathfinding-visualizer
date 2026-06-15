#include "api/MapSession.h"

#include "patterns/commands/MacroCommand.h"
#include "patterns/commands/MoveMarkerCommand.h"
#include "patterns/commands/SetCellCommand.h"

#include <stdexcept>

namespace pathplanner {

MapSession& MapSession::instance() {
    static MapSession session;
    return session;
}

nlohmann::json MapSession::buildResponse() const {
    return nlohmann::json{
        {"grid", GridSerializer::gridToJson(grid_)},
        {"canUndo", commands_.canUndo()},
        {"canRedo", commands_.canRedo()},
        {"undoDescription", commands_.undoDescription()},
        {"redoDescription", commands_.redoDescription()},
    };
}

nlohmann::json MapSession::loadGrid(const Grid& grid) {
    grid_ = grid;
    commands_.clear();
    return buildResponse();
}

std::unique_ptr<MapCommand> MapSession::parseCommand(const nlohmann::json& json) const {
    const std::string type = json.at("type").get<std::string>();
    if (type == "setCell") {
        return std::make_unique<SetCellCommand>(
            json.at("row").get<int>(),
            json.at("col").get<int>(),
            static_cast<CellType>(json.at("oldType").get<int>()),
            static_cast<CellType>(json.at("newType").get<int>()));
    }
    if (type == "moveStart") {
        return std::make_unique<MoveMarkerCommand>(
            MarkerKind::Start,
            Point{json.at("oldRow").get<int>(), json.at("oldCol").get<int>()},
            Point{json.at("newRow").get<int>(), json.at("newCol").get<int>()},
            static_cast<CellType>(json.at("oldAtNew").get<int>()));
    }
    if (type == "moveGoal") {
        return std::make_unique<MoveMarkerCommand>(
            MarkerKind::Goal,
            Point{json.at("oldRow").get<int>(), json.at("oldCol").get<int>()},
            Point{json.at("newRow").get<int>(), json.at("newCol").get<int>()},
            static_cast<CellType>(json.at("oldAtNew").get<int>()));
    }
    throw std::invalid_argument("Unknown command type: " + type);
}

nlohmann::json MapSession::applyMacro(const nlohmann::json& commandsJson) {
    if (!commandsJson.is_array() || commandsJson.empty()) {
        throw std::invalid_argument("Commands must be a non-empty array.");
    }

    std::vector<std::unique_ptr<MapCommand>> commands;
    commands.reserve(commandsJson.size());
    for (const auto& item : commandsJson) {
        commands.push_back(parseCommand(item));
    }

    if (commands.size() == 1) {
        commands_.execute(std::move(commands.front()), grid_);
    } else {
        commands_.execute(std::make_unique<MacroCommand>(std::move(commands)), grid_);
    }
    return buildResponse();
}

nlohmann::json MapSession::undo() {
    commands_.undo(grid_);
    return buildResponse();
}

nlohmann::json MapSession::redo() {
    commands_.redo(grid_);
    return buildResponse();
}

nlohmann::json MapSession::stateJson() const {
    return buildResponse();
}

}  // namespace pathplanner
