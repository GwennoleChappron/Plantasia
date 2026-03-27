#pragma once
#include <SFML/Graphics.hpp>
#include "ParticleSystem.hpp" // <-- NOUVEAU
#include <vector>

enum class PlantType { GRASS, FLOWER };

struct Blade {
    sf::Vector2f pos;
    float angle = 0.0f;
    float targetAngle = 0.0f;
    float maxHeight;
    float growth = 0.0f;
    float phase;
    int variant;
    PlantType type;
    float agitation; // <-- NOUVEAU pour les particules
};

class GrassSystem {
private:
    std::vector<Blade> m_blades;
    sf::Texture m_atlas;
    float m_timer = 0.0f;
    ParticleSystem m_particles; // <-- NOUVEAU
    void buildTexture();

public:
    GrassSystem(float width, float height);
    void update(float dt, sf::Vector2i mousePos);
    void draw(sf::RenderTarget& target);
};