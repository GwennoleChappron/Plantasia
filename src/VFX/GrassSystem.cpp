#include "GrassSystem.hpp"
#include <cmath>
#include <algorithm>

GrassSystem::GrassSystem(float width, float height) : m_particles(500) {
    buildTexture();
    
    // 300 brins pour un tapis dense
    for (int i = 0; i < 300; i++) {
        Blade b;
        b.pos = sf::Vector2f((float)(rand() % (int)width), height);
        b.maxHeight = 25.f + (rand() % 40);
        b.phase = (float)(rand() % 100);
        b.variant = rand() % 3; // 3 dessins d'herbe différents
        b.type = (rand() % 8 == 0) ? PlantType::FLOWER : PlantType::GRASS;
        b.agitation = 0.0f;
        m_blades.push_back(b);
    }
}

void GrassSystem::buildTexture() {
    sf::Image img;
    img.create(32, 16, sf::Color::Transparent);
    
    // --- PALETTE ---
    sf::Color gDark(30, 80, 40);
    sf::Color gMid(50, 110, 60);
    sf::Color gLight(80, 160, 90);
    sf::Color petal(255, 120, 160); // Rose
    sf::Color heart(255, 230, 150); // Jaune

    // --- DESSIN (Inspire de tes refs) ---
    // Herbe 1 (fine)
    for(int y=4; y<16; y++) img.setPixel(2, y, (y%2)?gDark:gMid);
    // Herbe 2 (touffue)
    for(int y=6; y<16; y++) {
        img.setPixel(6, y, gMid);
        img.setPixel(7, y, gDark);
        if(y>10) img.setPixel(5,y, gDark);
    }
    // Herbe 3 (claire)
    for(int y=2; y<16; y++) img.setPixel(11, y, gLight);
    
    // Fleur (Variante 4)
    for(int y=6; y<16; y++) img.setPixel(16, y, gMid); // Tige
    for(int x=15; x<18; x++) for(int y=4; y<6; y++) img.setPixel(x,y, petal); // Petales
    img.setPixel(16, 5, heart); // Cœur

    m_atlas.loadFromImage(img);
    m_atlas.setSmooth(false); // <--- CRUCIAL pour le pixel art !
}

void GrassSystem::update(float dt, sf::Vector2i mousePos) {
    m_timer += dt;
    for (auto& b : m_blades) {
        // Pousse (toujours la même)
        if (b.growth < 1.0f) b.growth += 0.6f * dt;

        // Vent naturel
        float wind = std::sin(m_timer * 2.5f + b.phase) * 6.0f;
        
        // Interaction souris
        float dist = std::sqrt(std::pow(mousePos.x - b.pos.x, 2) + std::pow(mousePos.y - b.pos.y, 2));
        float repulsion = 0.0f;
        
        if (dist < 100.f && mousePos.y > b.pos.y - 150.f) {
            repulsion = (mousePos.x < b.pos.x) ? (100.f - dist) : -(100.f - dist);
            
            // --- NOUVEAU : Emission de particules si agitation ---
            if (b.type == PlantType::FLOWER && std::abs(b.angle - b.targetAngle) > 2.0f) {
                if(rand()%3 == 0) {
                    m_particles.emit(
                        b.pos + sf::Vector2f(b.angle*0.1f, -b.maxHeight*0.8f), // Position (haut tige)
                        sf::Vector2f(repulsion * 0.5f + (rand()%20-10), -(50.f + rand()%100)), // Vitesse
                        sf::Color(255, 120, 160, 200), // Couleur
                        2.0f + (rand()%100)/100.f, // Lifetime
                        2.0f // Size
                    );
                }
            }
        }

        b.targetAngle = wind + repulsion * 0.7f;
        b.angle += (b.targetAngle - b.angle) * 7.0f * dt;
    }

    // Mise à jour de notre système de particules !
    m_particles.update(dt);
}

void GrassSystem::draw(sf::RenderTarget& target) {
    sf::Sprite s(m_atlas);
    s.setOrigin(4, 16); 

    // Les sprites font 8x16 pixels dans l'atlas
    const sf::IntRect grassRects[] = {
        sf::IntRect(0, 0, 8, 16),
        sf::IntRect(8, 0, 8, 16),
        sf::IntRect(16, 0, 8, 16)
    };
    sf::IntRect flowerRect(24, 0, 8, 16);

    for (const auto& b : m_blades) {
        if (b.type == PlantType::FLOWER) s.setTextureRect(flowerRect);
        else s.setTextureRect(grassRects[b.variant]);
        
        s.setPosition(b.pos);
        s.setRotation(b.angle);
        float scale = (b.maxHeight / 16.f) * b.growth;
        // Zoom x2 pour le pixel art, échelle fine en X
        s.setScale(scale * 1.5f, scale * 2.5f); 
        
        target.draw(s);
    }

    // Dessin des particules !
    target.draw(m_particles);
}