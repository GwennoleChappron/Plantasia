#include "StateWiki.hpp"
#include "Core/Application.hpp"
#include "Services/DatabaseManager.hpp"
#include "Data/EnumInfo.hpp"
#include "UI/Widgets.hpp"
#include <imgui.h>
#include <cstring>

// ─────────────────────────────────────────────────────────────────────────────
//  Couleurs locales — toutes référencées depuis la palette centrale.
//  Ne jamais écrire ImVec4(0.2f, …) directement dans les fonctions Draw.
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
    // Sélectionner la première plante par défaut si rien n'est sélectionné
    const auto& plantes = m_app->getDatabase().getPlantesTrieesParNom();
    if (!plantes.empty() && !m_selectedPlant)
        m_selectedPlant = plantes.front();
}

void StateWiki::onExit() {}

void StateWiki::update(float /*dt*/) {}

void StateWiki::draw(sf::RenderWindow& /*window*/) {}

// ─────────────────────────────────────────────────────────────────────────────
//  drawImGui — POINT D'ENTRÉE
//  Ne contient QUE la mise en page globale + des appels de sous-fonctions.
//  Objectif : < 30 lignes.
// ─────────────────────────────────────────────────────────────────────────────

void StateWiki::drawImGui() {
    const ImGuiIO& io = ImGui::GetIO();
    ImGui::SetNextWindowPos({ 0, 0 });
    ImGui::SetNextWindowSize(io.DisplaySize);
    ImGui::Begin("##Wiki", nullptr,
        ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoResize |
        ImGuiWindowFlags_NoMove     | ImGuiWindowFlags_NoBringToFrontOnFocus);

    DrawBreadcrumb();
    ImGui::Spacing();

    // Mise en page : panneau gauche 260px | séparateur | panneau droit (reste)
    DrawLeftPanel();
    ImGui::SameLine();
    ImGui::SeparatorEx(ImGuiSeparatorFlags_Vertical);
    ImGui::SameLine();
    DrawRightPanel();

    ImGui::End();
}

// ─────────────────────────────────────────────────────────────────────────────
//  DrawBreadcrumb
//  Ligne de navigation en haut : "Wiki  >  Plantes  >  Thym"
// ─────────────────────────────────────────────────────────────────────────────

void StateWiki::DrawBreadcrumb() const {
    ImGui::TextColored(C::Muted,      "Wiki");
    ImGui::SameLine(); ImGui::TextColored(C::Muted, ">");
    ImGui::SameLine(); ImGui::TextColored(C::Muted, "Plantes");

    if (m_selectedPlant) {
        ImGui::SameLine(); ImGui::TextColored(C::Muted, ">");
        ImGui::SameLine(); ImGui::TextColored(C::Accent, "%s",
            m_selectedPlant->nom.c_str());
    }

    // Fil secondaire si on a navigué vers un sol ou une bouture
    if (m_activeTab == 1 && m_selectedSoil) {
        ImGui::SameLine(); ImGui::TextColored(C::Muted, ">");
        ImGui::SameLine(); ImGui::TextColored(C::Warning, "%s",
            EnumInfo::label(EnumInfo::typeSolFromString(m_selectedSoil->typeSol)));
    }
}

// ─────────────────────────────────────────────────────────────────────────────
//  DrawLeftPanel
//  Panneau gauche fixe : champ de recherche + liste des plantes.
//  Largeur 260px, hauteur pleine fenêtre.
// ─────────────────────────────────────────────────────────────────────────────

void StateWiki::DrawLeftPanel() {
    ImGui::BeginChild("##LeftPanel", { 260.f, 0.f }, false);

    // -- Champ de recherche --
    ImGui::SetNextItemWidth(-1.f);  // pleine largeur du panneau
    if (ImGui::InputTextWithHint("##search", "Rechercher une plante…",
                                  m_searchBuffer, sizeof(m_searchBuffer))) {
        // Pas de recalcul ici : filtrerParNom() est O(n) sur 16 plantes,
        // c'est négligeable. Sur 1000+ plantes, stocker le résultat en cache.
    }
    ImGui::Spacing();

    // -- Liste filtrée --
    const DatabaseManager& db = m_app->getDatabase();
    const auto liste = (m_searchBuffer[0] == '\0')
        ? db.getPlantesTrieesParNom()           // liste pré-triée du cache
        : [&]() {                               // filtre à la volée
            auto res = db.filtrerParNom(m_searchBuffer);
            return res;
          }();

    ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, { 4.f, 3.f });
    for (const Plant* p : liste) {
        const bool selected = (p == m_selectedPlant);

        // Mettre en valeur la plante sélectionnée
        if (selected)
            ImGui::PushStyleColor(ImGuiCol_Header, { 0.10f, 0.35f, 0.18f, 1.f });

        // Score balcon affiché en mini badge à droite du nom
        char label[64];
        std::snprintf(label, sizeof(label), "%s###plant_%s",
                      p->nom.c_str(), p->nom.c_str());

        if (ImGui::Selectable(label, selected, 0, { 0, 18.f })) {
            m_selectedPlant   = p;
            m_selectedSoil    = nullptr;  // reset navigation secondaire
            m_selectedBouture = nullptr;
            m_activeTab       = 0;
        }

        // Score en overlay à droite
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
//  Panneau droit : onglets Plante / Sol / Bouture.
//  Chaque onglet délègue à sa propre sous-fonction.
// ─────────────────────────────────────────────────────────────────────────────

void StateWiki::DrawRightPanel() {
    ImGui::BeginChild("##RightPanel", { 0.f, 0.f }, false);

    if (!m_selectedPlant) {
        ImGui::TextColored(C::Muted, "Sélectionne une plante dans la liste.");
        ImGui::EndChild();
        return;
    }

    // -- Onglets --
    if (ImGui::BeginTabBar("##WikiTabs")) {

        if (ImGui::BeginTabItem("Plante")) {
            m_activeTab = 0;
            DrawPlantDetails(*m_selectedPlant);
            ImGui::EndTabItem();
        }

        // Onglet Sol : actif seulement si une plante avec sol est sélectionnée
        const Soil* soil = m_app->getDatabase().findSol(
            m_selectedPlant->solRecommande);
        if (soil && ImGui::BeginTabItem("Sol")) {
            m_activeTab    = 1;
            m_selectedSoil = soil;
            DrawSoilDetails(*soil);
            ImGui::EndTabItem();
        }

        // Onglet Bouture : actif seulement si la plante a des boutures
        if (!m_selectedPlant->bouturesCompatibles.empty()) {
            const Bouture* bouture = m_app->getDatabase().findBouture(
                m_selectedPlant->bouturesCompatibles[0]);
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
//  Orchestre les sections de la fiche plante.
//  Ne contient aucun contenu botanique — seulement des appels.
// ─────────────────────────────────────────────────────────────────────────────

void StateWiki::DrawPlantDetails(const Plant& plant) {
    DrawPlantHeader     (plant);
    ImGui::Spacing();
    DrawPlantCultureCard(plant);
    ImGui::Spacing();
    DrawPlantWaterCard  (plant);
    ImGui::Spacing();
    DrawPlantCalendar   (plant);
    ImGui::Spacing();
    DrawPlantRelations  (plant);

    // Conseils — section texte longue, pas besoin d'une sous-fonction dédiée
    ImGui::SeparatorText("Conseils & entretien");
    ImGui::PushStyleColor(ImGuiCol_ChildBg, C::CardBg);
    ImGui::BeginChild("##conseils", { 0.f, 120.f }, true);
    ImGui::PushTextWrapPos(0.f);
    if (!plant.conseilEntretien.empty())
        ImGui::TextColored(C::Titre, "%s", plant.conseilEntretien.c_str());
    if (!plant.astucePro.empty()) {
        ImGui::Spacing();
        ImGui::TextColored(C::Warning, "Pro : ");
        ImGui::SameLine();
        ImGui::TextColored(C::Secondaire, "%s", plant.astucePro.c_str());
    }
    ImGui::PopTextWrapPos();
    ImGui::EndChild();
    ImGui::PopStyleColor();
}

// ─────────────────────────────────────────────────────────────────────────────
//  DrawPlantHeader
//  Nom + nom scientifique + badges difficulté / rusticité.
// ─────────────────────────────────────────────────────────────────────────────

void StateWiki::DrawPlantHeader(const Plant& plant) const {
    // Nom principal — grande taille simulée par couleur forte
    ImGui::TextColored(C::Accent, "%s", plant.nom.c_str());

    // Nom scientifique en italique visuel (couleur atténuée)
    if (!plant.nomScientifique.empty()) {
        ImGui::SameLine();
        ImGui::TextColored(C::Muted, "— %s", plant.nomScientifique.c_str());
    }

    // Score balcon
    ImGui::SameLine(ImGui::GetContentRegionAvail().x - 80.f);
    ImGui::TextColored(C::Secondaire, "Score balcon : ");
    ImGui::SameLine();
    DrawGauge("##score", static_cast<float>(plant.scoreBalcon), 100.f, C::Accent);

    ImGui::Spacing();

    // Badges : difficulté + rusticité + exposition
    DrawTag(EnumInfo::label(plant.difficulte),
            EnumInfo::get(plant.difficulte).color);
    ImGui::SameLine(0.f, 6.f);
    DrawTag(EnumInfo::label(plant.rusticite),
            EnumInfo::get(plant.rusticite).color);
    ImGui::SameLine(0.f, 6.f);
    DrawTag(EnumInfo::label(plant.exposition),
            EnumInfo::get(plant.exposition).color);

    // Famille + origine en ligne discrète
    ImGui::Spacing();
    ImGui::TextColored(C::Muted, "%s  ·  %s  ·  %s",
        plant.famille.c_str(),
        plant.origine.c_str(),
        plant.feuillage.c_str());
}

// ─────────────────────────────────────────────────────────────────────────────
//  DrawPlantCultureCard
//  Card avec les conditions de culture : pot, exposition, dimensions.
// ─────────────────────────────────────────────────────────────────────────────

void StateWiki::DrawPlantCultureCard(const Plant& plant) const {
    ImGui::SeparatorText("Culture");

    ImGui::PushStyleColor(ImGuiCol_ChildBg, C::CardBg);
    ImGui::BeginChild("##culture_card", { 0.f, 110.f }, true);

    const float col2 = 200.f;  // alignement colonne 2

    DrawInfoRow("Pot minimum",  plant.volumePotMin.c_str());
    DrawInfoRow("Dimensions",   plant.dimensionsAdulte.c_str());
    DrawInfoRow("Zone climat",  plant.zoneClimat.c_str());
    DrawInfoRow("Vent",
        EnumInfo::label(plant.expositionVent),
        EnumInfo::get(plant.expositionVent).color);
    DrawInfoRow("Croissance",   plant.vitesseCroissance.c_str());
    DrawInfoRow("Multiplication", plant.multiplication.c_str());

    (void)col2;  // utilisé implicitement via DrawInfoRow

    ImGui::EndChild();
    ImGui::PopStyleColor();

    // Tooltip sol sur la ligne "conseil terre" — affiché sous la card
    if (!plant.conseilTerre.empty()) {
        ImGui::PushTextWrapPos(0.f);
        ImGui::TextColored(C::Muted, "%s", plant.conseilTerre.c_str());
        ImGui::PopTextWrapPos();
    }
}

// ─────────────────────────────────────────────────────────────────────────────
//  DrawPlantWaterCard
//  Besoin en eau + fréquences d'arrosage + tolérance sécheresse.
// ─────────────────────────────────────────────────────────────────────────────

void StateWiki::DrawPlantWaterCard(const Plant& plant) const {
    ImGui::SeparatorText("Arrosage");

    ImGui::PushStyleColor(ImGuiCol_ChildBg, C::CardBg);
    ImGui::BeginChild("##water_card", { 0.f, 90.f }, true);

    // Jauge besoin en eau (0-5)
    const auto& eau = EnumInfo::getBesoinEau(plant.besoinEau);
    ImGui::TextColored(C::Muted, "Besoin : ");
    ImGui::SameLine();
    DrawGauge("##eau", static_cast<float>(plant.besoinEau), 5.f,
              { eau.color.x, eau.color.y, eau.color.z, 1.f });
    ImGui::SameLine();
    ImGui::TextColored({ eau.color.x, eau.color.y, eau.color.z, 1.f },
                       "%s", eau.label);

    DrawInfoRow("Été",         plant.frequenceEte.c_str());
    DrawInfoRow("Hiver",       plant.frequenceHiver.c_str());
    DrawInfoRow("Sécheresse",  plant.toleranceSecheresse.c_str());
    DrawInfoRow("Excès eau",   plant.sensibiliteExcesEau.c_str());

    if (!plant.conseilArrosage.empty()) {
        ImGui::Spacing();
        ImGui::PushTextWrapPos(0.f);
        ImGui::TextColored(C::Muted, "%s", plant.conseilArrosage.c_str());
        ImGui::PopTextWrapPos();
    }

    ImGui::EndChild();
    ImGui::PopStyleColor();
}

// ─────────────────────────────────────────────────────────────────────────────
//  DrawPlantCalendar
//  Frise mensuelle colorée : floraison (rose) + récolte (vert).
// ─────────────────────────────────────────────────────────────────────────────

void StateWiki::DrawPlantCalendar(const Plant& plant) const {
    ImGui::SeparatorText("Calendrier");

    constexpr float CELL_W = 38.f;
    constexpr float CELL_H = 28.f;

    for (int mois = 1; mois <= 12; ++mois) {
        const auto& m = EnumInfo::getMois(mois);

        // Déterminer la couleur de la cellule
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
            bgColor  = { 0.45f, 0.20f, 0.50f, 0.8f };  // violet = les deux
            txtColor = C::Titre;
        } else if (floraison) {
            bgColor  = { 0.75f, 0.25f, 0.45f, 0.7f };  // rose
            txtColor = C::Titre;
        } else if (recolte) {
            bgColor  = { 0.12f, 0.45f, 0.22f, 0.7f };  // vert foncé
            txtColor = C::Titre;
        }

        // Cellule colorée
        ImGui::PushStyleColor(ImGuiCol_ChildBg, bgColor);
        char id[16]; std::snprintf(id, sizeof(id), "##cal%d", mois);
        ImGui::BeginChild(id, { CELL_W, CELL_H }, true,
                          ImGuiWindowFlags_NoScrollbar);
        ImGui::SetCursorPosX((CELL_W - ImGui::CalcTextSize(m.court).x) * 0.5f);
        ImGui::SetCursorPosY((CELL_H - ImGui::GetTextLineHeight()) * 0.5f);
        ImGui::TextColored(txtColor, "%s", m.court);
        ImGui::EndChild();
        ImGui::PopStyleColor();

        if (mois < 12) ImGui::SameLine(0.f, 2.f);
    }

    // Légende
    ImGui::Spacing();
    DrawTag("Floraison", { 0.75f, 0.25f, 0.45f, 1.f });
    ImGui::SameLine(0.f, 8.f);
    DrawTag("Récolte", { 0.12f, 0.45f, 0.22f, 1.f });
    ImGui::SameLine(0.f, 8.f);
    DrawTag("Les deux", { 0.45f, 0.20f, 0.50f, 1.f });
}

// ─────────────────────────────────────────────────────────────────────────────
//  DrawPlantRelations
//  Liens vers Sol et Boutures — boutons de navigation.
// ─────────────────────────────────────────────────────────────────────────────

void StateWiki::DrawPlantRelations(const Plant& plant) {
    ImGui::SeparatorText("Liens");

    const DatabaseManager& db = m_app->getDatabase();

    // -- Sol recommandé --
    const Soil* sol = db.findSol(plant.solRecommande);
    if (sol) {
        ImGui::TextColored(C::Muted, "Sol recommandé : ");
        ImGui::SameLine();
        const auto& meta = EnumInfo::get(EnumInfo::typeSolFromString(sol->typeSol));
        if (ImGui::SmallButton(meta.label.c_str())) {
            m_selectedSoil = sol;
            m_activeTab    = 1;  // basculer sur l'onglet Sol
        }
        if (ImGui::IsItemHovered())
            ImGui::SetTooltip("%s", meta.description.c_str());
    }

    // -- Sol alternatif --
    const Soil* solAlt = db.findSol(plant.solAlternatif);
    if (solAlt) {
        ImGui::SameLine(0.f, 16.f);
        ImGui::TextColored(C::Muted, "Alternatif : ");
        ImGui::SameLine();
        const auto& metaAlt = EnumInfo::get(
            EnumInfo::typeSolFromString(solAlt->typeSol));
        if (ImGui::SmallButton(
                (metaAlt.label + "##alt").c_str())) {
            m_selectedSoil = solAlt;
            m_activeTab    = 1;
        }
    }

    // -- Boutures compatibles --
    if (!plant.bouturesCompatibles.empty()) {
        ImGui::Spacing();
        ImGui::TextColored(C::Muted, "Boutures :");
        for (const auto& nomBouture : plant.bouturesCompatibles) {
            ImGui::SameLine(0.f, 8.f);
            const Bouture* b = db.findBouture(nomBouture);
            if (!b) continue;
            const auto& meta = EnumInfo::get(EnumInfo::parseTypeTige(b->typeTige));
            if (ImGui::SmallButton(b->nom.c_str())) {
                m_selectedBouture = b;
                m_activeTab       = 2;
            }
            if (ImGui::IsItemHovered())
                ImGui::SetTooltip("Taux de réussite : %s\n%s",
                    b->tauxReussite.c_str(),
                    meta.description.c_str());
        }
    }

    // -- Type racinaire → pot recommandé --
    ImGui::Spacing();
    const auto& racinMeta = EnumInfo::get(plant.typeRacinairePot);
    ImGui::TextColored(C::Muted, "Type racinaire : ");
    ImGui::SameLine();
    DrawTag(racinMeta.label.c_str(), racinMeta.color);
    if (ImGui::IsItemHovered())
        ImGui::SetTooltip("%s", racinMeta.description.c_str());

    const Pot* pot = db.findPot(
        [&]() -> std::string {
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
        ImGui::TextColored(C::Muted, "→ Pot : ");
        ImGui::SameLine();
        ImGui::TextColored(C::Secondaire, "%s  %d-%dL",
            pot->formePot.c_str(), pot->volumeMin, pot->volumeMax);
    }
}

// ─────────────────────────────────────────────────────────────────────────────
//  DrawSoilDetails
//  Fiche sol complète — pH, composition, CEC, corrections.
// ─────────────────────────────────────────────────────────────────────────────

void StateWiki::DrawSoilDetails(const Soil& soil) {
    // Titre
    const auto& solMeta = EnumInfo::get(
        EnumInfo::typeSolFromString(soil.typeSol));
    ImGui::TextColored(C::Accent, "%s", solMeta.label.c_str());
    ImGui::SameLine();
    ImGui::TextColored(C::Muted, "— %s", soil.utilisation.c_str());
    ImGui::Spacing();

    // -- Jauges principales --
    ImGui::SeparatorText("Propriétés");
    ImGui::PushStyleColor(ImGuiCol_ChildBg, C::CardBg);
    ImGui::BeginChild("##soil_props", { 0.f, 130.f }, true);

    DrawInfoRow("Texture",        soil.texture.c_str());
    DrawInfoRow("Drainage",       soil.drainage.c_str(),
        EnumInfo::get(EnumInfo::DrainageSol::BON).color); // TODO: parser depuis string
    DrawInfoRow("Rétention eau",  soil.retentionEau.c_str());
    DrawInfoRow("Richesse",       soil.richesse.c_str());

    // pH avec plage colorée
    char phStr[32];
    std::snprintf(phStr, sizeof(phStr), "%.1f – %.1f", soil.phMin, soil.phMax);
    const ImVec4 phColor = (soil.phMin < 6.f)  ? ImVec4{0.7f,0.4f,0.8f,1.f}  // acide → violet
                         : (soil.phMax > 7.5f) ? ImVec4{0.8f,0.7f,0.3f,1.f}  // basique → ocre
                         :                       C::Accent;                    // neutre → vert
    DrawInfoRow("pH", phStr, phColor);

    ImGui::EndChild();
    ImGui::PopStyleColor();

    // -- Indice spécialisé --
    if (!soil.indiceClé.empty()) {
        ImGui::Spacing();
        ImGui::TextColored(C::Muted, "Indice %s : ", soil.indiceClé.c_str());
        ImGui::SameLine();
        DrawGauge("##indice", static_cast<float>(soil.indiceValeur), 100.f, C::Accent);
        ImGui::SameLine();
        ImGui::TextColored(C::Secondaire, "%d / 100", soil.indiceValeur);
    }

    // -- Composition --
    ImGui::SeparatorText("Composition");
    for (const auto& composant : soil.composition) {
        DrawTag(composant.c_str(), C::Secondaire);
        ImGui::SameLine(0.f, 4.f);
    }

    // -- Plantes adaptées --
    if (!soil.adaptePour.empty()) {
        ImGui::SeparatorText("Plantes adaptées");
        for (const auto& nom : soil.adaptePour) {
            ImGui::TextColored(C::Accent, "•");
            ImGui::SameLine();
            ImGui::TextColored(C::Titre, "%s", nom.c_str());
        }
    }

    // -- Corrections pH --
    ImGui::SeparatorText("Corriger le pH");
    ImGui::PushStyleColor(ImGuiCol_ChildBg, C::CardBg);
    ImGui::BeginChild("##ph_correct", { 0.f, 80.f }, true);

    if (!soil.correctionBaisser.empty()) {
        ImGui::TextColored(C::Muted, "Baisser le pH : ");
        for (const auto& s : soil.correctionBaisser) {
            ImGui::SameLine(0.f, 6.f);
            DrawTag(s.c_str(), { 0.58f, 0.35f, 0.75f, 1.f });
        }
    }
    if (!soil.correctionAugmenter.empty()) {
        ImGui::TextColored(C::Muted, "Augmenter le pH : ");
        for (const auto& s : soil.correctionAugmenter) {
            ImGui::SameLine(0.f, 6.f);
            DrawTag(s.c_str(), { 0.85f, 0.52f, 0.12f, 1.f });
        }
    }

    ImGui::EndChild();
    ImGui::PopStyleColor();

    // -- Risques --
    if (!soil.risques.empty()) {
        ImGui::Spacing();
        ImGui::TextColored(C::Muted, "Risques : ");
        for (const auto& r : soil.risques) {
            ImGui::SameLine(0.f, 6.f);
            DrawTag(r.c_str(), C::Danger);
        }
    }
}

// ─────────────────────────────────────────────────────────────────────────────
//  DrawBoutureDetails
//  Fiche bouture : type, substrat, étapes numérotées.
// ─────────────────────────────────────────────────────────────────────────────

void StateWiki::DrawBoutureDetails(const Bouture& bouture) {
    // En-tête
    const auto& tigeMeta = EnumInfo::get(EnumInfo::parseTypeTige(bouture.typeTige));
    ImGui::TextColored(C::Accent, "%s", bouture.nom.c_str());
    ImGui::SameLine(0.f, 12.f);
    DrawTag(tigeMeta.label.c_str(), tigeMeta.color);
    ImGui::SameLine(0.f, 6.f);
    const auto& tauxMeta = EnumInfo::get(EnumInfo::parseTauxReussite(bouture.tauxReussite));
    DrawTag(tauxMeta.label.c_str(), tauxMeta.color);
    ImGui::Spacing();

    // -- Infos pratiques --
    ImGui::SeparatorText("Conditions");
    ImGui::PushStyleColor(ImGuiCol_ChildBg, C::CardBg);
    ImGui::BeginChild("##bouture_cond", { 0.f, 120.f }, true);

    // Période (badges saisonniers)
    ImGui::TextColored(C::Muted, "Période : ");
    for (const auto& s : bouture.periode) {
        ImGui::SameLine(0.f, 6.f);
        const auto& sMeta = EnumInfo::get(EnumInfo::parseSaison(s));
        DrawTag(sMeta.label.c_str(), sMeta.color);
    }

    // Longueur de coupe
    if (bouture.longueurMin > 0) {
        ImGui::NewLine();
        char lgStr[32];
        std::snprintf(lgStr, sizeof(lgStr), "%d – %d cm",
            bouture.longueurMin, bouture.longueurMax);
        DrawInfoRow("Longueur coupe", lgStr);
    }

    // Substrat + hormone
    DrawInfoRow("Substrat",
        EnumInfo::label(EnumInfo::parseSubstrat(bouture.substrat)),
        EnumInfo::get(EnumInfo::parseSubstrat(bouture.substrat)).color);
    DrawInfoRow("Hormone",
        EnumInfo::label(EnumInfo::parseHormone(bouture.hormoneBouturage)),
        EnumInfo::get(EnumInfo::parseHormone(bouture.hormoneBouturage)).color);
    DrawInfoRow("Humidité",
        EnumInfo::label(EnumInfo::parseHumidite(bouture.humidite)),
        EnumInfo::get(EnumInfo::parseHumidite(bouture.humidite)).color);

    // Température + enracinement
    char tmpStr[48], enrStr[48];
    std::snprintf(tmpStr, sizeof(tmpStr), "%d – %d °C",
        bouture.temperatureMin, bouture.temperatureMax);
    std::snprintf(enrStr, sizeof(enrStr), "%d – %d jours",
        bouture.enracinementMin, bouture.enracinementMax);
    DrawInfoRow("Température",   tmpStr);
    DrawInfoRow("Enracinement",  enrStr);

    ImGui::EndChild();
    ImGui::PopStyleColor();

    // -- Étapes numérotées --
    ImGui::SeparatorText("Étapes");
    for (int i = 0; i < (int)bouture.etapes.size(); ++i) {
        ImGui::TextColored(C::Accent, "%d.", i + 1);
        ImGui::SameLine();
        // Transformer la clé JSON en texte lisible (ex: COUPER_SOUS_NOEUD → Couper sous nœud)
        // TODO: connecter à EnumInfo si tu ajoutes les étapes
        ImGui::TextColored(C::Titre, "%s", bouture.etapes[i].c_str());
    }

    // -- Plantes concernées --
    if (!bouture.plantesConcernees.empty()) {
        ImGui::SeparatorText("Plantes concernées");
        for (const auto& nom : bouture.plantesConcernees) {
            ImGui::TextColored(C::Accent, "•");
            ImGui::SameLine();
            // Lien cliquable vers la fiche plante
            if (ImGui::SmallButton(nom.c_str())) {
                const Plant* p = m_app->getDatabase().findPlante(nom);
                if (p) {
                    m_selectedPlant = p;
                    m_activeTab     = 0;
                }
            }
        }
    }
}

// ─────────────────────────────────────────────────────────────────────────────
//  WIDGETS PARTAGÉS
//  Fonctions statiques, réutilisables partout dans le projet.
// ─────────────────────────────────────────────────────────────────────────────

// Ligne "label : valeur" alignée sur deux colonnes
void StateWiki::DrawInfoRow(const char* label,
                             const char* value,
                             const ImVec4& valueColor) {
    ImGui::TextColored(C::Muted, "%s", label);
    ImGui::SameLine(150.f);  // colonne fixe à 150px
    ImGui::TextColored(valueColor, "%s", value);
}

// Badge rectangulaire coloré (texte + fond semi-transparent)
void StateWiki::DrawTag(const char* text, const ImVec4& color) {
    // Fond semi-transparent de la couleur du tag
    const ImVec4 bg = { color.x * 0.25f, color.y * 0.25f, color.z * 0.25f, 0.8f };
    ImGui::PushStyleColor(ImGuiCol_ChildBg, bg);
    const ImVec2 textSize = ImGui::CalcTextSize(text);
    ImGui::BeginChild(text, { textSize.x + 12.f, textSize.y + 6.f }, false,
                      ImGuiWindowFlags_NoScrollbar | ImGuiWindowFlags_NoInputs);
    ImGui::SetCursorPos({ 6.f, 3.f });
    ImGui::TextColored(color, "%s", text);
    ImGui::EndChild();
    ImGui::PopStyleColor();
}

// Barre de progression fine et colorée
void StateWiki::DrawGauge(const char* label,
                           float value, float maxValue,
                           const ImVec4& color) {
    ImGui::PushStyleColor(ImGuiCol_PlotHistogram, color);
    ImGui::PushStyleColor(ImGuiCol_FrameBg,
        { color.x * 0.15f, color.y * 0.15f, color.z * 0.15f, 1.f });
    ImGui::ProgressBar(value / maxValue, { 80.f, 8.f }, "");
    ImGui::PopStyleColor(2);
    (void)label;
}