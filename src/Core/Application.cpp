#include "Application.hpp"
#include "imgui.h"
#include "imgui-SFML.h"
#include "States/StateMainMenu.hpp"

Application::Application() {
    m_window.create(sf::VideoMode(1100, 720), "Plantasia - Mon Jardin");
    m_window.setFramerateLimit(60);
    // On convertit le warning 'nodiscard' en appel propre
    (void)ImGui::SFML::Init(m_window);
    initStyleImGui();
    m_database.chargerEncyclopedie("encyclopedia.json");
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

        sf::Time dt = m_clock.restart();
        sf::Vector2i mousePos = sf::Mouse::getPosition(m_window);
        m_background->update(dt.asSeconds(), mousePos, m_window.getSize().x, m_window.getSize().y);
        ImGui::SFML::Update(m_window, dt);

        if (m_stateMachine.getActiveState()) {
            m_stateMachine.getActiveState()->update(dt.asSeconds());
        }

        m_window.clear(sf::Color(5, 8, 5));
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
//  PALETTE (On remet notre namespace Couleurs ici)
// ─────────────────────────────────────────────
namespace Couleurs {
    const ImVec4 Fond          = ImVec4(0.04f, 0.06f, 0.04f, 1.00f);
    const ImVec4 Surface       = ImVec4(0.07f, 0.10f, 0.07f, 0.90f); 
    const ImVec4 SurfaceHaute  = ImVec4(0.11f, 0.15f, 0.11f, 0.95f);
    const ImVec4 Bordure       = ImVec4(0.18f, 0.28f, 0.18f, 1.00f);
    const ImVec4 Accent        = ImVec4(0.20f, 0.75f, 0.40f, 1.00f);
    const ImVec4 AccentDoux    = ImVec4(0.15f, 0.55f, 0.30f, 1.00f);
    const ImVec4 AccentSombre  = ImVec4(0.10f, 0.35f, 0.18f, 1.00f);
    const ImVec4 Texte         = ImVec4(0.88f, 0.92f, 0.88f, 1.00f);
    const ImVec4 TexteDoux     = ImVec4(0.55f, 0.68f, 0.55f, 1.00f);
    const ImVec4 TexteInactif  = ImVec4(0.30f, 0.40f, 0.30f, 1.00f);
    const ImVec4 Danger        = ImVec4(0.75f, 0.22f, 0.22f, 1.00f);
    const ImVec4 DangerHover   = ImVec4(0.90f, 0.30f, 0.30f, 1.00f);
}

// ─────────────────────────────────────────────
//  STYLE IMGUI
// ─────────────────────────────────────────────
void Application::initStyleImGui() {
    ImGuiStyle& s = ImGui::GetStyle();
    s.WindowRounding    = 12.0f;
    s.FrameRounding     = 8.0f;
    s.PopupRounding     = 10.0f;
    s.ChildRounding     = 8.0f;
    s.WindowBorderSize  = 1.0f;
    s.ItemSpacing       = ImVec2(10.0f, 10.0f);
    s.FramePadding      = ImVec2(12.0f, 8.0f);
    s.WindowPadding     = ImVec2(18.0f, 16.0f);

    ImVec4* c = s.Colors;
    c[ImGuiCol_WindowBg]             = Couleurs::Surface;
    c[ImGuiCol_ChildBg]              = ImVec4(0.04f, 0.06f, 0.04f, 0.70f);
    c[ImGuiCol_Border]               = Couleurs::Bordure;
    c[ImGuiCol_FrameBg]              = Couleurs::SurfaceHaute;
    c[ImGuiCol_FrameBgHovered]       = ImVec4(0.15f, 0.22f, 0.15f, 1.0f);
    c[ImGuiCol_FrameBgActive]        = ImVec4(0.18f, 0.28f, 0.18f, 1.0f);
    c[ImGuiCol_Button]               = Couleurs::AccentSombre;
    c[ImGuiCol_ButtonHovered]        = Couleurs::AccentDoux;
    c[ImGuiCol_ButtonActive]         = Couleurs::Accent;
    c[ImGuiCol_Text]                 = Couleurs::Texte;
    c[ImGuiCol_TextDisabled]         = Couleurs::TexteInactif;
    
    c[ImGuiCol_Tab]                  = Couleurs::SurfaceHaute;
    c[ImGuiCol_TabHovered]           = Couleurs::AccentDoux;
    c[ImGuiCol_TabActive]            = Couleurs::AccentSombre;
    c[ImGuiCol_TabUnfocused]         = Couleurs::Surface;
    c[ImGuiCol_TabUnfocusedActive]   = Couleurs::SurfaceHaute;
}