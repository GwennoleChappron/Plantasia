#pragma once
#include <vector>
#include <string>
#include <ctime>
#include <SFML/System/Vector2.hpp>
#include <nlohmann/json.hpp>

enum class BalconyOrientation { North=0, NorthEast, East, SouthEast, South, SouthWest, West, NorthWest };

enum class WallType { NONE=0, WALL, RAILING }; // WALL=mur plein, RAILING=rambarde

struct GridCell {
    WallType  type        = WallType::NONE;
    float     wallHeight  = 2.5f;   // metres
    float     sunHours    = 0.0f;   
    float     shadowSoft  = 1.0f;   

    // --- NOUVEAU : Données volumétriques (Mises à jour à chaque frame) ---
    float     plantHeight = 0.0f;
    float     plantOpacity= 0.0f;

    bool isWall()    const { return type == WallType::WALL;    }
    bool isRailing() const { return type == WallType::RAILING; }
    bool isObstacle()const { return type != WallType::NONE;    }
};

struct BalconyConfig {
    int   width  = 40;
    int   height = 30;

    float latitude   = 48.5f;
    float longitude  = 7.75f;
    BalconyOrientation orientation = BalconyOrientation::South;

    int   year  = 2026;
    int   month = 3;
    int   day   = 27;
    float hour  = 12.0f;

    bool  hasRoof       = false;
    float roofHeight    = 2.5f;   // hauteur du toit en metres
    float defaultWallHeight    = 2.5f;
    float defaultRailingHeight = 0.9f;

    std::vector<std::vector<GridCell>> grid;

    void syncRealDate() {
        std::time_t t = std::time(nullptr);
        std::tm tmBuf{};
        localtime_s(&tmBuf, &t);
        std::tm* tm = &tmBuf;
        year  = tm->tm_year + 1900;
        month = tm->tm_mon  + 1;
        day   = tm->tm_mday;
    }

    void resize(int w, int h) {
        // Conserve les cellules existantes
        std::vector<std::vector<GridCell>> newGrid(h, std::vector<GridCell>(w));
        for (int r = 0; r < std::min(h, height); ++r)
            for (int c = 0; c < std::min(w, width); ++c)
                newGrid[r][c] = grid[r][c];
        grid   = std::move(newGrid);
        width  = w;
        height = h;
    }
};

inline void to_json(nlohmann::json& j, const BalconyConfig& cfg) {
    j = nlohmann::json{
        {"width",               cfg.width},
        {"height",              cfg.height},
        {"orientation",         static_cast<int>(cfg.orientation)},
        {"hasRoof",             cfg.hasRoof},
        {"roofHeight",          cfg.roofHeight},
        {"defaultWallHeight",   cfg.defaultWallHeight},
        {"defaultRailingHeight",cfg.defaultRailingHeight},
        {"latitude",            cfg.latitude}
    };

    std::vector<uint64_t> mask_wall, mask_rail;
    std::vector<float>    heights;

    for (int r = 0; r < cfg.height; ++r) {
        uint64_t bw = 0, br = 0;
        for (int c = 0; c < cfg.width; ++c) {
            const auto& cell = cfg.grid[r][c];
            if (cell.isWall())    { bw |= (1ULL<<c); heights.push_back(cell.wallHeight); }
            if (cell.isRailing()) { br |= (1ULL<<c); heights.push_back(cell.wallHeight); }
        }
        mask_wall.push_back(bw);
        mask_rail.push_back(br);
    }
    j["mask_wall"]    = mask_wall;
    j["mask_rail"]    = mask_rail;
    j["wall_heights"] = heights;
}

inline void from_json(const nlohmann::json& j, BalconyConfig& cfg) {
    cfg.width               = j.value("width",               40);
    cfg.height              = j.value("height",              30);
    cfg.orientation         = static_cast<BalconyOrientation>(j.value("orientation", 4));
    cfg.hasRoof             = j.value("hasRoof",             false);
    cfg.roofHeight          = j.value("roofHeight",          2.5f);
    cfg.defaultWallHeight   = j.value("defaultWallHeight",   2.5f);
    cfg.defaultRailingHeight= j.value("defaultRailingHeight",0.9f);
    cfg.latitude            = j.value("latitude",            48.5f);

    cfg.grid.assign(cfg.height, std::vector<GridCell>(cfg.width));

    auto mask_wall = j.value("mask_wall", std::vector<uint64_t>());
    auto mask_rail = j.value("mask_rail", std::vector<uint64_t>());
    auto heights   = j.value("wall_heights", std::vector<float>());
    int hi = 0;

    for (int r = 0; r < cfg.height; ++r) {
        uint64_t bw = (r < (int)mask_wall.size()) ? mask_wall[r] : 0;
        uint64_t br = (r < (int)mask_rail.size()) ? mask_rail[r] : 0;
        for (int c = 0; c < cfg.width; ++c) {
            auto& cell = cfg.grid[r][c];
            if (bw & (1ULL<<c)) {
                cell.type       = WallType::WALL;
                cell.wallHeight = (hi < (int)heights.size()) ? heights[hi++] : cfg.defaultWallHeight;
            } else if (br & (1ULL<<c)) {
                cell.type       = WallType::RAILING;
                cell.wallHeight = (hi < (int)heights.size()) ? heights[hi++] : cfg.defaultRailingHeight;
            }
        }
    }
}