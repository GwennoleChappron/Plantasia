#include "SunCalculator.hpp"
#include <cmath>
#include <algorithm>

SunPosition SunCalculator::compute(const BalconyConfig& cfg) {
    float timezone = std::round(cfg.longitude / 15.0f);
    float hourUTC  = cfg.hour - timezone;

    float d = (float)cfg.day + 30.6f*(cfg.month+1) + (cfg.year-2000)*365.25f;
    float g = toRad(357.529f + 0.9856003f*d);
    float q = 280.459f + 0.9856474f*d;
    float L = q + 1.915f*std::sin(g) + 0.020f*std::sin(2.0f*g);
    float R = toRad(L);
    float eps = toRad(23.439f - 0.0000004f*d);

    float dec = std::asin(std::sin(eps)*std::sin(R));
    float RA  = toDeg(std::atan2(std::cos(eps)*std::sin(R), std::cos(R))) / 15.0f;

    float lst = (100.46f + 0.985647f*d + cfg.longitude + 15.0f*hourUTC) / 15.0f;
    float ha  = toRad(15.0f*(lst - RA));
    float lat = toRad(cfg.latitude);

    float sAlt = std::sin(lat)*std::sin(dec) + std::cos(lat)*std::cos(dec)*std::cos(ha);
    float alt  = std::asin(sAlt);

    float cAz = (std::sin(dec) - std::sin(lat)*sAlt) / (std::cos(lat)*std::cos(alt));
    float az   = toDeg(std::acos(std::clamp(cAz,-1.f,1.f)));
    if (std::sin(ha) > 0) az = 360.0f - az;

    return { az, toDeg(alt) };
}

// Remplace isInShadow et softShadow par ces deux fonctions :

// Le rayon volumétrique qui traverse les objets translucides
float castVolumetricRay(const BalconyConfig& cfg, int tx, int ty, const SunPosition& sun, float angleOff) {
    float CELL_M = 0.05f;
    float bAz = static_cast<float>(cfg.orientation) * 45.0f;
    float relAz = SunCalculator::toRad(sun.azimuth + angleOff - bAz);
    sf::Vector2f sunDir(std::sin(relAz), std::cos(relAz)); 
    float tanElev = std::tan(SunCalculator::toRad(sun.elevation));

    if (cfg.hasRoof) {
        float distHorizCells = cfg.roofHeight / (tanElev * CELL_M);
        float srcX = tx + sunDir.x * distHorizCells;
        float srcY = ty + sunDir.y * distHorizCells;
        if (srcX >= 0 && srcX < cfg.width && srcY >= 0 && srcY < cfg.height)
            return 0.0f; 
    }

    float px = tx + 0.5f;
    float py = ty + 0.5f;
    float light = 1.0f; // On part avec 100% de lumière

    for (float d = 0.5f; d < 120.0f; d += 0.25f) {
        int cx = static_cast<int>(px + sunDir.x * d);
        int cy = static_cast<int>(py + sunDir.y * d);
        if (cx < 0 || cx >= cfg.width || cy < 0 || cy >= cfg.height) break;

        const auto& cell = cfg.grid[cy][cx];
        float rayZ = tanElev * (d * CELL_M); // La hauteur du rayon à cet instant

        // Loi de Beer-Lambert : on multiplie la lumière restante par la transparence
        if (cell.type == WallType::WALL && cell.wallHeight > rayZ) {
            return 0.0f; // Mur opaque = bloque tout
        } else if (cell.type == WallType::RAILING && cell.wallHeight > rayZ) {
            light *= 0.6f; // Rambarde = laisse passer 60%
        }
        
        // La plante bloque la lumière (indépendant du mur !)
        if (cell.plantHeight > rayZ) {
            light *= (1.0f - cell.plantOpacity); 
        }

        // Si la lumière est épuisée, on arrête le rayon pour économiser le CPU
        if (light < 0.01f) return 0.0f; 
    }
    return light;
}

float SunCalculator::softShadow(const BalconyConfig& cfg, int tx, int ty, const SunPosition& sun) {
    if (!sun.isAboveHorizon()) return 0.0f;
    
    // On lance 3 rayons pour adoucir les bords de l'ombre
    float lit = castVolumetricRay(cfg, tx, ty, sun, -0.4f) +
                castVolumetricRay(cfg, tx, ty, sun,  0.0f) +
                castVolumetricRay(cfg, tx, ty, sun,  0.4f);
                
    return lit / 3.0f;
}