#pragma once
#include <SFML/Graphics.hpp>
#include "ParticleSystem.hpp"
#include <vector>

enum class PlantType { GRASS, FLOWER };

struct Blade {
    sf::Vector2f pos;
    float angle      = 0.0f;
    float targetAngle= 0.0f;
    float maxHeight;
    float growth     = 0.0f;
    float phase;
    int   variant;
    PlantType type;
    float agitation;
};

class GrassSystem {
private:
    std::vector<Blade> m_blades;
    sf::Texture        m_atlas;
    float              m_timer     = 0.0f;
    float              m_windBoost = 0.0f; // injection externe (event vent)
    ParticleSystem     m_particles;
    sf::VertexArray    m_bladeVA;

    void buildTexture();
    void rebuildBatch();

public:
    GrassSystem(float width, float height);
    void update(float dt, sf::Vector2i mousePos);
    void draw(sf::RenderTarget& target);

    // Appelé par EventSystem pour les rafales de vent
    void addWindBoost(float w) { m_windBoost += w; }
};
