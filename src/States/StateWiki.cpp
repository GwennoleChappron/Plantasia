#include "StateWiki.hpp"
#include "Core/Application.hpp"
#include "Services/DatabaseManager.hpp"
#include "Data/EnumInfo.hpp"
#include "UI/Widgets.hpp"
#include <imgui.h>
#include <cstring>

// ─────────────────────────────────────────────────────────────────────────────
//  Couleurs locales — toutes référencées depuis la palette centrale.
// ─────────────────────────────────────────────────────────────────────────────

namespace C {
    constexpr ImVec4 Titre      = { 0.88f, 0.92f, 0.88f, 1.f };  // Palette::Blanc
    constexpr ImVec4 Secondaire = { 0.55f, 0.68f, 0.55f, 1.f };  // Palette::VertPale
    constexpr ImVec4 Accent     = { 0.20f, 0.75f, 0.40f, 1.f };  // Palette::VertVif
    constexpr ImVec4 Muted      = { 0.40f, 0.50f, 0.40f, 1.f };
    constexpr ImVec4 Danger     = { 0.75f, 0.22f, 0.22f, 1.f };
    constexpr ImVec4 Warning    = { 0.85f, 0.52f, 0.12f, 1.f };
    constexpr ImVec4 CardBg     = { 0.04f, 0.07f, 0.04f, 0.60f };
}

// ─────────────────────────────────────────────────────────────────────────────
//  Cycle de vie du State
// ─────────────────────────────────────────────────────────────────────────────

StateWiki::StateWiki(Application* app) : State(app) {}

void StateWiki::onEnter() {
    const auto& plantes = m_app->getDatabase().getPlantesTrieesParNom();
    if (!plantes.empty() && !m_selectedPlant)
        m_selectedPlant = plantes.front();
}

void StateWiki::onExit() {}
void StateWiki::update(float /*dt*/) {}
void StateWiki::draw(sf::RenderWindow& /*window*/) {}

// ─────────────────────────────────────────────────────────────────────────────
//  drawImGui — POINT D'ENTRÉE
// ─────────────────────────────────────────────────────────────────────────────

void StateWiki::drawImGui() {
    const ImGuiIO& io = ImGui::GetIO();
    ImGui::SetNextWindowPos({ 0, 0 });
    ImGui::SetNextWindowSize(io.DisplaySize);
    ImGui::Begin("##Wiki", nullptr,
        ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoResize |
        ImGuiWindowFlags_NoMove     | ImGuiWindowFlags_NoBringToFrontOnFocus);

    DrawBreadcrumb();
    UI::Gap();

    DrawLeftPanel();
    ImGui::SameLine();
    ImGui::SeparatorEx(ImGuiSeparatorFlags_Vertical);
    ImGui::SameLine();
    DrawRightPanel();

    ImGui::End();
}

// ─────────────────────────────────────────────────────────────────────────────
//  DrawBreadcrumb
// ─────────────────────────────────────────────────────────────────────────────

void StateWiki::DrawBreadcrumb() const {
    ImGui::TextColored(C::Muted, "Wiki");
    ImGui::SameLine(); ImGui::TextColored(C::Muted, ">");
    ImGui::SameLine(); ImGui::TextColored(C::Muted, "Plantes");

    if (m_selectedPlant) {
        ImGui::SameLine(); ImGui::TextColored(C::Muted, ">");
        ImGui::SameLine(); ImGui::TextColored(C::Accent, "%s", m_selectedPlant->nom.c_str());
    }

    if (m_activeTab == 1 && m_selectedSoil) {
        ImGui::SameLine(); ImGui::TextColored(C::Muted, ">");
        ImGui::SameLine(); ImGui::TextColored(C::Warning, "%s",
            EnumInfo::label(EnumInfo::typeSolFromString(m_selectedSoil->typeSol)));
    }
}

// ─────────────────────────────────────────────────────────────────────────────
//  DrawLeftPanel
// ─────────────────────────────────────────────────────────────────────────────

void StateWiki::DrawLeftPanel() {
    ImGui::BeginChild("##LeftPanel", { 260.f, 0.f }, false);

    ImGui::SetNextItemWidth(-1.f);
    ImGui::InputTextWithHint("##search", "Rechercher une plante…",
                              m_searchBuffer, sizeof(m_searchBuffer));
    UI::Gap();

    const DatabaseManager& db = m_app->getDatabase();
    const auto liste = (m_searchBuffer[0] == '\0')
        ? db.getPlantesTrieesParNom()
        : db.filtrerParNom(m_searchBuffer);

    ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, { 4.f, 3.f });
    for (const Plant* p : liste) {
        const bool selected = (p == m_selectedPlant);

        if (selected)
            ImGui::PushStyleColor(ImGuiCol_Header, { 0.10f, 0.35f, 0.18f, 1.f });

        char label[64];
        std::snprintf(label, sizeof(label), "%s###plant_%s", p->nom.c_str(), p->nom.c_str());

        if (ImGui::Selectable(label, selected, 0, { 0, 18.f })) {
            m_selectedPlant   = p;
            m_selectedSoil    = nullptr;
            m_selectedBouture = nullptr;
            m_activeTab       = 0;
        }

        ImGui::SameLine(220.f);
        ImGui::TextColored(C::Secondaire, "%d", p->scoreBalcon);

        if (selected)
            ImGui::PopStyleColor();
    }
    ImGui::PopStyleVar();

    ImGui::EndChild();
}

// ─────────────────────────────────────────────────────────────────────────────
//  DrawRightPanel
// ─────────────────────────────────────────────────────────────────────────────

void StateWiki::DrawRightPanel() {
    ImGui::BeginChild("##RightPanel", { 0.f, 0.f }, false);

    if (!m_selectedPlant) {
        ImGui::TextColored(C::Muted, "Sélectionne une plante dans la liste.");
        ImGui::EndChild();
        return;
    }

    if (ImGui::BeginTabBar("##WikiTabs")) {

        if (ImGui::BeginTabItem("Plante")) {
            m_activeTab = 0;
            DrawPlantDetails(*m_selectedPlant);
            ImGui::EndTabItem();
        }

        const Soil* soil = m_app->getDatabase().findSol(m_selectedPlant->solRecommande);
        if (soil && ImGui::BeginTabItem("Sol")) {
            m_activeTab    = 1;
            m_selectedSoil = soil;
            DrawSoilDetails(*soil);
            ImGui::EndTabItem();
        }

        if (!m_selectedPlant->bouturesCompatibles.empty()) {
            const Bouture* bouture = m_app->getDatabase().findBouture(m_selectedPlant->bouturesCompatibles[0]);
            if (bouture && ImGui::BeginTabItem("Bouture")) {
                m_activeTab       = 2;
                m_selectedBouture = bouture;
                DrawBoutureDetails(*bouture);
                ImGui::EndTabItem();
            }
        }

        ImGui::EndTabBar();
    }

    ImGui::EndChild();
}

// ─────────────────────────────────────────────────────────────────────────────
//  DrawPlantDetails
// ─────────────────────────────────────────────────────────────────────────────

void StateWiki::DrawPlantDetails(const Plant& plant) {
    DrawPlantHeader(plant);
    UI::Gap();
    DrawPlantCultureCard(plant);
    UI::Gap();
    DrawPlantWaterCard(plant);
    UI::Gap();
    DrawPlantCalendar(plant);
    UI::Gap();
    DrawPlantRelations(plant);

    // Utilisation de InfoCard avec height 0.f pour s'auto-redimensionner au texte !
    UI::InfoCard("Conseils & entretien", "##conseils", 0.f, [&]() {
        if (!plant.conseilEntretien.empty())
            UI::BodyText(plant.conseilEntretien.c_str());
        
        if (!plant.astucePro.empty()) {
            UI::Gap();
            UI::LabelAccent("Pro :", plant.astucePro.c_str(), C::Secondaire);
        }
    });
}

// ─────────────────────────────────────────────────────────────────────────────
//  DrawPlantHeader
// ─────────────────────────────────────────────────────────────────────────────

void StateWiki::DrawPlantHeader(const Plant& plant) const {
    ImGui::TextColored(C::Accent, "%s", plant.nom.c_str());

    if (!plant.nomScientifique.empty()) {
        ImGui::SameLine();
        ImGui::TextColored(C::Muted, "— %s", plant.nomScientifique.c_str());
    }

    // Gauge simplifiée avec Text intégré
    ImGui::SameLine(ImGui::GetContentRegionAvail().x - 120.f);
    UI::Gauge(plant.scoreBalcon, 100, C::Accent, "Score : ");

    UI::Gap();

    UI::TagBadge(EnumInfo::label(plant.difficulte), EnumInfo::get(plant.difficulte).color);
    ImGui::SameLine(0.f, 6.f);
    UI::TagBadge(EnumInfo::label(plant.rusticite), EnumInfo::get(plant.rusticite).color);
    ImGui::SameLine(0.f, 6.f);
    UI::TagBadge(EnumInfo::label(plant.exposition), EnumInfo::get(plant.exposition).color);

    UI::Gap();
    ImGui::TextColored(C::Muted, "%s  ·  %s  ·  %s",
        plant.famille.c_str(), plant.origine.c_str(), plant.feuillage.c_str());
}

// ─────────────────────────────────────────────────────────────────────────────
//  DrawPlantCultureCard
// ─────────────────────────────────────────────────────────────────────────────

void StateWiki::DrawPlantCultureCard(const Plant& plant) const {
    UI::InfoCard("Culture", "##culture_card", 110.f, [&]() {
        UI::InfoRow("Pot minimum",    plant.volumePotMin);
        UI::InfoRow("Dimensions",     plant.dimensionsAdulte);
        UI::InfoRow("Zone climat",    plant.zoneClimat);
        UI::InfoRow("Vent",           EnumInfo::label(plant.expositionVent), EnumInfo::get(plant.expositionVent).color);
        UI::InfoRow("Croissance",     plant.vitesseCroissance);
        UI::InfoRow("Multiplication", plant.multiplication);
    });

    if (!plant.conseilTerre.empty()) {
        UI::BodyText(plant.conseilTerre.c_str());
    }
}

// ─────────────────────────────────────────────────────────────────────────────
//  DrawPlantWaterCard
// ─────────────────────────────────────────────────────────────────────────────

void StateWiki::DrawPlantWaterCard(const Plant& plant) const {
    UI::InfoCard("Arrosage", "##water_card", 90.f, [&]() {
        const auto& eau = EnumInfo::getBesoinEau(plant.besoinEau);
        
        // Jauge Segmentée parfaite pour les données discrètes 0-5
        ImGui::TextColored(C::Muted, "Besoin : ");
        ImGui::SameLine(UI::Theme::ColLabel);
        UI::GaugeSegmented(plant.besoinEau, 5, { eau.color.x, eau.color.y, eau.color.z, 1.f });
        ImGui::SameLine();
        ImGui::TextColored({ eau.color.x, eau.color.y, eau.color.z, 1.f }, "%s", eau.label);

        UI::InfoRow("Été",        plant.frequenceEte);
        UI::InfoRow("Hiver",      plant.frequenceHiver);
        UI::InfoRow("Sécheresse", plant.toleranceSecheresse);
        UI::InfoRow("Excès eau",  plant.sensibiliteExcesEau);
    });

    if (!plant.conseilArrosage.empty()) {
        UI::BodyText(plant.conseilArrosage.c_str());
    }
}

// ─────────────────────────────────────────────────────────────────────────────
//  DrawPlantCalendar
// ─────────────────────────────────────────────────────────────────────────────

void StateWiki::DrawPlantCalendar(const Plant& plant) const {
    UI::Section("Calendrier");

    constexpr float CELL_W = 38.f;
    constexpr float CELL_H = 28.f;

    for (int mois = 1; mois <= 12; ++mois) {
        const auto& m = EnumInfo::getMois(mois);

        const bool floraison = (plant.floraisonDebut > 0 && plant.floraisonFin > 0)
            && ((plant.floraisonDebut <= plant.floraisonFin)
                    ? (mois >= plant.floraisonDebut && mois <= plant.floraisonFin)
                    : (mois >= plant.floraisonDebut || mois <= plant.floraisonFin));

        const bool recolte = (plant.recolteDebut > 0 && plant.recolteFin > 0)
            && ((plant.recolteDebut <= plant.recolteFin)
                    ? (mois >= plant.recolteDebut && mois <= plant.recolteFin)
                    : (mois >= plant.recolteDebut || mois <= plant.recolteFin));

        ImVec4 bgColor = C::CardBg;
        ImVec4 txtColor = C::Muted;

        if (floraison && recolte) {
            bgColor  = { 0.45f, 0.20f, 0.50f, 0.8f }; 
            txtColor = C::Titre;
        } else if (floraison) {
            bgColor  = { 0.75f, 0.25f, 0.45f, 0.7f }; 
            txtColor = C::Titre;
        } else if (recolte) {
            bgColor  = { 0.12f, 0.45f, 0.22f, 0.7f }; 
            txtColor = C::Titre;
        }

        ImGui::PushStyleColor(ImGuiCol_ChildBg, bgColor);
        char id[16]; std::snprintf(id, sizeof(id), "##cal%d", mois);
        ImGui::BeginChild(id, { CELL_W, CELL_H }, true, ImGuiWindowFlags_NoScrollbar);
        ImGui::SetCursorPosX((CELL_W - ImGui::CalcTextSize(m.court).x) * 0.5f);
        ImGui::SetCursorPosY((CELL_H - ImGui::GetTextLineHeight()) * 0.5f);
        ImGui::TextColored(txtColor, "%s", m.court);
        ImGui::EndChild();
        ImGui::PopStyleColor();

        if (mois < 12) ImGui::SameLine(0.f, 2.f);
    }

    UI::Gap();
    UI::TagBadge("Floraison", { 0.75f, 0.25f, 0.45f, 1.f }); ImGui::SameLine(0.f, 8.f);
    UI::TagBadge("Récolte",   { 0.12f, 0.45f, 0.22f, 1.f }); ImGui::SameLine(0.f, 8.f);
    UI::TagBadge("Les deux",  { 0.45f, 0.20f, 0.50f, 1.f });
}

// ─────────────────────────────────────────────────────────────────────────────
//  DrawPlantRelations
// ─────────────────────────────────────────────────────────────────────────────

void StateWiki::DrawPlantRelations(const Plant& plant) {
    UI::Section("Liens");

    const DatabaseManager& db = m_app->getDatabase();

    // -- Sol recommandé --
    const Soil* sol = db.findSol(plant.solRecommande);
    if (sol) {
        ImGui::TextColored(C::Muted, "Sol recommandé : ");
        ImGui::SameLine();
        const auto& meta = EnumInfo::get(EnumInfo::typeSolFromString(sol->typeSol));
        
        UI::NavButton(meta.label.c_str(), [&]() {
            m_selectedSoil = sol; m_activeTab = 1;
        }, meta.description.c_str());
    }

    // -- Sol alternatif --
    const Soil* solAlt = db.findSol(plant.solAlternatif);
    if (solAlt) {
        ImGui::SameLine(0.f, 16.f);
        ImGui::TextColored(C::Muted, "Alternatif : ");
        ImGui::SameLine();
        const auto& metaAlt = EnumInfo::get(EnumInfo::typeSolFromString(solAlt->typeSol));
        
        UI::NavButton(metaAlt.label + "##alt", [&]() {
            m_selectedSoil = solAlt; m_activeTab = 1;
        }, metaAlt.description.c_str());
    }

    // -- Boutures compatibles --
    if (!plant.bouturesCompatibles.empty()) {
        UI::Gap();
        ImGui::TextColored(C::Muted, "Boutures :");
        for (const auto& nomBouture : plant.bouturesCompatibles) {
            ImGui::SameLine(0.f, 8.f);
            const Bouture* b = db.findBouture(nomBouture);
            if (!b) continue;
            
            const auto& meta = EnumInfo::get(EnumInfo::parseTypeTige(b->typeTige));
            std::string tooltip = "Taux de réussite : " + b->tauxReussite + "\n" + meta.description;
            
            UI::NavButton(b->nom.c_str(), [&]() {
                m_selectedBouture = b; m_activeTab = 2;
            }, tooltip.c_str());
        }
    }

    // -- Type racinaire → pot recommandé --
    UI::Gap();
    const auto& racinMeta = EnumInfo::get(plant.typeRacinairePot);
    ImGui::TextColored(C::Muted, "Type racinaire : ");
    ImGui::SameLine();
    UI::TagBadge(racinMeta.label.c_str(), racinMeta.color, racinMeta.description.c_str());

    const Pot* pot = db.findPot([&]() -> std::string {
        switch (plant.typeRacinairePot) {
            case TypeRacinaireEnum::FASCICULE: return "FASCICULE";
            case TypeRacinaireEnum::PIVOTANT:  return "PIVOTANT";
            case TypeRacinaireEnum::TRACANT:   return "TRACANT";
            case TypeRacinaireEnum::LIGNEUX:   return "LIGNEUX";
            default:                           return "";
        }
    }());
    
    if (pot) {
        ImGui::SameLine(0.f, 12.f);
        char potStr[64];
        std::snprintf(potStr, sizeof(potStr), "%s  %d-%dL", pot->formePot.c_str(), pot->volumeMin, pot->volumeMax);
        UI::LabelAccent("→ Pot :", potStr, C::Secondaire);
    }
}

// ─────────────────────────────────────────────────────────────────────────────
//  DrawSoilDetails
// ─────────────────────────────────────────────────────────────────────────────

void StateWiki::DrawSoilDetails(const Soil& soil) {
    const auto& solMeta = EnumInfo::get(EnumInfo::typeSolFromString(soil.typeSol));
    ImGui::TextColored(C::Accent, "%s", solMeta.label.c_str());
    ImGui::SameLine();
    ImGui::TextColored(C::Muted, "— %s", soil.utilisation.c_str());
    UI::Gap();

    UI::InfoCard("Propriétés", "##soil_props", 130.f, [&]() {
        UI::InfoRow("Texture",       soil.texture);
        UI::InfoRow("Drainage",      soil.drainage, EnumInfo::get(EnumInfo::DrainageSol::BON).color); // À adapter
        UI::InfoRow("Rétention eau", soil.retentionEau);
        UI::InfoRow("Richesse",      soil.richesse);

        char phStr[32];
        std::snprintf(phStr, sizeof(phStr), "%.1f – %.1f", soil.phMin, soil.phMax);
        const ImVec4 phColor = (soil.phMin < 6.f)  ? ImVec4{0.7f,0.4f,0.8f,1.f} 
                             : (soil.phMax > 7.5f) ? ImVec4{0.8f,0.7f,0.3f,1.f} 
                             : C::Accent;
        UI::InfoRow("pH", phStr, phColor);
    });

    if (!soil.indiceClé.empty()) {
        UI::Gap();
        ImGui::TextColored(C::Muted, "Indice %s : ", soil.indiceClé.c_str());
        ImGui::SameLine(UI::Theme::ColLabel);
        
        char suffix[16];
        std::snprintf(suffix, sizeof(suffix), "%d / 100", soil.indiceValeur);
        UI::Gauge(soil.indiceValeur, 100, C::Accent, nullptr, suffix);
    }

    UI::Section("Composition");
    for (const auto& composant : soil.composition) {
        UI::TagBadge(composant.c_str(), C::Secondaire);
        ImGui::SameLine(0.f, 4.f);
    }

    if (!soil.adaptePour.empty()) {
        UI::Section("Plantes adaptées");
        for (const auto& nom : soil.adaptePour) {
            ImGui::TextColored(C::Accent, "•");
            ImGui::SameLine();
            ImGui::TextColored(C::Titre, "%s", nom.c_str());
        }
    }

    UI::InfoCard("Corriger le pH", "##ph_correct", 80.f, [&]() {
        if (!soil.correctionBaisser.empty()) {
            ImGui::TextColored(C::Muted, "Baisser le pH : ");
            for (const auto& s : soil.correctionBaisser) {
                ImGui::SameLine(0.f, 6.f);
                UI::TagBadge(s.c_str(), { 0.58f, 0.35f, 0.75f, 1.f });
            }
        }
        if (!soil.correctionAugmenter.empty()) {
            ImGui::TextColored(C::Muted, "Augmenter le pH : ");
            for (const auto& s : soil.correctionAugmenter) {
                ImGui::SameLine(0.f, 6.f);
                UI::TagBadge(s.c_str(), { 0.85f, 0.52f, 0.12f, 1.f });
            }
        }
    });

    if (!soil.risques.empty()) {
        UI::Gap();
        ImGui::TextColored(C::Muted, "Risques : ");
        for (const auto& r : soil.risques) {
            ImGui::SameLine(0.f, 6.f);
            UI::TagBadge(r.c_str(), C::Danger);
        }
    }
}

// ─────────────────────────────────────────────────────────────────────────────
//  DrawBoutureDetails
// ─────────────────────────────────────────────────────────────────────────────

void StateWiki::DrawBoutureDetails(const Bouture& bouture) {
    const auto& tigeMeta = EnumInfo::get(EnumInfo::parseTypeTige(bouture.typeTige));
    ImGui::TextColored(C::Accent, "%s", bouture.nom.c_str());
    ImGui::SameLine(0.f, 12.f);
    UI::TagBadge(tigeMeta.label.c_str(), tigeMeta.color);
    ImGui::SameLine(0.f, 6.f);
    
    const auto& tauxMeta = EnumInfo::get(EnumInfo::parseTauxReussite(bouture.tauxReussite));
    UI::TagBadge(tauxMeta.label.c_str(), tauxMeta.color);
    UI::Gap();

    UI::InfoCard("Conditions", "##bouture_cond", 120.f, [&]() {
        ImGui::TextColored(C::Muted, "Période : ");
        for (const auto& s : bouture.periode) {
            ImGui::SameLine(0.f, 6.f);
            const auto& sMeta = EnumInfo::get(EnumInfo::parseSaison(s));
            UI::TagBadge(sMeta.label.c_str(), sMeta.color);
        }

        if (bouture.longueurMin > 0) {
            ImGui::NewLine();
            char lgStr[32];
            std::snprintf(lgStr, sizeof(lgStr), "%d – %d cm", bouture.longueurMin, bouture.longueurMax);
            UI::InfoRow("Longueur coupe", lgStr);
        }

        UI::InfoRow("Substrat", EnumInfo::label(EnumInfo::parseSubstrat(bouture.substrat)), EnumInfo::get(EnumInfo::parseSubstrat(bouture.substrat)).color);
        UI::InfoRow("Hormone",  EnumInfo::label(EnumInfo::parseHormone(bouture.hormoneBouturage)), EnumInfo::get(EnumInfo::parseHormone(bouture.hormoneBouturage)).color);
        UI::InfoRow("Humidité", EnumInfo::label(EnumInfo::parseHumidite(bouture.humidite)), EnumInfo::get(EnumInfo::parseHumidite(bouture.humidite)).color);

        char tmpStr[48], enrStr[48];
        std::snprintf(tmpStr, sizeof(tmpStr), "%d – %d °C", bouture.temperatureMin, bouture.temperatureMax);
        std::snprintf(enrStr, sizeof(enrStr), "%d – %d jours", bouture.enracinementMin, bouture.enracinementMax);
        UI::InfoRow("Température",  tmpStr);
        UI::InfoRow("Enracinement", enrStr);
    });

    UI::Section("Étapes");
    for (int i = 0; i < (int)bouture.etapes.size(); ++i) {
        ImGui::TextColored(C::Accent, "%d.", i + 1);
        ImGui::SameLine();
        ImGui::TextColored(C::Titre, "%s", bouture.etapes[i].c_str());
    }

    if (!bouture.plantesConcernees.empty()) {
        UI::Section("Plantes concernées");
        for (const auto& nom : bouture.plantesConcernees) {
            ImGui::TextColored(C::Accent, "•");
            ImGui::SameLine();
            
            UI::NavButton(nom, [&]() {
                const Plant* p = m_app->getDatabase().findPlante(nom);
                if (p) {
                    m_selectedPlant = p; m_activeTab = 0;
                }
            });
        }
    }
}