#include "StateWiki.hpp"
#include <algorithm>
#include <ctime>
#include <cmath> // IMPORTANT pour std::clamp

// --- HELPERS LOCAUX POUR L'UI ---
namespace {
    const float PI = 3.1415926535f;

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

    void DrawJaugeCirculaire(const char* label, float valeur, float valeur_max, ImVec2 centre, float rayon, ImU32 couleurJauge, ImU32 couleurFond) {
        ImDrawList* dl = ImGui::GetWindowDrawList();
        float epaisseur = 6.0f; 

        dl->AddCircle(centre, rayon, couleurFond, 32, epaisseur);

        float fraction = std::clamp(valeur / valeur_max, 0.0f, 1.0f);
        if (fraction > 0.01f) {
            float angle_min = -PI / 2.0f; 
            float angle_max = angle_min + (fraction * PI * 2.0f);
            
            dl->PathClear();
            dl->PathArcTo(centre, rayon, angle_min, angle_max, 32);
            dl->PathStroke(couleurJauge, 0, epaisseur); 
        }

        char buf[16];
        snprintf(buf, sizeof(buf), "%.0f", valeur);
        ImVec2 tSize = ImGui::CalcTextSize(buf);
        dl->AddText(ImVec2(centre.x - tSize.x * 0.5f, centre.y - tSize.y * 0.5f), IM_COL32(255, 255, 255, 255), buf);

        ImVec2 lSize = ImGui::CalcTextSize(label);
        dl->AddText(ImVec2(centre.x - lSize.x * 0.5f, centre.y + rayon + 8.0f), IM_COL32(180, 200, 180, 255), label);
    }

    const char* SoleilLabel(ExpositionSoleil e) {
        switch(e) {
            case ExpositionSoleil::PLEIN_SOLEIL: return "Plein Soleil";
            case ExpositionSoleil::MI_OMBRE: return "Mi-Ombre";
            case ExpositionSoleil::OMBRE_CLAIRE: return "Ombre Claire";
            case ExpositionSoleil::OMBRE_DENSE: return "Ombre Dense";
        }
        return "?";
    }

    const char* VentLabel(ExpositionVent v) {
        switch(v) {
            case ExpositionVent::ABRITE: return "Abrite (Sensible au vent)";
            case ExpositionVent::MODERE: return "Modere";
            case ExpositionVent::COULOIR_DE_VENT: return "Couloir de vent";
        }
        return "?";
    }
}

StateWiki::StateWiki(Application* app) : State(app) {}

void StateWiki::onEnter() {
    chargerTextures();
}

void StateWiki::chargerTextures() {
    for (const auto& pair : m_app->getDatabase().getAllPlantes()) {
        const std::string& nom = pair.first; 
        if (m_plantIconTextures.find(nom) == m_plantIconTextures.end()) {
            sf::Texture icon;
            if (!icon.loadFromFile("assets/" + nom + "_icon.png")) {
                sf::Image img;
                img.create(64, 64, sf::Color(40, 100, 50, 200));
                icon.loadFromImage(img);
            }
            icon.setSmooth(true);
            m_plantIconTextures[nom] = std::move(icon);
        }
    }
}
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
    ImGui::TextColored(ImVec4(0.55f, 0.68f, 0.55f, 1.00f), "- Base de donnees botanique et pedologique");
    ImGui::Dummy({0,6});

    float hauteurContenu = ImGui::GetContentRegionAvail().y;
    float largeurDetail  = ImGui::GetContentRegionAvail().x - LARGEUR_LISTE - 16.f;

    // ════════ PANNEAU GAUCHE (LA LISTE) ════════
    ImGui::BeginChild("##gauche", {LARGEUR_LISTE, hauteurContenu}, false);
    
    // Bascule Plantes / Sols
    ImGui::PushStyleVar(ImGuiStyleVar_FrameRounding, 4.0f);
    if (ImGui::Button(m_modeAffichage == 0 ? ">> PLANTES <<" : "Plantes", ImVec2(LARGEUR_LISTE / 2 - 4, 30))) m_modeAffichage = 0;
    ImGui::SameLine();
    if (ImGui::Button(m_modeAffichage == 1 ? ">> SOLS <<" : "Sols", ImVec2(LARGEUR_LISTE / 2 - 4, 30))) m_modeAffichage = 1;
    ImGui::PopStyleVar();
    ImGui::Spacing();

    // -- SI MODE PLANTES --
    if (m_modeAffichage == 0) {
        ImGui::SetNextItemWidth(-1.f);
        ImGui::InputTextWithHint("##rech", "Rechercher une plante...", m_filtreRecherche, sizeof(m_filtreRecherche));
        ImGui::Spacing();

        const auto& allPlantesMap = m_app->getDatabase().getAllPlantes();
        std::vector<const Plante*> plantesTriees;
        for (const auto& pair : allPlantesMap) {
            plantesTriees.push_back(&pair.second);
        }
        std::sort(plantesTriees.begin(), plantesTriees.end(), [](const Plante* a, const Plante* b) {
            return a->nom < b->nom;
        });

        std::string filtre(m_filtreRecherche);
        std::transform(filtre.begin(), filtre.end(), filtre.begin(), ::tolower);

        ImGui::BeginChild("##scroll_liste", {-1, -1}, false);
        for (size_t i = 0; i < plantesTriees.size(); i++) {
            const Plante* p = plantesTriees[i];
            
            std::string n = p->nom;
            // Sécurité contre les accents
            std::transform(n.begin(), n.end(), n.begin(), [](unsigned char c){ return std::tolower(c); });
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
    } 
    // -- SI MODE SOLS --
    else {
        ImGui::BeginChild("##scroll_liste", {-1, -1}, false);
        const auto& allSols = m_app->getDatabase().getAllSols();
        for (const auto& pair : allSols) {
            const Sol& s = pair.second;
            
            ImGui::PushID(s.type_sol.c_str());
            bool sel  = (m_solSelectionne == s.type_sol);
            ImVec2 pos0 = ImGui::GetCursorScreenPos();
            ImVec2 sz   = {ImGui::GetContentRegionAvail().x, 50.f}; 

            ImGui::InvisibleButton("##c", sz);
            if (ImGui::IsItemClicked()) m_solSelectionne = sel ? "" : s.type_sol;

            ImDrawList* dlL = ImGui::GetWindowDrawList();
            if (sel) dlL->AddRectFilled(pos0, {pos0.x+sz.x, pos0.y+sz.y}, IM_COL32(60, 45, 25, 255), 8.f);
            else if (ImGui::IsItemHovered()) dlL->AddRectFilled(pos0, {pos0.x+sz.x, pos0.y+sz.y}, IM_COL32(50, 35, 20, 200), 8.f);
            else dlL->AddRectFilled(pos0, {pos0.x+sz.x, pos0.y+sz.y}, IM_COL32(30, 20, 15, 150), 8.f);

            // Bandeau couleur "Terre"
            dlL->AddRectFilled(pos0, {pos0.x + 6.f, pos0.y+sz.y}, IM_COL32(139, 69, 19, 255), 8.f, ImDrawFlags_RoundCornersLeft);

            ImGui::SetCursorScreenPos({pos0.x+18, pos0.y+10});
            ImGui::TextColored(sel ? ImVec4(0.9f, 0.7f, 0.4f, 1.f) : ImVec4(0.8f, 0.7f, 0.6f, 1.f), "%s", s.type_sol.c_str());
            ImGui::SetCursorScreenPos({pos0.x+18, pos0.y+28});
            ImGui::TextColored(ImVec4(0.6f, 0.6f, 0.6f, 1.f), "%s", s.texture.c_str());

            ImGui::SetCursorScreenPos({pos0.x, pos0.y+sz.y + 4.f});
            ImGui::PopID();
        }
        ImGui::EndChild();
    }
    ImGui::EndChild(); // FIN DU PANNEAU GAUCHE

    ImGui::SameLine(0, 16.f);

    // ════════ PANNEAU DROIT (LES DETAILS) ════════
    ImGui::BeginChild("##detail", {largeurDetail, hauteurContenu}, true);

    // DÉFINITION DES LAMBDAS (Utiles pour les plantes ET les sols)
    auto SectionInfo = [&](const char* titre, const char* contenu, ImVec4 color) {
        if (!contenu || std::strlen(contenu) == 0) return;
        ImGui::TextColored(color, "[ %s ]", titre);
        ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(0.55f, 0.68f, 0.55f, 1.00f));
        ImGui::TextWrapped("%s", contenu);
        ImGui::PopStyleColor();
        ImGui::Dummy({ 0, 8 });
    };

    auto AfficherListe = [](const char* titre, const std::vector<std::string>& liste, ImVec4 col) {
        if (liste.empty()) return;
        ImGui::TextColored(col, "[ %s ]", titre);
        for (const auto& item : liste) {
            ImGui::TextColored(ImVec4(0.8f, 0.8f, 0.8f, 1.f), " - %s", item.c_str());
        }
        ImGui::Dummy({0, 8});
    };

    // -- SI MODE PLANTES ET PLANTE SÉLECTIONNÉE --
    if (m_modeAffichage == 0) {
        const Plante* p = m_app->getDatabase().getPlante(m_planteSelectionnee);
        if (p != nullptr) {
            ImGui::BeginGroup();
            if (m_plantIconTextures.count(p->nom)) {
                ImTextureID texID = (ImTextureID)(intptr_t)m_plantIconTextures[p->nom].getNativeHandle();
                ImGui::Image(texID, ImVec2(96, 96));
                ImGui::SameLine();
            }
            
            ImGui::BeginGroup();
            ImGui::SetWindowFontScale(1.4f);
            ImGui::TextColored(ImVec4(0.20f, 0.75f, 0.40f, 1.00f), "%s", p->nom.c_str());
            ImGui::SetWindowFontScale(1.0f);
            if (!p->nom_scientifique.empty())
                ImGui::TextDisabled("( %s )", p->nom_scientifique.c_str());
            ImGui::TextColored(ImVec4(0.55f, 0.68f, 0.55f, 1.00f), "Famille : %s", p->famille.c_str());
            ImGui::EndGroup();
            ImGui::EndGroup();

            ImGui::SameLine(ImGui::GetContentRegionAvail().x - 120.f);
            if (ImGui::Button("+ Adopter", ImVec2(120, 40))) { 
                UserPlant nouvellePlante;
                nouvellePlante.nom_espece = p->nom;
                nouvellePlante.surnom = "Mon " + p->nom;
                nouvellePlante.volume_pot_actuel_L = 5;
                std::time_t t = std::time(nullptr);
                std::tm tmBuf{};
                localtime_s(&tmBuf, &t); 
                nouvellePlante.jour_achat  = tmBuf.tm_mday;
                nouvellePlante.mois_achat  = tmBuf.tm_mon + 1;
                nouvellePlante.annee_achat = tmBuf.tm_year + 1900;
                m_app->getUserBalcony().ajouterPlante(nouvellePlante);
                m_app->getUserBalcony().sauvegarderProfil("mon_balcon.json");
            }

            ImGui::Separator();

            if (ImGui::BeginTabBar("WikiTabs")) {
                
                // ONGLET 1 : BOTANIQUE
                if (ImGui::BeginTabItem("Botanique")) {
                    ImGui::Spacing();
                    SectionInfo("Origine", p->origine.c_str(), ImVec4(0.9f, 0.7f, 0.4f, 1.f));
                    SectionInfo("Dimensions Adulte", p->dimensions_adulte.c_str(), ImVec4(0.7f, 0.9f, 0.7f, 1.f));
                    SectionInfo("Feuillage", p->feuillage.c_str(), ImVec4(0.4f, 0.8f, 0.4f, 1.f));
                    SectionInfo("Description", p->notes.c_str(), ImVec4(0.88f, 0.92f, 0.88f, 1.f));
                    ImGui::EndTabItem();
                }

                // ONGLET 2 : CULTURE & EAU
                if (ImGui::BeginTabItem("Culture & Eau")) {
                    ImGui::Spacing();
                    
                    ImGui::Columns(2, "cols_expo", false);
                    SectionInfo("Exposition", SoleilLabel(p->exposition_soleil), ImVec4(0.9f, 0.8f, 0.2f, 1.f));
                    ImGui::NextColumn();
                    SectionInfo("Vent", VentLabel(p->exposition_vent), ImVec4(0.6f, 0.8f, 0.9f, 1.f));
                    ImGui::Columns(1);
                    
                    SectionInfo("Zone Climat", p->zone_climat.c_str(), CouleurRusticite(p->rusticite));
                    SectionInfo("Volume du pot min.", p->volume_pot_min.c_str(), ImVec4(0.8f, 0.8f, 0.8f, 1.f));
                    SectionInfo("Substrat recommande", p->conseil_terre.c_str(), ImVec4(0.8f, 0.6f, 0.3f, 1.f));
                    
                    // ════ LIENS CROISÉS VERS LES SOLS ════
                    std::vector<std::string> sols_compatibles;
                    for (const auto& pair : m_app->getDatabase().getAllSols()) {
                        const Sol& sol = pair.second;
                        bool match = false;
                        for (const std::string& planteAdaptee : sol.adapte_pour) {
                            std::string nomPlante = p->nom;
                            std::string nomAdapte = planteAdaptee;
                            
                            // Conversion sécurisée en minuscules
                            std::transform(nomPlante.begin(), nomPlante.end(), nomPlante.begin(), [](unsigned char c){ return std::tolower(c); });
                            std::transform(nomAdapte.begin(), nomAdapte.end(), nomAdapte.begin(), [](unsigned char c){ return std::tolower(c); });
                            
                            if (nomPlante.find(nomAdapte) != std::string::npos || nomAdapte.find(nomPlante) != std::string::npos) {
                                match = true; break;
                            }
                        }
                        if (match) sols_compatibles.push_back(sol.type_sol);
                    }

                    if (!sols_compatibles.empty()) {
                        ImGui::Separator();
                        ImGui::Spacing();
                        ImGui::TextColored(ImVec4(0.8f, 0.5f, 0.2f, 1.f), "Recommandation Pedologique (Cliquez pour voir) :");
                        for (const std::string& nomSol : sols_compatibles) {
                            ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.5f, 0.3f, 0.15f, 1.0f)); 
                            ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4(0.6f, 0.4f, 0.2f, 1.0f));
                            ImGui::PushStyleColor(ImGuiCol_ButtonActive, ImVec4(0.4f, 0.2f, 0.1f, 1.0f));
                            std::string labelBouton = "  " + nomSol + " ";
                            if (ImGui::Button(labelBouton.c_str())) {
                                m_modeAffichage = 1;        // Bascule Sols
                                m_solSelectionne = nomSol;  // Ouvre fiche sol
                            }
                            ImGui::PopStyleColor(3);
                            ImGui::SameLine();
                        }
                        ImGui::NewLine();
                    }
                    
                    ImGui::Separator(); ImGui::Dummy({0,4});
                    
                    SectionInfo("Conseil Arrosage", p->conseil_arrosage.c_str(), ImVec4(0.55f, 0.68f, 0.55f, 1.00f));
                    ImGui::Columns(2, "cols_arrosage", false);
                    SectionInfo("Frequence Ete", p->frequence_arrosage_ete.c_str(), ImVec4(0.9f, 0.6f, 0.3f, 1.f));
                    ImGui::NextColumn();
                    SectionInfo("Frequence Hiver", p->frequence_arrosage_hiver.c_str(), ImVec4(0.4f, 0.7f, 0.9f, 1.f));
                    ImGui::Columns(1);
                    
                    ImGui::EndTabItem();
                }

                // ONGLET 3 : ENTRETIEN & SANTE
                if (ImGui::BeginTabItem("Entretien & Sante")) {
                    ImGui::Spacing();
                    SectionInfo("Taille & Soins", p->conseil_entretien.c_str(), ImVec4(0.5f, 0.8f, 0.5f, 1.f));
                    SectionInfo("Rempotage", p->rempotage.c_str(), ImVec4(0.6f, 0.5f, 0.8f, 1.f));
                    SectionInfo("Compagnonnage", p->compagnonnage.c_str(), ImVec4(0.4f, 0.9f, 0.6f, 1.f));
                    
                    if (!p->maladies.empty()) {
                        ImGui::Separator(); ImGui::Dummy({0,4});
                        SectionInfo("Maladies & Parasites", p->maladies.c_str(), ImVec4(0.9f, 0.4f, 0.4f, 1.f));
                    }
                    ImGui::EndTabItem();
                }

                // ONGLET 4 : USAGES & PRÉCAUTIONS
                if (ImGui::BeginTabItem("Usages")) {
                    ImGui::Spacing();
                    if (!p->vertus_medicinales.empty()) 
                        SectionInfo("Vertus & Usages", p->vertus_medicinales.c_str(), ImVec4(0.3f, 0.8f, 0.9f, 1.f));
                    
                    if (!p->precautions.empty()) 
                        SectionInfo("Precautions", p->precautions.c_str(), ImVec4(0.9f, 0.5f, 0.3f, 1.f));
                        
                    if (p->toxicite_animaux == "NON") {
                        SectionInfo("Toxicite Animaux", "Sans danger (Pet-friendly)", ImVec4(0.4f, 0.9f, 0.5f, 1.f));
                    } else if (!p->toxicite_animaux.empty()) {
                        SectionInfo("Toxicite Animaux", p->toxicite_animaux.c_str(), ImVec4(0.9f, 0.3f, 0.3f, 1.f));
                    }
                    ImGui::EndTabItem();
                }

                // ONGLET 5 : STATISTIQUES
                if (ImGui::BeginTabItem("Statistiques")) {
                    ImGui::Spacing();
                    ImGui::TextColored(ImVec4(0.4f, 0.9f, 0.5f, 1.f), "Profil de culture");
                    ImGui::Dummy(ImVec2(0, 20.f));

                    ImVec2 posCursor = ImGui::GetCursorScreenPos();
                    float rayon = 25.0f;
                    float ecart = 90.0f;

                    DrawJaugeCirculaire("Eau", (float)p->besoin_eau, 4.0f, ImVec2(posCursor.x + 40.f, posCursor.y + 30.f), rayon, IM_COL32(80, 180, 255, 255), IM_COL32(30, 60, 80, 255));

                    float diffVal = (p->niveau_difficulte == NiveauDifficulte::FACILE) ? 1.f :
                                    (p->niveau_difficulte == NiveauDifficulte::MOYEN) ? 2.f :
                                    (p->niveau_difficulte == NiveauDifficulte::DIFFICILE) ? 3.f : 4.f;
                    
                    ImU32 colDiff = (diffVal <= 1.f) ? IM_COL32(100, 255, 100, 255) : 
                                    (diffVal <= 2.f) ? IM_COL32(255, 200, 50, 255) : IM_COL32(255, 80, 50, 255);
                    
                    DrawJaugeCirculaire("Difficulte", diffVal, 4.0f, ImVec2(posCursor.x + 40.f + ecart, posCursor.y + 30.f), rayon, colDiff, IM_COL32(60, 50, 40, 255));

                    DrawJaugeCirculaire("Score", (float)p->score_balcon, 100.0f, ImVec2(posCursor.x + 40.f + ecart * 2, posCursor.y + 30.f), rayon, IM_COL32(255, 215, 0, 255), IM_COL32(80, 70, 20, 255));

                    ImGui::Dummy(ImVec2(0, 80.f)); 
                    
                    if (p->contrainte_majeure != "Aucune" && !p->contrainte_majeure.empty()) {
                        ImGui::Separator();
                        ImGui::Spacing();
                        ImGui::TextColored(ImVec4(0.9f, 0.4f, 0.4f, 1.f), "[ Attention ]");
                        ImGui::TextWrapped("%s", p->contrainte_majeure.c_str());
                    }
                    ImGui::EndTabItem();
                }
                ImGui::EndTabBar();
            }
            
            ImGui::Spacing();
            ImGui::Separator();
            ImGui::Spacing();
            SectionInfo("Astuce de Pro", p->astuce_pro.c_str(), ImVec4(1.0f, 0.8f, 0.2f, 1.f));

        } else {
            // Message si aucune plante sélectionnée
            ImVec2 avail = ImGui::GetContentRegionAvail();
            ImVec2 pos = ImGui::GetCursorScreenPos();
            ImDrawList* dl = ImGui::GetWindowDrawList();
            dl->AddCircleFilled(ImVec2(pos.x + avail.x*0.5f, pos.y + avail.y*0.4f), 50.f, IM_COL32(35, 60, 40, 150));
            const char* msg = "Selectionnez une plante de l'encyclopedie";
            ImVec2 tsz = ImGui::CalcTextSize(msg);
            ImGui::SetCursorScreenPos(ImVec2(pos.x + (avail.x - tsz.x)*0.5f, pos.y + avail.y*0.4f + 80.f));
            ImGui::TextColored(ImVec4(0.30f, 0.40f, 0.30f, 1.00f), "%s", msg);
        }
    } 
    // -- SI MODE SOLS ET SOL SÉLECTIONNÉ --
    else if (m_modeAffichage == 1) {
        const Sol* s = m_app->getDatabase().getSol(m_solSelectionne);
        if (s) {
            ImGui::SetWindowFontScale(1.4f);
            ImGui::TextColored(ImVec4(0.8f, 0.5f, 0.2f, 1.0f), "%s", s->type_sol.c_str());
            ImGui::SetWindowFontScale(1.0f);
            ImGui::TextColored(ImVec4(0.6f, 0.6f, 0.6f, 1.0f), "Utilisation : %s", s->utilisation.c_str());
            ImGui::Separator();
            ImGui::Spacing();

            if (ImGui::BeginTabBar("SolsTabs")) {
                if (ImGui::BeginTabItem("Caracteristiques")) {
                    ImGui::Spacing();
                    ImGui::Columns(2, "cols_sols", false);
                    ImGui::Text("Drainage : %s", s->drainage.c_str());
                    ImGui::Text("Retention eau : %s", s->retention_eau.c_str());
                    ImGui::Text("Richesse : %s", s->richesse.c_str());
                    ImGui::NextColumn();
                    ImGui::Text("pH Typique : %s", s->ph_typique.c_str());
                    ImGui::Text("Aeration : %s", s->aeration.c_str());
                    ImGui::Text("CEC : %s", s->cec.c_str());
                    ImGui::Columns(1);
                    ImGui::Dummy({0, 10});

                    AfficherListe("Composition", s->composition, ImVec4(0.5f, 0.8f, 0.5f, 1.f));
                    AfficherListe("Adapte pour", s->adapte_pour, ImVec4(0.4f, 0.8f, 0.9f, 1.f));
                    AfficherListe("Problemes Frequents", s->problemes_frequents, ImVec4(0.9f, 0.4f, 0.4f, 1.f));
                    ImGui::EndTabItem();
                }

                if (ImGui::BeginTabItem("Corrections pH & Structure")) {
                    ImGui::Spacing();
                    AfficherListe("Pour Baisser le pH (Acidifier)", s->correction_baisser_ph, ImVec4(0.4f, 0.7f, 0.9f, 1.f));
                    AfficherListe("Pour Augmenter le pH (Alcaliniser)", s->correction_augmenter_ph, ImVec4(0.9f, 0.7f, 0.4f, 1.f));
                    AfficherListe("Amelioration de Structure", s->amelioration_structure, ImVec4(0.6f, 0.8f, 0.5f, 1.f));
                    ImGui::EndTabItem();
                }
                ImGui::EndTabBar();
            }
        } else {
            // Message si aucun sol sélectionné
            ImVec2 avail = ImGui::GetContentRegionAvail();
            ImVec2 pos = ImGui::GetCursorScreenPos();
            ImDrawList* dl = ImGui::GetWindowDrawList();
            dl->AddCircleFilled(ImVec2(pos.x + avail.x*0.5f, pos.y + avail.y*0.4f), 50.f, IM_COL32(60, 45, 25, 150));
            const char* msg = "Selectionnez un type de substrat/sol";
            ImVec2 tsz = ImGui::CalcTextSize(msg);
            ImGui::SetCursorScreenPos(ImVec2(pos.x + (avail.x - tsz.x)*0.5f, pos.y + avail.y*0.4f + 80.f));
            ImGui::TextColored(ImVec4(0.40f, 0.30f, 0.20f, 1.00f), "%s", msg);
        }
    }
    
    ImGui::EndChild(); // FIN DU PANNEAU DROIT
    ImGui::End(); // FIN DE LA FENETRE PRINCIPALE
}