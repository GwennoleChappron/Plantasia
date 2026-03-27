#include "StateWiki.hpp"
#include <algorithm>

// --- HELPERS LOCAUX POUR L'UI ---
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

StateWiki::StateWiki(Application* app) : State(app) {}

void StateWiki::onEnter() {}
void StateWiki::onExit() {}
void StateWiki::update(float dt) {}
void StateWiki::draw(sf::RenderWindow& window) {}

void StateWiki::DessinerCalendrierVisuel(const char* label, int debut, int fin, ImVec4 col) {
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

void StateWiki::drawImGui() {
    float W = (float)m_app->getWindow().getSize().x;
    float H = (float)m_app->getWindow().getSize().y;
    const float LARGEUR_LISTE = 340.f;
    const float MARGE = 45.f; 

    ImGui::SetNextWindowPos({MARGE, MARGE});
    ImGui::SetNextWindowSize({W - MARGE*2, H - MARGE*2});
    ImGui::Begin("##principal", nullptr, ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove);

    // Header avec Bouton Retour
    if (ImGui::Button("< Retour", ImVec2(80, 0))) {
        m_app->getStateMachine().removeState(); 
    }
    ImGui::SameLine(0, 20.f);
    ImGui::TextColored(ImVec4(0.20f, 0.75f, 0.40f, 1.00f), "ENCYCLOPEDIE");
    ImGui::SameLine();
    ImGui::TextColored(ImVec4(0.55f, 0.68f, 0.55f, 1.00f), "- Base de donnees botanique");
    ImGui::Dummy({0,6});

    float hauteurContenu = ImGui::GetContentRegionAvail().y;
    float largeurDetail  = ImGui::GetContentRegionAvail().x - LARGEUR_LISTE - 16.f;

    // ════════ PANNEAU GAUCHE (LISTE) ════════
    ImGui::BeginChild("##gauche", {LARGEUR_LISTE, hauteurContenu}, false);
    
    ImGui::SetNextItemWidth(-1.f);
    ImGui::InputTextWithHint("##rech", "Rechercher une plante...", m_filtreRecherche, sizeof(m_filtreRecherche));
    ImGui::Spacing();

    // 1. On récupère toutes les plantes de l'encyclopédie et on les trie par nom
    const auto& allPlantesMap = m_app->getDatabase().getAllPlantes();
    std::vector<const Plante*> plantesTriees;
    for (const auto& pair : allPlantesMap) {
        plantesTriees.push_back(&pair.second);
    }
    std::sort(plantesTriees.begin(), plantesTriees.end(), [](const Plante* a, const Plante* b) {
        return a->nom < b->nom; // Tri alphabétique
    });

    std::string filtre(m_filtreRecherche);
    std::transform(filtre.begin(), filtre.end(), filtre.begin(), ::tolower);

    ImGui::BeginChild("##scroll_liste", {-1, -1}, false);
    for (size_t i = 0; i < plantesTriees.size(); i++) {
        const Plante* p = plantesTriees[i];
        
        std::string n = p->nom;
        std::transform(n.begin(), n.end(), n.begin(), ::tolower);
        if (!filtre.empty() && n.find(filtre) == std::string::npos) continue;

        ImGui::PushID((int)i);
        bool sel  = (m_planteSelectionnee == p->nom);
        ImVec2 pos0 = ImGui::GetCursorScreenPos();
        ImVec2 sz   = {ImGui::GetContentRegionAvail().x, 58.f}; 

        ImGui::InvisibleButton("##c", sz);
        bool hov = ImGui::IsItemHovered();
        if (ImGui::IsItemClicked()) m_planteSelectionnee = sel ? "" : p->nom;

        ImDrawList* dlL = ImGui::GetWindowDrawList();
        
        if (sel)       dlL->AddRectFilled(pos0, {pos0.x+sz.x, pos0.y+sz.y}, IM_COL32(25,60,35,255), 8.f);
        else if (hov)  dlL->AddRectFilled(pos0, {pos0.x+sz.x, pos0.y+sz.y}, IM_COL32(20,40,25,200), 8.f);
        else           dlL->AddRectFilled(pos0, {pos0.x+sz.x, pos0.y+sz.y}, IM_COL32(15,22,15,150), 8.f);
        
        ImVec4 colType = CouleurTypePlante(p->type);
        dlL->AddRectFilled(pos0, {pos0.x + 6.f, pos0.y+sz.y}, ImGui::ColorConvertFloat4ToU32(colType), 8.f, ImDrawFlags_RoundCornersLeft);

        ImVec4 cNom = sel ? ImVec4(0.20f, 0.75f, 0.40f, 1.00f) : (hov ? ImVec4(0.88f, 0.92f, 0.88f, 1.00f) : ImVec4(0.85f,0.90f,0.85f,1.f));
        ImGui::SetCursorScreenPos({pos0.x+18, pos0.y+10});
        ImGui::TextColored(cNom, "%s", p->nom.c_str());
        
        ImGui::SetCursorScreenPos({pos0.x+18, pos0.y+32});
        ImGui::TextColored(CouleurRusticite(p->rusticite), "- %s", RusiciteLabel(p->rusticite));

        ImGui::SetCursorScreenPos({pos0.x, pos0.y+sz.y + 4.f});
        ImGui::PopID();
    }
    ImGui::EndChild();
    ImGui::EndChild();

    ImGui::SameLine(0, 16.f);

    // ════════ PANNEAU DROIT (DETAILS) ════════
    ImGui::BeginChild("##detail", {largeurDetail, hauteurContenu}, true);

    const Plante* p = m_app->getDatabase().getPlante(m_planteSelectionnee);

    if (p != nullptr) {
        
        // --- Header Plante et Bouton d'ajout ---
        ImGui::BeginGroup();
        ImGui::SetWindowFontScale(1.4f);
        ImGui::TextColored(ImVec4(0.20f, 0.75f, 0.40f, 1.00f), "%s", p->nom.c_str());
        ImGui::SetWindowFontScale(1.0f);
        ImGui::EndGroup();

        ImGui::SameLine(ImGui::GetContentRegionAvail().x - 140.f);
        
        // LE BOUTON MAGIQUE QUI CREE LE PONT !
        ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.2f, 0.6f, 0.3f, 1.f));
        ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4(0.3f, 0.8f, 0.4f, 1.f));
        if (ImGui::Button("+ Adopter", ImVec2(140, 30))) {
            UserPlant nouvellePlante;
            nouvellePlante.nom_espece = p->nom;
            nouvellePlante.surnom = p->nom; // Par défaut, on lui donne le nom de l'espèce
            nouvellePlante.jour_achat = 27; // On met une date par défaut
            nouvellePlante.mois_achat = 3;
            nouvellePlante.annee_achat = 2026;
            
            m_app->getUserBalcony().ajouterPlante(nouvellePlante);
            m_app->getUserBalcony().sauvegarderProfil("mon_balcon.json");
        }
        ImGui::PopStyleColor(2);

        ImGui::Separator();
        ImGui::Spacing();

        if (p->floraison_debut > 0 || p->recolte_debut > 0) {
            if (p->floraison_debut > 0) 
                DessinerCalendrierVisuel("Periode de Floraison", p->floraison_debut, p->floraison_fin, ImVec4(0.9f, 0.5f, 0.8f, 1.0f));
            if (p->recolte_debut > 0) 
                DessinerCalendrierVisuel("Periode de Recolte", p->recolte_debut, p->recolte_fin, ImVec4(0.5f, 0.8f, 0.4f, 1.0f));
        }

        if (ImGui::BeginTabBar("OngletsConseils")) {
            auto SectionInfo = [&](const char* icon, const char* titre, const char* contenu, ImVec4 color) {
                if (!contenu || std::strlen(contenu) == 0) return;
                ImGui::TextColored(color, "[ %s ]", titre); // On remplace l'icône par des crochets propres
                ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(0.55f, 0.68f, 0.55f, 1.00f));
                ImGui::TextWrapped("%s", contenu);
                ImGui::PopStyleColor();
                ImGui::Dummy({0,8});
            };

            if (ImGui::BeginTabItem("Besoins & Eau")) {
                ImGui::Spacing();
                SectionInfo("-", "Volume du pot min.", p->volume_pot_min.c_str(), ImVec4(0.8f, 0.8f, 0.8f, 1.f));
                SectionInfo("-", "Substrat recommande", p->conseil_terre.c_str(), ImVec4(0.8f, 0.6f, 0.3f, 1.f));
                
                ImGui::Separator();
                ImGui::Dummy({0,4});
                
                ImGui::TextColored(ImVec4(0.3f, 0.8f, 1.0f, 1.f), "Besoin en eau");
                ImVec2 p0 = ImGui::GetCursorScreenPos();
                ImDrawList* dl = ImGui::GetWindowDrawList();
                for (int i = 0; i < 4; i++) {
                    ImU32 col = (i < p->besoin_eau) ? IM_COL32(80, 200, 255, 255) : IM_COL32(40, 60, 70, 255);
                    dl->AddRectFilled(ImVec2(p0.x + i * 12.f, p0.y + 2.f), ImVec2(p0.x + i * 12.f + 8.f, p0.y + 14.f), col, 2.f);
                }
                ImGui::Dummy({0, 16});
                
                SectionInfo("-", "Conseil Arrosage", p->conseil_arrosage.c_str(), ImVec4(0.55f, 0.68f, 0.55f, 1.00f));
                
                ImGui::Columns(2, "colonnes_arrosage", false);
                SectionInfo("-", "Frequence Ete", p->frequence_arrosage_ete.c_str(), ImVec4(0.9f, 0.8f, 0.3f, 1.f));
                ImGui::NextColumn();
                SectionInfo("-", "Frequence Hiver", p->frequence_arrosage_hiver.c_str(), ImVec4(0.6f, 0.8f, 0.9f, 1.f));
                ImGui::Columns(1);
                
                ImGui::EndTabItem();
            }

            if (ImGui::BeginTabItem("Entretien & Sante")) {
                ImGui::Spacing();
                SectionInfo("-", "Taille & Soins", p->conseil_entretien.c_str(), ImVec4(0.5f, 0.8f, 0.5f, 1.f));
                SectionInfo("-", "Rempotage", p->rempotage.c_str(), ImVec4(0.6f, 0.5f, 0.8f, 1.f));
                
                if (!p->maladies.empty()) {
                    ImGui::Separator();
                    ImGui::Dummy({0,4});
                    SectionInfo("-", "Maladies & Parasites", p->maladies.c_str(), ImVec4(0.9f, 0.4f, 0.4f, 1.f));
                }
                ImGui::EndTabItem();
            }

            if (!p->compagnonnage.empty() || !p->toxicite_animaux.empty() || !p->notes.empty()) {
                if (ImGui::BeginTabItem("Autour de la plante")) {
                    ImGui::Spacing();
                    SectionInfo("-", "Compagnonnage", p->compagnonnage.c_str(), ImVec4(0.4f, 0.9f, 0.6f, 1.f));
                    
                    if (p->toxicite_animaux == "OUI") {
                        SectionInfo("!", "Toxicite Animaux", "Plante toxique pour les animaux de compagnie !", ImVec4(0.75f, 0.22f, 0.22f, 1.00f));
                    } else if (p->toxicite_animaux == "NON") {
                        SectionInfo("-", "Toxicite Animaux", "Sans danger (Pet-friendly)", ImVec4(0.5f, 0.8f, 0.5f, 1.f));
                    }
                    SectionInfo("-", "Notes", p->notes.c_str(), ImVec4(0.30f, 0.40f, 0.30f, 1.00f));
                    ImGui::EndTabItem();
                }
            }
            ImGui::EndTabBar();
        }

        if (!p->astuce_pro.empty()) {
            ImGui::Dummy({0, 10});
            ImVec2 p0 = ImGui::GetCursorScreenPos();
            ImVec2 avail = ImGui::GetContentRegionAvail();
            
            ImGui::GetWindowDrawList()->AddRectFilled(p0, ImVec2(p0.x + avail.x, p0.y + 60.f), IM_COL32(30, 50, 20, 180), 8.f);
            ImGui::GetWindowDrawList()->AddRectFilled(p0, ImVec2(p0.x + 6.f, p0.y + 60.f), IM_COL32(80, 200, 100, 255), 8.f, ImDrawFlags_RoundCornersLeft);

            ImGui::SetCursorScreenPos(ImVec2(p0.x + 15.f, p0.y + 8.f));
            ImGui::TextColored(ImVec4(0.4f, 0.9f, 0.5f, 1.f), "Astuce de Pro");
            ImGui::SetCursorScreenPos(ImVec2(p0.x + 15.f, p0.y + 28.f));
            
            ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(0.88f, 0.92f, 0.88f, 1.00f));
            ImGui::PushTextWrapPos(p0.x + avail.x - 10.f);
            ImGui::Text("%s", p->astuce_pro.c_str());
            ImGui::PopTextWrapPos();
            ImGui::PopStyleColor();
            ImGui::Dummy({0, 30}); 
        }
    } else {
        ImVec2 avail = ImGui::GetContentRegionAvail();
        ImVec2 pos = ImGui::GetCursorScreenPos();
        ImDrawList* dl = ImGui::GetWindowDrawList();
        
        dl->AddCircleFilled(ImVec2(pos.x + avail.x*0.5f, pos.y + avail.y*0.4f), 50.f, IM_COL32(35, 60, 40, 150));
        
        const char* msg = "Selectionnez une plante de l'encyclopedie";
        ImVec2 tsz = ImGui::CalcTextSize(msg);
        ImGui::SetCursorScreenPos(ImVec2(pos.x + (avail.x - tsz.x)*0.5f, pos.y + avail.y*0.4f + 80.f));
        ImGui::TextColored(ImVec4(0.30f, 0.40f, 0.30f, 1.00f), "%s", msg);
    }

    ImGui::EndChild();
    ImGui::End();
}