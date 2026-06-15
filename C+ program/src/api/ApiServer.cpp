#include "api/ApiServer.h"

#include "api/GridSerializer.h"
#include "api/MapSession.h"
#include "api/PathfindingService.h"
#include "core/MazeGenerator.h"
#include "patterns/AlgorithmFactory.h"

#include <httplib.h>

#include <filesystem>
#include <fstream>
#include <iostream>
#include <sstream>

#ifdef _WIN32
#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif
#include <windows.h>
#endif

namespace pathplanner {

namespace fs = std::filesystem;

namespace {

std::string readTextFile(const std::string& path) {
    std::ifstream in(path, std::ios::binary);
    if (!in) {
        return {};
    }
    std::ostringstream buffer;
    buffer << in.rdbuf();
    return buffer.str();
}

AlgorithmType algorithmFromId(int id) {
    if (!AlgorithmFactory::isValid(id)) {
        throw std::invalid_argument("Invalid algorithm id.");
    }
    return static_cast<AlgorithmType>(id);
}

bool validateGrid(const Grid& grid) {
    return grid.isWalkable(grid.start().row, grid.start().col) &&
           grid.isWalkable(grid.goal().row, grid.goal().col);
}

}  // namespace

bool ApiServer::checkWebAssets(const std::string& webRoot) {
    return fs::exists(webRoot + "/index.html") &&
           fs::exists(webRoot + "/vendor/bootstrap/bootstrap.min.css") &&
           fs::exists(webRoot + "/js/app.js");
}

bool ApiServer::openBrowser(const std::string& url) {
#ifdef _WIN32
    HINSTANCE result = ShellExecuteA(nullptr, "open", url.c_str(), nullptr, nullptr, SW_SHOWNORMAL);
    return reinterpret_cast<intptr_t>(result) > 32;
#else
    const std::string cmd = "xdg-open \"" + url + "\"";
    return std::system(cmd.c_str()) == 0;
#endif
}

int ApiServer::run(int port, const std::string& webRoot, const std::string& mapsDir) {
    if (!checkWebAssets(webRoot)) {
        std::cerr << "Web 资源不完整，请先运行 scripts\\setup_frontend.bat\n";
        return 1;
    }

    httplib::Server server;

    server.set_read_timeout(120, 0);
    server.set_write_timeout(120, 0);
    server.set_default_headers({{"Cache-Control", "no-cache"}});

    server.Get("/", [webRoot](const httplib::Request&, httplib::Response& res) {
        const std::string html = readTextFile(webRoot + "/index.html");
        res.set_content(html, "text/html; charset=utf-8");
    });

    server.Get(R"(/css/(.*))", [webRoot](const httplib::Request& req, httplib::Response& res) {
        const std::string path = webRoot + "/css/" + req.matches[1].str();
        const std::string content = readTextFile(path);
        if (content.empty()) {
            res.status = 404;
            return;
        }
        res.set_content(content, "text/css; charset=utf-8");
    });

    server.Get(R"(/js/(.*))", [webRoot](const httplib::Request& req, httplib::Response& res) {
        const std::string path = webRoot + "/js/" + req.matches[1].str();
        const std::string content = readTextFile(path);
        if (content.empty()) {
            res.status = 404;
            return;
        }
        res.set_content(content, "application/javascript; charset=utf-8");
    });

    server.Get(R"(/vendor/(.*))", [webRoot](const httplib::Request& req, httplib::Response& res) {
        const std::string relative = req.matches[1].str();
        const std::string path = webRoot + "/vendor/" + relative;
        const std::string content = readTextFile(path);
        if (content.empty()) {
            res.status = 404;
            return;
        }

        std::string mime = "application/octet-stream";
        if (relative.find(".css") != std::string::npos) {
            mime = "text/css; charset=utf-8";
        } else if (relative.find(".js") != std::string::npos) {
            mime = "application/javascript; charset=utf-8";
        } else if (relative.find(".woff2") != std::string::npos) {
            mime = "font/woff2";
        }
        res.set_content(content, mime);
    });

    server.Get("/api/algorithms", [](const httplib::Request&, httplib::Response& res) {
        res.set_content(GridSerializer::algorithmsToJson().dump(), "application/json; charset=utf-8");
    });

    server.Get("/api/maps", [mapsDir](const httplib::Request&, httplib::Response& res) {
        nlohmann::json maps = nlohmann::json::array();
        if (fs::exists(mapsDir) && fs::is_directory(mapsDir)) {
            for (const auto& entry : fs::directory_iterator(mapsDir)) {
                if (!entry.is_regular_file()) {
                    continue;
                }
                const std::string filename = entry.path().filename().string();
                if (filename.size() < 4 || filename.substr(filename.size() - 4) != ".txt") {
                    continue;
                }
                maps.push_back({{"id", filename}, {"name", filename}});
            }
        }
        res.set_content(maps.dump(), "application/json; charset=utf-8");
    });

    server.Get(R"(/api/maps/(.+))", [mapsDir](const httplib::Request& req, httplib::Response& res) {
        try {
            const std::string filename = req.matches[1].str();
            const std::string path = mapsDir + "/" + filename;
            if (!fs::exists(path)) {
                res.status = 404;
                res.set_content(R"({"error":"地图不存在"})", "application/json; charset=utf-8");
                return;
            }
            const Grid grid = Grid::fromFile(path);
            const nlohmann::json response = MapSession::instance().loadGrid(grid);
            res.set_content(response.dump(), "application/json; charset=utf-8");
        } catch (const std::exception& ex) {
            res.status = 400;
            res.set_content(std::string(R"({"error":")") + ex.what() + R"("})", "application/json; charset=utf-8");
        }
    });

    server.Post("/api/pathfind", [](const httplib::Request& req, httplib::Response& res) {
        try {
            const nlohmann::json body = nlohmann::json::parse(req.body);
            const int algorithmId = body.at("algorithm").get<int>();
            Grid grid = GridSerializer::gridFromJson(body.at("grid"));

            if (!validateGrid(grid)) {
                res.status = 400;
                res.set_content(R"({"error":"起点或终点不可达，请检查位置"})", "application/json; charset=utf-8");
                return;
            }

            const PathResult result = PathfindingService::run(algorithmFromId(algorithmId), grid);
            nlohmann::json response = GridSerializer::pathResultToJson(result, algorithmId);
            response["grid"] = GridSerializer::gridToJson(grid);
            res.set_content(response.dump(), "application/json; charset=utf-8");
        } catch (const std::exception& ex) {
            res.status = 400;
            res.set_content(std::string(R"({"error":")") + ex.what() + R"("})", "application/json; charset=utf-8");
        }
    });

    server.Post("/api/compare", [](const httplib::Request& req, httplib::Response& res) {
        try {
            const nlohmann::json body = nlohmann::json::parse(req.body);
            Grid grid = GridSerializer::gridFromJson(body.at("grid"));

            if (!validateGrid(grid)) {
                res.status = 400;
                res.set_content(R"({"error":"起点或终点不可达，请检查位置"})", "application/json; charset=utf-8");
                return;
            }

            std::vector<AlgorithmType> selected;
            if (body.contains("algorithms") && body.at("algorithms").is_array()) {
                for (const auto& item : body.at("algorithms")) {
                    const int id = item.get<int>();
                    if (AlgorithmFactory::isValid(id)) {
                        selected.push_back(static_cast<AlgorithmType>(id));
                    }
                }
            }
            if (selected.empty()) {
                selected = AlgorithmFactory::allTypes();
            }

            const std::vector<PathResult> results = PathfindingService::compareSelected(grid, selected);
            nlohmann::json jsonResults = nlohmann::json::array();
            for (std::size_t i = 0; i < results.size(); ++i) {
                const int id = static_cast<int>(selected[i]);
                jsonResults.push_back(GridSerializer::pathResultToJson(results[i], id));
            }

            nlohmann::json response{
                {"grid", GridSerializer::gridToJson(grid)},
                {"results", jsonResults},
            };
            res.set_content(response.dump(), "application/json; charset=utf-8");
        } catch (const std::exception& ex) {
            res.status = 400;
            res.set_content(std::string(R"({"error":")") + ex.what() + R"("})", "application/json; charset=utf-8");
        }
    });

    server.Post("/api/maze/random", [](const httplib::Request& req, httplib::Response& res) {
        try {
            const nlohmann::json body = nlohmann::json::parse(req.body);
            const int rows = body.value("rows", 15);
            const int cols = body.value("cols", 21);
            const unsigned seed = body.value("seed", 0U);

            Grid grid = MazeGenerator::generate(rows, cols, seed);

            const nlohmann::json response = MapSession::instance().loadGrid(grid);
            res.set_content(response.dump(), "application/json; charset=utf-8");
        } catch (const std::exception& ex) {
            res.status = 400;
            res.set_content(std::string(R"({"error":")") + ex.what() + R"("})", "application/json; charset=utf-8");
        }
    });

    server.Post("/api/map/scatter", [](const httplib::Request& req, httplib::Response& res) {
        try {
            const nlohmann::json body = nlohmann::json::parse(req.body);
            Grid grid = GridSerializer::gridFromJson(body.at("grid"));
            const double density = body.value("density", 0.12);
            const unsigned seed = body.value("seed", 0U);

            if (!validateGrid(grid)) {
                res.status = 400;
                res.set_content(R"({"error":"起点或终点不可达"})", "application/json; charset=utf-8");
                return;
            }

            Grid scattered = grid;
            for (unsigned attempt = 0; attempt < 8; ++attempt) {
                scattered = MazeGenerator::addRandomObstacles(grid, density, seed + attempt * 31U);
                if (scattered.rows() != grid.rows() || scattered.cols() != grid.cols()) {
                    break;
                }
                bool changed = false;
                for (int r = 0; r < grid.rows() && !changed; ++r) {
                    for (int c = 0; c < grid.cols(); ++c) {
                        if (scattered.at(r, c) != grid.at(r, c)) {
                            changed = true;
                            break;
                        }
                    }
                }
                if (changed) {
                    break;
                }
            }

            if (!MazeGenerator::isConnected(scattered)) {
                res.status = 400;
                res.set_content(R"({"error":"无法添加障碍：会破坏起点到终点的通路"})", "application/json; charset=utf-8");
                return;
            }

            bool added = false;
            for (int r = 0; r < grid.rows() && !added; ++r) {
                for (int c = 0; c < grid.cols(); ++c) {
                    if (scattered.at(r, c) != grid.at(r, c)) {
                        added = true;
                        break;
                    }
                }
            }
            if (!added) {
                res.status = 400;
                res.set_content(R"({"error":"未能添加障碍，请降低地图障碍密度或重试"})", "application/json; charset=utf-8");
                return;
            }

            const nlohmann::json response = MapSession::instance().loadGrid(scattered);
            res.set_content(response.dump(), "application/json; charset=utf-8");
        } catch (const std::exception& ex) {
            res.status = 400;
            res.set_content(std::string(R"({"error":")") + ex.what() + R"("})", "application/json; charset=utf-8");
        }
    });

    server.Get("/api/session/state", [](const httplib::Request&, httplib::Response& res) {
        res.set_content(MapSession::instance().stateJson().dump(), "application/json; charset=utf-8");
    });

    server.Post("/api/session/load", [](const httplib::Request& req, httplib::Response& res) {
        try {
            const nlohmann::json body = nlohmann::json::parse(req.body);
            Grid grid = GridSerializer::gridFromJson(body.at("grid"));
            res.set_content(MapSession::instance().loadGrid(grid).dump(), "application/json; charset=utf-8");
        } catch (const std::exception& ex) {
            res.status = 400;
            res.set_content(std::string(R"({"error":")") + ex.what() + R"("})", "application/json; charset=utf-8");
        }
    });

    server.Post("/api/session/macro", [](const httplib::Request& req, httplib::Response& res) {
        try {
            const nlohmann::json body = nlohmann::json::parse(req.body);
            res.set_content(MapSession::instance().applyMacro(body.at("commands")).dump(),
                            "application/json; charset=utf-8");
        } catch (const std::exception& ex) {
            res.status = 400;
            res.set_content(std::string(R"({"error":")") + ex.what() + R"("})", "application/json; charset=utf-8");
        }
    });

    server.Post("/api/session/undo", [](const httplib::Request&, httplib::Response& res) {
        res.set_content(MapSession::instance().undo().dump(), "application/json; charset=utf-8");
    });

    server.Post("/api/session/redo", [](const httplib::Request&, httplib::Response& res) {
        res.set_content(MapSession::instance().redo().dump(), "application/json; charset=utf-8");
    });

    server.Post("/api/maps/save", [mapsDir](const httplib::Request& req, httplib::Response& res) {
        try {
            const nlohmann::json body = nlohmann::json::parse(req.body);
            const std::string filename = body.at("filename").get<std::string>();
            if (filename.find("..") != std::string::npos || filename.find('/') != std::string::npos ||
                filename.find('\\') != std::string::npos) {
                throw std::invalid_argument("Invalid filename.");
            }

            Grid grid = GridSerializer::gridFromJson(body.at("grid"));
            fs::create_directories(mapsDir);
            const std::string path = mapsDir + "/" + filename;
            if (!grid.saveToFile(path)) {
                throw std::runtime_error("Failed to save map.");
            }
            res.set_content(R"({"ok":true})", "application/json; charset=utf-8");
        } catch (const std::exception& ex) {
            res.status = 400;
            res.set_content(std::string(R"({"error":")") + ex.what() + R"("})", "application/json; charset=utf-8");
        }
    });

    const std::string url = "http://127.0.0.1:" + std::to_string(port) + "/";
    std::cout << "Web 界面已启动: " << url << '\n';
    std::cout << "按 Ctrl+C 停止服务\n\n";

    openBrowser(url);

    if (!server.listen("127.0.0.1", port)) {
        std::cerr << "无法绑定端口 " << port << "，可能已被占用。\n";
        return 1;
    }
    return 0;
}

}  // namespace pathplanner
