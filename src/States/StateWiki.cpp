#include "StateWiki.hpp"
#include "Core/Application.hpp"
#include "Services/DatabaseManager.hpp"
#include "Data/EnumInfo.hpp"
#include "UI/Widgets.hpp"
#include <imgui.h>
#include <imgui_internal.h>
#include <cstring>
#include <cmath>
#include <algorithm>

// ─────────────────────────────────────────────────────────────────────────────
//  Couleurs locales
// ─────────────────────────────────────────────────────────────────────────────

namespace C {
    constexpr ImVec4 Titre      = { 0.88f, 0.92f, 0.88f, 1.f };
    constexpr ImVec4 Secondaire = { 0.55f, 0.68f, 0.55f, 1.f };
    constexpr ImVec4 Accent     = { 0.20f, 0.75f, 0.40f, 1.f };
    constexpr ImVec4 Muted      = { 0.40f, 0.50f, 0.40f, 1.f };
    constexpr ImVec4 Danger     = { 0.75f, 0.22f, 0.22f, 1.f };
    constexpr ImVec4 Warning    = { 0.85f, 0.52f, 0.12f, 1.f };
    constexpr ImVec4 CardBg     = { 0.04f, 0.07f, 0.04f, 0.60f };
}

// ─────────────────────────────────────────────────────────────────────────────
//  Cycle de vie
// ─────────────────────────────────────────────────────────────────────────────

StateWiki::StateWiki(Application* app) : State(app) {}

void StateWiki::onEnter() {
    const auto& plantes = m_app->getDatabase().getPlantesTrieesParNom();
    if (!plantes.empty() && !m_selectedPlant) {
        m_selectedPlant = plantes.front();
        m_animScore     = (float)m_selectedPlant->scoreBalcon;
        m_dropAnim      = (float)m_selectedPlant->besoinEau;
        m_fadeAlpha     = 1.f;
        m_panelTime     = 0.f;
    }
}

void StateWiki::onExit() {}
void StateWiki::draw(sf::RenderWindow& /*window*/) {}

// ─────────────────────────────────────────────────────────────────────────────
//  update()
// ─────────────────────────────────────────────────────────────────────────────

void StateWiki::update(float dt) {
    if (!m_selectedPlant) return;

    m_panelTime += dt;

    if (m_selectedPlant != m_prevPlant) {
        m_fadeAlpha     = 0.f;       
        m_etapeReveal   = 0.f;       
        m_donutSweep    = 0.f;
             
        m_prevPlant     = m_selectedPlant;
    }

    if (m_activeTab != m_prevTab) {
        m_donutSweep  = 0.f;
        m_etapeReveal = 0.f;
        m_vineProgress = 0.f;
        m_prevTab     = m_activeTab;
    }

    m_fadeAlpha = UI::Anim::StepSmooth(m_fadeAlpha, 1.f, 6.f);
    m_vineProgress = UI::Anim::StepSmooth(m_vineProgress, 1.f, 2.5f);

    const float targetScore = (float)m_selectedPlant->scoreBalcon;
    m_animScore = UI::Anim::StepSmooth(m_animScore, targetScore, 8.f);

    const float targetDrop = (float)m_selectedPlant->besoinEau;
    m_dropAnim = UI::Anim::StepSmooth(m_dropAnim, targetDrop, 7.f);

    m_donutSweep = std::min(1.f, m_donutSweep + dt * 2.f);
    m_etapeReveal += dt / 0.25f;
}

// ─────────────────────────────────────────────────────────────────────────────
//  drawImGui
// ─────────────────────────────────────────────────────────────────────────────

void StateWiki::drawImGui() {
    const ImGuiIO& io = ImGui::GetIO();
    ImGui::SetNextWindowPos({ 0, 0 });
    ImGui::SetNextWindowSize(io.DisplaySize);
    ImGui::Begin("##Wiki", nullptr,
        ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoResize |
        ImGuiWindowFlags_NoMove     | ImGuiWindowFlags_NoBringToFrontOnFocus);

    UI::NavButtonPrimary("< Retour", [&](){ m_app->getStateMachine().removeState(); });
    DrawBreadcrumb();
    UI::Gap();

    DrawLeftPanel();
    ImGui::SameLine();
    ImGui::SeparatorEx(ImGuiSeparatorFlags_Vertical);
    ImGui::SameLine();
    DrawRightPanel();

    ImGui::End();
}

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
        ImGui::SameLine(); ImGui::TextColored(C::Warning, "%s", EnumInfo::label(m_selectedSoil->typeSol));
    }
}

// ─────────────────────────────────────────────────────────────────────────────
//  DrawLeftPanel (Liste animée)
// ─────────────────────────────────────────────────────────────────────────────

void StateWiki::DrawLeftPanel() {
    ImGui::BeginChild("##LeftPanel", { 280.f, 0.f }, false);

    ImGui::SetNextItemWidth(-1.f);
    ImGui::InputTextWithHint("##search", "Rechercher une plante…", m_searchBuffer, sizeof(m_searchBuffer));
    UI::Gap();

    const DatabaseManager& db = m_app->getDatabase();
    const auto liste = (m_searchBuffer[0] == '\0') ? db.getPlantesTrieesParNom() : db.filtrerParNom(m_searchBuffer);

    ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, { 4.f, 8.f }); 

    int index = 0;
    for (const Plant* p : liste) {
        ImGuiID id = ImGui::GetID(p->nom.c_str());
        const bool selected = (p == m_selectedPlant);

        float itemTime = UI::Anim::Stagger(index, m_panelTime, 0.04f); 
        float enterProgress = std::clamp(itemTime * 3.0f, 0.f, 1.f); 
        
        if (enterProgress < 0.01f) { index++; continue; } 
        
        ImGui::BeginGroup(); 
        UI::Anim::ApplySlideFade(enterProgress, 15.f);

        ImVec2 basePos = ImGui::GetCursorScreenPos();
        ImVec2 itemSize = { 260.f, 26.f }; 
        
        ImRect itemRect(basePos, {basePos.x + itemSize.x, basePos.y + itemSize.y});
        bool isHovered = ImGui::IsMouseHoveringRect(itemRect.Min, itemRect.Max);

        float glow = UI::Anim::HoverProgress(id, isHovered || selected);
        float targetScale = (isHovered && !selected) ? 1.04f : 1.0f; 
        float scale = UI::Anim::Spring(id, targetScale, 300.f, 15.f); 

        float offsetX = (itemSize.x * scale - itemSize.x) * 0.5f;
        float offsetY = (itemSize.y * scale - itemSize.y) * 0.5f;
        ImVec2 scaledPos = { basePos.x - offsetX, basePos.y - offsetY };
        ImVec2 scaledSize = { itemSize.x * scale, itemSize.y * scale };

        const ImVec4 bgBase     = { 0.04f, 0.06f, 0.04f, 1.f };
        const ImVec4 bgHover    = { 0.12f, 0.28f, 0.15f, 1.f }; 
        const ImVec4 bgSelected = { 0.18f, 0.55f, 0.28f, 1.f }; 
        ImVec4 bgFinal = selected ? bgSelected : UI::Anim::LerpColor(bgBase, bgHover, glow);

        ImDrawList* dl = ImGui::GetWindowDrawList();
        dl->AddRectFilled(scaledPos, {scaledPos.x + scaledSize.x, scaledPos.y + scaledSize.y}, 
                          ImGui::ColorConvertFloat4ToU32(bgFinal), 6.f * scale);

        ImVec4 textColor = UI::Anim::LerpColor(C::Muted, C::Titre, std::max(glow, enterProgress));
        dl->AddText({scaledPos.x + 12.f, scaledPos.y + 4.f * scale}, 
                    ImGui::ColorConvertFloat4ToU32(textColor), p->nom.c_str());

        char scoreTxt[16]; std::snprintf(scoreTxt, sizeof(scoreTxt), "%d", p->scoreBalcon);
        ImVec2 scoreSz = ImGui::CalcTextSize(scoreTxt);
        dl->AddText({scaledPos.x + scaledSize.x - scoreSz.x - 12.f, scaledPos.y + 4.f * scale}, 
                    ImGui::ColorConvertFloat4ToU32(UI::Anim::WithAlpha(C::Secondaire, 0.5f + glow * 0.5f)), scoreTxt);

        ImGui::SetCursorScreenPos(basePos);
        if (ImGui::InvisibleButton(p->nom.c_str(), itemSize)) {
            m_selectedPlant = p;
            m_activeTab = 0;
            m_selectedSoil = nullptr;
            m_selectedBouture = nullptr;
        }

        ImGui::PopStyleVar(); 
        ImGui::EndGroup();

        ImGui::SetCursorScreenPos({ basePos.x, basePos.y + itemSize.y + ImGui::GetStyle().ItemSpacing.y });
        index++;
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

    const float alpha = UI::Anim::EaseOut(m_fadeAlpha);
    ImGui::PushStyleVar(ImGuiStyleVar_Alpha, alpha);

    if (ImGui::BeginTabBar("##WikiTabs")) {
        if (ImGui::BeginTabItem("Plante")) {
            m_activeTab = 0;
            DrawPlantDetails(*m_selectedPlant);
            ImGui::EndTabItem();
        }

        const Soil* soil = m_app->getDatabase().findSol(m_selectedPlant->solEnum);
        if (soil && ImGui::BeginTabItem("Sol")) {
            m_activeTab    = 1;
            m_selectedSoil = soil;
            DrawSoilDetails(*soil);
            ImGui::EndTabItem();
        }

        if (!m_selectedPlant->bouturesCompatibles.empty()) {
            const Bouture* bouture = m_app->getDatabase().findBouture(m_selectedPlant->bouturesCompatibles.front());
            if (bouture && ImGui::BeginTabItem("Bouture")) {
                m_activeTab       = 2;
                m_selectedBouture = bouture;
                DrawBoutureDetails(*bouture);
                ImGui::EndTabItem();
            }
        }
        ImGui::EndTabBar();
    }

    ImGui::PopStyleVar(); 
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

    UI::InfoCard("Conseils & entretien", "##conseils", 0.f, [&]() {
        if (!plant.conseilEntretien.empty()) UI::BodyText(plant.conseilEntretien.c_str());
        if (!plant.astucePro.empty()) {
            UI::Gap(); UI::LabelAccent("Pro :", plant.astucePro.c_str(), C::Secondaire);
        }
    });
}

// ─────────────────────────────────────────────────────────────────────────────
//  DrawPlantHeader (Avec Wiggle Toxique)
// ─────────────────────────────────────────────────────────────────────────────

void StateWiki::DrawPlantHeader(const Plant& plant) {
    ImGui::TextColored(C::Accent, "%s", plant.nom.c_str());
    if (!plant.nomScientifique.empty()) {
        ImGui::SameLine(); ImGui::TextColored(C::Muted, "— %s", plant.nomScientifique.c_str());
    }

    ImGui::SameLine(ImGui::GetContentRegionAvail().x - 120.f);
    UI::Gauge(m_animScore, 100.f, C::Accent, "Score : ");

    ImGui::SameLine(); ImGui::TextColored(C::Secondaire, "%d", (int)m_animScore);
    UI::Gap();

    UI::TagBadge(EnumInfo::label(plant.difficulte),    EnumInfo::get(plant.difficulte).color);
    ImGui::SameLine(0.f, 6.f);
    UI::TagBadge(EnumInfo::label(plant.rusticite),     EnumInfo::get(plant.rusticite).color);
    ImGui::SameLine(0.f, 6.f);
    UI::TagBadge(EnumInfo::label(plant.expositionSoleil), EnumInfo::get(plant.expositionSoleil).color);

    if (plant.toxiciteAnimaux) {
        ImGui::SameLine(0.f, 6.f);
        
        // ── WIGGLE (Tremblement au survol) ──
        ImVec2 tagPos = ImGui::GetCursorScreenPos();
        ImRect tagRect(tagPos, {tagPos.x + 70.f, tagPos.y + 20.f}); // Zone approximative
        bool isHovered = ImGui::IsMouseHoveringRect(tagRect.Min, tagRect.Max);
        
        static bool wasHovered = false;
        bool justHovered = isHovered && !wasHovered;
        wasHovered = isHovered;

        float wiggleX = UI::Anim::Wiggle(ImGui::GetID("Toxique"), justHovered, 150.f);
        ImGui::SetCursorPosX(ImGui::GetCursorPosX() + wiggleX);

        const float p = UI::Anim::Pulse(1.5f);
        const ImVec4 dangerPulse = { 0.75f + p * 0.20f, 0.10f + p * 0.05f, 0.10f + p * 0.05f, 1.f };
        UI::TagBadge("Toxique", dangerPulse, plant.toxiciteNote.empty() ? "Dangereux pour les animaux" : plant.toxiciteNote.c_str());
    }

    UI::Gap();
    ImGui::TextColored(C::Muted, "%s  ·  %s  ·  %s",
        plant.famille.c_str(), plant.origine.c_str(), EnumInfo::label(plant.feuillage));
}

// ─────────────────────────────────────────────────────────────────────────────
//  DrawPlantCultureCard & WaterCard
// ─────────────────────────────────────────────────────────────────────────────

void StateWiki::DrawPlantCultureCard(const Plant& plant) const {
    UI::InfoCard("Culture", "##culture_card", 110.f, [&]() {
        UI::InfoRow("Pot minimum",    plant.volumePotMin);
        UI::InfoRow("Dimensions",     plant.dimensionsAdulte);
        UI::InfoRow("Zone climat",    plant.zoneClimat);
        UI::InfoRow("Vent",           EnumInfo::get(plant.expositionVent));
        UI::InfoRow("Croissance",     EnumInfo::get(plant.vitesseCroissance));
        UI::InfoRow("Multiplication", plant.multiplication);
    });
    if (!plant.solTexte.empty()) UI::BodyText(plant.solTexte.c_str());
}

void StateWiki::DrawPlantWaterCard(const Plant& plant) {
    UI::InfoCard("Arrosage", "##water_card", 90.f, [&]() {
        const auto& eau = EnumInfo::getBesoinEau(plant.besoinEau);
        const ImVec4 eauColor = { eau.color.x, eau.color.y, eau.color.z, 1.f };

        ImGui::TextColored(C::Muted, "Besoin : ");
        ImGui::SameLine(UI::Theme::ColLabel);
        UI::GaugeWaterDrops((int)m_dropAnim, 5, eauColor);
        ImGui::SameLine(0.f, 6.f);
        ImGui::TextColored(eauColor, "%s", eau.label);

        UI::InfoRow("Été",        plant.frequenceEte);
        UI::InfoRow("Hiver",      plant.frequenceHiver);
        UI::InfoRow("Sécheresse", EnumInfo::get(plant.toleranceSecheresse));
        UI::InfoRow("Excès eau",  EnumInfo::get(plant.sensibiliteExcesEau));
    });
    if (!plant.conseilArrosage.empty()) UI::BodyText(plant.conseilArrosage.c_str());
}

// ─────────────────────────────────────────────────────────────────────────────
//  DrawPlantCalendar
// ─────────────────────────────────────────────────────────────────────────────

void StateWiki::DrawPlantCalendar(const Plant& plant) const {
    UI::SectionAnimated("Calendrier", m_vineProgress, true);

    constexpr float CELL_W = 38.f;
    constexpr float CELL_H = 28.f;
    static float monthHover[13] = {};

    for (int mois = 1; mois <= 12; ++mois) {
        const auto& m = EnumInfo::getMois(mois);

        const bool floraison = (plant.floraisonDebut > 0 && plant.floraisonFin > 0) &&
            ((plant.floraisonDebut <= plant.floraisonFin) ? (mois >= plant.floraisonDebut && mois <= plant.floraisonFin) : (mois >= plant.floraisonDebut || mois <= plant.floraisonFin));
        const bool recolte = (plant.recolteDebut > 0 && plant.recolteFin > 0) &&
            ((plant.recolteDebut <= plant.recolteFin) ? (mois >= plant.recolteDebut && mois <= plant.recolteFin) : (mois >= plant.recolteDebut || mois <= plant.recolteFin));

        ImVec4 bgBase = C::CardBg;
        ImVec4 txtColor = C::Muted;
        if (floraison && recolte) { bgBase = { 0.45f, 0.20f, 0.50f, 0.8f }; txtColor = C::Titre; }
        else if (floraison)       { bgBase = { 0.75f, 0.25f, 0.45f, 0.7f }; txtColor = C::Titre; }
        else if (recolte)         { bgBase = { 0.12f, 0.45f, 0.22f, 0.7f }; txtColor = C::Titre; }

        ImVec2 cellPos = ImGui::GetCursorScreenPos();
        bool isHovered = ImGui::IsMouseHoveringRect(cellPos, { cellPos.x + CELL_W, cellPos.y + CELL_H });
            
        monthHover[mois] = UI::Anim::StepSmooth(monthHover[mois], isHovered ? 1.f : 0.f, 12.f);

        const ImVec4 bgHoverColor = { std::min(1.f, bgBase.x+0.15f), std::min(1.f, bgBase.y+0.15f), std::min(1.f, bgBase.z+0.15f), std::min(1.f, bgBase.w+0.15f) };
        const ImVec4 bgFinal = UI::Anim::LerpColor(bgBase, bgHoverColor, monthHover[mois]);

        ImGui::PushStyleColor(ImGuiCol_ChildBg, bgFinal);
        char id[16]; std::snprintf(id, sizeof(id), "##cal%d", mois);
        ImGui::BeginChild(id, { CELL_W, CELL_H }, true, ImGuiWindowFlags_NoScrollbar);

        ImGui::SetCursorPosX((CELL_W - ImGui::CalcTextSize(m.court).x) * 0.5f);
        ImGui::SetCursorPosY((CELL_H - ImGui::GetTextLineHeight()) * 0.5f);

        const ImVec4 txtFinal = UI::Anim::LerpColor(txtColor, { 1.f, 1.f, 1.f, 1.f }, monthHover[mois] * 0.3f);
        ImGui::TextColored(txtFinal, "%s", m.court);

        ImGui::EndChild();
        ImGui::PopStyleColor();

        if (isHovered) {
            std::string tip = m.long_;
            if (floraison) tip += "\nFloraison";
            if (recolte)   tip += "\nRécolte";
            ImGui::SetTooltip("%s", tip.c_str());
        }

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
    UI::SectionAnimated("Plantes concernées", m_vineProgress);
    const DatabaseManager& db = m_app->getDatabase();

    const Soil* sol = db.findSol(plant.solEnum);
    if (sol) {
        ImGui::TextColored(C::Muted, "Sol recommandé : "); ImGui::SameLine();
        const auto& meta = EnumInfo::get(sol->typeSol);
        UI::NavButton(meta.label.c_str(), [&](){ m_selectedSoil = sol; m_activeTab = 1; }, meta.description.c_str());
    }

    const Soil* solAlt = db.findSol(plant.solAlternatifEnum);
    if (solAlt) {
        ImGui::SameLine(0.f, 16.f); ImGui::TextColored(C::Muted, "Alternatif : "); ImGui::SameLine();
        const auto& metaAlt = EnumInfo::get(solAlt->typeSol);
        UI::NavButton(metaAlt.label + "##alt", [&](){ m_selectedSoil = solAlt; m_activeTab = 1; }, metaAlt.description.c_str());
    }

    if (!plant.bouturesCompatibles.empty()) {
        UI::Gap(); ImGui::TextColored(C::Muted, "Boutures :");
        for (const auto& nomBouture : plant.bouturesCompatibles) {
            ImGui::SameLine(0.f, 8.f);
            const Bouture* b = db.findBouture(nomBouture);
            if (!b) continue;
            const auto& meta = EnumInfo::get(b->typeTige);
            std::string tooltip = "Taux : " + std::string(EnumInfo::label(b->tauxReussite)) + "\n" + meta.description;
            UI::NavButton(EnumInfo::label(b->typeTige), [&](){ m_selectedBouture = b; m_activeTab = 2; }, tooltip.c_str());
        }
    }

    UI::Gap();
    const auto& racinMeta = EnumInfo::get(plant.typeRacinaireEnum);
    ImGui::TextColored(C::Muted, "Type racinaire : "); ImGui::SameLine();
    UI::TagBadge(racinMeta.label.c_str(), racinMeta.color, racinMeta.description.c_str());

    const Racine* racine = db.findRacine(plant.typeRacinaireEnum);
    if (racine) {
        ImGui::SameLine(0.f, 12.f);
        char racineStr[64];
        std::snprintf(racineStr, sizeof(racineStr), "%s  %d-%dL", EnumInfo::label(racine->formePot), racine->volumeMin, racine->volumeMax);
        UI::LabelAccent("→ Pot :", racineStr, C::Secondaire);
    }
}

// ─────────────────────────────────────────────────────────────────────────────
//  DrawSoilDetails — Donut avec Breath
// ─────────────────────────────────────────────────────────────────────────────

void StateWiki::DrawSoilDetails(const Soil& soil) {
    const auto& solMeta = EnumInfo::get(soil.typeSol);
    ImGui::TextColored(C::Accent, "%s", solMeta.label.c_str()); ImGui::SameLine();
    ImGui::TextColored(C::Muted, "— %s", soil.utilisation.c_str());
    UI::Gap();

    UI::InfoCard("Propriétés", "##soil_props", 130.f, [&]() {
        UI::InfoRow("Texture",       EnumInfo::get(soil.texture));
        UI::InfoRow("Drainage",      EnumInfo::get(soil.drainage));
        UI::InfoRow("Rétention eau", EnumInfo::get(soil.retentionEau));
        UI::InfoRow("Richesse",      EnumInfo::get(soil.richesse));

        char phStr[32]; std::snprintf(phStr, sizeof(phStr), "%.1f – %.1f", soil.phMin, soil.phMax);
        const ImVec4 phColor = (soil.phMin < 6.f) ? ImVec4{0.7f, 0.4f, 0.8f, 1.f} : (soil.phMax > 7.5f) ? ImVec4{0.8f, 0.7f, 0.3f, 1.f} : C::Accent;
        UI::InfoRow("pH", phStr, phColor);
    });

    if (!soil.indiceClef.empty()) {
        UI::Gap();
        ImGui::TextColored(C::Muted, "Indice %s : ", soil.indiceClef.c_str()); ImGui::SameLine(UI::Theme::ColLabel);
        char suffix[16]; std::snprintf(suffix, sizeof(suffix), "%d / 100", soil.indiceValeur);
        UI::Gauge(soil.indiceValeur, 100, C::Accent, nullptr, suffix);
    }

    UI::SectionAnimated("Plantes concernées", m_vineProgress);
    UI::InfoCardRaw("##composition_card", 0.f, [&]() {
        
        // ── RESPIRATION (Gonfle légèrement le donut en permanence) ──
        const float breath = UI::Anim::Breath(1.5f, 0.04f); // 4% d'amplitude

        const float outerRadius = 35.f * breath;
        const float innerRadius = 20.f * breath;
        const float thickness   = outerRadius - innerRadius;
        const float midRadius   = innerRadius + thickness * 0.5f;

        float estTotalWidth = (35.f * 2.f) + 25.f + 130.f; // Base statique pour le centrage
        float availWidth    = ImGui::GetContentRegionAvail().x;
        float offsetX       = std::max(0.f, (availWidth - estTotalWidth) * 0.5f);
        ImGui::SetCursorPosX(ImGui::GetCursorPosX() + offsetX);

        ImDrawList* dl  = ImGui::GetWindowDrawList();
        ImVec2      pos = ImGui::GetCursorScreenPos();
        ImVec2   center = { pos.x + 35.f, pos.y + 35.f }; // On garde le centre fixe

        const float sweepFactor = UI::Anim::EaseOut(m_donutSweep);
        float currentAngle = -3.14159265f / 2.f;

        ImGui::BeginGroup();
        for (const auto& composant : soil.composition) {
            float percentage = composant.second;
            if (percentage <= 0.f) continue;
            float angleSweep = (percentage / 100.f) * (2.f * 3.14159265f) * sweepFactor;
            const ImVec4& color = EnumInfo::get(composant.first).color;

            int numSeg = std::max(4, (int)(32.f * (percentage / 100.f)));
            dl->PathArcTo(center, midRadius, currentAngle, currentAngle + angleSweep, numSeg);
            dl->PathStroke(ImGui::ColorConvertFloat4ToU32(color), 0, thickness);
            currentAngle += (percentage / 100.f) * (2.f * 3.14159265f); 
        }
        ImGui::Dummy({ 70.f, 70.f }); // Dummy statique pour ne pas casser le layout
        ImGui::EndGroup();

        ImGui::SameLine(0.f, 25.f);

        ImGui::BeginGroup();
        float legendH = (float)soil.composition.size() * ImGui::GetTextLineHeightWithSpacing();
        float offsetY = std::max(0.f, (70.f - legendH) * 0.5f);
        if (offsetY > 0) ImGui::Dummy({ 0.f, offsetY });

        for (const auto& composant : soil.composition) {
            if (composant.second <= 0.f) continue;
            const auto& meta = EnumInfo::get(composant.first);

            ImVec2 p = ImGui::GetCursorScreenPos();
            float lineH = ImGui::GetTextLineHeight(), r = lineH * 0.35f;
            dl->AddCircleFilled({ p.x + r, p.y + lineH * 0.5f }, r, ImGui::ColorConvertFloat4ToU32(meta.color));
            ImGui::SetCursorScreenPos({ p.x + r * 2.f + 8.f, p.y });

            char legStr[64]; std::snprintf(legStr, sizeof(legStr), "%2.0f%%  %s", composant.second, meta.label.c_str());
            ImGui::TextColored(UI::Theme::Titre, "%s", legStr);
        }
        ImGui::EndGroup();
    });

    if (!soil.adaptePour.empty()) {
        UI::SectionAnimated("Plantes concernées", m_vineProgress);
        for (const auto& nom : soil.adaptePour) {
            ImGui::TextColored(C::Accent, "•"); ImGui::SameLine(); ImGui::TextColored(C::Titre, "%s", nom.c_str());
        }
    }

    UI::InfoCard("Corriger le pH", "##ph_correct", 0.f, [&]() {
        if (!soil.correctionBaisser.empty()) {
            ImGui::TextColored(C::Muted, "Baisser le pH : ");
            bool any = false;
            for (const auto& s : soil.correctionBaisser) {
                const char* lbl = EnumInfo::label(s);
                if (lbl && lbl[0] != '\0' && std::strcmp(lbl,"Inconnu") != 0 && std::strcmp(lbl,"?") != 0) {
                    ImGui::SameLine(0.f, 6.f); UI::TagBadge(lbl, { 0.58f, 0.35f, 0.75f, 1.f }); any = true;
                }
            }
            if (any) ImGui::NewLine();
        }
        if (!soil.correctionAugmenter.empty()) {
            ImGui::TextColored(C::Muted, "Augmenter le pH : ");
            for (const auto& s : soil.correctionAugmenter) {
                const char* lbl = EnumInfo::label(s);
                if (lbl && lbl[0] != '\0' && std::strcmp(lbl,"Inconnu") != 0 && std::strcmp(lbl,"?") != 0) {
                    ImGui::SameLine(0.f, 6.f); UI::TagBadge(lbl, { 0.85f, 0.52f, 0.12f, 1.f });
                }
            }
        }
    });

    if (!soil.risques.empty()) {
        UI::Gap(); ImGui::TextColored(C::Muted, "Risques : ");
        for (const auto& r : soil.risques) {
            ImGui::SameLine(0.f, 6.f); UI::TagBadge(EnumInfo::label(r), C::Danger);
        }
    }
}

// ─────────────────────────────────────────────────────────────────────────────
//  DrawBoutureDetails
// ─────────────────────────────────────────────────────────────────────────────

void StateWiki::DrawBoutureDetails(const Bouture& bouture) {
    const auto& tigeMeta  = EnumInfo::get(bouture.typeTige);
    const auto& tauxMeta  = EnumInfo::get(bouture.tauxReussite);

    ImGui::TextColored(C::Accent, "%s", EnumInfo::label(bouture.typeTige)); ImGui::SameLine(0.f, 12.f);
    UI::TagBadge(tigeMeta.label.c_str(), tigeMeta.color); ImGui::SameLine(0.f, 6.f);
    UI::TagBadge(tauxMeta.label.c_str(), tauxMeta.color);
    UI::Gap();

    UI::InfoCard("Conditions", "##bouture_cond", 120.f, [&]() {
        ImGui::TextColored(C::Muted, "Période : ");
        for (const auto& s : bouture.periode) {
            ImGui::SameLine(0.f, 6.f);
            const auto& sMeta = EnumInfo::get(s);
            UI::TagBadge(sMeta.label.c_str(), sMeta.color);
        }
        if (bouture.longueurMin > 0) {
            ImGui::NewLine();
            char lgStr[32]; std::snprintf(lgStr, sizeof(lgStr), "%d – %d cm", bouture.longueurMin, bouture.longueurMax);
            UI::InfoRow("Longueur coupe", lgStr);
        }
        UI::InfoRow("Substrat", EnumInfo::get(bouture.substrat));
        UI::InfoRow("Hormone",  EnumInfo::get(bouture.hormoneBouturage));
        UI::InfoRow("Humidité", EnumInfo::get(bouture.humidite));

        char tmpStr[48], enrStr[48];
        std::snprintf(tmpStr, sizeof(tmpStr), "%d – %d °C", bouture.temperatureMin, bouture.temperatureMax);
        std::snprintf(enrStr, sizeof(enrStr), "%d – %d jours", bouture.enracinementMin, bouture.enracinementMax);
        UI::InfoRow("Température",  tmpStr);
        UI::InfoRow("Enracinement", enrStr);
    });

    UI::SectionAnimated("Plantes concernées", m_vineProgress);
    const int nEtapes = (int)bouture.etapes.size();
    for (int i = 0; i < nEtapes; ++i) {
        const float etapeAlpha = std::min(1.f, std::max(0.f, m_etapeReveal - (float)i));
        if (etapeAlpha <= 0.01f) continue; 

        ImGui::PushStyleVar(ImGuiStyleVar_Alpha, ImGui::GetStyle().Alpha * etapeAlpha);
        ImGui::TextColored(C::Accent, "%d.", i + 1); ImGui::SameLine();
        ImGui::TextColored(C::Titre, "%s", EnumInfo::label(bouture.etapes[i]));
        ImGui::PopStyleVar();
    }

    if (!bouture.plantesConcernees.empty()) {
        UI::SectionAnimated("Plantes concernées", m_vineProgress);
        for (const auto& nom : bouture.plantesConcernees) {
            ImGui::TextColored(C::Accent, "•"); ImGui::SameLine();
            UI::NavButton(nom, [&]() {
                const Plant* p = m_app->getDatabase().findPlante(nom);
                if (p) { m_selectedPlant = p; m_activeTab = 0; }
            });
        }
    }
}