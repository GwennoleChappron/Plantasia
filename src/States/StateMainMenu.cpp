#include "StateMainMenu.hpp"
#include "imgui.h"
#include "StateCatalogue.hpp"
#include "StateBalconySim.hpp"
#include "StateWiki.hpp"
#include "StateCalendar.hpp"

// On inclura bientôt les autres états ici, par exemple :
// #include "StateCatalogue.hpp" 

StateMainMenu::StateMainMenu(Application* app) : State(app) {}

void StateMainMenu::onEnter() {}
void StateMainMenu::onExit() {}
void StateMainMenu::update(float dt) {}
void StateMainMenu::draw(sf::RenderWindow& window) {
    // On ne dessine rien en SFML pur ici, le fond étoilé/boids s'en charge déjà !
}

void StateMainMenu::drawImGui() {
    // Paramétrage de la fenêtre ImGui pour qu'elle soit invisible et centrée
    ImGuiWindowFlags flags = ImGuiWindowFlags_NoDecoration | ImGuiWindowFlags_NoMove | 
                             ImGuiWindowFlags_NoSavedSettings | ImGuiWindowFlags_NoBackground;
                             
    const ImGuiViewport* viewport = ImGui::GetMainViewport();
    ImGui::SetNextWindowPos(viewport->WorkPos);
    ImGui::SetNextWindowSize(viewport->WorkSize);

    if (ImGui::Begin("MainMenuOverlay", nullptr, flags)) {
        
        // Calcul pour centrer le contenu
        float windowWidth = ImGui::GetWindowSize().x;
        float windowHeight = ImGui::GetWindowSize().y;
        float menuWidth = 300.0f;
        
        ImGui::SetCursorPos(ImVec2((windowWidth - menuWidth) * 0.5f, windowHeight * 0.25f));
        ImGui::BeginChild("MenuSection", ImVec2(menuWidth, 550.0f), false, flags);

        // Titre
        ImGui::SetWindowFontScale(2.5f);
        ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(0.20f, 0.75f, 0.40f, 1.00f)); // Vert Accent
        
        // Centrer le texte "PLANTASIA"
        float textWidth = ImGui::CalcTextSize("PLANTASIA").x;
        ImGui::SetCursorPosX((menuWidth - textWidth) * 0.5f);
        ImGui::Text("PLANTASIA");
        
        ImGui::PopStyleColor();
        ImGui::SetWindowFontScale(1.0f);
        
        // Sous-titre
        ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(0.55f, 0.68f, 0.55f, 1.00f));
        textWidth = ImGui::CalcTextSize("Mon écosystème de balcon").x;
        ImGui::SetCursorPosX((menuWidth - textWidth) * 0.5f);
        ImGui::Text("Mon ecosystème de balcon");
        ImGui::PopStyleColor();
        
        ImGui::Dummy(ImVec2(0.0f, 40.0f)); // Espace

        // --- LES BOUTONS DE NAVIGATION ---
        ImVec2 buttonSize(menuWidth, 45.0f);
        
        if (ImGui::Button("🌿 Mon Balcon (Catalogue)", buttonSize)) {
        // On "empile" le StateCatalogue par-dessus le Menu Principal
        // Le "false" indique qu'on ne remplace pas le menu, on le met juste en pause en dessous.
        m_app->getStateMachine().addState(std::make_unique<StateCatalogue>(m_app), false);
        }
        
        ImGui::Dummy(ImVec2(0.0f, 10.0f));
        
        if (ImGui::Button("☀️ Simulateur d'Ensoleillement", buttonSize)) {
         m_app->getStateMachine().addState(std::make_unique<StateBalconySim>(m_app), false);
        }
        
        ImGui::Dummy(ImVec2(0.0f, 10.0f));
        
        if (ImGui::Button("Wiki des Plantes", buttonSize)) {
            m_app->getStateMachine().addState(std::make_unique<StateWiki>(m_app), false);
        }
        
        ImGui::Dummy(ImVec2(0.0f, 10.0f));
        
        // Le tout nouveau bouton !
        if (ImGui::Button("Calendrier & Taches", buttonSize)) {
            m_app->getStateMachine().addState(std::make_unique<StateCalendar>(m_app), false);
        }
        
        // Bouton Quitter (rouge)
        ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.5f, 0.15f, 0.15f, 1.0f));
        ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4(0.75f, 0.22f, 0.22f, 1.0f));
        if (ImGui::Button("Quitter", buttonSize)) {
            m_app->getWindow().close();
        }
        ImGui::PopStyleColor(2);

        ImGui::EndChild();
    }
    ImGui::End();
}