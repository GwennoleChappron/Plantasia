#pragma once
#include <SFML/Graphics.hpp>
#include "BoidsSwarm.hpp"
#include "VineSystem.hpp"
#include "GrassSystem.hpp"
#include <vector>

struct Luciole {
    sf::Vector2f basePos;
    float offset;
    float speed;
    float radius;
};

class BackgroundEngine {
private:
    BoidsSwarm m_swarm;
    std::vector<Luciole> m_lucioles;
    GrassSystem m_grass;
    VineSystem m_vines;
    
    float m_temps = 0.0f;

    float m_timeScale = 1.0f;       
    float m_lucioleIntensity = 0.8f;
    
    bool m_enableLucioles = true;
    bool m_enableBoids = true;
    bool m_enableVines = true;
    bool m_enableGrass = true;

    // BLOOM
    sf::RenderTexture m_glowTexture; 
    sf::Sprite m_glowSprite;         
    sf::Shader m_blurShader;         
    bool m_shaderActif = false;
    
    float m_bloomRadius = 10.0f;      
    float m_bloomMultiplier = 2.2f;  
    float m_bloomThreshold = 0.15f;
    float m_bloomSoftness = 0.5f;

    // 🔥 NOUVEAU : contrôle visuel
    float m_glowSizeMultiplier = 3.5f;
    float m_coreBrightness = 1.2f;

public:
    BackgroundEngine(float width, float height);
    void update(float dt, sf::Vector2i mousePos, float width, float height);
    void draw(sf::RenderTarget& target);
    void drawImGui(); 
};