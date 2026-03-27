#pragma once
#include "Core/State.hpp"
#include "Core/Application.hpp"
#include "Data/BalconyConfig.hpp"
#include "Services/SunCalculator.hpp"
#include "Services/SunGPU.hpp" 
#include <SFML/Graphics.hpp>

enum class EditMode { MUR=0, OBSERVER, PLANTE, RAMBARDE };

class StateBalconySim : public State {
private:
    BalconyConfig m_cfg;
    SunPosition   m_sun;
    bool          m_sunMapDirty = true;

    EditMode m_editMode        = EditMode::OBSERVER;
    int      m_selectedPlantIdx = -1;
    bool     m_showSunMap      = false;
    bool     m_showShadows     = true;

    const float CELL_PX = 15.0f;
    int m_newW = 40, m_newH = 30;

    sf::VertexArray m_gridVA;
    SunGPU m_sunGPU;

    // --- 🎥 NOUVEAU : LA CAMÉRA ---
    sf::Vector2f m_cameraPos;
    float        m_cameraZoom = 1.0f;
    sf::Vector2i m_lastMousePos;
    bool         m_isPanning = false;

    void frameCamera(); // Fonction pour tout recadrer automatiquement
    // ------------------------------

    void updateGridVertices();
    void handleInput();
    void drawCompass();
    void recalcSun();
    void markSunMapDirty();

    // 🧹 Fini les 'offset' manuels !
    void renderGrid       (sf::RenderWindow& window);
    void renderShadowLayer(sf::RenderWindow& window);
    void renderSunMapLayer(sf::RenderWindow& window);
    void renderPlants     (sf::RenderWindow& window);

public:
    StateBalconySim(Application* app);
    void onEnter()  override;
    void onExit()   override;
    void update(float dt) override;
    void draw(sf::RenderWindow&) override;
    void drawImGui() override;
    void bakePlantsIntoGrid();
};