#include "Application.hpp"
#include "imgui.h"
#include "imgui-SFML.h"
#include "States/StateMainMenu.hpp"
#include "UI/ColorTheme.hpp" // Ta nouvelle source unique de vérité

Application::Application() {
    m_window.create(sf::VideoMode(1100, 720), "Plantasia - Mon Jardin");
    m_window.setFramerateLimit(60);
    (void)ImGui::SFML::Init(m_window);
    
    initStyleImGui();
    
    m_database.chargerTout();
    m_userBalcony.chargerProfil("mon_balcon.json");
    m_background = std::make_unique<BackgroundEngine>(1100.0f, 720.0f);
    m_stateMachine.addState(std::make_unique<StateMainMenu>(this));
}

Application::~Application() {
    ImGui::SFML::Shutdown();
}

void Application::run() {
    while (m_window.isOpen()) {
        m_stateMachine.processStateChanges();

        sf::Event event;
        while (m_window.pollEvent(event)) {
            ImGui::SFML::ProcessEvent(m_window, event);
            if (event.type == sf::Event::Closed) m_window.close();
        }

        sf::Time     dt       = m_clock.restart();
        sf::Vector2i mousePos = sf::Mouse::getPosition(m_window);
        m_background->update(dt.asSeconds(), mousePos,
                             (float)m_window.getSize().x, (float)m_window.getSize().y);
        ImGui::SFML::Update(m_window, dt);

        if (m_stateMachine.getActiveState())
            m_stateMachine.getActiveState()->update(dt.asSeconds());

        // BackgroundEngine::draw() dessine maintenant le gradient de ciel,
        // donc on ne clear plus avec une couleur fixe ici.
        m_window.clear(sf::Color::Black); // fallback si shader absent
        m_background->draw(m_window);

        if (m_stateMachine.getActiveState()) {
            m_stateMachine.getActiveState()->draw(m_window);
            m_stateMachine.getActiveState()->drawImGui();
        }

        m_background->drawImGui();

        ImGui::SFML::Render(m_window);
        m_window.display();
    }
}

// ─────────────────────────────────────────────
//  STYLE IMGUI
// ─────────────────────────────────────────────
void Application::initStyleImGui() {
    ImGuiStyle& s = ImGui::GetStyle();
    
    // 1. Configuration de la géométrie et des espacements
    s.WindowRounding   = 12.0f;
    s.FrameRounding    = 8.0f;
    s.PopupRounding    = 10.0f;
    s.ChildRounding    = 8.0f;
    s.WindowBorderSize = 1.0f;
    s.ItemSpacing      = ImVec2(10.0f, 10.0f);
    s.FramePadding     = ImVec2(12.0f,  8.0f);
    s.WindowPadding    = ImVec2(18.0f, 16.0f);

    // 2. Application de la palette de couleurs centralisée
    Theme::applyToImGui(s);
}