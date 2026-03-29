#pragma once
#include <SFML/Graphics.hpp>
#include <vector>

struct Liane {
    float startX;
    float targetLength;
    float currentLength;
    float phase;
    float swaySpeed;
    float agitation;
    float scaleMultiplier;
};

class VineSystem {
private:
    std::vector<Liane> m_lianes;
    sf::Texture        m_vineTexture;
    float              m_temps          = 0.0f;
    float              m_globalAgitation= 0.0f; // injection externe (event vent)

    void genererTextureLiane();

public:
    VineSystem(float width, float height);
    void update(float dt, sf::Vector2i mousePos, float width, float height);
    void draw(sf::RenderTarget& target);

    // Appelé par EventSystem
    void addAgitation(float a) { m_globalAgitation += a; }
};
