#pragma once
#include <vector>
#include <string>
#include <ctime>
#include <SFML/System/Vector2.hpp>

enum class BalconyOrientation { North = 0, NorthEast, East, SouthEast, South, SouthWest, West, NorthWest };

struct GridCell {
    bool  isWall       = false;
    float wallHeight   = 2.5f;
    float sunHours     = 0.0f;
};

struct BalconyConfig {
    // --- NOUVEAU : Dimensions dynamiques ---
    int width = 40;
    int height = 30;

    float latitude  = 48.5f;
    float longitude = 7.75f;
    BalconyOrientation orientation = BalconyOrientation::South;

    int   year = 2026;
    int   month = 3;
    int   day = 27;
    float hour = 12.0f;

    bool hasRoof = false; //
    float roofOpacity = 0.8f;

    // --- GRILLE DYNAMIQUE ---
    // On utilise un vector de vector pour pouvoir changer la taille plus tard
    std::vector<std::vector<GridCell>> grid;

    float defaultWallHeight = 2.5f;

    void syncRealDate() {
        std::time_t t = std::time(nullptr);
        std::tm* tm   = std::localtime(&t);
        year  = tm->tm_year + 1900;
        month = tm->tm_mon  + 1;
        day   = tm->tm_mday;
    }
};