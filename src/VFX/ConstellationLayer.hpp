#pragma once
#include <SFML/Graphics.hpp>
#include <vector>

// ConstellationLayer : relie les lucioles proches entre elles avec des lignes lumineuses.
// Ne possède pas ses propres lucioles — il lit les positions calculées par BackgroundEngine
// et les connecte si la distance est inférieure à m_maxDist.
// Doit être dessiné AVANT le bloom pass pour que les lignes bénéficient du glow.

class ConstellationLayer {
private:
    sf::VertexArray m_lines;
    float           m_maxDist      = 120.f; // distance max pour connecter 2 lucioles
    float           m_maxDistSq    = 120.f * 120.f;
    int             m_maxLinks     = 80;    // limite pour les perfs

public:
    ConstellationLayer() {
        m_lines.setPrimitiveType(sf::Lines);
    }

    void setMaxDist(float d) {
        m_maxDist   = d;
        m_maxDistSq = d * d;
    }

    // Appelé par BackgroundEngine après avoir calculé les positions des lucioles.
    // positions : liste de (x, y, alpha) — alpha pour l'opacité de la ligne
    void rebuild(const std::vector<sf::Vector3f>& positions) {
        m_lines.clear();
        int links = 0;

        for (int i = 0; i < (int)positions.size() && links < m_maxLinks; ++i) {
            for (int j = i + 1; j < (int)positions.size() && links < m_maxLinks; ++j) {
                float dx = positions[i].x - positions[j].x;
                float dy = positions[i].y - positions[j].y;
                float distSq = dx*dx + dy*dy;

                if (distSq < m_maxDistSq) {
                    // Opacité inversement proportionnelle à la distance
                    float t      = 1.f - std::sqrt(distSq) / m_maxDist;
                    float alphaA = positions[i].z * t * 80.f; // max 80/255
                    float alphaB = positions[j].z * t * 80.f;

                    sf::Vertex va, vb;
                    va.position = {positions[i].x, positions[i].y};
                    va.color    = sf::Color(160, 255, 180, (sf::Uint8)std::clamp(alphaA, 0.f, 255.f));
                    vb.position = {positions[j].x, positions[j].y};
                    vb.color    = sf::Color(160, 255, 180, (sf::Uint8)std::clamp(alphaB, 0.f, 255.f));

                    m_lines.append(va);
                    m_lines.append(vb);
                    ++links;
                }
            }
        }
    }

    // Rendu normal (lignes sur la scène)
    void draw(sf::RenderTarget& target) {
        if (m_lines.getVertexCount() > 0)
            target.draw(m_lines);
    }

    // Rendu dans la glow texture (half-res : positions divisées par 2)
    void drawGlow(sf::RenderTarget& glowTarget) {
        if (m_lines.getVertexCount() == 0) return;

        // Copie avec positions halved pour la glow texture
        sf::VertexArray half(sf::Lines, m_lines.getVertexCount());
        for (unsigned int i = 0; i < m_lines.getVertexCount(); ++i) {
            half[i].position = m_lines[i].position * 0.5f;
            half[i].color    = m_lines[i].color;
        }
        glowTarget.draw(half);
    }

    float getMaxDist() const { return m_maxDist; }
};
