#include "ParticleSystem.hpp"
#include <cmath>

ParticleSystem::ParticleSystem(int maxParticles) {
    m_particles.reserve(maxParticles);
    m_vertices.setPrimitiveType(sf::Quads); // Chaque particule est un carré (4 sommets)
    m_vertices.resize(maxParticles * 4);
}

void ParticleSystem::emit(sf::Vector2f pos, sf::Vector2f vel, sf::Color col, float lifetime, float size) {
    if (m_particles.size() >= m_particles.capacity()) return;

    Particle p;
    p.pos = pos;
    p.vel = vel;
    p.color = col;
    p.lifetime = p.maxLifetime = lifetime;
    p.size = size;
    m_particles.push_back(p);
}

void ParticleSystem::update(float dt) {
    // On efface le VertexArray
    for (int i = 0; i < m_vertices.getVertexCount(); i++) {
        m_vertices[i].color = sf::Color::Transparent;
    }

    // Mise à jour de chaque particule
    for (int i = 0; i < m_particles.size(); i++) {
        auto& p = m_particles[i];
        
        p.lifetime -= dt;

        // Si la particule est morte, on l'enlève
        if (p.lifetime <= 0.0f) {
            m_particles.erase(m_particles.begin() + i);
            i--; // On recule l'index pour compenser
            continue;
        }

        // Physique simple (vitesse)
        p.pos += p.vel * dt;

        // Calcul de l'alpha (transparence) pour l'estompement
        float ratio = p.lifetime / p.maxLifetime;
        p.color.a = static_cast<sf::Uint8>(255 * ratio);

        // Mise à jour des sommets du VertexArray
        int vIdx = i * 4;
        m_vertices[vIdx].position = p.pos;
        m_vertices[vIdx + 1].position = p.pos + sf::Vector2f(p.size, 0);
        m_vertices[vIdx + 2].position = p.pos + sf::Vector2f(p.size, p.size);
        m_vertices[vIdx + 3].position = p.pos + sf::Vector2f(0, p.size);

        for (int j = 0; j < 4; j++) {
            m_vertices[vIdx + j].color = p.color;
        }
    }
}

void ParticleSystem::draw(sf::RenderTarget& target, sf::RenderStates states) const {
    states.transform *= getTransform();
    states.texture = nullptr; // Pas de texture pour l'instant
    target.draw(m_vertices, states);
}