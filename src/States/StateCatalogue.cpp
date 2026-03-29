#include "StateCatalogue.hpp"
#include <algorithm>
#include <cmath>
namespace {
    const std::string catalogueOutlineShaderCode = R"(
    uniform sampler2D texture;
    uniform vec2 texSize;

    void main() {
        vec2 off = vec2(1.0, 1.0) / texSize;
        vec4 col = texture2D(texture, gl_TexCoord[0].xy);

        if (col.a == 0.0) {
            float alpha = texture2D(texture, gl_TexCoord[0].xy + vec2(off.x, 0.0)).a +
                          texture2D(texture, gl_TexCoord[0].xy - vec2(off.x, 0.0)).a +
                          texture2D(texture, gl_TexCoord[0].xy + vec2(0.0, off.y)).a +
                          texture2D(texture, gl_TexCoord[0].xy - vec2(0.0, off.y)).a;
            
            if (alpha > 0.0) gl_FragColor = vec4(1.0, 1.0, 1.0, 1.0); 
            else gl_FragColor = vec4(0.0, 0.0, 0.0, 0.0); 
        } else {
            gl_FragColor = col; 
        }
    }
    )";
}

// --- HELPERS LOCAUX ---
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

StateCatalogue::StateCatalogue(Application* app) : State(app) {}
// Remplace onEnter() par :
void StateCatalogue::onEnter() {
    if (sf::Shader::isAvailable())
        m_outlineShader.loadFromMemory(catalogueOutlineShaderCode, sf::Shader::Fragment);
    chargerTextures();
}

// Ajoute cette méthode (après onEnter) :
void StateCatalogue::chargerTextures() {
    for (const auto& p : m_app->getUserBalcony().getMesPlantes()) {
        // PNG principal (galerie drag&drop)
        if (m_plantTextures.find(p.nom_espece) == m_plantTextures.end()) {
            sf::Texture tex;
            if (!tex.loadFromFile("assets/" + p.nom_espece + ".png")) {
                // Fallback : carré vert 64x64
                sf::Image img;
                img.create(64, 64, sf::Color(40, 100, 50, 200));
                tex.loadFromImage(img);
            }
            tex.setSmooth(true);
            m_plantTextures[p.nom_espece] = std::move(tex);
        }

        // PNG icône (fiche détail)
        if (m_plantIconTextures.find(p.nom_espece) == m_plantIconTextures.end()) {
            sf::Texture icon;
            // Cherche d'abord nom_espece_icon.png, sinon reuse le principal
            if (!icon.loadFromFile("assets/" + p.nom_espece + "_icon.png"))
                icon = m_plantTextures[p.nom_espece]; // fallback = même PNG
            icon.setSmooth(true);
            m_plantIconTextures[p.nom_espece] = std::move(icon);
        }
    }
}
void StateCatalogue::onExit() {}
void StateCatalogue::update(float dt) {
    m_time += dt;
}
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
            UserPlant& editablePlant = const_cast<UserPlant&>(mesPlantes[m_indexSelectionne]);
            ImDrawList* dl = ImGui::GetWindowDrawList();
            
            // VARIABLES CORRIGÉES ICI !
            ImVec2 panelPos  = ImGui::GetCursorScreenPos(); 
            float  panelW    = largeurDetail - 24.f;

            // ═══ FOND DÉSATURÉ ═══
            if (m_plantTextures.count(up.nom_espece)) {
                sf::Texture& bgTex = m_plantTextures.at(up.nom_espece);
                ImTextureID  bgID  = (ImTextureID)(intptr_t)bgTex.getNativeHandle();

                float bgH    = 180.f;
                float bgW    = panelW;
                ImVec2 bgMin = panelPos;
                ImVec2 bgMax = {panelPos.x + bgW, panelPos.y + bgH};

                float healthT = editablePlant.health;
                sf::Uint8 rT = (sf::Uint8)(180 + (1.f - healthT) * 60.f);
                sf::Uint8 gT = (sf::Uint8)(180 * healthT);
                sf::Uint8 bT = (sf::Uint8)(180 * healthT);

                dl->AddImage(bgID, bgMin, bgMax, ImVec2(0,0), ImVec2(1,1), IM_COL32(rT, gT, bT, 35)); 
                dl->AddRectFilledMultiColor(bgMin, bgMax, IM_COL32(7,10,7,0), IM_COL32(7,10,7,0), IM_COL32(7,10,7,230), IM_COL32(7,10,7,230));
            }

            // ═══ ICÔNE PNG à droite du titre ═══
            float iconSize = 90.f;
            float iconX    = panelPos.x + panelW - iconSize - 8.f;
            float iconY    = panelPos.y + 8.f;

            if (m_plantIconTextures.count(up.nom_espece)) {
                sf::Texture& iconTex = m_plantIconTextures.at(up.nom_espece);
                ImTextureID  iconID  = (ImTextureID)(intptr_t)iconTex.getNativeHandle();
                float texW = (float)iconTex.getSize().x;
                float texH = (float)iconTex.getSize().y;
                float ratio = (texW > 0) ? texH / texW : 1.f;
                float iW = iconSize;
                float iH = iconSize * ratio;

                float tilt = (1.f - editablePlant.hydration) * 18.f + (1.f - editablePlant.health) * 22.f;
                float bounceOff = (editablePlant.happiness > 0.7f) ? std::sin(m_time * 3.5f) * 3.f * editablePlant.happiness : 0.f;

                ImVec2 iMin = {iconX,      iconY + bounceOff};
                ImVec2 iMax = {iconX + iW, iconY + iH + bounceOff};

                float h = editablePlant.health;
                sf::Uint8 tintG = (sf::Uint8)(180 + 75.f * h);
                sf::Uint8 tintB = (sf::Uint8)(180 + 75.f * h);

                if (tilt > 2.f) {
                    float shear = tilt * 0.3f;
                    dl->AddImageQuad(iconID, {iMin.x + shear, iMin.y}, {iMax.x + shear, iMin.y}, {iMax.x, iMax.y}, {iMin.x, iMax.y},
                        ImVec2(0,0), ImVec2(1,0), ImVec2(1,1), ImVec2(0,1), IM_COL32(255, tintG, tintB, 255));
                } else {
                    dl->AddImage(iconID, iMin, iMax, ImVec2(0,0), ImVec2(1,1), IM_COL32(255, tintG, tintB, 255));
                }

                if (editablePlant.happiness > 0.85f) {
                    float pulse = 0.5f + 0.5f * std::sin(m_time * 3.f);
                    dl->AddRect({iMin.x - 3.f, iMin.y - 3.f}, {iMax.x + 3.f, iMax.y + 3.f}, IM_COL32(80, 220, 110, (int)(pulse * 160.f)), 6.f, 0, 2.f);
                }

                if (editablePlant.hydration < 0.3f) {
                    float pulse = 0.5f + 0.5f * std::sin(m_time * 5.f);
                    dl->AddCircle({iconX + iW*0.5f, iconY + iH*0.5f + bounceOff}, iW * 0.6f + pulse * 4.f, IM_COL32(80, 160, 255, (int)(pulse * 140.f)), 24, 1.5f);
                }
            }

            ImGui::SetCursorScreenPos({panelPos.x, panelPos.y + 4.f});

            // ═══ TITRE ET INFOS SCIENTIFIQUES ═══
            ImGui::SetWindowFontScale(1.4f);
            ImGui::TextColored({0.20f,0.75f,0.40f,1.f}, "%s", up.surnom.c_str());
            ImGui::SetWindowFontScale(1.0f);

            ImGui::TextColored(ImVec4(0.55f, 0.68f, 0.55f, 1.00f), "Espece : %s", refP->nom.c_str());
            if (!refP->nom_scientifique.empty()) {
                ImGui::TextDisabled("( %s ) - Famille : %s", refP->nom_scientifique.c_str(), refP->famille.c_str());
            }
            
            // Affichage des autres noms s'ils existent
            if (!refP->autres_noms.empty()) {
                std::string alias = "Aussi appele : ";
                for (size_t k = 0; k < refP->autres_noms.size(); ++k) {
                    alias += refP->autres_noms[k] + (k < refP->autres_noms.size() - 1 ? ", " : "");
                }
                ImGui::TextColored(ImVec4(0.4f, 0.6f, 0.4f, 0.8f), "%s", alias.c_str());
            }
            
            // --- EDITEUR DE DATE D'ADOPTION ---
            ImGui::Spacing();
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

            if (dateChanged) {
                m_app->getUserBalcony().sauvegarderProfil("mon_balcon.json");
            }

            ImGui::Separator();
            ImGui::Spacing();

            if (refP->floraison_debut > 0 || refP->recolte_debut > 0) {
                if (refP->floraison_debut > 0) 
                    DessinerCalendrierVisuel("Periode de Floraison", refP->floraison_debut, refP->floraison_fin, ImVec4(0.9f, 0.5f, 0.8f, 1.0f));
                if (refP->recolte_debut > 0) 
                    DessinerCalendrierVisuel("Periode de Recolte", refP->recolte_debut, refP->recolte_fin, ImVec4(0.5f, 0.8f, 0.4f, 1.0f));
            }

            // ═══ ONGLETS D'INFORMATIONS COMPLETS ═══
            if (ImGui::BeginTabBar("OngletsConseils")) {
                auto SectionInfo = [&](const char* titre, const char* contenu, ImVec4 color) {
                    if (!contenu || std::strlen(contenu) == 0) return;
                    ImGui::TextColored(color, "[ %s ]", titre);
                    ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(0.55f, 0.68f, 0.55f, 1.00f));
                    ImGui::TextWrapped("%s", contenu);
                    ImGui::PopStyleColor();
                    ImGui::Dummy({0,8});
                };

                // ONGLET 1 : BOTANIQUE
                if (ImGui::BeginTabItem("Botanique")) {
                    ImGui::Spacing();
                    SectionInfo("Origine", refP->origine.c_str(), ImVec4(0.9f, 0.7f, 0.4f, 1.f));
                    SectionInfo("Dimensions Adulte", refP->dimensions_adulte.c_str(), ImVec4(0.7f, 0.9f, 0.7f, 1.f));
                    SectionInfo("Feuillage", refP->feuillage.c_str(), ImVec4(0.4f, 0.8f, 0.4f, 1.f));
                    SectionInfo("Description", refP->notes.c_str(), ImVec4(0.88f, 0.92f, 0.88f, 1.f));
                    ImGui::EndTabItem();
                }

                // ONGLET 2 : CULTURE & EAU
                if (ImGui::BeginTabItem("Culture & Eau")) {
                    ImGui::Spacing();
                    
                    ImGui::Columns(2, "cols_expo", false);
                    SectionInfo("Exposition", SoleilLabel(refP->exposition_soleil), ImVec4(0.9f, 0.8f, 0.2f, 1.f));
                    ImGui::NextColumn();
                    SectionInfo("Vent", VentLabel(refP->exposition_vent), ImVec4(0.6f, 0.8f, 0.9f, 1.f));
                    ImGui::Columns(1);
                    
                    SectionInfo("Zone Climat", refP->zone_climat.c_str(), CouleurRusticite(refP->rusticite));
                    SectionInfo("Volume du pot min.", refP->volume_pot_min.c_str(), ImVec4(0.8f, 0.8f, 0.8f, 1.f));
                    SectionInfo("Substrat recommande", refP->conseil_terre.c_str(), ImVec4(0.8f, 0.6f, 0.3f, 1.f));
                    
                    ImGui::Separator(); ImGui::Dummy({0,4});
                    
                    SectionInfo("Conseil Arrosage", refP->conseil_arrosage.c_str(), ImVec4(0.55f, 0.68f, 0.55f, 1.00f));
                    ImGui::Columns(2, "cols_arrosage", false);
                    SectionInfo("Frequence Ete", refP->frequence_arrosage_ete.c_str(), ImVec4(0.9f, 0.6f, 0.3f, 1.f));
                    ImGui::NextColumn();
                    SectionInfo("Frequence Hiver", refP->frequence_arrosage_hiver.c_str(), ImVec4(0.4f, 0.7f, 0.9f, 1.f));
                    ImGui::Columns(1);
                    
                    ImGui::EndTabItem();
                }

                // ONGLET 3 : ENTRETIEN & SANTE
                if (ImGui::BeginTabItem("Entretien & Sante")) {
                    ImGui::Spacing();
                    SectionInfo("Taille & Soins", refP->conseil_entretien.c_str(), ImVec4(0.5f, 0.8f, 0.5f, 1.f));
                    SectionInfo("Rempotage", refP->rempotage.c_str(), ImVec4(0.6f, 0.5f, 0.8f, 1.f));
                    SectionInfo("Compagnonnage", refP->compagnonnage.c_str(), ImVec4(0.4f, 0.9f, 0.6f, 1.f));
                    
                    if (!refP->maladies.empty()) {
                        ImGui::Separator(); ImGui::Dummy({0,4});
                        SectionInfo("Maladies & Parasites", refP->maladies.c_str(), ImVec4(0.9f, 0.4f, 0.4f, 1.f));
                    }
                    ImGui::EndTabItem();
                }

                // ONGLET 4 : USAGES & PRÉCAUTIONS
                if (ImGui::BeginTabItem("Usages")) {
                    ImGui::Spacing();
                    if (!refP->vertus_medicinales.empty()) 
                        SectionInfo("Vertus & Usages", refP->vertus_medicinales.c_str(), ImVec4(0.3f, 0.8f, 0.9f, 1.f));
                    
                    if (!refP->precautions.empty()) 
                        SectionInfo("Precautions", refP->precautions.c_str(), ImVec4(0.9f, 0.5f, 0.3f, 1.f));
                        
                    if (refP->toxicite_animaux == "NON") {
                        SectionInfo("Toxicite Animaux", "Sans danger (Pet-friendly)", ImVec4(0.4f, 0.9f, 0.5f, 1.f));
                    } else if (!refP->toxicite_animaux.empty()) {
                        SectionInfo("Toxicite Animaux", refP->toxicite_animaux.c_str(), ImVec4(0.9f, 0.3f, 0.3f, 1.f));
                    }
                    ImGui::EndTabItem();
                }

                // ONGLET 5 : STATISTIQUES (Avec les jauges circulaires !)
                if (ImGui::BeginTabItem("Statistiques")) {
                    ImGui::Spacing();
                    ImGui::TextColored(ImVec4(0.4f, 0.9f, 0.5f, 1.f), "Profil de culture");
                    ImGui::Dummy(ImVec2(0, 20.f)); 

                    ImVec2 posCursor = ImGui::GetCursorScreenPos();
                    float rayon = 25.0f;
                    float ecart = 90.0f; 

                    DrawJaugeCirculaire("Eau", (float)refP->besoin_eau, 4.0f, 
                        ImVec2(posCursor.x + 40.f, posCursor.y + 30.f), rayon, 
                        IM_COL32(80, 180, 255, 255), IM_COL32(30, 60, 80, 255));

                    float diffVal = (refP->niveau_difficulte == NiveauDifficulte::FACILE) ? 1.f :
                                    (refP->niveau_difficulte == NiveauDifficulte::MOYEN) ? 2.f :
                                    (refP->niveau_difficulte == NiveauDifficulte::DIFFICILE) ? 3.f : 4.f;
                    
                    ImU32 colDiff = (diffVal <= 1.f) ? IM_COL32(100, 255, 100, 255) : 
                                    (diffVal <= 2.f) ? IM_COL32(255, 200, 50, 255) : IM_COL32(255, 80, 50, 255);
                    
                    DrawJaugeCirculaire("Difficulte", diffVal, 4.0f, 
                        ImVec2(posCursor.x + 40.f + ecart, posCursor.y + 30.f), rayon, 
                        colDiff, IM_COL32(60, 50, 40, 255));

                    DrawJaugeCirculaire("Score", (float)refP->score_balcon, 100.0f, 
                        ImVec2(posCursor.x + 40.f + ecart * 2, posCursor.y + 30.f), rayon, 
                        IM_COL32(255, 215, 0, 255), IM_COL32(80, 70, 20, 255));

                    ImGui::Dummy(ImVec2(0, 80.f)); 

                    if (refP->contrainte_majeure != "Aucune" && !refP->contrainte_majeure.empty()) {
                        ImGui::Separator();
                        ImGui::Spacing();
                        ImGui::TextColored(ImVec4(0.9f, 0.4f, 0.4f, 1.f), "[ Attention ]");
                        ImGui::TextWrapped("%s", refP->contrainte_majeure.c_str());
                    }
                    ImGui::EndTabItem();
                }

                ImGui::EndTabBar();
            }
        }
    } else {
        // ══ ÉTAT VIDE : L'ÉTAGÈRE INTERACTIVE ══
        auto& mesPlantes = m_app->getUserBalcony().getMesPlantesRef();
        
        ImVec2 canvas_p0 = ImGui::GetCursorScreenPos();
        ImVec2 canvas_sz = ImGui::GetContentRegionAvail();
        
        ImGui::InvisibleButton("canvas", canvas_sz);
        bool is_hovered = ImGui::IsItemHovered();
        ImVec2 mouse_pos = ImGui::GetIO().MousePos;
        ImDrawList* dl = ImGui::GetWindowDrawList();

        const float PLANT_SCALE = 0.7f;
        if (ImGui::IsMouseClicked(ImGuiMouseButton_Left) && is_hovered) {
            for (int i = (int)mesPlantes.size() - 1; i >= 0; i--) {
                auto& p = mesPlantes[i];
                if (m_plantTextures.count(p.nom_espece)) {
                    sf::Vector2u texSize = m_plantTextures[p.nom_espece].getSize();
                    if (p.position_catalogue.x < 0) {
                        p.position_catalogue = sf::Vector2f(20.f + (i % 4) * 100.f, 20.f + (i / 4) * 120.f);
                    }
                    ImVec2 pMin = {canvas_p0.x + p.position_catalogue.x, canvas_p0.y + p.position_catalogue.y};
                    ImVec2 pMax = {pMin.x + (texSize.x * PLANT_SCALE), pMin.y + (texSize.y * PLANT_SCALE)};

                    if (mouse_pos.x >= pMin.x && mouse_pos.x <= pMax.x &&
                        mouse_pos.y >= pMin.y && mouse_pos.y <= pMax.y) {
                        m_draggedPlantIdx = i;
                        m_dragOffset = sf::Vector2f(p.position_catalogue.x - (mouse_pos.x - canvas_p0.x),
                                                    p.position_catalogue.y - (mouse_pos.y - canvas_p0.y));
                        break;
                    }
                }
            }
        }

        if (ImGui::IsMouseDragging(ImGuiMouseButton_Left) && m_draggedPlantIdx != -1) {
            mesPlantes[m_draggedPlantIdx].position_catalogue = sf::Vector2f(
                (mouse_pos.x - canvas_p0.x) + m_dragOffset.x,
                (mouse_pos.y - canvas_p0.y) + m_dragOffset.y
            );
        }

        if (ImGui::IsMouseReleased(ImGuiMouseButton_Left) && m_draggedPlantIdx != -1) {
            m_draggedPlantIdx = -1;
            m_app->getUserBalcony().sauvegarderProfil("mon_balcon.json");
        }

        for (int i = 0; i < (int)mesPlantes.size(); i++) {
            auto& p = mesPlantes[i];
            
            if (p.position_catalogue.x < 0) {
                p.position_catalogue = sf::Vector2f(20.f + (i % 4) * 100.f, 20.f + (i / 4) * 120.f);
            }

            if (m_plantTextures.count(p.nom_espece)) {
                sf::Texture& tex = m_plantTextures[p.nom_espece];
                ImVec2 pMin = {canvas_p0.x + p.position_catalogue.x, canvas_p0.y + p.position_catalogue.y};
                ImVec2 pMax = {pMin.x + (tex.getSize().x * PLANT_SCALE), pMin.y + (tex.getSize().y * PLANT_SCALE)};
                
                if (m_draggedPlantIdx == i) {
                    if (m_renderTex.getSize().x != tex.getSize().x || m_renderTex.getSize().y != tex.getSize().y) {
                        m_renderTex.create(tex.getSize().x, tex.getSize().y);
                    }
                    m_renderTex.clear(sf::Color::Transparent);
                    sf::Sprite tempSpr(tex);
                    
                    m_outlineShader.setUniform("texture", sf::Shader::CurrentTexture);
                    m_outlineShader.setUniform("texSize", sf::Vector2f(tex.getSize()));
                    
                    m_renderTex.draw(tempSpr, &m_outlineShader);
                    m_renderTex.display();

                    dl->AddImage((ImTextureID)(intptr_t)m_renderTex.getTexture().getNativeHandle(), 
                                 pMin, pMax, ImVec2(0, 1), ImVec2(1, 0));
                } else {
                    dl->AddImage((ImTextureID)(intptr_t)tex.getNativeHandle(), pMin, pMax);
                }

                std::string label = p.surnom;
                ImVec2 textSz = ImGui::CalcTextSize(label.c_str());
                float scaledWidth = tex.getSize().x * PLANT_SCALE;
                dl->AddText({pMin.x + (scaledWidth - textSz.x)*0.5f, pMax.y + 5.f}, IM_COL32(200, 220, 200, 255), label.c_str());
            }
        }
    }
    
    ImGui::EndChild();
    ImGui::End();
}