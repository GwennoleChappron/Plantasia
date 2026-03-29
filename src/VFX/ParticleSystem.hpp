#pragma once
#include <SFML/Graphics.hpp>
#include <vector>

struct Particle {
    sf::Vector2f pos;
    sf::Vector2f vel;
    sf::Color color;
    float lifetime;
    float maxLifetime;
    float size;
};

class ParticleSystem : public sf::Drawable, public sf::Transformable {
private:
    std::vector<Particle> m_particles;
    sf::VertexArray m_vertices;

public:
    ParticleSystem(int maxParticles = 1000);

    void emit(sf::Vector2f pos, sf::Vector2f vel, sf::Color col, float lifetime, float size);
    void update(float dt);

private:
    virtual void draw(sf::RenderTarget& target, sf::RenderStates states) const;
};
