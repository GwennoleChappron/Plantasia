#pragma once
#include "Core/State.hpp"
#include "Data/BalconyConfig.hpp"
#include "Services/SunCalculator.hpp"
#include <SFML/Graphics.hpp>

class StateBalconySim : public State {
private:
    // --- Données de Simulation ---
    BalconyConfig m_cfg;
    SunPosition   m_sun;           
    bool          m_sunMapDirty = true;  

    // --- Paramètres d'Affichage ---
    int  m_editMode = 0;            
    bool m_showSunMap    = false;   
    bool m_showShadows   = true;    
    const float CELL_PX  = 15.0f;   // On reste sur tes 15px par case

    // --- Méthodes Internes ---
    void handleInput();
    void drawCompass();              
    void recalcSun();
    void markSunMapDirty();
    void renderGrid(sf::RenderWindow& window);
    void renderShadowLayer(sf::RenderWindow& window);
    void renderSunMapLayer(sf::RenderWindow& window);

public:
    StateBalconySim(Application* app);
    void onEnter() override;
    void onExit() override;
    void update(float dt) override;
    void draw(sf::RenderWindow& window) override;
    void drawImGui() override;
};