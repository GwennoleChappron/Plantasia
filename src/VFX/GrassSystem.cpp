#include "GrassSystem.hpp"
#include <cmath>
#include <algorithm>
#include <random>

static std::mt19937 rng_grass(42);

void GrassSystem::buildTexture() {
    sf::Image img;
    img.create(32, 16, sf::Color::Transparent);

    sf::Color gDark (30,  80,  40);
    sf::Color gMid  (50,  110, 60);
    sf::Color gLight(80,  160, 90);
    sf::Color petal (255, 120, 160);
    sf::Color heart (255, 230, 150);

    // Herbe 1 (fine)
    for (int y = 4; y < 16; y++) img.setPixel(2, y, (y%2) ? gDark : gMid);

    // Herbe 2 (touffue)
    for (int y = 6; y < 16; y++) {
        img.setPixel(6, y, gMid);
        img.setPixel(7, y, gDark);
        if (y > 10) img.setPixel(5, y, gDark);
    }

    // Herbe 3 (claire)
    for (int y = 2; y < 16; y++) img.setPixel(11, y, gLight);

    // Fleur
    for (int y = 6; y < 16; y++) img.setPixel(16, y, gMid);
    for (int x = 15; x < 18; x++)
        for (int y = 4; y < 6; y++)
            img.setPixel(x, y, petal);
    img.setPixel(16, 5, heart);

    m_atlas.loadFromImage(img);
    m_atlas.setSmooth(false);
}

GrassSystem::GrassSystem(float width, float height) : m_particles(500) {
    buildTexture();
    std::uniform_real_distribution<float> distX(0.f, width);
    std::uniform_real_distribution<float> distH(25.f, 65.f);
    std::uniform_real_distribution<float> distP(0.f, 100.f);
    std::uniform_int_distribution<int>    distV(0, 2);
    std::uniform_int_distribution<int>    distF(0, 7);

    m_bladeVA.setPrimitiveType(sf::Quads);

    for (int i = 0; i < 300; i++) {
        Blade b;
        b.pos       = {distX(rng_grass), height};
        b.maxHeight = distH(rng_grass);
        b.phase     = distP(rng_grass);
        b.variant   = distV(rng_grass);
        b.type      = (distF(rng_grass) == 0) ? PlantType::FLOWER : PlantType::GRASS;
        b.agitation = 0.f;
        m_blades.push_back(b);
    }
}

void GrassSystem::update(float dt, sf::Vector2i mousePos) {
    m_timer += dt;

    // Amortissement du vent injecté
    m_windBoost *= std::pow(0.02f, dt);

    for (auto& b : m_blades) {
        if (b.growth < 1.f) b.growth += 0.6f * dt;

        float wind = std::sin(m_timer * 2.5f + b.phase) * 6.f
                   + m_windBoost * std::sin(m_timer * 5.f + b.phase) * 12.f;

        float dx = mousePos.x - b.pos.x;
        float dy = mousePos.y - b.pos.y;
        float distSq = dx*dx + dy*dy;
        float repulsion = 0.f;

        if (distSq < 100.f*100.f && mousePos.y > b.pos.y - 150.f) {
            float dist = std::sqrt(distSq);
            repulsion = (mousePos.x < b.pos.x) ? (100.f - dist) : -(100.f - dist);

            if (b.type == PlantType::FLOWER && std::abs(b.angle - b.targetAngle) > 2.f) {
                std::uniform_int_distribution<int> d3(0, 2);
                if (d3(rng_grass) == 0) {
                    std::uniform_real_distribution<float> jitter(-10.f, 10.f);
                    std::uniform_real_distribution<float> life(2.f, 3.f);
                    m_particles.emit(
                        b.pos + sf::Vector2f(b.angle*0.1f, -b.maxHeight*0.8f),
                        sf::Vector2f(repulsion*0.5f + jitter(rng_grass), -(50.f + std::abs(jitter(rng_grass))*5)),
                        sf::Color(255, 120, 160, 200),
                        life(rng_grass), 2.f
                    );
                }
            }
        }

        b.targetAngle = wind + repulsion * 0.7f;
        b.angle += (b.targetAngle - b.angle) * 7.f * dt;
    }

    m_particles.update(dt);
    rebuildBatch();
}

void GrassSystem::rebuildBatch() {
    const sf::IntRect grassRects[] = {
        {0,0,8,16}, {8,0,8,16}, {16,0,8,16}
    };
    const sf::IntRect flowerRect = {24,0,8,16};
    const float atlasW = 32.f, atlasH = 16.f;

    int n = (int)m_blades.size();
    m_bladeVA.resize(n * 4);

    for (int i = 0; i < n; ++i) {
        const auto& b  = m_blades[i];
        const auto& tr = (b.type == PlantType::FLOWER) ? flowerRect : grassRects[b.variant];

        float scale  = (b.maxHeight / 16.f) * b.growth;
        float scaleX = scale * 1.5f;
        float scaleY = scale * 2.5f;
        float w = tr.width  * scaleX;
        float h = tr.height * scaleY;

        float rad = b.angle * 3.14159f / 180.f;
        float cs = std::cos(rad), sn = std::sin(rad);

        sf::Vector2f corners[4] = {
            {-4.f*scaleX,  0.f},
            { w-4.f*scaleX, 0.f},
            { w-4.f*scaleX, -h},
            {-4.f*scaleX,  -h}
        };

        int v = i * 4;
        for (int j = 0; j < 4; ++j) {
            float rx = corners[j].x * cs - corners[j].y * sn;
            float ry = corners[j].x * sn + corners[j].y * cs;
            m_bladeVA[v+j].position = {b.pos.x + rx, b.pos.y + ry};
            m_bladeVA[v+j].color    = sf::Color::White;
        }

        float u0 = (float)tr.left / atlasW;
        float u1 = (float)(tr.left + tr.width) / atlasW;
        float v0 = (float)tr.top / atlasH;
        float v1 = (float)(tr.top + tr.height) / atlasH;
        m_bladeVA[v+0].texCoords = {u0*32.f, v1*16.f};
        m_bladeVA[v+1].texCoords = {u1*32.f, v1*16.f};
        m_bladeVA[v+2].texCoords = {u1*32.f, v0*16.f};
        m_bladeVA[v+3].texCoords = {u0*32.f, v0*16.f};
    }
}

void GrassSystem::draw(sf::RenderTarget& target) {
    sf::RenderStates st;
    st.texture = &m_atlas;
    target.draw(m_bladeVA, st);
    target.draw(m_particles);
}
