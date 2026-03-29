#include "ParticleSystem.hpp"
#include <cmath>

ParticleSystem::ParticleSystem(int maxParticles) {
    m_particles.reserve(maxParticles);
    m_vertices.setPrimitiveType(sf::Quads);
}

void ParticleSystem::emit(sf::Vector2f pos, sf::Vector2f vel,
                          sf::Color col, float lifetime, float size) {
    if ((int)m_particles.size() >= (int)m_particles.capacity()) return;
    m_particles.push_back({pos, vel, col, lifetime, lifetime, size});
}

void ParticleSystem::update(float dt) {
    for (int i = 0; i < (int)m_particles.size(); ) {
        auto& p = m_particles[i];
        p.lifetime -= dt;
        if (p.lifetime <= 0.f) {
            m_particles[i] = m_particles.back();
            m_particles.pop_back();
            continue;
        }
        p.pos += p.vel * dt;
        p.vel.y += 40.f * dt; // gravité légère
        p.color.a = (sf::Uint8)(255.f * p.lifetime / p.maxLifetime);
        ++i;
    }

    int n = (int)m_particles.size();
    m_vertices.resize(n * 4);
    for (int i = 0; i < n; ++i) {
        const auto& p = m_particles[i];
        int v = i * 4;
        m_vertices[v+0].position = p.pos;
        m_vertices[v+1].position = p.pos + sf::Vector2f(p.size, 0);
        m_vertices[v+2].position = p.pos + sf::Vector2f(p.size, p.size);
        m_vertices[v+3].position = p.pos + sf::Vector2f(0, p.size);
        for (int j = 0; j < 4; ++j) m_vertices[v+j].color = p.color;
    }
}

void ParticleSystem::draw(sf::RenderTarget& target, sf::RenderStates states) const {
    states.transform *= getTransform();
    states.texture = nullptr;
    target.draw(m_vertices, states);
}
