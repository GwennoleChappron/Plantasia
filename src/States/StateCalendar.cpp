#include "StateCalendar.hpp"
#include "imgui.h"

StateCalendar::StateCalendar(Application* app) : State(app) {}

void StateCalendar::onEnter() {
    m_app->getCalendar().generateTasks(m_app->getUserBalcony(), m_app->getDatabase(), m_app->getCalendar().getCurrentMonth());
}

void StateCalendar::onExit() {}
void StateCalendar::update(float dt) {}
void StateCalendar::draw(sf::RenderWindow& window) {}

void StateCalendar::drawImGui() {
    ImGui::SetNextWindowPos({45.f, 45.f});
    ImGui::SetNextWindowSize({(float)m_app->getWindow().getSize().x - 90.f, (float)m_app->getWindow().getSize().y - 90.f});
    ImGui::Begin("##calendar", nullptr, ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove);

    if (ImGui::Button("< Retour", ImVec2(80, 0))) {
        m_app->getStateMachine().removeState(); 
    }
    ImGui::SameLine(0, 20.f);
    ImGui::TextColored(ImVec4(0.20f, 0.75f, 0.40f, 1.00f), "CALENDRIER");
    ImGui::SameLine();
    ImGui::TextColored(ImVec4(0.55f, 0.68f, 0.55f, 1.00f), "- Vue Mensuelle Visuelle");
    ImGui::Dummy({0,6});
    ImGui::Separator();
    ImGui::Dummy({0,6});

    const char* mois[] = {"Janvier", "Fevrier", "Mars", "Avril", "Mai", "Juin", "Juillet", "Aout", "Septembre", "Octobre", "Novembre", "Decembre"};
    int currentMonthIdx = m_app->getCalendar().getCurrentMonth() - 1;
    
    ImGui::SetNextItemWidth(150.f);
    if (ImGui::Combo("##mois", &currentMonthIdx, mois, 12)) {
        m_app->getCalendar().generateTasks(m_app->getUserBalcony(), m_app->getDatabase(), currentMonthIdx + 1);
    }

    ImGui::Dummy({0, 10});

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
                    ImGui::TextColored(ImVec4(0.5f, 0.6f, 0.5f, 1.f), "%d", jourCourant); // Numéro du jour
                    
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
                        ImU32 color = IM_COL32(100, 200, 100, 255); // Vert par défaut
                        if (t->type == TaskType::ARROSAGE) color = IM_COL32(80, 180, 250, 255); // Bleu clair
                        if (t->type == TaskType::REMPOTAGE) color = IM_COL32(180, 120, 80, 255); // Marron/Terre
                        if (t->type == TaskType::RECOLTE) color = IM_COL32(220, 140, 50, 255); // Orange

                        // Dessin du cercle !
                        dl->AddCircleFilled(ImVec2(p0.x + offsetX, p0.y + 15.f), 6.f, color);
                        offsetX += 16.f;
                    }

                    // --- LE TOOLTIP (BULLE D'INFO) AU SURVOL ---
                    ImGui::SetCursorScreenPos(ImVec2(p0.x, p0.y - 15.f)); // Remonter au niveau du texte du jour
                    // On crée un bouton invisible qui recouvre toute la case
                    ImGui::InvisibleButton((std::string("##case") + std::to_string(jourCourant)).c_str(), ImVec2(ImGui::GetContentRegionAvail().x, 80.f));
                    
                    if (ImGui::IsItemHovered() && !tachesDuJour.empty()) {
                        ImGui::BeginTooltip();
                        ImGui::TextColored(ImVec4(0.4f, 0.9f, 0.5f, 1.f), "Agenda du %d %s :", jourCourant, mois[currentMonthIdx]);
                        ImGui::Separator();
                        for (auto* t : tachesDuJour) {
                            ImGui::Text("- %s : %s", t->nomPlante.c_str(), t->titre.c_str());
                            ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(0.6f, 0.6f, 0.6f, 1.f));
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