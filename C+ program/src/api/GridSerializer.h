#pragma once

#include "core/Grid.h"
#include "core/PathResult.h"

#include <json.hpp>
#include <string>
#include <vector>

namespace pathplanner {

class GridSerializer {
public:
    static nlohmann::json gridToJson(const Grid& grid);
    static Grid gridFromJson(const nlohmann::json& json);
    static nlohmann::json pathResultToJson(const PathResult& result, int algorithmId);
    static nlohmann::json algorithmsToJson();
};

}  // namespace pathplanner
