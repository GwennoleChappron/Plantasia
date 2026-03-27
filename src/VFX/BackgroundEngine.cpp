#include "BackgroundEngine.hpp"
#include <cmath>
#include <imgui.h>

BackgroundEngine::BackgroundEngine(float width, float height) 
    : m_swarm(60, width, height), m_vines(width, height), m_grass(width, height) // Initialisation du nouveau système !
{
    // Création des lucioles (et c'est tout ce qu'il reste ici !)
    for (int i = 0; i < 80; i++) {
        Luciole l;
        l.basePos = sf::Vector2f(std::fmod(i * 137.5f, width), std::fmod(i * 93.1f, height));
        l.offset = (float)i;
        l.speed = 5.f + (i % 8);
        l.radius = 2.0f + (i % 4) * 0.8f;
        m_lucioles.push_back(l);
    }
}

void BackgroundEngine::update(float dt, sf::Vector2i mousePos, float width, float height) {
    m_temps += dt;
    
    // On met à jour l'essaim
    m_swarm.update(dt, sf::Vector2f((float)mousePos.x, (float)mousePos.y), width, height);
    
    // On met à jour notre nouveau système de lianes indépendant !
    m_vines.update(dt, mousePos, width, height);

    m_grass.update(dt, mousePos);
}

void BackgroundEngine::draw(sf::RenderTarget& target) {
    // 1. Dessin des Lucioles
    sf::CircleShape circle;
    for (int i = 0; i < m_lucioles.size(); i++) {
        const auto& l = m_lucioles[i];
        float x = l.basePos.x + std::sin(m_temps * 0.4f + l.offset) * 50.f;
        float y = l.basePos.y - std::fmod(m_temps * l.speed, target.getSize().y);
        if (y < 0) y += target.getSize().y;
        float alpha = (std::sin(m_temps * 2.0f + l.offset) * 0.5f + 0.5f) * 255.f;
        circle.setRadius(l.radius);
        circle.setPosition(x, y);
        circle.setFillColor(sf::Color(190, 255, 140, (sf::Uint8)alpha));
        target.draw(circle);
    }

    // 2. Dessin des Boids
    m_swarm.draw(target);
    
    // 3. Dessin des Lianes
    m_vines.draw(target);

    m_grass.draw(target);
}