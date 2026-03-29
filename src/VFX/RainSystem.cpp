#include "RainSystem.hpp"
#include <cmath>
#include <algorithm>

std::mt19937 RainSystem::s_rng(555);

RainSystem::RainSystem(float width, float height)
    : m_width(width), m_height(height)
{
    m_drops.reserve(m_maxDrops);
    m_dropVA.setPrimitiveType(sf::Lines);

    // Pré-spawn de gouttes déjà en vol
    for (int i = 0; i < m_maxDrops / 2; ++i)
        spawnDrop();
}

void RainSystem::setIntensity(float i) {
    m_intensity = std::clamp(i, 0.f, 1.f);
    m_maxDrops  = (int)(50 + 350 * m_intensity);
    m_drops.reserve(m_maxDrops);
}

void RainSystem::spawnDrop() {
    if ((int)m_drops.size() >= m_maxDrops) return;

    std::uniform_real_distribution<float> dX(-50.f, m_width + 50.f);
    std::uniform_real_distribution<float> dSpd(400.f, 700.f);
    std::uniform_real_distribution<float> dLen(8.f, 20.f);
    std::uniform_real_distribution<float> dAlpha(100.f, 180.f);
    std::uniform_real_distribution<float> dY(0.f, m_height);

    RainDrop d;
    d.pos   = { dX(s_rng), (float)(s_rng() % (int)m_height) - m_height };
    d.speed = dSpd(s_rng);
    d.length= dLen(s_rng);
    d.alpha = dAlpha(s_rng) * m_intensity;
    m_drops.push_back(d);
}

void RainSystem::spawnRipple(float x, float y) {
    // Limite le nombre de ripples pour les perfs
    if ((int)m_ripples.size() > 60) return;

    std::uniform_real_distribution<float> dR(6.f, 14.f);
    Ripple r;
    r.pos         = {x, y};
    r.radius      = 0.f;
    r.maxRadius   = dR(s_rng);
    r.maxLifetime = 0.6f;
    r.lifetime    = r.maxLifetime;
    m_ripples.push_back(r);
}

void RainSystem::rebuildVA() {
    int n = (int)m_drops.size();
    m_dropVA.resize(n * 2);

    float rad     = m_windAngle * 3.14159f / 180.f;
    float sinA    = std::sin(rad);
    float cosA    = std::cos(rad);

    for (int i = 0; i < n; ++i) {
        const auto& d = m_drops[i];

        // Haut de la goutte
        sf::Vertex& va = m_dropVA[i*2];
        va.position = d.pos;
        va.color    = sf::Color(180, 200, 255, (sf::Uint8)d.alpha);

        // Bas (direction selon vent)
        float dx = sinA * d.length;
        float dy = cosA * d.length;
        sf::Vertex& vb = m_dropVA[i*2+1];
        vb.position = { d.pos.x + dx, d.pos.y + dy };
        vb.color    = sf::Color(180, 200, 255, 0); // fade vers transparent
    }
}

void RainSystem::update(float dt) {
    if (m_intensity < 0.01f) {
        m_drops.clear();
        m_ripples.clear();
        return;
    }

    m_timer += dt;

    // Spawn de gouttes
    float spawnRate  = m_maxDrops * 2.f; // renouvellement rapide
    m_spawnAccum    += spawnRate * dt;
    while (m_spawnAccum >= 1.f) {
        spawnDrop();
        m_spawnAccum -= 1.f;
    }

    float rad  = m_windAngle * 3.14159f / 180.f;
    float sinA = std::sin(rad);
    float cosA = std::cos(rad);

    for (int i = 0; i < (int)m_drops.size(); ) {
        auto& d = m_drops[i];

        d.pos.x += sinA * d.speed * dt;
        d.pos.y += cosA * d.speed * dt;

        // Impact au sol
        if (d.pos.y > m_height) {
            spawnRipple(d.pos.x, m_height - 4.f);

            // Notifie l'herbe
            if (m_onImpact)
                m_onImpact(d.pos.x, m_intensity * 0.3f);

            // Swap & pop
            m_drops[i] = m_drops.back();
            m_drops.pop_back();
            continue;
        }

        // Hors écran gauche/droite → reset
        if (d.pos.x < -60.f || d.pos.x > m_width + 60.f) {
            m_drops[i] = m_drops.back();
            m_drops.pop_back();
            continue;
        }

        ++i;
    }

    // Update ripples
    for (int i = 0; i < (int)m_ripples.size(); ) {
        auto& r = m_ripples[i];
        r.lifetime -= dt;
        if (r.lifetime <= 0.f) {
            m_ripples[i] = m_ripples.back();
            m_ripples.pop_back();
            continue;
        }
        float t   = 1.f - r.lifetime / r.maxLifetime;
        r.radius  = r.maxRadius * t;
        ++i;
    }

    rebuildVA();
}

void RainSystem::draw(sf::RenderTarget& target) {
    if (m_intensity < 0.01f) return;

    // Gouttes
    if (m_dropVA.getVertexCount() > 0)
        target.draw(m_dropVA);

    // Ripples (ellipses aplaties au sol)
    sf::CircleShape ellipse;
    ellipse.setFillColor(sf::Color::Transparent);
    for (const auto& r : m_ripples) {
        float t      = 1.f - r.lifetime / r.maxLifetime;
        sf::Uint8 a  = (sf::Uint8)((1.f - t) * 120.f * m_intensity);
        ellipse.setOutlineColor(sf::Color(180, 210, 255, a));
        ellipse.setOutlineThickness(1.f);
        ellipse.setRadius(r.radius);
        ellipse.setOrigin(r.radius, r.radius);
        // Aplatissement horizontal
        ellipse.setScale(1.f, 0.35f);
        ellipse.setPosition(r.pos);
        target.draw(ellipse);
    }
}
