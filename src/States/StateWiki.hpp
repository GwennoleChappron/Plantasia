#pragma once
#include "Core/State.hpp"
#include "Core/Application.hpp"
#include "imgui.h"
#include <string>
#include <vector>

class StateWiki : public State {
private:
    std::string m_planteSelectionnee; // Le nom de la plante cliquée
    char m_filtreRecherche[128] = {};
    std::map<std::string, sf::Texture> m_plantIconTextures;
    int m_modeAffichage = 0; // 0 = Plantes, 1 = Sols
    std::string m_solSelectionne;
    void chargerTextures();

    // Helper pour le calendrier
    void DessinerCalendrierVisuel(const char* label, int debut, int fin, ImVec4 col);

public:
    StateWiki(Application* app);
    
    void onEnter() override;
    void onExit() override;
    void update(float dt) override;
    void draw(sf::RenderWindow& window) override;
    void drawImGui() override;
};