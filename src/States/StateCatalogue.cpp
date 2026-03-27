#include "StateCatalogue.hpp"
#include <algorithm>

// --- HELPERS LOCAUX ---
namespace {
    const char* RusiciteLabel(Rusticite r) {
        switch (r) {
            case Rusticite::RUSTIQUE:      return "Rustique";
            case Rusticite::SEMI_RUSTIQUE: return "Semi-rustique";
            case Rusticite::FRAGILE:       return "Fragile";
            case Rusticite::GELIVE:        return "Gélive";
            case Rusticite::TROPICALE:     return "Tropicale";
        }
        return "?";
    }

    ImVec4 CouleurRusticite(Rusticite r) {
        switch (r) {
            case Rusticite::RUSTIQUE:      return ImVec4(0.30f, 0.80f, 0.45f, 1.0f);
            case Rusticite::SEMI_RUSTIQUE: return ImVec4(0.60f, 0.80f, 0.30f, 1.0f);
            case Rusticite::FRAGILE:       return ImVec4(0.90f, 0.70f, 0.20f, 1.0f);
            case Rusticite::GELIVE:        return ImVec4(0.50f, 0.80f, 0.95f, 1.0f);
            case Rusticite::TROPICALE:     return ImVec4(0.95f, 0.55f, 0.25f, 1.0f);
        }
        return ImVec4(0.88f, 0.92f, 0.88f, 1.00f);
    }

    ImVec4 CouleurTypePlante(TypePlante t) {
        switch (t) {
            case TypePlante::AROMATIQUE: return ImVec4(0.4f, 0.8f, 0.5f, 1.0f); 
            case TypePlante::FRUITIER:   return ImVec4(1.0f, 0.6f, 0.2f, 1.0f); 
            case TypePlante::FLEUR:      return ImVec4(1.0f, 0.5f, 0.7f, 1.0f); 
            case TypePlante::LEGUME:     return ImVec4(0.3f, 0.7f, 1.0f, 1.0f); 
        }
        return ImVec4(0.55f, 0.68f, 0.55f, 1.00f);
    }
}

StateCatalogue::StateCatalogue(Application* app) : State(app) {}
void StateCatalogue::onEnter() {}
void StateCatalogue::onExit() {}
void StateCatalogue::update(float dt) {}
void StateCatalogue::draw(sf::RenderWindow& window) {}

void StateCatalogue::DessinerCalendrierVisuel(const char* label, int debut, int fin, ImVec4 col) {
    ImGui::TextColored(ImVec4(0.55f, 0.68f, 0.55f, 1.00f), "%s", label);
    ImVec2 p = ImGui::GetCursorScreenPos();
    float w = ImGui::GetContentRegionAvail().x;
    float step = w / 12.f;
    ImDrawList* dl = ImGui::GetWindowDrawList();
    const char* initMois[] = {"J","F","M","A","M","J","J","A","S","O","N","D"};
    
    for(int i = 0; i < 12; i++) {
        bool on = false;
        if (debut > 0 && fin > 0) {
            int d = debut - 1, f = fin - 1;
            if (d <= f) on = (i >= d && i <= f);
            else        on = (i >= d || i <= f);
        }
        ImVec2 pmin(p.x + i * step + 2, p.y);
        ImVec2 pmax(p.x + (i + 1) * step - 2, p.y + 20); 
        dl->AddRectFilled(pmin, pmax, on ? ImGui::ColorConvertFloat4ToU32(col) : IM_COL32(35,45,35,255), 4.f);
        if (on) {
            ImVec2 tsz = ImGui::CalcTextSize(initMois[i]);
            dl->AddText(ImVec2(pmin.x + (step - 4 - tsz.x) * 0.5f, pmin.y + 3), IM_COL32(10,20,10,255), initMois[i]);
        }
    }
    ImGui::Dummy(ImVec2(0, 26)); 
}

void StateCatalogue::drawImGui() {
    float W = (float)m_app->getWindow().getSize().x;
    float H = (float)m_app->getWindow().getSize().y;
    const float LARGEUR_LISTE = 340.f;
    const float MARGE = 45.f; 

    ImGui::SetNextWindowPos({MARGE, MARGE});
    ImGui::SetNextWindowSize({W - MARGE*2, H - MARGE*2});
    ImGui::Begin("##principal", nullptr, ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove);

    if (ImGui::Button("< Retour", ImVec2(80, 0))) {
        m_app->getStateMachine().removeState(); 
    }
    ImGui::SameLine(0, 20.f);
    ImGui::TextColored(ImVec4(0.20f, 0.75f, 0.40f, 1.00f), "PLANTASIA");
    ImGui::SameLine();
    ImGui::TextColored(ImVec4(0.55f, 0.68f, 0.55f, 1.00f), "- Mon Balcon");
    ImGui::Dummy({0,6});

    float hauteurContenu = ImGui::GetContentRegionAvail().y;
    float largeurDetail  = ImGui::GetContentRegionAvail().x - LARGEUR_LISTE - 16.f;

    // ════════ PANNEAU GAUCHE (MES PLANTES) ════════
    ImGui::BeginChild("##gauche", {LARGEUR_LISTE, hauteurContenu}, false);
    
    ImGui::SetNextItemWidth(-1.f);
    ImGui::InputTextWithHint("##rech", "Rechercher...", m_filtreRecherche, sizeof(m_filtreRecherche));
    ImGui::Spacing();

    const auto& mesPlantes = m_app->getUserBalcony().getMesPlantes();
    std::string filtre(m_filtreRecherche);
    std::transform(filtre.begin(), filtre.end(), filtre.begin(), ::tolower);

    ImGui::BeginChild("##scroll_liste", {-1, -1}, false);
    for (int i = 0; i < (int)mesPlantes.size(); i++) {
        const UserPlant& up = mesPlantes[i];
        
        // On interroge la base de données pour avoir les infos scientifiques (couleur, type)
        const Plante* refP = m_app->getDatabase().getPlante(up.nom_espece);
        if (!refP) continue;

        std::string n = up.surnom;
        std::transform(n.begin(), n.end(), n.begin(), ::tolower);
        if (!filtre.empty() && n.find(filtre) == std::string::npos) continue;

        ImGui::PushID(i);
        bool sel  = (m_indexSelectionne == i);
        ImVec2 pos0 = ImGui::GetCursorScreenPos();
        ImVec2 sz   = {ImGui::GetContentRegionAvail().x, 58.f}; 

        ImGui::InvisibleButton("##c", sz);
        bool hov = ImGui::IsItemHovered();
        if (ImGui::IsItemClicked()) m_indexSelectionne = sel ? -1 : i;

        ImDrawList* dlL = ImGui::GetWindowDrawList();
        
        if (sel)       dlL->AddRectFilled(pos0, {pos0.x+sz.x, pos0.y+sz.y}, IM_COL32(25,60,35,255), 8.f);
        else if (hov)  dlL->AddRectFilled(pos0, {pos0.x+sz.x, pos0.y+sz.y}, IM_COL32(20,40,25,200), 8.f);
        else           dlL->AddRectFilled(pos0, {pos0.x+sz.x, pos0.y+sz.y}, IM_COL32(15,22,15,150), 8.f);
        
        ImVec4 colType = CouleurTypePlante(refP->type);
        dlL->AddRectFilled(pos0, {pos0.x + 6.f, pos0.y+sz.y}, ImGui::ColorConvertFloat4ToU32(colType), 8.f, ImDrawFlags_RoundCornersLeft);

        ImVec4 cNom = sel ? ImVec4(0.20f, 0.75f, 0.40f, 1.00f) : (hov ? ImVec4(0.88f, 0.92f, 0.88f, 1.00f) : ImVec4(0.85f,0.90f,0.85f,1.f));
        ImGui::SetCursorScreenPos({pos0.x+18, pos0.y+10});
        ImGui::TextColored(cNom, "%s", up.surnom.c_str());
        
        ImGui::SetCursorScreenPos({pos0.x+18, pos0.y+32});
        ImGui::TextColored(CouleurRusticite(refP->rusticite), "- %s", RusiciteLabel(refP->rusticite));

        ImGui::SetCursorScreenPos({pos0.x, pos0.y+sz.y + 4.f});
        ImGui::PopID();
    }
    ImGui::EndChild();
    ImGui::EndChild();

    ImGui::SameLine(0, 16.f);

    // ════════ PANNEAU DROIT (DETAILS) ════════
    ImGui::BeginChild("##detail", {largeurDetail, hauteurContenu}, true);

    if (m_indexSelectionne >= 0 && m_indexSelectionne < (int)mesPlantes.size()) {
        const UserPlant& up = mesPlantes[m_indexSelectionne];
        const Plante* refP = m_app->getDatabase().getPlante(up.nom_espece);

        if (refP) {
            ImGui::SetWindowFontScale(1.4f);
            ImGui::TextColored(ImVec4(0.20f, 0.75f, 0.40f, 1.00f), "%s", up.surnom.c_str());
            ImGui::SetWindowFontScale(1.0f);
            
            // DONNÉES UTILISATEUR !
            // On a besoin de récupérer une référence non-constante pour pouvoir la modifier
            UserPlant& editablePlant = const_cast<UserPlant&>(mesPlantes[m_indexSelectionne]);

            ImGui::TextColored(ImVec4(0.55f, 0.68f, 0.55f, 1.00f), "Espece : %s", refP->nom.c_str());
            
            // --- EDITEUR DE DATE D'ADOPTION ---
            ImGui::TextColored(ImVec4(0.88f, 0.92f, 0.88f, 1.00f), "Date d'adoption :");
            ImGui::SameLine();
            
            ImGui::PushItemWidth(60);
            bool dateChanged = false;
            if (ImGui::DragInt("##jour", &editablePlant.jour_achat, 0.5f, 1, 31, "%02d")) dateChanged = true;
            ImGui::SameLine(); ImGui::Text("/"); ImGui::SameLine();
            if (ImGui::DragInt("##mois", &editablePlant.mois_achat, 0.5f, 1, 12, "%02d")) dateChanged = true;
            ImGui::SameLine(); ImGui::Text("/"); ImGui::SameLine();
            if (ImGui::DragInt("##annee", &editablePlant.annee_achat, 0.5f, 2000, 2050)) dateChanged = true;
            ImGui::PopItemWidth();

            ImGui::SameLine(0, 20);
            ImGui::PushItemWidth(100);
            ImGui::TextColored(ImVec4(0.88f, 0.92f, 0.88f, 1.00f), "Pot (Litres) :");
            ImGui::SameLine();
            if (ImGui::DragInt("##pot", &editablePlant.volume_pot_actuel_L, 0.5f, 1, 500)) dateChanged = true;
            ImGui::PopItemWidth();

            // Si on a modifié la date ou le pot, on sauvegarde dans le JSON !
            if (dateChanged) {
                m_app->getUserBalcony().sauvegarderProfil("mon_balcon.json");
            }

            ImGui::Separator();
            ImGui::Spacing();

            // Calendriers
            if (refP->floraison_debut > 0 || refP->recolte_debut > 0) {
                if (refP->floraison_debut > 0) 
                    DessinerCalendrierVisuel("Periode de Floraison", refP->floraison_debut, refP->floraison_fin, ImVec4(0.9f, 0.5f, 0.8f, 1.0f));
                if (refP->recolte_debut > 0) 
                    DessinerCalendrierVisuel("Periode de Recolte", refP->recolte_debut, refP->recolte_fin, ImVec4(0.5f, 0.8f, 0.4f, 1.0f));
            }

            // Les onglets normaux
            if (ImGui::BeginTabBar("OngletsConseils")) {
                auto SectionInfo = [&](const char* icon, const char* titre, const char* contenu, ImVec4 color) {
                    if (!contenu || std::strlen(contenu) == 0) return;
                    ImGui::TextColored(color, "[ %s ]", titre);
                    ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(0.55f, 0.68f, 0.55f, 1.00f));
                    ImGui::TextWrapped("%s", contenu);
                    ImGui::PopStyleColor();
                    ImGui::Dummy({0,8});
                };

                if (ImGui::BeginTabItem("Besoins & Eau")) {
                    ImGui::Spacing();
                    SectionInfo("-", "Volume du pot min.", refP->volume_pot_min.c_str(), ImVec4(0.8f, 0.8f, 0.8f, 1.f));
                    SectionInfo("-", "Substrat recommande", refP->conseil_terre.c_str(), ImVec4(0.8f, 0.6f, 0.3f, 1.f));
                    
                    ImGui::Separator();
                    ImGui::Dummy({0,4});
                    
                    ImGui::TextColored(ImVec4(0.3f, 0.8f, 1.0f, 1.f), "Besoin en eau");
                    ImVec2 p0 = ImGui::GetCursorScreenPos();
                    ImDrawList* dl = ImGui::GetWindowDrawList();
                    for (int i = 0; i < 4; i++) {
                        ImU32 col = (i < refP->besoin_eau) ? IM_COL32(80, 200, 255, 255) : IM_COL32(40, 60, 70, 255);
                        dl->AddRectFilled(ImVec2(p0.x + i * 12.f, p0.y + 2.f), ImVec2(p0.x + i * 12.f + 8.f, p0.y + 14.f), col, 2.f);
                    }
                    ImGui::Dummy({0, 16});
                    
                    SectionInfo("-", "Conseil Arrosage", refP->conseil_arrosage.c_str(), ImVec4(0.55f, 0.68f, 0.55f, 1.00f));
                    ImGui::EndTabItem();
                }

                if (ImGui::BeginTabItem("Entretien & Sante")) {
                    ImGui::Spacing();
                    SectionInfo("-", "Taille & Soins", refP->conseil_entretien.c_str(), ImVec4(0.5f, 0.8f, 0.5f, 1.f));
                    SectionInfo("-", "Rempotage", refP->rempotage.c_str(), ImVec4(0.6f, 0.5f, 0.8f, 1.f));
                    ImGui::EndTabItem();
                }
                ImGui::EndTabBar();
            }
        }
    } else {
        ImVec2 avail = ImGui::GetContentRegionAvail();
        ImVec2 pos = ImGui::GetCursorScreenPos();
        ImDrawList* dl = ImGui::GetWindowDrawList();
        dl->AddCircleFilled(ImVec2(pos.x + avail.x*0.5f, pos.y + avail.y*0.4f), 50.f, IM_COL32(35, 60, 40, 150));
        
        const char* msg = mesPlantes.empty() ? "Votre balcon est vide ! Allez dans le Wiki pour adopter des plantes." 
                                             : "Selectionnez une de vos plantes";
        ImVec2 tsz = ImGui::CalcTextSize(msg);
        ImGui::SetCursorScreenPos(ImVec2(pos.x + (avail.x - tsz.x)*0.5f, pos.y + avail.y*0.4f + 80.f));
        ImGui::TextColored(ImVec4(0.30f, 0.40f, 0.30f, 1.00f), "%s", msg);
    }

    ImGui::EndChild();
    ImGui::End();
}