#include "StateCalendar.hpp"
#include "Core/Application.hpp"
#include "Services/DataBaseManager.hpp"
#include "UI/ColorTheme.hpp"
#include "UI/Widgets.hpp"
#include "Data/EnumInfo.hpp"
#include "imgui.h"
#include <vector>
#include <string>

StateCalendar::StateCalendar(Application* app) : State(app) {}

void StateCalendar::onEnter() {
    m_app->getCalendar().generateTasks(m_app->getUserBalcony(), m_app->getDatabase(), m_app->getCalendar().getCurrentMonth());
}

void StateCalendar::onExit() {}
void StateCalendar::update(float dt) {}
void StateCalendar::draw(sf::RenderWindow& window) {}

void StateCalendar::drawImGui() {
    float W = (float)m_app->getWindow().getSize().x;
    float H = (float)m_app->getWindow().getSize().y;
    const float MARGE = 45.f;

    ImGui::SetNextWindowPos({MARGE, MARGE});
    ImGui::SetNextWindowSize({W - MARGE * 2.f, H - MARGE * 2.f});
    ImGui::Begin("##calendar", nullptr, ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove);

    // ── TOP BAR ──
    UI::NavButtonPrimary("< Retour", [&](){ m_app->getStateMachine().removeState(); });
    ImGui::SameLine(0, 20.f);
    UI::LabelAccent("CALENDRIER", "- Vue Mensuelle Visuelle", Theme::TextSecondary);
    UI::Gap(6.f);
    ImGui::Separator();
    UI::Gap(6.f);

    // ── SÉLECTION DU MOIS (Lié à la source unique de vérité EnumInfo) ──
    int currentMonthIdx = m_app->getCalendar().getCurrentMonth() - 1;
    
    // Génération dynamique du tableau pour le Combo ImGui
    const char* moisNoms[12];
    for (int i = 0; i < 12; ++i) {
        moisNoms[i] = EnumInfo::getMois(i + 1).long_;
    }
    
    ImGui::SetNextItemWidth(150.f);
    if (ImGui::Combo("##mois", &currentMonthIdx, moisNoms, 12)) {
        m_app->getCalendar().generateTasks(m_app->getUserBalcony(), m_app->getDatabase(), currentMonthIdx + 1);
    }

    UI::Gap(10.f);

    // ════════ LA GRILLE DU CALENDRIER ════════
    int joursDansMois = 31;
    if (currentMonthIdx == 1) joursDansMois = 28;
    if (currentMonthIdx == 3 || currentMonthIdx == 5 || currentMonthIdx == 8 || currentMonthIdx == 10) joursDansMois = 30;

    // Tableau de 7 colonnes avec bordures
    if (ImGui::BeginTable("GrilleMois", 7, ImGuiTableFlags_Borders | ImGuiTableFlags_RowBg | ImGuiTableFlags_SizingStretchSame)) {
        const char* joursSemaine[] = {"Lun", "Mar", "Mer", "Jeu", "Ven", "Sam", "Dim"};
        for (int i = 0; i < 7; i++) {
            ImGui::TableSetupColumn(joursSemaine[i]);
        }
        ImGui::TableHeadersRow();

        int jourCourant = 1;
        auto& tasks = m_app->getCalendar().getTasks();

        for (int row = 0; row < 5; row++) {
            ImGui::TableNextRow(ImGuiTableRowFlags_None, 80.0f); // Hauteur des cases : 80 pixels
            
            for (int col = 0; col < 7; col++) {
                ImGui::TableSetColumnIndex(col);
                
                if (jourCourant <= joursDansMois) {
                    ImGui::TextColored(Theme::TextSecondary, "%d", jourCourant); // Numéro du jour
                    
                    // --- FILTRAGE DES TACHES POUR CE JOUR ---
                    std::vector<Task*> tachesDuJour;
                    for (auto& t : tasks) {
                        if (t.jour == jourCourant) tachesDuJour.push_back(&t);
                    }

                    // --- DESSIN DES PASTILLES DE COULEUR ---
                    ImVec2 p0 = ImGui::GetCursorScreenPos();
                    ImDrawList* dl = ImGui::GetWindowDrawList();
                    float offsetX = 5.f; // Décalage pour mettre les pastilles côte à côte
                    
                    for (auto* t : tachesDuJour) {
                        // Correspondance sémantique des couleurs avec ColorTheme.hpp
                        ImVec4 colorVec = Theme::PlantGreen; // Entretien par défaut
                        if (t->type == TaskType::ARROSAGE)  colorVec = Theme::InfoBlue;
                        if (t->type == TaskType::REMPOTAGE) colorVec = Theme::darken(Theme::WarningOrange, 0.5f); // Marron/Terre
                        if (t->type == TaskType::RECOLTE)   colorVec = Theme::WarningOrange;

                        ImU32 colorU32 = ImGui::ColorConvertFloat4ToU32(colorVec);

                        // Dessin du cercle
                        dl->AddCircleFilled(ImVec2(p0.x + offsetX, p0.y + 15.f), 6.f, colorU32);
                        offsetX += 16.f;
                    }

                    // --- LE TOOLTIP (BULLE D'INFO) AU SURVOL ---
                    ImGui::SetCursorScreenPos(ImVec2(p0.x, p0.y - 15.f)); // Remonter au niveau du texte du jour
                    
                    // On crée un bouton invisible qui recouvre toute la case
                    ImGui::InvisibleButton((std::string("##case") + std::to_string(jourCourant)).c_str(), 
                                           ImVec2(ImGui::GetContentRegionAvail().x, 80.f));
                    
                    if (ImGui::IsItemHovered() && !tachesDuJour.empty()) {
                        ImGui::BeginTooltip();
                        ImGui::TextColored(Theme::PlantGreen, "Agenda du %d %s :", jourCourant, moisNoms[currentMonthIdx]);
                        ImGui::Separator();
                        
                        for (auto* t : tachesDuJour) {
                            ImGui::Text("- %s : %s", t->nomPlante.c_str(), t->titre.c_str());
                            
                            // Description affichée en texte atténué (Muted)
                            ImGui::PushStyleColor(ImGuiCol_Text, Theme::TextMuted);
                            ImGui::Text("  %s", t->description.c_str());
                            ImGui::PopStyleColor();
                        }
                        ImGui::EndTooltip();
                    }

                    jourCourant++;
                }
            }
        }
        ImGui::EndTable();
    }

    ImGui::End();
}