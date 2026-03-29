#pragma once
#include "Core/State.hpp"
#include "Core/Application.hpp"
#include "imgui.h"
#include <string>
#include <map>

class StateCatalogue : public State {
private:
    int   m_indexSelectionne = -1;
    char  m_filtreRecherche[128] = {};
    float m_time     = 0.f;
    float m_saveTimer = 0.f;

    // Textures PNG
    std::map<std::string, sf::Texture> m_plantTextures;     // nom_espece → PNG pleine taille
    std::map<std::string, sf::Texture> m_plantIconTextures; // nom_espece → PNG _icon

    // Drag & drop
    int          m_draggedPlantIdx = -1;
    sf::Vector2f m_dragOffset;

    // Outline shader (drag)
    sf::Shader        m_outlineShader;
    sf::RenderTexture m_renderTex;

    void DessinerCalendrierVisuel(const char* label, int debut, int fin, ImVec4 col);
    void chargerTextures();

public:
    StateCatalogue(Application* app);
    void onEnter()  override;
    void onExit()   override;
    void update(float dt) override;
    void draw(sf::RenderWindow& window) override;
    void drawImGui() override;
};