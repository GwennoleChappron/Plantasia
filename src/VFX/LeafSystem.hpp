#pragma once
#include <SFML/Graphics.hpp>
#include <vector>
#include <random>

enum class Season { SPRING, SUMMER, AUTUMN, WINTER };

struct Leaf {
    sf::Vector2f pos;
    sf::Vector2f vel;
    float rotation;
    float rotSpeed;
    float scale;
    float lifetime;     // temps restant avant reset
    float maxLifetime;
    sf::Color color;
    float wobblePhase;
    float wobbleSpeed;
};

class LeafSystem {
private:
    std::vector<Leaf> m_leaves;
    sf::VertexArray   m_va;
    float             m_width, m_height;
    float             m_timer      = 0.f;
    float             m_windX      = 0.f;   // vent horizontal global
    Season            m_season     = Season::AUTUMN;
    int               m_maxLeaves  = 60;
    float             m_spawnRate  = 0.f;   // feuilles/sec courantes
    float             m_spawnAccum = 0.f;

    // Couleurs par saison
    sf::Color randomLeafColor(std::mt19937& rng) const;
    void resetLeaf(Leaf& l, std::mt19937& rng, bool fromTop);
    void rebuildVA();

    static std::mt19937 s_rng;

public:
    LeafSystem(float width, float height);

    void setSeason(Season s);
    Season getSeason() const { return m_season; }

    // Appelé par EventSystem (rafale de feuilles)
    void burst(int count = 30);

    // Injection de vent (synchro avec GrassSystem / VineSystem)
    void addWind(float w) { m_windX += w; }

    void update(float dt);
    void draw(sf::RenderTarget& target);
};
