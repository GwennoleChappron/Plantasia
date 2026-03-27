#pragma once
#include <SFML/Graphics.hpp>
#include <vector>

struct Boid {
    sf::Vector2f position;
    sf::Vector2f velocity;
};

class BoidsSwarm {
private:
    std::vector<Boid> m_boids;
    sf::ConvexShape m_shape; // La forme triangulaire d'un boid

    // Paramètres de l'essaim
    float m_maxSpeed = 100.0f;
    float m_perceptionRadius = 300.0f;
    float m_mouseRepulsionRadius = 200.0f;

public:
    BoidsSwarm(int count, float width, float height);
    void update(float dt, sf::Vector2f mousePos, float width, float height);
    void draw(sf::RenderTarget& target);
};