#pragma once
#include <SFML/Graphics.hpp>

// Forward declaration pour éviter les inclusions circulaires
class Application;

class State {
protected:
    Application* m_app; // Permet à l'état d'accéder aux managers et à la fenêtre

public:
    State(Application* app) : m_app(app) {}
    virtual ~State() = default;

    virtual void onEnter() = 0;              // Appelé quand on arrive sur cet écran
    virtual void onExit() = 0;               // Appelé quand on quitte cet écran
    virtual void update(float dt) = 0;       // Logique (animations, calculs)
    virtual void draw(sf::RenderWindow& window) = 0; // Rendu SFML (Lianes, sprites)
    virtual void drawImGui() = 0;            // Rendu de l'interface ImGui
};