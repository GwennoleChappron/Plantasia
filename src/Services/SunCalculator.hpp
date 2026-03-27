#pragma once
#include "Data/BalconyConfig.hpp"
#include <SFML/System/Vector2.hpp>

struct SunPosition {
    float azimuth;
    float elevation;
    bool isAboveHorizon() const { return elevation > 0.0f; }
};

class SunCalculator {
public:
    static SunPosition compute(const BalconyConfig& cfg);
    
    static float softShadow (const BalconyConfig& cfg, int tx, int ty, const SunPosition& sun);
    static float toRad(float deg) { return deg * 3.14159265f / 180.0f; }
    static float toDeg(float rad) { return rad * 180.0f / 3.14159265f; }
};