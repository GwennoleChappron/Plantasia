#pragma once
#include "Core/State.hpp"
#include "Data/Plant.hpp"
#include "Data/Soil.hpp"
#include "Data/PotBouture.hpp"
#include <string>

// ─────────────────────────────────────────────────────────────────────────────
//  StateWiki
//
//  Règle d'or : drawImGui() ne contient QUE des appels à des sous-fonctions.
//  Chaque sous-fonction fait UNE chose et tient en < 150 lignes.
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
    // ── État de navigation ────────────────────────────────────────────────
    const Plant*   m_selectedPlant   = nullptr;
    const Soil*    m_selectedSoil    = nullptr;
    const Bouture* m_selectedBouture = nullptr;

    // Filtre de recherche dans la liste des plantes
    char m_searchBuffer[128] = "";

    // Onglet actif dans le panneau de détail (0=Plante 1=Sol 2=Bouture)
    int m_activeTab = 0;

    // ── Sous-fonctions de rendu ───────────────────────────────────────────
    //  Chacune reçoit uniquement ce dont elle a besoin — pas de this implicite
    //  sur tout le state.

    // Barre de navigation en haut (breadcrumb + titre de section)
    void DrawBreadcrumb() const;

    // Panneau gauche : liste des plantes + champ de recherche
    void DrawLeftPanel();

    // Panneau droit : contenu selon l'onglet actif
    void DrawRightPanel();

    // -- Onglets du panneau droit --
    void DrawPlantDetails  (const Plant&   plant);
    void DrawSoilDetails   (const Soil&    soil);
    void DrawBoutureDetails(const Bouture& bouture);

    // -- Sections internes de DrawPlantDetails --
    void DrawPlantHeader    (const Plant& plant) const;
    void DrawPlantCultureCard(const Plant& plant) const;
    void DrawPlantWaterCard  (const Plant& plant) const;
    void DrawPlantCalendar   (const Plant& plant) const;
    void DrawPlantRelations  (const Plant& plant);   // non-const : peut changer m_selectedSoil

};