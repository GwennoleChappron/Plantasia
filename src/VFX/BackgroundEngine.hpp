#pragma once
#include <SFML/Graphics.hpp>
#include "BoidsSwarm.hpp"
#include "VineSystem.hpp"
#include "GrassSystem.hpp"
#include <vector>

struct Luciole {
    sf::Vector2f basePos;
    float offset;
    float speed;
    float radius;
};


class BackgroundEngine {
private:
    BoidsSwarm m_swarm;
    std::vector<Luciole> m_lucioles;
    GrassSystem m_grass;
    float m_temps = 0.0f;

    VineSystem m_vines;

public:
    BackgroundEngine(float width, float height);
    void update(float dt, sf::Vector2i mousePos, float width, float height);
    void draw(sf::RenderTarget& target);
};