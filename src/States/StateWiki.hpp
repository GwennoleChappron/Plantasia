#pragma once
#include "Core/State.hpp"
#include "Data/Plante.hpp"
#include "Data/Soil.hpp"
#include "Data/RacineBouture.hpp"
#include <string>

// ─────────────────────────────────────────────────────────────────────────────
//  StateWiki — v2 avec animations
// ─────────────────────────────────────────────────────────────────────────────

class StateWiki : public State {
public:
    explicit StateWiki(Application* app);

    void onEnter()  override;
    void onExit()   override;
    void update(float dt) override;
    void draw(sf::RenderWindow& window) override;
    void drawImGui() override;

private:
    // ── Navigation ────────────────────────────────────────────────────────────
    const Plant*   m_selectedPlant   = nullptr;
    const Soil*    m_selectedSoil    = nullptr;
    const Bouture* m_selectedBouture = nullptr;
    char           m_searchBuffer[128] = "";
    int            m_activeTab        = 0;

    // ── État animation ────────────────────────────────────────────────────────
    //
    //  Toutes les animations sont pilotées par des floats dans [0, 1] ou [0, cible].
    //  Mise à jour dans update() via dt pour être indépendantes du framerate.

    // Fade panneau droit au changement de plante (0=transparent → 1=opaque)
    float        m_fadeAlpha   = 1.f;

    // Score balcon affiché (float pour lerp smooth vers scoreBalcon cible)
    float        m_animScore   = 0.f;

    // Gouttes d'eau : valeur float animée vers besoinEau (ex: 0.f → 3.f)
    float        m_dropAnim    = 0.f;

    // Donut sol : angle de sweep animé (0 → 2π en ~0.5s à l'entrée de l'onglet)
    float        m_donutSweep  = 0.f;   // fraction [0, 1] de l'angle total

    // Étapes bouture : combien d'étapes visibles (animé 0 → N, une par 0.25s)
    float        m_etapeReveal = 0.f;

    // Détection de changement de sélection (pour déclencher les animations)
    const Plant* m_prevPlant   = nullptr;
    int          m_prevTab     = 0;

    float m_panelTime = 0.0f; // Chronomètre pour animer la liste de gauche en cascade
    float m_vineProgress = 0.f; // 0-1 pour faire pousser une liane décorative à l'entrée du State

    // ── Helpers internes ──────────────────────────────────────────────────────
    void DrawBreadcrumb()    const;
    void DrawLeftPanel();
    void DrawRightPanel();

    void DrawPlantDetails   (const Plant&   plant);
    void DrawPlantHeader    (const Plant&   plant);
    void DrawPlantCultureCard(const Plant&  plant) const;
    void DrawPlantWaterCard  (const Plant&  plant);        // non-const : m_dropAnim
    void DrawPlantCalendar   (const Plant&  plant) const;
    void DrawPlantRelations  (const Plant&  plant);

    void DrawSoilDetails    (const Soil&    soil);
    void DrawBoutureDetails (const Bouture& bouture);
};