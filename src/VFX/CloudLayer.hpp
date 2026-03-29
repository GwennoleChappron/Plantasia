#pragma once
#include <SFML/Graphics.hpp>
#include <vector>
#include <random>

struct Cloud {
    sf::Vector2f pos;
    float speed;          // pixels/sec
    float scale;          // taille globale
    float alpha;          // opacité de base (50-100)
    float parallaxFactor; // 0.3 = arrière-plan lent, 1.0 = avant
    // Boules qui composent le nuage (offsets relatifs)
    struct Puff { float ox, oy, r; };
    std::vector<Puff> puffs;
};

class CloudLayer {
private:
    std::vector<Cloud> m_clouds;
    float m_width;
    float m_height;
    float m_windOffset = 0.f; // injection externe (event vent)

    void spawnCloud(float x, float y, std::mt19937& rng, bool backLayer);

public:
    CloudLayer(float width, float height);
    void update(float dt);
    void draw(sf::RenderTarget& target);

    // Rafale de vent externe
    void addWind(float w) { m_windOffset += w; }
};
