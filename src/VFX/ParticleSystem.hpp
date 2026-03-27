#pragma once
#include <SFML/Graphics.hpp>
#include <vector>

struct Particle {
    sf::Vector2f pos;
    sf::Vector2f vel; // Vitesse
    sf::Color color;
    float lifetime;     // Temps restant
    float maxLifetime;  // Temps total (pour le dégradé)
    float size;
};

class ParticleSystem : public sf::Drawable, public sf::Transformable {
private:
    std::vector<Particle> m_particles;
    sf::VertexArray m_vertices; // Pour un dessin super rapide

public:
    ParticleSystem(int maxParticles = 1000);
    
    // Ajoute une particule
    void emit(sf::Vector2f pos, sf::Vector2f vel, sf::Color col, float lifetime, float size);
    
    void update(float dt);

private:
    virtual void draw(sf::RenderTarget& target, sf::RenderStates states) const;
};