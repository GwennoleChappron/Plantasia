#pragma once
#include <vector>
#include <cmath>
#include <algorithm>
#include <SFML/System/Vector2.hpp>
#include "Data/BalconyConfig.hpp"

struct SunPosition {
    float azimuth;   // 0=Nord, 90=Est, 180=Sud, 270=Ouest
    float elevation; // Angle vertical (négatif = nuit)
    bool isAboveHorizon() const { return elevation > 0.0f; }
};

class SunCalculator {
public:
    // Calcul de la position précise du soleil (Meeus)
    static SunPosition compute(const BalconyConfig& cfg);

    // Teste si une case (tx, ty) est dans l'ombre d'un mur
    static bool isInShadow(const BalconyConfig& cfg, int tx, int ty, const SunPosition& sun);

    // Recalcule les heures de soleil pour toute la grille
    static void computeDailySunMap(BalconyConfig& cfg);

private:
    static constexpr float PI = 3.14159265f;
    static float toRad(float deg) { return deg * (PI / 180.0f); }
    static float toDeg(float rad) { return rad * (180.0f / PI); }
};