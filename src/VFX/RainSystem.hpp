#pragma once
#include <SFML/Graphics.hpp>
#include <vector>
#include <random>
#include <functional>

struct RainDrop {
    sf::Vector2f pos;
    float speed;
    float length;   // longueur de la goutte (pixels)
    float alpha;
};

struct Ripple {
    sf::Vector2f pos;
    float radius;
    float maxRadius;
    float lifetime;
    float maxLifetime;
};

class RainSystem {
private:
    std::vector<RainDrop> m_drops;
    std::vector<Ripple>   m_ripples;
    sf::VertexArray       m_dropVA;
    float                 m_width, m_height;
    float                 m_timer      = 0.f;
    float                 m_intensity  = 0.8f; // 0 = aucune pluie, 1 = averse
    float                 m_windAngle  = -8.f; // degrés (inclinaison pluie)
    int                   m_maxDrops   = 300;
    float                 m_spawnAccum = 0.f;

    // Callback : notifie le BackgroundEngine quand une goutte touche le sol
    // pour que la GrassSystem soit informée
    std::function<void(float x, float strength)> m_onImpact;

    static std::mt19937 s_rng;

    void spawnDrop();
    void spawnRipple(float x, float y);
    void rebuildVA();

public:
    RainSystem(float width, float height);

    void setIntensity(float i);              // 0→1
    float getIntensity() const { return m_intensity; }
    void setWindAngle(float deg) { m_windAngle = deg; }

    // Callback impact (pour agiter l'herbe)
    void setOnImpact(std::function<void(float x, float strength)> cb) {
        m_onImpact = cb;
    }

    void update(float dt);
    void draw(sf::RenderTarget& target);
};
