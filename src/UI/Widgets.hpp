#pragma once
#include "ColorTheme.hpp"
#include <imgui.h>
#include <functional>
#include <string>

// ═════════════════════════════════════════════════════════════════════════════
//  Widgets.hpp — Bibliothèque de widgets ImGui réutilisables pour Plantasia
//
//  Tous les widgets sont dans le namespace UI::.
//  Aucune dépendance au State, à Application, ni aux données métier.
//  Chaque widget est autonome : appelable depuis n'importe quel State.
//
//  Sommaire :
//    1. UI::TagBadge     — badge coloré avec fond semi-transparent
//    2. UI::InfoCard     — bloc card avec titre et contenu tabulaire
//    3. UI::Gauge        — barre de progression fine et colorée
//    4. UI::NavButton    — bouton avec action callback + tooltip optionnel
//
//  Dépendances externes : imgui.h uniquement
// ═════════════════════════════════════════════════════════════════════════════


// ─────────────────────────────────────────────────────────────────────────────
//  Constantes internes partagées par tous les widgets
//  Cohérentes avec Application::initStyleImGui() et Palette::
// ─────────────────────────────────────────────────────────────────────────────

namespace UI {
namespace Theme {
    constexpr ImVec4 Muted    = { 0.40f, 0.50f, 0.40f, 1.f };
    constexpr ImVec4 Titre    = { 0.88f, 0.92f, 0.88f, 1.f };
    constexpr ImVec4 Accent   = { 0.20f, 0.75f, 0.40f, 1.f };
    constexpr ImVec4 CardBg   = { 0.04f, 0.07f, 0.04f, 0.60f };
    constexpr ImVec4 CardBord = { 0.18f, 0.28f, 0.18f, 1.00f };
    constexpr float  ColLabel = 150.f;   // offset colonne "valeur" dans InfoRow
    constexpr float  TagPadX  =  10.f;   // padding horizontal tag
    constexpr float  TagPadY  =   4.f;   // padding vertical tag
}
} // namespace UI


// ═════════════════════════════════════════════════════════════════════════════
//  1.  UI::TagBadge
// ═════════════════════════════════════════════════════════════════════════════
//
//  Badge texte + fond coloré semi-transparent.
//  Adapte sa largeur au contenu. Ne crée pas de collision d'ID ImGui.
//
//  Problème résolu vs DrawTag() dans StateWiki :
//    L'ancienne version utilisait le texte comme ID de BeginChild().
//    Si deux tags ont le même texte dans la même frame → comportement indéfini.
//    Cette version génère un ID unique via ImGui::PushID / PopID.
//
//  Usage :
//    UI::TagBadge("Facile",  Palette::VertVif);
//    UI::TagBadge("Rustique", Palette::OrangeDoux);
//    UI::TagBadge(plant.nom.c_str(), C::Accent, "Cliquer pour voir la fiche");
//
//  Inline dans la même ligne :
//    UI::TagBadge("A", colorA); ImGui::SameLine(0.f, 4.f);
//    UI::TagBadge("B", colorB);
// ─────────────────────────────────────────────────────────────────────────────

namespace UI {

// Variante simple — affichage pur, pas cliquable
inline void TagBadge(const char* text, const ImVec4& color) {
    const ImVec2 textSize = ImGui::CalcTextSize(text);
    const ImVec2 size     = { textSize.x + Theme::TagPadX * 2.f,
                               textSize.y + Theme::TagPadY * 2.f };

    // Fond semi-transparent dérivé de la couleur du tag
    const ImVec4 bg = { color.x * 0.22f, color.y * 0.22f,
                         color.z * 0.22f, 0.85f };

    // Dessiner le fond via DrawList (pas de BeginChild → pas de collision ID)
    const ImVec2 pos = ImGui::GetCursorScreenPos();
    ImDrawList* dl   = ImGui::GetWindowDrawList();
    dl->AddRectFilled(
        pos,
        { pos.x + size.x, pos.y + size.y },
        ImGui::ColorConvertFloat4ToU32(bg),
        4.f   // coins arrondis
    );
    // Bordure fine de la couleur principale
    dl->AddRect(
        pos,
        { pos.x + size.x, pos.y + size.y },
        ImGui::ColorConvertFloat4ToU32({ color.x, color.y, color.z, 0.45f }),
        4.f, 0, 0.5f
    );

    // Texte centré dans le badge — InvisibleButton pour réserver l'espace
    ImGui::InvisibleButton(text, size);
    // Re-dessiner le texte par-dessus (ImGui::InvisibleButton ne le fait pas)
    dl->AddText(
        { pos.x + Theme::TagPadX, pos.y + Theme::TagPadY },
        ImGui::ColorConvertFloat4ToU32(color),
        text
    );
}

// Variante avec tooltip — affiche une info au survol
inline void TagBadge(const char* text, const ImVec4& color,
                      const char* tooltip) {
    TagBadge(text, color);
    if (tooltip && ImGui::IsItemHovered())
        ImGui::SetTooltip("%s", tooltip);
}

// Variante avec std::string (évite .c_str() à l'appel)
inline void TagBadge(const std::string& text, const ImVec4& color,
                      const char* tooltip = nullptr) {
    TagBadge(text.c_str(), color, tooltip);
}

} // namespace UI


// ═════════════════════════════════════════════════════════════════════════════
//  2.  UI::InfoCard
// ═════════════════════════════════════════════════════════════════════════════
//
//  Bloc "card" avec fond sombre, bordure, titre de section et contenu libre.
//  Remplace le pattern répété 8 fois dans StateWiki :
//    ImGui::PushStyleColor(ImGuiCol_ChildBg, C::CardBg);
//    ImGui::BeginChild("##id", {0, height}, true);
//    ...contenu...
//    ImGui::EndChild();
//    ImGui::PopStyleColor();
//
//  Usage — hauteur fixe :
//    UI::InfoCard("Culture", "##culture", 110.f, [&]() {
//        UI::InfoRow("Pot", plant.volumePotMin.c_str());
//        UI::InfoRow("Sol", plant.phSol.c_str());
//    });
//
//  Usage — hauteur automatique (0.f) :
//    UI::InfoCard("Notes", "##notes", 0.f, [&]() {
//        ImGui::TextWrapped("%s", plant.notes.c_str());
//    });
//
//  Usage — sans titre :
//    UI::InfoCard(nullptr, "##card", 80.f, [&]() { ... });
// ─────────────────────────────────────────────────────────────────────────────

namespace UI {

// Rendu d'une seule ligne "label [colonne] valeur coloré"
// Extraction de DrawInfoRow — désormais dans UI:: et accessible partout.
inline void InfoRow(const char* label, const char* value,
                     const ImVec4& valueColor = Theme::Titre,
                     float colOffset          = Theme::ColLabel) {
    ImGui::TextColored(Theme::Muted, "%s", label);
    ImGui::SameLine(colOffset);
    ImGui::TextColored(valueColor, "%s", value);
}

// Surcharge std::string
inline void InfoRow(const char* label, const std::string& value,
                     const ImVec4& valueColor = Theme::Titre,
                     float colOffset          = Theme::ColLabel) {
    InfoRow(label, value.c_str(), valueColor, colOffset);
}

// La card principale
inline void InfoCard(const char*            title,
                      const char*            id,
                      float                  height,
                      std::function<void()>  content,
                      const ImVec4&          bgColor  = Theme::CardBg,
                      const ImVec4&          rimColor = Theme::CardBord) {
    // Titre de section avec SeparatorText si fourni
    if (title && title[0] != '\0')
        ImGui::SeparatorText(title);

    // Colorer le fond et la bordure de la child window
    ImGui::PushStyleColor(ImGuiCol_ChildBg,    bgColor);
    ImGui::PushStyleColor(ImGuiCol_Border,     rimColor);

    // height = 0 → auto-resize sur le contenu
    const ImVec2 size = { 0.f, height };
    const bool   border = true;
    ImGui::BeginChild(id, size, border);

    ImGui::Spacing();
    content();   // ← tout le contenu de la card est ici
    ImGui::Spacing();

    ImGui::EndChild();
    ImGui::PopStyleColor(2);
}

// Variante compacte sans titre (utile pour les cards dans des grilles)
inline void InfoCardRaw(const char*           id,
                         float                 height,
                         std::function<void()> content,
                         const ImVec4&         bgColor  = Theme::CardBg,
                         const ImVec4&         rimColor = Theme::CardBord) {
    InfoCard(nullptr, id, height, std::move(content), bgColor, rimColor);
}

} // namespace UI


// ═════════════════════════════════════════════════════════════════════════════
//  3.  UI::Gauge
// ═════════════════════════════════════════════════════════════════════════════
//
//  Barre de progression colorée et fine. 3 variantes selon le contexte.
//
//  Problème résolu vs DrawGauge() dans StateWiki :
//    L'ancienne version ignorait le paramètre `label` (void label).
//    Cette version l'affiche optionnellement, et propose une variante
//    segmentée pour les niveaux discrets (besoin en eau 0-5 = 5 segments).
//
//  Usage — barre continue (score balcon, progression) :
//    UI::Gauge(plant.scoreBalcon, 100, C::Accent);           // barre seule
//    UI::Gauge(plant.scoreBalcon, 100, C::Accent, "Score");  // + label avant
//    UI::Gauge(plant.scoreBalcon, 100, C::Accent, "Score", "95/100"); // + valeur après
//
//  Usage — segments discrets (besoin en eau 0-5) :
//    UI::GaugeSegmented(plant.besoinEau, 5, eau.color);
//
//  Usage — inline dans une InfoRow :
//    ImGui::TextColored(Theme::Muted, "Score :");
//    ImGui::SameLine(150.f);
//    UI::Gauge(95, 100, C::Accent);
//    ImGui::SameLine();
//    ImGui::TextColored(C::Secondaire, "95 / 100");
// ─────────────────────────────────────────────────────────────────────────────

namespace UI {

// Barre continue — la plus fréquente
inline void Gauge(float         value,
                   float         maxValue,
                   const ImVec4& color,
                   const char*   labelBefore = nullptr,
                   const char*   labelAfter  = nullptr,
                   ImVec2        size        = { 80.f, 8.f }) {
    if (labelBefore && labelBefore[0] != '\0') {
        ImGui::TextColored(Theme::Muted, "%s", labelBefore);
        ImGui::SameLine();
    }

    const float ratio = (maxValue > 0.f) ? (value / maxValue) : 0.f;

    ImGui::PushStyleColor(ImGuiCol_PlotHistogram, color);
    ImGui::PushStyleColor(ImGuiCol_FrameBg,
        { color.x * 0.15f, color.y * 0.15f, color.z * 0.15f, 1.f });
    // Overlay vide = pas de texte "0%" par défaut de ProgressBar
    ImGui::ProgressBar(ratio, size, "");
    ImGui::PopStyleColor(2);

    if (labelAfter && labelAfter[0] != '\0') {
        ImGui::SameLine();
        ImGui::TextColored(color, "%s", labelAfter);
    }
}

// Surcharge int (besoin en eau, score /100…)
inline void Gauge(int           value,
                   int           maxValue,
                   const ImVec4& color,
                   const char*   labelBefore = nullptr,
                   const char*   labelAfter  = nullptr,
                   ImVec2        size        = { 80.f, 8.f }) {
    Gauge(static_cast<float>(value),
          static_cast<float>(maxValue),
          color, labelBefore, labelAfter, size);
}

// Barre segmentée — pour les niveaux discrets (besoin eau 0-5, difficulté 1-3)
// Dessine N petits rectangles séparés par un gap de 2px.
inline void GaugeSegmented(int           value,
                             int           maxValue,
                             const ImVec4& colorOn,
                             const ImVec4& colorOff  = { 0.15f, 0.20f, 0.15f, 1.f },
                             float         segWidth  = 14.f,
                             float         segHeight =  8.f,
                             float         gap       =  2.f) {
    ImDrawList* dl  = ImGui::GetWindowDrawList();
    ImVec2      pos = ImGui::GetCursorScreenPos();

    const float totalW = maxValue * segWidth + (maxValue - 1) * gap;

    for (int i = 0; i < maxValue; ++i) {
        const float x0 = pos.x + i * (segWidth + gap);
        const ImVec4& c = (i < value) ? colorOn : colorOff;
        dl->AddRectFilled(
            { x0, pos.y },
            { x0 + segWidth, pos.y + segHeight },
            ImGui::ColorConvertFloat4ToU32(c),
            2.f
        );
    }
    // Réserver l'espace dans le layout
    ImGui::Dummy({ totalW, segHeight });
}

} // namespace UI


// ═════════════════════════════════════════════════════════════════════════════
//  4.  UI::NavButton
// ═════════════════════════════════════════════════════════════════════════════
//
//  Bouton de navigation — exécute un callback à la place de modifier
//  des variables directement dans la fonction de rendu.
//
//  Pourquoi un callback et pas juste un bool retourné ?
//  → Un bool force le if() à l'appel, et donc la logique de navigation
//    reste dans la fonction de rendu. Le callback sépare "détecter le clic"
//    de "que faire quand on clique".
//
//  Deux variantes :
//    - NavButton  : bouton texte discret (style SmallButton)
//    - NavButtonPrimary : bouton plein vert (action principale)
//
//  Usage — navigation vers un sol :
//    UI::NavButton("Terreau méditerranéen", [&]() {
//        m_selectedSoil = sol;
//        m_activeTab    = 1;
//    });
//
//  Usage — avec tooltip :
//    UI::NavButton(solMeta.label.c_str(), [&]() {
//        m_selectedSoil = sol;
//        m_activeTab    = 1;
//    }, "pH 7.0-8.5 · Très drainant");
//
//  Usage — bouton principal (retour menu, valider) :
//    UI::NavButtonPrimary("Retour au menu", [&]() {
//        m_app->getStateMachine().addState(...);
//    });
//
//  Usage — désactivé (plante sans bouture) :
//    UI::NavButton("Aucune bouture", nullptr, "Cette plante ne se bouture pas",
//                  /*enabled=*/false);
// ─────────────────────────────────────────────────────────────────────────────

namespace UI {

// Variante discrète — style SmallButton (remplace les ImGui::SmallButton inline)
inline void NavButton(const char*           label,
                       std::function<void()> onClic,
                       const char*           tooltip = nullptr,
                       bool                  enabled = true) {
    if (!enabled) ImGui::BeginDisabled();

    // Générer un ID stable même si le label change entre frames
    ImGui::PushID(label);

    if (ImGui::SmallButton(label) && onClic)
        onClic();

    ImGui::PopID();

    if (!enabled) ImGui::EndDisabled();

    if (tooltip && ImGui::IsItemHovered(ImGuiHoveredFlags_AllowWhenDisabled))
        ImGui::SetTooltip("%s", tooltip);
}

// Surcharge std::string
inline void NavButton(const std::string&    label,
                       std::function<void()> onClic,
                       const char*           tooltip = nullptr,
                       bool                  enabled = true) {
    NavButton(label.c_str(), std::move(onClic), tooltip, enabled);
}

// Variante primaire — bouton plein, couleur accent (action principale)
inline void NavButtonPrimary(const char*           label,
                               std::function<void()> onClic,
                               const char*           tooltip = nullptr,
                               bool                  enabled = true) {
    if (!enabled) ImGui::BeginDisabled();

    ImGui::PushID(label);
    ImGui::PushStyleColor(ImGuiCol_Button,
        { 0.10f, 0.35f, 0.18f, 1.f });   // AccentSombre
    ImGui::PushStyleColor(ImGuiCol_ButtonHovered,
        { 0.15f, 0.55f, 0.30f, 1.f });   // AccentDoux
    ImGui::PushStyleColor(ImGuiCol_ButtonActive,
        { 0.20f, 0.75f, 0.40f, 1.f });   // Accent

    if (ImGui::Button(label) && onClic)
        onClic();

    ImGui::PopStyleColor(3);
    ImGui::PopID();

    if (!enabled) ImGui::EndDisabled();

    if (tooltip && ImGui::IsItemHovered(ImGuiHoveredFlags_AllowWhenDisabled))
        ImGui::SetTooltip("%s", tooltip);
}

// Variante icône + texte — pour les boutons de navigation avec préfixe visuel
// Ex : NavButtonIcon("→ Voir le sol", "TERREAU_MEDITERRANEEN", callback)
inline void NavButtonIcon(const char*           icon,
                            const char*           label,
                            std::function<void()> onClic,
                            const char*           tooltip = nullptr,
                            bool                  enabled = true) {
    // Construire "→ Terreau méditerranéen##label" pour un ID unique
    std::string full = std::string(icon) + " " + label;
    full += "##";
    full += label;
    NavButton(full.c_str(), std::move(onClic), tooltip, enabled);
}

} // namespace UI


// ═════════════════════════════════════════════════════════════════════════════
//  HELPERS SUPPLÉMENTAIRES
//  Petits patterns très fréquents qui ne méritent pas un widget complet.
// ═════════════════════════════════════════════════════════════════════════════

namespace UI {

// Séparateur titré — wrapper de SeparatorText, ajuste l'espacement
inline void Section(const char* title) {
    ImGui::Spacing();
    ImGui::SeparatorText(title);
}

// Ligne de texte muted + texte accent sur la même ligne
// Ex : "Sol recommandé : [Terreau méditerranéen]"
inline void LabelAccent(const char* label, const char* value,
                          const ImVec4& valueColor = Theme::Accent) {
    ImGui::TextColored(Theme::Muted, "%s", label);
    ImGui::SameLine(0.f, 4.f);
    ImGui::TextColored(valueColor, "%s", value);
}

// Wrapper texte enveloppé dans la largeur disponible
inline void BodyText(const char* text,
                      const ImVec4& color = Theme::Titre) {
    ImGui::PushTextWrapPos(0.f);
    ImGui::TextColored(color, "%s", text);
    ImGui::PopTextWrapPos();
}

// Ligne vide calibrée — plus lisible que ImGui::Spacing() x3
inline void Gap(float pixels = 8.f) {
    ImGui::Dummy({ 0.f, pixels });
}

} // namespace UI