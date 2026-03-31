#pragma once
#include <vector>
#include <string>
#include <ctime>
#include <SFML/System/Vector2.hpp>
#include <nlohmann/json.hpp>

enum class BalconyOrientation { North=0, NorthEast, East, SouthEast, South, SouthWest, West, NorthWest };

enum class WallType { NONE=0, WALL, RAILING, GLASS }; // NOUVEAU: GLASS

struct GridCell {
    WallType  type        = WallType::NONE;
    float     wallHeight  = 2.5f;   // metres
    float     sunHours    = 0.0f;   
    float     shadowSoft  = 1.0f;   

    float     plantHeight = 0.0f;
    float     plantOpacity= 0.0f;

    bool isWall()    const { return type == WallType::WALL;    }
    bool isRailing() const { return type == WallType::RAILING; }
    bool isGlass()   const { return type == WallType::GLASS;   } // NOUVEAU
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
    float roofHeight    = 2.5f; 
    float defaultWallHeight    = 2.5f;
    float defaultRailingHeight = 0.9f;
    float defaultGlassHeight   = 2.5f; // NOUVEAU

    std::vector<std::vector<GridCell>> grid;

    void syncRealDate() {
        std::time_t t = std::time(nullptr);
        std::tm tmBuf{};
        #if defined(_WIN32)
            localtime_s(&tmBuf, &t);
        #else
            localtime_r(&t, &tmBuf);
        #endif
        year  = tmBuf.tm_year + 1900;
        month = tmBuf.tm_mon  + 1;
        day   = tmBuf.tm_mday;
    }

    void resize(int w, int h) {
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
        {"defaultGlassHeight",  cfg.defaultGlassHeight},
        {"latitude",            cfg.latitude}
    };

    std::vector<uint64_t> mask_wall, mask_rail, mask_glass;
    std::vector<float>    heights;

    int total_cells = cfg.width * cfg.height;
    int num_blocks = (total_cells + 63) / 64; 
    mask_wall.assign(num_blocks, 0);
    mask_rail.assign(num_blocks, 0);
    mask_glass.assign(num_blocks, 0);

    for (int r = 0; r < cfg.height; ++r) {
        for (int c = 0; c < cfg.width; ++c) {
            int idx = r * cfg.width + c;
            int block = idx / 64;
            int bit   = idx % 64;

            const auto& cell = cfg.grid[r][c];
            if (cell.isWall())    { 
                mask_wall[block] |= (1ULL << bit); 
                heights.push_back(cell.wallHeight); 
            }
            if (cell.isRailing()) { 
                mask_rail[block] |= (1ULL << bit); 
                heights.push_back(cell.wallHeight); 
            }
            if (cell.isGlass()) { 
                mask_glass[block] |= (1ULL << bit); 
                heights.push_back(cell.wallHeight); 
            }
        }
    }
    j["mask_wall"]  = mask_wall;
    j["mask_rail"]  = mask_rail;
    j["mask_glass"] = mask_glass;
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
    cfg.defaultGlassHeight  = j.value("defaultGlassHeight",  2.5f);
    cfg.latitude            = j.value("latitude",            48.5f);

    cfg.grid.assign(cfg.height, std::vector<GridCell>(cfg.width));

    auto mask_wall  = j.value("mask_wall", std::vector<uint64_t>());
    auto mask_rail  = j.value("mask_rail", std::vector<uint64_t>());
    auto mask_glass = j.value("mask_glass", std::vector<uint64_t>());
    auto heights    = j.value("wall_heights", std::vector<float>());
    int hi = 0;

    for (int r = 0; r < cfg.height; ++r) {
        for (int c = 0; c < cfg.width; ++c) {
            int idx = r * cfg.width + c;
            int block = idx / 64;
            int bit   = idx % 64;

            auto& cell = cfg.grid[r][c];
            
            bool isW = (block < mask_wall.size()) && (mask_wall[block] & (1ULL << bit));
            bool isR = (block < mask_rail.size()) && (mask_rail[block] & (1ULL << bit));
            bool isG = (block < mask_glass.size()) && (mask_glass[block] & (1ULL << bit));

            if (isW) {
                cell.type       = WallType::WALL;
                cell.wallHeight = (hi < heights.size()) ? heights[hi++] : cfg.defaultWallHeight;
            } else if (isR) {
                cell.type       = WallType::RAILING;
                cell.wallHeight = (hi < heights.size()) ? heights[hi++] : cfg.defaultRailingHeight;
            } else if (isG) {
                cell.type       = WallType::GLASS;
                cell.wallHeight = (hi < heights.size()) ? heights[hi++] : cfg.defaultGlassHeight;
            }
        }
    }
}