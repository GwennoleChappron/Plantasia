#include "LeafSystem.hpp"
#include <cmath>
#include <algorithm>

std::mt19937 LeafSystem::s_rng(999);

// ── Couleurs selon saison ───────────────────────────────────────────────────
sf::Color LeafSystem::randomLeafColor(std::mt19937& rng) const {
    std::uniform_int_distribution<int> d(0, 4);
    int v = d(rng);

    switch (m_season) {
        case Season::SPRING: {
            // Pétales : rose, blanc, vert tendre
            const sf::Color palette[] = {
                sf::Color(255,180,200), sf::Color(255,210,220),
                sf::Color(200,230,150), sf::Color(240,255,200),
                sf::Color(255,150,180)
            };
            return palette[v];
        }
        case Season::SUMMER: {
            // Feuilles vertes estivales — rares, principalement herbe
            const sf::Color palette[] = {
                sf::Color(60,140,50),  sf::Color(80,160,60),
                sf::Color(50,120,40),  sf::Color(100,180,70),
                sf::Color(70,150,55)
            };
            return palette[v];
        }
        case Season::AUTUMN: {
            // Orange, rouge, jaune, brun
            const sf::Color palette[] = {
                sf::Color(210, 80, 20),  sf::Color(230,140, 20),
                sf::Color(200, 50, 20),  sf::Color(180,120, 30),
                sf::Color(240,170, 40)
            };
            return palette[v];
        }
        case Season::WINTER: {
            // Flocons / feuilles mortes gris-beige
            const sf::Color palette[] = {
                sf::Color(220,220,230), sf::Color(200,200,210),
                sf::Color(180,170,160), sf::Color(210,205,195),
                sf::Color(230,225,220)
            };
            return palette[v];
        }
    }
    return sf::Color::White;
}

// ── Init / reset d'une feuille ──────────────────────────────────────────────
void LeafSystem::resetLeaf(Leaf& l, std::mt19937& rng, bool fromTop) {
    std::uniform_real_distribution<float> dX(0.f, m_width);
    std::uniform_real_distribution<float> dVx(-30.f, 30.f);
    std::uniform_real_distribution<float> dVy(40.f, 120.f);
    std::uniform_real_distribution<float> dRot(-180.f, 180.f);
    std::uniform_real_distribution<float> dRspd(-90.f, 90.f);
    std::uniform_real_distribution<float> dScale(0.4f, 1.0f);
    std::uniform_real_distribution<float> dLife(4.f, 9.f);
    std::uniform_real_distribution<float> dPhase(0.f, 6.28f);
    std::uniform_real_distribution<float> dWspd(0.8f, 2.5f);

    l.pos        = { dX(rng), fromTop ? -20.f : (float)(rng() % (int)m_height) };
    l.vel        = { dVx(rng), dVy(rng) };
    l.rotation   = dRot(rng);
    l.rotSpeed   = dRspd(rng);
    l.scale      = dScale(rng);
    l.maxLifetime= dLife(rng);
    l.lifetime   = l.maxLifetime;
    l.color      = randomLeafColor(rng);
    l.wobblePhase= dPhase(rng);
    l.wobbleSpeed= dWspd(rng);
}

// ── Constructeur ────────────────────────────────────────────────────────────
LeafSystem::LeafSystem(float width, float height)
    : m_width(width), m_height(height)
{
    m_va.setPrimitiveType(sf::Quads);
    setSeason(Season::AUTUMN); // default
}

void LeafSystem::setSeason(Season s) {
    m_season = s;
    m_leaves.clear();
    m_spawnRate = 0.f;

    switch (s) {
        case Season::SPRING:  m_spawnRate = 4.f;  m_maxLeaves = 40; break;
        case Season::SUMMER:  m_spawnRate = 0.5f; m_maxLeaves = 10; break;
        case Season::AUTUMN:  m_spawnRate = 10.f; m_maxLeaves = 60; break;
        case Season::WINTER:  m_spawnRate = 6.f;  m_maxLeaves = 50; break;
    }

    // Pré-popule avec quelques feuilles déjà en vol
    int preSpawn = m_maxLeaves / 2;
    m_leaves.resize(preSpawn);
    for (auto& l : m_leaves)
        resetLeaf(l, s_rng, false); // position aléatoire sur l'écran
}

void LeafSystem::burst(int count) {
    int toAdd = std::min(count, m_maxLeaves - (int)m_leaves.size());
    for (int i = 0; i < toAdd; ++i) {
        Leaf l;
        resetLeaf(l, s_rng, true);
        // Burst : vitesse horizontale plus forte
        std::uniform_real_distribution<float> dVx(-80.f, 80.f);
        l.vel.x = dVx(s_rng);
        m_leaves.push_back(l);
    }
}

// ── Update ──────────────────────────────────────────────────────────────────
void LeafSystem::update(float dt) {
    m_timer += dt;

    // Amortissement du vent injecté
    m_windX *= std::pow(0.01f, dt);

    // Spawn de nouvelles feuilles
    m_spawnAccum += m_spawnRate * dt;
    while (m_spawnAccum >= 1.f && (int)m_leaves.size() < m_maxLeaves) {
        Leaf l;
        resetLeaf(l, s_rng, true);
        m_leaves.push_back(l);
        m_spawnAccum -= 1.f;
    }
    if (m_spawnAccum >= 1.f) m_spawnAccum = 0.f;

    for (int i = 0; i < (int)m_leaves.size(); ) {
        auto& l = m_leaves[i];
        l.lifetime -= dt;

        if (l.lifetime <= 0.f || l.pos.y > m_height + 30.f) {
            // Swap & pop O(1)
            m_leaves[i] = m_leaves.back();
            m_leaves.pop_back();
            continue;
        }

        // Wobble horizontal (balancement)
        float wobble = std::sin(m_timer * l.wobbleSpeed + l.wobblePhase) * 25.f;
        l.pos.x += (l.vel.x + wobble + m_windX * 60.f) * dt;
        l.pos.y += l.vel.y * dt;

        // Ralentissement vertical léger (air resistance)
        l.vel.y = std::min(l.vel.y + 8.f * dt, 140.f);

        l.rotation += l.rotSpeed * dt;

        // Fade in/out
        float lifeRatio = l.lifetime / l.maxLifetime;
        float alpha = lifeRatio < 0.15f ? (lifeRatio / 0.15f) : 1.f;
        l.color.a = (sf::Uint8)(alpha * 220.f);

        ++i;
    }

    rebuildVA();
}

// ── VertexArray (losange simple pour chaque feuille) ────────────────────────
void LeafSystem::rebuildVA() {
    int n = (int)m_leaves.size();
    m_va.resize(n * 4);

    for (int i = 0; i < n; ++i) {
        const auto& l = m_leaves[i];

        // Losange 6×10 (scaled), pivote autour du centre
        float hw = 6.f * l.scale;
        float hh = 10.f * l.scale;
        float rad = l.rotation * 3.14159f / 180.f;
        float cs  = std::cos(rad), sn = std::sin(rad);

        // 4 coins du losange (haut, droite, bas, gauche)
        sf::Vector2f offsets[4] = {
            { 0.f, -hh}, { hw, 0.f}, { 0.f,  hh}, {-hw, 0.f}
        };

        int v = i * 4;
        for (int j = 0; j < 4; ++j) {
            float rx = offsets[j].x * cs - offsets[j].y * sn;
            float ry = offsets[j].x * sn + offsets[j].y * cs;
            m_va[v+j].position = {l.pos.x + rx, l.pos.y + ry};
            m_va[v+j].color    = l.color;
        }
    }
}

void LeafSystem::draw(sf::RenderTarget& target) {
    if (m_va.getVertexCount() > 0)
        target.draw(m_va);
}
