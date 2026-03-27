#include "BoidsSwarm.hpp"
#include <cmath>
#include <cstdlib>

// Petite fonction utilitaire pour la longueur d'un vecteur
float length(const sf::Vector2f& v) { return std::sqrt(v.x*v.x + v.y*v.y); }
sf::Vector2f normalize(const sf::Vector2f& v) { 
    float len = length(v); 
    return (len > 0) ? sf::Vector2f(v.x/len, v.y/len) : sf::Vector2f(0,0); 
}

BoidsSwarm::BoidsSwarm(int count, float width, float height) {
    // On dessine un petit "cerf-volant" pour représenter le boid
    m_shape.setPointCount(3);
    m_shape.setPoint(0, sf::Vector2f(6.0f, 0.0f));  // Nez
    m_shape.setPoint(1, sf::Vector2f(-4.0f, 3.0f)); // Aile gauche
    m_shape.setPoint(2, sf::Vector2f(-4.0f, -3.0f));// Aile droite
    m_shape.setFillColor(sf::Color(100, 200, 120, 150)); // Vert d'eau transparent

    for (int i = 0; i < count; ++i) {
        Boid b;
        b.position = sf::Vector2f(rand() % (int)width, rand() % (int)height);
        float angle = (rand() % 360) * 3.14159f / 180.f;
        b.velocity = sf::Vector2f(std::cos(angle), std::sin(angle)) * m_maxSpeed;
        m_boids.push_back(b);
    }
}

void BoidsSwarm::update(float dt, sf::Vector2f mousePos, float width, float height) {
    for (auto& b : m_boids) {
        sf::Vector2f separation(0, 0), alignment(0, 0), cohesion(0, 0);
        int total = 0;

        for (const auto& other : m_boids) {
            if (&b == &other) continue;
            sf::Vector2f diff = b.position - other.position;
            float dist = length(diff);

            if (dist < m_perceptionRadius) {
                separation += diff / (dist * dist); // Plus c'est près, plus on repousse fort
                alignment += other.velocity;
                cohesion += other.position;
                total++;
            }
        }

        if (total > 0) {
            alignment /= (float)total;
            cohesion = (cohesion / (float)total) - b.position;
        }

        // Règle 4 : Fuir la souris !
        sf::Vector2f mouseForce(0, 0);
        sf::Vector2f mouseDiff = b.position - mousePos;
        float mouseDist = length(mouseDiff);
        if (mouseDist < m_mouseRepulsionRadius && mouseDist > 0) {
            mouseForce = normalize(mouseDiff) * (300.0f / mouseDist); // Force explosive
        }

        // On additionne les forces (avec des poids arbitraires pour équilibrer le vol)
        b.velocity += (separation * 15.f + alignment * 0.1f + cohesion * 0.2f + mouseForce * 50.f) * dt;
        
        // Limiter la vitesse
        float speed = length(b.velocity);
        if (speed > m_maxSpeed) b.velocity = (b.velocity / speed) * m_maxSpeed;
        else if (speed < m_maxSpeed * 0.5f) b.velocity = (b.velocity / speed) * (m_maxSpeed * 0.5f);

        b.position += b.velocity * dt;

        // "Wrap" de l'écran (si on sort à gauche, on réapparaît à droite)
        if (b.position.x < 0) b.position.x += width;
        if (b.position.x > width) b.position.x -= width;
        if (b.position.y < 0) b.position.y += height;
        if (b.position.y > height) b.position.y -= height;
    }
}

void BoidsSwarm::draw(sf::RenderTarget& target) {
    for (const auto& b : m_boids) {
        m_shape.setPosition(b.position);
        float angle = std::atan2(b.velocity.y, b.velocity.x) * 180.f / 3.14159265f;
        m_shape.setRotation(angle);
        target.draw(m_shape);
    }
}