#pragma once
#include <vector>
#include <string>
#include <algorithm>
#include "Data/Plante.hpp"
#include "Data/Soil.hpp"
#include "Data/PotBouture.hpp"

// ─────────────────────────────────────────────────────────────────────────────
//  DatabaseManager
//
//  Point d'accès unique aux données chargées.
//  Charge une seule fois au démarrage (Application::Application()).
//  L'UI ne touche jamais aux fichiers JSON directement.
//
//  Usage dans un State :
//    const auto& plantes = m_app->getDatabase().getPlantes();
//    const Plant* p      = m_app->getDatabase().findPlante("Thym");
//    const Soil*  s      = m_app->getDatabase().findSol("TERREAU_MEDITERRANEEN");
// ─────────────────────────────────────────────────────────────────────────────

class DatabaseManager {
public:
    // ── Chargement (appelé une seule fois dans Application()) ─────────────

    // Charge les 4 fichiers JSON. Retourne true si tout s'est bien passé.
    bool chargerTout(
        const std::string& cheminPlantes  = "assets/JSON/encyclopedia.json",
        const std::string& cheminSols     = "assets/JSON/sols.json",
        const std::string& cheminPots     = "assets/JSON/pots.json",
        const std::string& cheminBoutures = "assets/JSON/boutures.json"
    );

    // ── Accès en lecture (const) ──────────────────────────────────────────

    const std::vector<Plant>&   getPlantes()  const { return m_plantes; }
    const std::vector<Soil>&    getSols()     const { return m_sols; }
    const std::vector<Pot>&     getPots()     const { return m_pots; }
    const std::vector<Bouture>& getBoutures() const { return m_boutures; }

    // ── Recherche par clé ─────────────────────────────────────────────────

    // Retourne nullptr si non trouvé.
    const Plant*   findPlante (const std::string& nom)         const;
    const Soil*    findSol    (const std::string& typeSol)     const;
    const Pot*     findPot    (const std::string& typeRacin)   const;
    const Bouture* findBouture(const std::string& nom)         const;

    // ── Listes pré-triées (calculées une seule fois au chargement) ────────

    // Plantes triées par nom alphabétique.
    const std::vector<const Plant*>& getPlantesTrieesParNom() const {
        return m_plantesTrieesParNom;
    }

    // Plantes triées par score balcon décroissant.
    const std::vector<const Plant*>& getPlantesTrieesParScore() const {
        return m_plantesTrieesParScore;
    }

    // ── Filtres (recalculés uniquement si le filtre change) ───────────────

    // Retourne les plantes dont le nom contient la chaîne (insensible à la casse).
    std::vector<const Plant*> filtrerParNom(const std::string& recherche) const;

    // Retourne les plantes compatibles avec un sol donné.
    std::vector<const Plant*> filtrerParSol(const std::string& typeSol) const;

    bool estCharge() const { return m_charge; }

private:
    // ── Données brutes ────────────────────────────────────────────────────
    std::vector<Plant>   m_plantes;
    std::vector<Soil>    m_sols;
    std::vector<Pot>     m_pots;
    std::vector<Bouture> m_boutures;

    // ── Cache des listes triées (calculé une seule fois) ──────────────────
    std::vector<const Plant*> m_plantesTrieesParNom;
    std::vector<const Plant*> m_plantesTrieesParScore;

    bool m_charge = false;

    void construireCache();   // appelé en fin de chargerTout()
};