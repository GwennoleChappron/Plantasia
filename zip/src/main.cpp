#ifdef _WIN32
#include <windows.h>
#pragma comment(linker, "/SUBSYSTEM:console")
#endif

#include <SFML/Graphics.hpp>
#include "imgui.h"
#include "imgui-SFML.h"
#include "Catalogue.hpp"
#include <iostream>
#include <string>

void AppliquerStyle() {
    ImGuiStyle& style = ImGui::GetStyle();
    style.WindowRounding = 10.0f;
    style.FrameRounding = 5.0f;
    style.Colors[ImGuiCol_WindowBg] = ImVec4(0.08f, 0.12f, 0.08f, 1.00f); // Vert très sombre
    style.Colors[ImGuiCol_Header] = ImVec4(0.20f, 0.40f, 0.20f, 1.00f);    // Vert feuille
    style.Colors[ImGuiCol_Button] = ImVec4(0.25f, 0.45f, 0.25f, 1.00f);
    style.Colors[ImGuiCol_ButtonHovered] = ImVec4(0.35f, 0.65f, 0.35f, 1.00f);
    style.Colors[ImGuiCol_ButtonActive] = ImVec4(0.15f, 0.35f, 0.15f, 1.00f);
}

int main(int argc, char** argv) {
    // Fix pour les accents dans la console Windows
#ifdef _WIN32
    SetConsoleOutputCP(CP_UTF8);
#endif

    sf::RenderWindow window(sf::VideoMode(1000, 700), "Plantasia - Mon Jardin");
    window.setFramerateLimit(60);

    if (!ImGui::SFML::Init(window)) return -1;

    AppliquerStyle();

    Catalogue monBalcon;
    monBalcon.charger("balcon.json");

    int indexPlanteSelectionnee = -1; 
    sf::Clock deltaClock;

    while (window.isOpen()) {
        sf::Event event;
        while (window.pollEvent(event)) {
            ImGui::SFML::ProcessEvent(window, event);
            if (event.type == sf::Event::Closed) window.close();
        }

        ImGui::SFML::Update(window, deltaClock.restart());

        // --- FENÊTRE PRINCIPALE (CATALOGUE) ---
        ImGui::SetNextWindowPos(ImVec2(0, 0));
        sf::Vector2f winSize = window.getView().getSize();
        ImGui::SetNextWindowSize(ImVec2(winSize.x, winSize.y)); 
        
        ImGui::Begin("CataloguePrincipal", nullptr, ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove);

        ImGui::TextColored(ImVec4(0.5f, 1.0f, 0.5f, 1.0f), "🌿 MON BALCON BOTANIQUE");
        ImGui::Separator();
        ImGui::Spacing();

        if (ImGui::BeginChild("ScrollingRegion")) {
            // Utilisation d'une Table pour un alignement parfait
            if (ImGui::BeginTable("GrillePlantes", 2, ImGuiTableFlags_SizingStretchSame | ImGuiTableFlags_Resizable)) {
                
                const auto& listePlantes = monBalcon.getPlantes();
                for (int i = 0; i < (int)listePlantes.size(); i++) {
                    ImGui::TableNextColumn();
                    ImGui::PushID(i);
                    
                    ImGui::BeginGroup();
                    ImGui::Text("Plante : %s", listePlantes[i].getNom().c_str());
                    
                    // On utilise un ID interne (##) pour que chaque bouton soit unique
                    std::string label = "Voir Details##" + std::to_string(i);
                    if (ImGui::Button(label.c_str())) {
                        indexPlanteSelectionnee = i;
                        std::cout << "Selection : " << listePlantes[i].getNom() << std::endl;
                    }
                    ImGui::EndGroup();

                    ImGui::Spacing();
                    ImGui::Separator();
                    ImGui::PopID();
                }
                ImGui::EndTable();
            }
            ImGui::EndChild();
        }
        ImGui::End();

        // --- DÉTAILS (Version Popup Modale) ---
        if (indexPlanteSelectionnee != -1) {
            // On prépare le nom pour le titre
            const auto& plante = monBalcon.getPlantes()[indexPlanteSelectionnee];
            std::string titrePopup = "Fiche Detaillee###DetailWindow";

            // On force l'ouverture d'un popup
            ImGui::OpenPopup(titrePopup.c_str());

            // Positionnement au centre de l'écran
            ImGui::SetNextWindowPos(ImVec2(winSize.x * 0.5f, winSize.y * 0.5f), ImGuiCond_Always, ImVec2(0.5f, 0.5f));

            if (ImGui::BeginPopupModal(titrePopup.c_str(), nullptr, ImGuiWindowFlags_AlwaysAutoResize)) {
                
                ImGui::TextColored(ImVec4(0.5f, 1.0f, 0.5f, 1.0f), "🌱 %s", plante.getNom().c_str());
                ImGui::Separator();
                ImGui::Spacing();

                // Affichage des infos
                ImGui::Text("Nom commun : %s", plante.getNom().c_str());
                
                // Si tu veux tester l'affichage d'une autre info (si présente dans ton JSON/Plante) :
                // ImGui::Text("Type : %s", "Plante de balcon"); 

                ImGui::Spacing();
                ImGui::Separator();

                if (ImGui::Button("Fermer", ImVec2(120, 0))) {
                    indexPlanteSelectionnee = -1;
                    ImGui::CloseCurrentPopup();
                }
                ImGui::EndPopup();
            }
        }

        // --- RENDU ---
        window.clear(sf::Color(20, 30, 20)); 
        ImGui::SFML::Render(window);
        window.display();
    }

    ImGui::SFML::Shutdown();
    return 0;
}