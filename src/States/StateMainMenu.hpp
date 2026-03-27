#pragma once
#include "Core/State.hpp"
#include "Core/Application.hpp"


class StateMainMenu : public State {
public:
    StateMainMenu(Application* app);
    
    void onEnter() override;
    void onExit() override;
    void update(float dt) override;
    void draw(sf::RenderWindow& window) override;
    void drawImGui() override;
};