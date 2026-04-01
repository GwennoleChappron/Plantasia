#include "StateCatalogue.hpp"
#include <algorithm>
#include <cmath>

#include "UI/ColorTheme.hpp"
#include "UI/Widgets.hpp"
#include "Data/EnumInfo.hpp"
#include "Services/DatabaseManager.hpp"
#include "Core/Application.hpp"

namespace {
    // ── SHADER ──
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

    // ── HELPERS LOCAUX RESTANTS ──
    // On conserve uniquement ce qui n'est pas (encore) dans EnumInfo.hpp
    ImVec4 CouleurTypePlante(TypePlante t) {
        switch (t) {
            case TypePlante::AROMATIQUE: return Theme::PlantGreen; 
            case TypePlante::FRUITIER:   return Theme::WarningOrange; 
            case TypePlante::FLEUR:      return Theme::Floraison; 
            case TypePlante::LEGUME:     return Theme::InfoBlue; 
        }
        return Theme::TextSecondary;
    }
}

// ─────────────────────────────────────────────────────────────────────────────
//  CYCLE DE VIE
// ─────────────────────────────────────────────────────────────────────────────

StateCatalogue::StateCatalogue(Application* app) : State(app) {}

void StateCatalogue::onEnter() {
    if (sf::Shader::isAvailable())
        m_outlineShader.loadFromMemory(catalogueOutlineShaderCode, sf::Shader::Fragment);
    chargerTextures();
}

void StateCatalogue::chargerTextures() {
    for (const auto& p : m_app->getUserBalcony().getMesPlantes()) {
        if (m_plantTextures.find(p.nom_espece) == m_plantTextures.end()) {
            sf::Texture tex;
            if (!tex.loadFromFile("assets/" + p.nom_espece + ".png")) {
                sf::Image img;
                img.create(64, 64, sf::Color(40, 100, 50, 200));
                tex.loadFromImage(img);
            }
            tex.setSmooth(true);
            m_plantTextures[p.nom_espece] = std::move(tex);
        }

        if (m_plantIconTextures.find(p.nom_espece) == m_plantIconTextures.end()) {
            sf::Texture icon;
            if (!icon.loadFromFile("assets/" + p.nom_espece + "_icon.png"))
                icon = m_plantTextures[p.nom_espece]; 
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

// ─────────────────────────────────────────────────────────────────────────────
//  CALENDRIER VISUEL (Refactorisé avec ColorTheme)
// ─────────────────────────────────────────────────────────────────────────────

void StateCatalogue::DessinerCalendrierVisuel(const char* label, int debut, int fin, ImVec4 col) {
    ImGui::TextColored(Theme::TextSecondary, "%s", label);
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
        
        // Utilisation de GaugeInactif pour les cellules éteintes
        ImU32 cellColor = on ? ImGui::ColorConvertFloat4ToU32(col) : ImGui::ColorConvertFloat4ToU32(Theme::GaugeInactif);
        dl->AddRectFilled(pmin, pmax, cellColor, 4.f);
        
        if (on) {
            ImVec2 tsz = ImGui::CalcTextSize(initMois[i]);
            dl->AddText(ImVec2(pmin.x + (step - 4 - tsz.x) * 0.5f, pmin.y + 3), IM_COL32(10,20,10,255), initMois[i]);
        }
    }
    UI::Gap(26.f); 
}

// ─────────────────────────────────────────────────────────────────────────────
//  INTERFACE PRINCIPALE IMGUI
// ─────────────────────────────────────────────────────────────────────────────

void StateCatalogue::drawImGui() {
    float W = (float)m_app->getWindow().getSize().x;
    float H = (float)m_app->getWindow().getSize().y;
    const float LARGEUR_LISTE = 340.f;
    const float MARGE = 45.f; 

    ImGui::SetNextWindowPos({MARGE, MARGE});
    ImGui::SetNextWindowSize({W - MARGE*2, H - MARGE*2});
    ImGui::Begin("##principal", nullptr, ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove);

    // ── TOP BAR ──
    UI::NavButtonPrimary("< Retour", [&](){ m_app->getStateMachine().removeState(); });
    ImGui::SameLine(0, 20.f);
    UI::LabelAccent("PLANTASIA", "- Mon Balcon", Theme::TextSecondary);
    UI::Gap(6.f);

    float hauteurContenu = ImGui::GetContentRegionAvail().y;
    float largeurDetail  = ImGui::GetContentRegionAvail().x - LARGEUR_LISTE - 16.f;

    // ════════ PANNEAU GAUCHE (MES PLANTES) ════════
    ImGui::BeginChild("##gauche", {LARGEUR_LISTE, hauteurContenu}, false);
    
    ImGui::SetNextItemWidth(-1.f);
    ImGui::InputTextWithHint("##rech", "Rechercher...", m_filtreRecherche, sizeof(m_filtreRecherche));
    UI::Gap();

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
        
        // Fonds thémés
        ImU32 bgCol = ImGui::ColorConvertFloat4ToU32(
            sel ? Theme::darken(Theme::PlantDark, 0.8f) : 
           (hov ? Theme::SurfaceHaute : Theme::Surface)
        );
        dlL->AddRectFilled(pos0, {pos0.x+sz.x, pos0.y+sz.y}, bgCol, 8.f);
        
        // Bandeau couleur type
        ImVec4 colType = CouleurTypePlante(refP->type);
        dlL->AddRectFilled(pos0, {pos0.x + 6.f, pos0.y+sz.y}, ImGui::ColorConvertFloat4ToU32(colType), 8.f, ImDrawFlags_RoundCornersLeft);

        ImVec4 cNom = sel ? Theme::PlantGreen : (hov ? Theme::TextPrimary : Theme::TextSecondary);
        ImGui::SetCursorScreenPos({pos0.x+18, pos0.y+10});
        ImGui::TextColored(cNom, "%s", up.surnom.c_str());
        
        // Utilisation d'EnumInfo pour la rusticité
        ImGui::SetCursorScreenPos({pos0.x+18, pos0.y+32});
        ImGui::TextColored(EnumInfo::get(refP->rusticite).color, "- %s", EnumInfo::label(refP->rusticite));

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
                ImU32 fadeColor = ImGui::ColorConvertFloat4ToU32(Theme::withAlpha(Theme::Fond, 0.9f));
                dl->AddRectFilledMultiColor(bgMin, bgMax, IM_COL32(7,10,7,0), IM_COL32(7,10,7,0), fadeColor, fadeColor);
            }

            // ═══ ICÔNE PNG ANIMÉE ═══
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
            ImGui::TextColored(Theme::PlantGreen, "%s", up.surnom.c_str());
            ImGui::SetWindowFontScale(1.0f);

            ImGui::TextColored(Theme::TextSecondary, "Espèce : %s", refP->nom.c_str());
            if (!refP->nom_scientifique.empty()) {
                ImGui::TextDisabled("( %s ) - Famille : %s", refP->nom_scientifique.c_str(), refP->famille.c_str());
            }
            
            if (!refP->autres_noms.empty()) {
                std::string alias = "Aussi appelé : ";
                for (size_t k = 0; k < refP->autres_noms.size(); ++k) {
                    alias += refP->autres_noms[k] + (k < refP->autres_noms.size() - 1 ? ", " : "");
                }
                ImGui::TextColored(Theme::TextMuted, "%s", alias.c_str());
            }
            
            // --- ÉDITEUR DE DATE D'ADOPTION ---
            UI::Gap();
            ImGui::TextColored(Theme::TextPrimary, "Date d'adoption :");
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
            ImGui::TextColored(Theme::TextPrimary, "Pot (Litres) :");
            ImGui::SameLine();
            if (ImGui::DragInt("##pot", &editablePlant.volume_pot_actuel_L, 0.5f, 1, 500)) dateChanged = true;
            ImGui::PopItemWidth();

            if (dateChanged) {
                m_app->getUserBalcony().sauvegarderProfil("mon_balcon.json");
            }

            ImGui::Separator();
            UI::Gap();

            if (refP->floraison_debut > 0 || refP->recolte_debut > 0) {
                if (refP->floraison_debut > 0) 
                    DessinerCalendrierVisuel("Période de Floraison", refP->floraison_debut, refP->floraison_fin, Theme::Floraison);
                if (refP->recolte_debut > 0) 
                    DessinerCalendrierVisuel("Période de Récolte", refP->recolte_debut, refP->recolte_fin, Theme::PlantGreen);
            }

            // ═══ ONGLETS D'INFORMATIONS ═══
            if (ImGui::BeginTabBar("OngletsConseils")) {

                if (ImGui::BeginTabItem("Botanique")) {
                    UI::InfoCardRaw("##bot", 0.f, [&]() {
                        UI::InfoRow("Origine", refP->origine);
                        UI::InfoRow("Dimensions Adulte", refP->dimensions_adulte);
                        UI::InfoRow("Feuillage", refP->feuillage);
                    });
                    if (!refP->notes.empty()) {
                        UI::Section("Description");
                        UI::BodyText(refP->notes.c_str());
                    }
                    ImGui::EndTabItem();
                }

                if (ImGui::BeginTabItem("Culture & Eau")) {
                    UI::InfoCardRaw("##cult", 0.f, [&]() {
                        UI::InfoRow("Exposition", EnumInfo::label(refP->exposition_soleil), EnumInfo::get(refP->exposition_soleil).color);
                        UI::InfoRow("Vent", EnumInfo::label(refP->exposition_vent), EnumInfo::get(refP->exposition_vent).color);
                        UI::InfoRow("Zone Climat", refP->zone_climat);
                        UI::InfoRow("Volume du pot min.", refP->volume_pot_min);
                        UI::InfoRow("Substrat recommandé", refP->sol_recommande, Theme::WarningOrange);
                    });
                    
                    UI::InfoCardRaw("##eau", 0.f, [&]() {
                        UI::InfoRow("Fréquence Été", refP->frequence_arrosage_ete, Theme::Amber);
                        UI::InfoRow("Fréquence Hiver", refP->frequence_arrosage_hiver, Theme::InfoBlue);
                    });

                    if (!refP->conseil_arrosage.empty()) {
                        UI::Section("Conseil Arrosage");
                        UI::BodyText(refP->conseil_arrosage.c_str());
                    }
                    ImGui::EndTabItem();
                }

                if (ImGui::BeginTabItem("Entretien & Santé")) {
                    UI::InfoCardRaw("##entr", 0.f, [&]() {
                        UI::InfoRow("Type racinaire", refP->type_racinaire_pot);
                        UI::InfoRow("Compagnonnage", refP->compagnonnage);
                    });

                    if (!refP->conseil_entretien.empty()) {
                        UI::Section("Taille & Soins");
                        UI::BodyText(refP->conseil_entretien.c_str());
                    }
                    
                    if (!refP->maladies.empty()) {
                        UI::Section("Maladies & Parasites");
                        UI::BodyText(refP->maladies.c_str(), Theme::DangerRed);
                    }
                    ImGui::EndTabItem();
                }

                if (ImGui::BeginTabItem("Usages")) {
                    UI::InfoCardRaw("##usa", 0.f, [&]() {
                        if (refP->toxicite_animaux == "NON") {
                            UI::InfoRow("Toxicité Animaux", "Sans danger (Pet-friendly)", Theme::PlantGreen);
                        } else if (!refP->toxicite_animaux.empty()) {
                            UI::InfoRow("Toxicité Animaux", refP->toxicite_animaux, Theme::DangerRed);
                        }
                    });

                    if (!refP->vertus_medicinales.empty()) {
                        UI::Section("Vertus & Usages");
                        UI::BodyText(refP->vertus_medicinales.c_str());
                    }
                    
                    if (!refP->precautions.empty()) {
                        UI::Section("Précautions");
                        UI::BodyText(refP->precautions.c_str(), Theme::WarningOrange);
                    }
                    ImGui::EndTabItem();
                }

                // ONGLET 5 : STATISTIQUES (Remplacé par UI::Gauge)
                if (ImGui::BeginTabItem("Statistiques")) {
                    UI::Gap();
                    UI::InfoCard("Profil de culture", "##stats", 0.f, [&]() {
                        // Jauge Besoin Eau (Segmentée 0-5)
                        const auto& eau = EnumInfo::getBesoinEau(refP->besoin_eau);
                        ImGui::TextColored(Theme::TextMuted, "Besoin en eau :");
                        ImGui::SameLine(UI::ColLabel);
                        UI::GaugeSegmented(refP->besoin_eau, 5, { eau.color.x, eau.color.y, eau.color.z, 1.f });
                        ImGui::SameLine();
                        ImGui::TextColored({ eau.color.x, eau.color.y, eau.color.z, 1.f }, "%s", eau.label);
                        UI::Gap(4.f);

                        // Jauge Difficulté (Segmentée 1-3)
                        const auto& diff = EnumInfo::get(refP->niveau_difficulte);
                        int diffInt = (refP->niveau_difficulte == NiveauDifficulte::FACILE) ? 1 :
                                      (refP->niveau_difficulte == NiveauDifficulte::MOYEN) ? 2 : 3;
                        ImGui::TextColored(Theme::TextMuted, "Difficulté :");
                        ImGui::SameLine(UI::ColLabel);
                        UI::GaugeSegmented(diffInt, 3, diff.color);
                        ImGui::SameLine();
                        ImGui::TextColored(diff.color, "%s", diff.label.c_str());
                        UI::Gap(4.f);

                        // Jauge Score Balcon (Continue 0-100)
                        ImGui::TextColored(Theme::TextMuted, "Score Balcon :");
                        ImGui::SameLine(UI::ColLabel);
                        char scoreStr[16];
                        std::snprintf(scoreStr, sizeof(scoreStr), "%d / 100", refP->score_balcon);
                        UI::Gauge(refP->score_balcon, 100, Theme::Amber, nullptr, scoreStr);
                    });

                    if (refP->contrainte_majeure != "Aucune" && !refP->contrainte_majeure.empty()) {
                        UI::Gap();
                        UI::InfoCardRaw("##alerte", 0.f, [&]() {
                            UI::LabelAccent("Attention :", refP->contrainte_majeure.c_str(), Theme::DangerRed);
                        }, Theme::withAlpha(Theme::DangerRed, 0.1f), Theme::DangerRed);
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
                dl->AddText({pMin.x + (scaledWidth - textSz.x)*0.5f, pMax.y + 5.f}, 
                            ImGui::ColorConvertFloat4ToU32(Theme::TextPrimary), label.c_str());
            }
        }
    }
    
    ImGui::EndChild();
    ImGui::End();
}