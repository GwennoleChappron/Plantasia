#include "SunCalculator.hpp"

SunPosition SunCalculator::compute(const BalconyConfig& cfg) {
    // Approximation du jour julien et position écliptique
    float timezone = std::round(cfg.longitude / 15.0f);
    float hourUTC = cfg.hour - timezone;
    
    // Simplification de l'algorithme de Meeus pour les besoins du balcon
    float d = (float)cfg.day + 30.6f * (cfg.month + 1) + (cfg.year - 2000) * 365.25f; 
    float g = toRad(357.529f + 0.9856003f * d);
    float q = 280.459f + 0.9856474f * d;
    float L = q + 1.915f * std::sin(g) + 0.020f * std::sin(2.0f * g);
    float R = toRad(L);
    float eps = toRad(23.439f - 0.0000004f * d);
    
    float dec = std::asin(std::sin(eps) * std::sin(R));
    float RA = toDeg(std::atan2(std::cos(eps) * std::sin(R), std::cos(R))) / 15.0f;
    
    float lst = (100.46f + 0.985647f * d + cfg.longitude + 15.0f * hourUTC) / 15.0f;
    float ha = toRad(15.0f * (lst - RA));
    float lat = toRad(cfg.latitude);

    float sAlt = std::sin(lat) * std::sin(dec) + std::cos(lat) * std::cos(dec) * std::cos(ha);
    float alt = std::asin(sAlt);
    
    float cAz = (std::sin(dec) - std::sin(lat) * sAlt) / (std::cos(lat) * std::cos(alt));
    float az = toDeg(std::acos(std::clamp(cAz, -1.0f, 1.0f)));
    if (std::sin(ha) > 0) az = 360.0f - az;

    return { az, toDeg(alt) };
}

bool SunCalculator::isInShadow(const BalconyConfig& cfg, int tx, int ty, const SunPosition& sun) {
    if (!sun.isAboveHorizon()) return true;

    float cellSizeM = 0.05f; // Tes 5cm par case
    
    // Direction du rayon arrivant au sol (inverse de l'azimut soleil)
    // On ajuste selon l'orientation du balcon
    float bAz = static_cast<float>(cfg.orientation) * 45.0f;
    float relAz = toRad(sun.azimuth - bAz);
    
    sf::Vector2f dir(-std::sin(relAz), std::cos(relAz));
    float tanElev = std::tan(toRad(sun.elevation));

    // On remonte le rayon pour voir s'il tape un mur
    for (float distCase = 1.0f; distCase < 100.0f; distCase += 1.0f) {
        int cx = tx + static_cast<int>(dir.x * distCase);
        int cy = ty + static_cast<int>(dir.y * distCase);

        if (cx < 0 || cx >= cfg.width || cy < 0 || cy >= cfg.height) break;

        const auto& cell = cfg.grid[cy][cx];
        if (cell.isWall) {
            // Un mur fait de l'ombre si : hauteur / distance > tan(elevation)
            float distMetres = distCase * cellSizeM;
            if (cell.wallHeight / distMetres > tanElev) return true;
        }
    }
    return false;
}

void SunCalculator::computeDailySunMap(BalconyConfig& cfg) {
    for (auto& row : cfg.grid) for (auto& c : row) c.sunHours = 0.0f;

    for (float h = 6.0f; h <= 20.0f; h += 0.5f) { // Pas de 30min
        cfg.hour = h;
        SunPosition sun = compute(cfg);
        if (!sun.isAboveHorizon()) continue;

        for (int y = 0; y < cfg.height; ++y) {
            for (int x = 0; x < cfg.width; ++x) {
                if (!cfg.grid[y][x].isWall && !isInShadow(cfg, x, y, sun)) {
                    cfg.grid[y][x].sunHours += 0.5f;
                }
            }
        }
    }
}