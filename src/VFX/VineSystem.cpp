#include "VineSystem.hpp"
#include <cmath>
#include <algorithm>
#include <cstdlib>

VineSystem::VineSystem(float width, float height) {
    genererTextureLiane();

    // On génère des lianes sur toute la largeur, mais on filtre selon la distance au centre
    float centerX = width / 2.0f;
    
    for (int i = 0; i < 40; i++) {
        float x = (float)(rand() % (int)width);
        
        // Calcul de la distance au bord (0 = au centre, 1 = tout au bord)
        float distFromCenter = std::abs(x - centerX) / centerX;
        
        // 1. DENSITÉ : On interdit presque totalement les lianes au centre absolu
        if (distFromCenter < 0.25f) continue; 
        
        // On a moins de chance de faire pousser une liane si on s'approche du centre
        if ((rand() % 100) / 100.f > distFromCenter + 0.2f) continue;

        Liane l;
        l.startX = x;
        // 2. LONGUEUR : Très longues sur les bords, très courtes vers le milieu
        l.targetLength = 50.f + 600.f * distFromCenter + (rand() % 100);
        l.currentLength = 0.0f; // Elle commence à 0, elle va pousser !
        
        l.phase = (float)(rand() % 100);
        l.swaySpeed = 0.3f + (rand() % 40) / 100.f;
        l.agitation = 0.0f;
        
        // 3. TAILLE : Gros pixels sur les bords, tout fins vers le milieu
        l.scaleMultiplier = 0.8f + 2.2f * distFromCenter; 
        
        m_lianes.push_back(l);
    }
}

void VineSystem::genererTextureLiane() {
    sf::Image img;
    img.create(8, 16, sf::Color::Transparent);
    
    for(int y = 0; y < 16; ++y) {
        img.setPixel(3, y, sf::Color(30, 80, 40));
        img.setPixel(4, y, sf::Color(40, 100, 50));
    }
    img.setPixel(2, 4, sf::Color(50, 120, 60));
    img.setPixel(1, 4, sf::Color(60, 140, 70));
    img.setPixel(2, 5, sf::Color(40, 100, 50));
    
    img.setPixel(5, 10, sf::Color(50, 120, 60));
    img.setPixel(6, 10, sf::Color(60, 140, 70));
    img.setPixel(5, 11, sf::Color(40, 100, 50));

    m_vineTexture.loadFromImage(img);
    m_vineTexture.setSmooth(false);
}

void VineSystem::update(float dt, sf::Vector2i mousePos, float width, float height) {
    m_temps += dt;

    for (auto& l : m_lianes) {
        // --- CROISSANCE ---
        if (l.currentLength < l.targetLength) {
            l.currentLength += 80.0f * dt; // Pousse de 80 pixels par seconde
        }

        // --- INTERACTION SOURIS (BEAUCOUP PLUS SÈCHE) ---
        if (mousePos.y > 0 && mousePos.y < l.currentLength + 30.f) {
            float currentX = l.startX + std::sin(m_temps * l.swaySpeed + l.phase) * (mousePos.y * 0.02f);
            
            if (std::abs(mousePos.x - currentX) < 20.f * l.scaleMultiplier) {
                // On plafonne très bas (0.6f max) pour éviter qu'elle parte dans tous les sens
                l.agitation = std::min(l.agitation + 4.0f * dt, 0.6f); 
            }
        }
        
        // --- AMORTISSEMENT LINÉAIRE (Fini la latence flottante !) ---
        l.agitation -= 1.5f * dt; 
        if (l.agitation < 0.0f) l.agitation = 0.0f;
    }
}

void VineSystem::draw(sf::RenderTarget& target) {
    sf::Sprite vineSprite(m_vineTexture);
    vineSprite.setOrigin(4.f, 0.f);

    for (const auto& l : m_lianes) {
        float segmentHeight = 16.f * l.scaleMultiplier;
        int numSegments = (int)(l.currentLength / segmentHeight);
        
        for (int s = 0; s <= numSegments; s++) {
            float y = s * segmentHeight;
            if (y > l.currentLength) break;
            
            float vent = std::sin(m_temps * l.swaySpeed + l.phase + y * 0.01f);
            float tremblement = std::sin(m_temps * 10.f + y * 0.1f) * l.agitation * 3.f;
            
            float sway = vent * (y * 0.03f) + tremblement * (y / l.targetLength);
            float x = l.startX + sway;

            float nextY = y + segmentHeight;
            float nextVent = std::sin(m_temps * l.swaySpeed + l.phase + nextY * 0.01f);
            float nextTremblement = std::sin(m_temps * 10.f + nextY * 0.1f) * l.agitation * 3.f;
            float nextX = l.startX + nextVent * (nextY * 0.03f) + nextTremblement * (nextY / l.targetLength);
            
            float angle = std::atan2(nextX - x, segmentHeight) * -180.f / 3.14159f;

            // EFFET DE POUSSE : Si c'est le dernier segment, on le réduit pour faire "nouveau né"
            float distanceToTip = l.currentLength - y;
            float growthScale = std::min(1.0f, distanceToTip / (segmentHeight * 2.0f));
            float finalScale = l.scaleMultiplier * growthScale;

            vineSprite.setPosition(x, y);
            vineSprite.setRotation(angle);
            vineSprite.setScale(finalScale, finalScale);
            
            sf::Uint8 nuance = 255 - (int)(l.swaySpeed * 60.f); 
            vineSprite.setColor(sf::Color(nuance, nuance, nuance, 255));

            target.draw(vineSprite);
        }
    }
}