#pragma once
#include <SFML/Graphics.hpp>

// Interface de base pour tous les layers du background.
// Chaque système (herbe, lianes, boids, nuages, lucioles) implémente cette interface.
class IBackgroundLayer {
public:
    virtual ~IBackgroundLayer() = default;

    virtual void update(float dt) = 0;
    virtual void draw(sf::RenderTarget& target) = 0;

    // Optionnel : rendu dans la glow texture (bloom)
    // Seul LucioleLayer override ceci.
    virtual void drawGlow(sf::RenderTarget& target) {}

    bool enabled = true;
};
