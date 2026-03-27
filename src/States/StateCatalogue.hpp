#pragma once
#include "Core/State.hpp"
#include "Core/Application.hpp"
#include "imgui.h"
#include <string>

class StateCatalogue : public State {
private:
    int m_indexSelectionne = -1;
    char m_filtreRecherche[128] = {};

    void DessinerCalendrierVisuel(const char* label, int debut, int fin, ImVec4 col);

public:
    StateCatalogue(Application* app);
    
    void onEnter() override;
    void onExit() override;
    void update(float dt) override;
    void draw(sf::RenderWindow& window) override;
    void drawImGui() override;
};