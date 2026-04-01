#pragma once
#include <imgui.h>

// ═════════════════════════════════════════════════════════════════════════════
//  ColorTheme.hpp — Source unique de vérité pour toutes les couleurs de Plantasia
//
//  RÈGLE : Plus jamais ImVec4(0.75f, 0.22f, 0.22f, ...) dans un .cpp
//          On écrit Theme::DangerRed.
//
//  Ce fichier remplace et consolide :
//    ▸ namespace Couleurs::   dans Application.cpp
//    ▸ namespace C::          dans StateWiki.cpp
//    ▸ namespace UI::Theme    dans Widgets.hpp
//    ▸ namespace Palette::    dans EnumInfo.hpp
//
//  Après intégration, tous ces namespaces deviennent des alias :
//    using namespace Couleurs = Theme;   // ou supprimer et remplacer
//
//  Usage :
//    ImGui::TextColored(Theme::PlantGreen,   "Thym");
//    ImGui::TextColored(Theme::TextMuted,    "Lamiaceae");
//    ImGui::TextColored(Theme::WarningOrange,"Attention au gel");
//    ImGui::TextColored(Theme::DangerRed,    "Toxique");
//
//  Couleurs dérivées (jamais hardcoder les variantes alpha) :
//    Theme::withAlpha(Theme::PlantGreen, 0.4f)   → vert semi-transparent
//    Theme::darken(Theme::PlantGreen, 0.25f)     → fond de badge vert
// ═════════════════════════════════════════════════════════════════════════════

namespace Theme {

// ─────────────────────────────────────────────────────────────────────────────
//  GROUPE 1 — Fonds & Surfaces
//
//  Hiérarchie : Fond (le plus sombre) → Surface → SurfaceHaute (le plus clair)
//  CardBg est semi-transparent pour laisser passer le fond de fenêtre.
// ─────────────────────────────────────────────────────────────────────────────

//  Fond de fenêtre principal — quasi-noir verdâtre
constexpr ImVec4 Fond          = { 0.04f, 0.06f, 0.04f, 1.00f };

//  Fond d'une surface normale (panels, sidebars)
constexpr ImVec4 Surface       = { 0.07f, 0.10f, 0.07f, 0.90f };

//  Fond d'une surface surélevée (hover, frame actif)
constexpr ImVec4 SurfaceHaute  = { 0.11f, 0.15f, 0.11f, 0.95f };

//  Fond d'une card (BeginChild avec bordure) — semi-transparent intentionnel
//  L'alpha à 0.60 laisse apparaître le BackgroundEngine derrière.
constexpr ImVec4 CardBg        = { 0.04f, 0.07f, 0.04f, 0.60f };

//  Fond d'un segment de jauge inactif (GaugeSegmented)
constexpr ImVec4 GaugeInactif  = { 0.15f, 0.20f, 0.15f, 1.00f };


// ─────────────────────────────────────────────────────────────────────────────
//  GROUPE 2 — Vert Plantasia (identité visuelle)
//
//  Toujours utiliser ces 4 valeurs, dans cet ordre de luminosité.
//  Ne jamais créer un "vert intermédiaire" ad hoc.
//
//  PlantGreen  → accent principal, textes importants, boutons actifs
//  PlantMid    → bouton hover, liens secondaires
//  PlantDark   → bouton normal, header sélectionné, fond accent
//  PlantSombre → très discret, utilisé dans les fonds de sections actives
//  PlantRecolte → couleur spécifique au calendrier récolte (vert forêt)
// ─────────────────────────────────────────────────────────────────────────────

//  ▶ Vert vif — accent principal, titres de plantes, bouton actif
constexpr ImVec4 PlantGreen    = { 0.20f, 0.75f, 0.40f, 1.00f };

//  Vert moyen — hover, liens, tags positifs
constexpr ImVec4 PlantMid      = { 0.15f, 0.55f, 0.30f, 1.00f };

//  Vert sombre — fond de bouton normal, header sélectionné dans liste
constexpr ImVec4 PlantDark     = { 0.10f, 0.35f, 0.18f, 1.00f };

//  Vert forêt — couleur récolte dans le calendrier
constexpr ImVec4 PlantRecolte  = { 0.12f, 0.45f, 0.22f, 1.00f };


// ─────────────────────────────────────────────────────────────────────────────
//  GROUPE 3 — Textes
//
//  4 niveaux de lisibilité, du plus fort au plus discret.
//  Règle : ne jamais utiliser une couleur de fond comme couleur de texte.
// ─────────────────────────────────────────────────────────────────────────────

//  Texte principal — corps, valeurs, contenus
constexpr ImVec4 TextPrimary   = { 0.88f, 0.92f, 0.88f, 1.00f };

//  Texte secondaire — métadonnées, noms scientifiques, sous-titres
constexpr ImVec4 TextSecondary = { 0.55f, 0.68f, 0.55f, 1.00f };

//  Texte muted — labels de champs (avant les valeurs dans InfoRow)
constexpr ImVec4 TextMuted     = { 0.40f, 0.50f, 0.40f, 1.00f };

//  Texte désactivé — éléments non interactifs, placeholders
constexpr ImVec4 TextDisabled  = { 0.30f, 0.40f, 0.30f, 1.00f };


// ─────────────────────────────────────────────────────────────────────────────
//  GROUPE 4 — Bordures
// ─────────────────────────────────────────────────────────────────────────────

//  Bordure standard — cards, séparateurs, frames
constexpr ImVec4 BorderDefault = { 0.18f, 0.28f, 0.18f, 1.00f };

//  Bordure hover — frame survolé, input focus
constexpr ImVec4 BorderHover   = { 0.28f, 0.42f, 0.28f, 1.00f };


// ─────────────────────────────────────────────────────────────────────────────
//  GROUPE 5 — Couleurs sémantiques
//
//  Chaque couleur a un sens précis. Ne pas les interchanger.
//
//    DangerRed    → toxicité, maladies graves, gel fatal, asphyxie racinaire
//    WarningOrange → attention, astuce pro, hormone recommandée, lessivage
//    InfoBlue     → eau, arrosage, humidité, information neutre
//    Amber        → alerte douce, saison été, température élevée
//    Violet       → acidité, pH bas, terre de bruyère, correction pH baisser
// ─────────────────────────────────────────────────────────────────────────────

//  ▶ Rouge danger — toxique, maladie, incompatibilité critique
constexpr ImVec4 DangerRed     = { 0.75f, 0.22f, 0.22f, 1.00f };

//  Rouge danger survolé (boutons destructeurs)
constexpr ImVec4 DangerHover   = { 0.90f, 0.30f, 0.30f, 1.00f };

//  ▶ Orange warning — attention, astuce, recommandation forte
constexpr ImVec4 WarningOrange = { 0.85f, 0.52f, 0.12f, 1.00f };

//  ▶ Bleu info — eau, humidité, arrosage, information générale
constexpr ImVec4 InfoBlue      = { 0.30f, 0.60f, 0.90f, 1.00f };

//  ▶ Ambre — été, chaleur, saison chaude, niveau modéré
constexpr ImVec4 Amber         = { 0.88f, 0.80f, 0.20f, 1.00f };

//  ▶ Violet — acidité, pH acide, terre de bruyère, correction baisser
constexpr ImVec4 Violet        = { 0.58f, 0.35f, 0.75f, 1.00f };


// ─────────────────────────────────────────────────────────────────────────────
//  GROUPE 6 — Domaine botanique spécifique
//
//  Couleurs avec une signification propre à Plantasia.
//  À utiliser uniquement dans le contexte correspondant.
// ─────────────────────────────────────────────────────────────────────────────

//  Calendrier : mois de floraison
constexpr ImVec4 Floraison           = { 0.75f, 0.25f, 0.45f, 1.00f };

//  Calendrier : mois où floraison ET récolte se chevauchent
constexpr ImVec4 FloraisonEtRecolte  = { 0.45f, 0.20f, 0.50f, 1.00f };

//  pH acide (< 6.0) — utilisé dans DrawSoilDetails et EnumInfo
constexpr ImVec4 PhAcide             = { 0.70f, 0.40f, 0.80f, 1.00f };

//  pH basique (> 7.5) — sols calcaires, méditerranéens
constexpr ImVec4 PhBasique           = { 0.80f, 0.70f, 0.30f, 1.00f };


// ─────────────────────────────────────────────────────────────────────────────
//  HELPERS — couleurs dérivées à la volée
//
//  Ces fonctions évitent de hardcoder les variantes semi-transparentes.
//  Toujours utiliser ces fonctions plutôt que de créer une constante alpha.
//
//  Exemples courants :
//    Theme::withAlpha(Theme::PlantGreen, 0.15f)  → fond de badge vert
//    Theme::darken(Theme::DangerRed, 0.25f)      → fond de tag danger
//    Theme::withAlpha(Theme::CardBg, 0.8f)       → card plus opaque
// ─────────────────────────────────────────────────────────────────────────────

//  Modifier l'alpha d'une couleur existante
[[nodiscard]] constexpr ImVec4 withAlpha(ImVec4 c, float a) noexcept {
    return { c.x, c.y, c.z, a };
}

//  Assombrir une couleur d'un facteur t ∈ [0, 1] (t=0 → noir, t=1 → inchangé)
[[nodiscard]] constexpr ImVec4 darken(ImVec4 c, float t) noexcept {
    return { c.x * t, c.y * t, c.z * t, c.w };
}

//  Fond semi-transparent pour un badge/tag d'une couleur donnée
//  Reproduce le comportement de DrawTag() : fond = couleur * 0.22, alpha = 0.85
[[nodiscard]] constexpr ImVec4 tagBackground(ImVec4 c) noexcept {
    return { c.x * 0.22f, c.y * 0.22f, c.z * 0.22f, 0.85f };
}

//  Fond de jauge inactif dérivé de la couleur active
[[nodiscard]] constexpr ImVec4 gaugeBackground(ImVec4 c) noexcept {
    return { c.x * 0.15f, c.y * 0.15f, c.z * 0.15f, 1.00f };
}

//  Interpolation linéaire entre deux couleurs (t ∈ [0, 1])
//  Utile pour les couleurs de mois dans le calendrier
[[nodiscard]] constexpr ImVec4 lerp(ImVec4 a, ImVec4 b, float t) noexcept {
    return {
        a.x + (b.x - a.x) * t,
        a.y + (b.y - a.y) * t,
        a.z + (b.z - a.z) * t,
        a.w + (b.w - a.w) * t
    };
}
} // namespace Theme


// ─────────────────────────────────────────────────────────────────────────────
//  APPLICATION DES COULEURS IMGUI
//
//  Fonction à appeler UNE SEULE FOIS dans Application::initStyleImGui().
//  Remplace tous les ImVec4 literals dans cette fonction.
//
//  Usage :
//    void Application::initStyleImGui() {
//        ImGuiStyle& s = ImGui::GetStyle();
//        // ... roundings, paddings...
//        Theme::applyToImGui(s);
//    }
// ─────────────────────────────────────────────────────────────────────────────

namespace Theme {

inline void applyToImGui(ImGuiStyle& s) {
    ImVec4* c = s.Colors;

    // Fonds
    c[ImGuiCol_WindowBg]            = Surface;
    c[ImGuiCol_ChildBg]             = withAlpha(Fond, 0.70f);
    c[ImGuiCol_PopupBg]             = Surface;

    // Bordures
    c[ImGuiCol_Border]              = BorderDefault;
    c[ImGuiCol_BorderShadow]        = withAlpha(Fond, 0.00f);

    // Frames (InputText, Combo, etc.)
    c[ImGuiCol_FrameBg]             = SurfaceHaute;
    c[ImGuiCol_FrameBgHovered]      = lerp(SurfaceHaute, PlantDark, 0.35f);
    c[ImGuiCol_FrameBgActive]       = BorderDefault;

    // Titre de fenêtre
    c[ImGuiCol_TitleBg]             = Fond;
    c[ImGuiCol_TitleBgActive]       = PlantDark;
    c[ImGuiCol_TitleBgCollapsed]    = withAlpha(Fond, 0.75f);

    // Scrollbar
    c[ImGuiCol_ScrollbarBg]         = withAlpha(Fond, 0.60f);
    c[ImGuiCol_ScrollbarGrab]       = PlantDark;
    c[ImGuiCol_ScrollbarGrabHovered]= PlantMid;
    c[ImGuiCol_ScrollbarGrabActive] = PlantGreen;

    // Checkmark / Slider / Knob
    c[ImGuiCol_CheckMark]           = PlantGreen;
    c[ImGuiCol_SliderGrab]          = PlantMid;
    c[ImGuiCol_SliderGrabActive]    = PlantGreen;

    // Boutons
    c[ImGuiCol_Button]              = PlantDark;
    c[ImGuiCol_ButtonHovered]       = PlantMid;
    c[ImGuiCol_ButtonActive]        = PlantGreen;

    // Header (Selectable, CollapsingHeader)
    c[ImGuiCol_Header]              = withAlpha(PlantDark, 0.80f);
    c[ImGuiCol_HeaderHovered]       = PlantMid;
    c[ImGuiCol_HeaderActive]        = PlantGreen;

    // Séparateur
    c[ImGuiCol_Separator]           = BorderDefault;
    c[ImGuiCol_SeparatorHovered]    = PlantMid;
    c[ImGuiCol_SeparatorActive]     = PlantGreen;

    // Resize grip
    c[ImGuiCol_ResizeGrip]          = withAlpha(PlantDark, 0.25f);
    c[ImGuiCol_ResizeGripHovered]   = withAlpha(PlantMid,  0.67f);
    c[ImGuiCol_ResizeGripActive]    = PlantGreen;

    // Onglets
    c[ImGuiCol_Tab]                 = SurfaceHaute;
    c[ImGuiCol_TabHovered]          = PlantMid;
    c[ImGuiCol_TabActive]           = PlantDark;
    c[ImGuiCol_TabUnfocused]        = Surface;
    c[ImGuiCol_TabUnfocusedActive]  = SurfaceHaute;

    // Textes
    c[ImGuiCol_Text]                = TextPrimary;
    c[ImGuiCol_TextDisabled]        = TextDisabled;
    c[ImGuiCol_TextSelectedBg]      = withAlpha(PlantMid, 0.35f);

    // Jauge (ProgressBar, PlotHistogram)
    c[ImGuiCol_PlotLines]           = PlantMid;
    c[ImGuiCol_PlotLinesHovered]    = PlantGreen;
    c[ImGuiCol_PlotHistogram]       = PlantGreen;
    c[ImGuiCol_PlotHistogramHovered]= PlantMid;

    // Drag/Drop
    c[ImGuiCol_DragDropTarget]      = withAlpha(Amber, 0.90f);

    // Navigation (gamepad/keyboard)
    c[ImGuiCol_NavHighlight]        = PlantGreen;
    c[ImGuiCol_NavWindowingHighlight]= withAlpha(TextPrimary, 0.70f);
    c[ImGuiCol_NavWindowingDimBg]   = withAlpha(TextSecondary, 0.20f);

    // Modal
    c[ImGuiCol_ModalWindowDimBg]    = withAlpha(Fond, 0.70f);
}

} // namespace Theme