#include "CloudLayer.hpp"
#include <cmath>
#include <algorithm>

static std::mt19937 rng_cloud(777);

void CloudLayer::spawnCloud(float x, float y, std::mt19937& rng, bool backLayer) {
    std::uniform_real_distribution<float> rScale(backLayer ? 0.4f : 0.7f,
                                                  backLayer ? 0.8f : 1.2f);
    std::uniform_real_distribution<float> rAlpha(backLayer ? 30.f : 55.f,
                                                   backLayer ? 55.f : 85.f);
    std::uniform_real_distribution<float> rSpeed(backLayer ? 4.f : 10.f,
                                                   backLayer ? 10.f : 22.f);
    std::uniform_int_distribution<int>    rPuffs(3, 6);
    std::uniform_real_distribution<float> rPuffX(-60.f, 60.f);
    std::uniform_real_distribution<float> rPuffY(-20.f, 10.f);
    std::uniform_real_distribution<float> rPuffR(18.f, 38.f);

    Cloud c;
    c.pos           = {x, y};
    c.scale         = rScale(rng);
    c.alpha         = rAlpha(rng);
    c.speed         = rSpeed(rng);
    c.parallaxFactor= backLayer ? 0.35f : 0.85f;

    int n = rPuffs(rng);
    for (int i = 0; i < n; i++) {
        Cloud::Puff p;
        p.ox = rPuffX(rng) * c.scale;
        p.oy = rPuffY(rng) * c.scale;
        p.r  = rPuffR(rng) * c.scale;
        c.puffs.push_back(p);
    }
    m_clouds.push_back(std::move(c));
}

CloudLayer::CloudLayer(float width, float height)
    : m_width(width), m_height(height)
{
    std::uniform_real_distribution<float> dX(0.f, width);
    std::uniform_real_distribution<float> dY(30.f, height * 0.45f);

    // Couche arrière (lente, petite) : 6 nuages
    for (int i = 0; i < 6; i++)
        spawnCloud(dX(rng_cloud), dY(rng_cloud), rng_cloud, true);

    // Couche avant (rapide, grande) : 4 nuages
    for (int i = 0; i < 4; i++)
        spawnCloud(dX(rng_cloud), dY(rng_cloud) * 0.6f, rng_cloud, false);
}

void CloudLayer::update(float dt) {
    // Amortir le vent injecté
    m_windOffset *= std::pow(0.005f, dt);

    for (auto& c : m_clouds) {
        float effectiveSpeed = c.speed * (1.f + m_windOffset * c.parallaxFactor);
        c.pos.x += effectiveSpeed * dt;

        // Recycle à droite → réapparaît à gauche
        if (c.pos.x > m_width + 150.f) {
            c.pos.x = -150.f;
            std::uniform_real_distribution<float> dY(30.f, m_height * 0.4f);
            c.pos.y = dY(rng_cloud);
        }
    }
}

void CloudLayer::draw(sf::RenderTarget& target) {
    // Trier par parallaxFactor : arrière d'abord
    // (les clouds sont déjà dans le bon ordre depuis le constructeur)

    sf::CircleShape circle;
    circle.setOutlineThickness(0.f);

    for (const auto& c : m_clouds) {
        for (const auto& puff : c.puffs) {
            float r = puff.r;
            circle.setRadius(r);
            circle.setOrigin(r, r);
            circle.setPosition(c.pos.x + puff.ox, c.pos.y + puff.oy);

            // Couleur blanc-gris selon la couche
            sf::Uint8 brightness = (sf::Uint8)(200 + c.parallaxFactor * 55.f);
            sf::Uint8 alpha      = (sf::Uint8)c.alpha;
            circle.setFillColor(sf::Color(brightness, brightness, brightness, alpha));

            target.draw(circle);
        }
    }
}
