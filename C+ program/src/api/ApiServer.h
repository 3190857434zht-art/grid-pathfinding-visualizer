#pragma once

#include <string>

namespace pathplanner {

class ApiServer {
public:
    int run(int port, const std::string& webRoot, const std::string& mapsDir);
    static bool openBrowser(const std::string& url);
    static bool checkWebAssets(const std::string& webRoot);
};

}  // namespace pathplanner
