#pragma once
#include <SFML/Graphics.hpp>
#include <vector>

struct Liane {
    float startX;
    float targetLength;   // La longueur finale qu'elle doit atteindre
    float currentLength;  // La longueur actuelle (elle pousse !)
    float phase;
    float swaySpeed;
    float agitation;
    float scaleMultiplier;// Plus petite vers le centre de l'écran
};

class VineSystem {
private:
    std::vector<Liane> m_lianes;
    sf::Texture m_vineTexture;
    float m_temps = 0.0f;

    void genererTextureLiane();

public:
    VineSystem(float width, float height);
    void update(float dt, sf::Vector2i mousePos, float width, float height);
    void draw(sf::RenderTarget& target);
};