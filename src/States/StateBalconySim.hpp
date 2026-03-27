#pragma once
#include "Core/State.hpp"
#include "Data/BalconyConfig.hpp"
#include "Services/SunCalculator.hpp"
#include <SFML/Graphics.hpp>

class StateBalconySim : public State {
private:
    BalconyConfig m_cfg;
    SunPosition   m_sun;           
    bool          m_sunMapDirty = true;  

    int  m_editMode = 0;            
    int  m_selectedPlantIndex = -1; // <--- AJOUTÉ
    bool m_showSunMap    = false;   
    bool m_showShadows   = true;    
    const float CELL_PX  = 15.0f;   

    void handleInput();
    void drawCompass();              
    void recalcSun();
    void markSunMapDirty();

    // MISES À JOUR : Ajout de l'argument sf::Vector2f offset
    void renderGrid(sf::RenderWindow& window, sf::Vector2f offset);
    void renderShadowLayer(sf::RenderWindow& window, sf::Vector2f offset);
    void renderSunMapLayer(sf::RenderWindow& window, sf::Vector2f offset);
    void renderPlants(sf::RenderWindow& window, sf::Vector2f offset); // <--- AJOUTÉ

public:
    StateBalconySim(Application* app);
    void onEnter() override;
    void onExit() override;
    void update(float dt) override;
    void draw(sf::RenderWindow& window) override;
    void drawImGui() override;
};