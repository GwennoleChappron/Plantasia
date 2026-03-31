#pragma once
#include <string>
#include <vector>

// ─────────────────────────────────────────────────────────────────────────────
//  ENUMS  (correspondent exactement aux valeurs dans encyclopedia.json)
// ─────────────────────────────────────────────────────────────────────────────

enum class ExpositionSoleil {
    PLEIN_SOLEIL,
    MI_OMBRE,
    INCONNU
};

enum class ExpositionVent {
    MODERE,
    ABRITE,
    INCONNU
};

enum class Rusticite {
    RUSTIQUE,
    SEMI_RUSTIQUE,
    FRAGILE,
    INCONNU
};

enum class NiveauDifficulte {
    FACILE,
    MOYEN,
    DIFFICILE,
    INCONNU
};

enum class TypeRacinaireEnum {
    FASCICULE,
    PIVOTANT,
    TRACANT,
    LIGNEUX,
    INCONNU
};

// ─────────────────────────────────────────────────────────────────────────────
//  STRUCT PLANTE
//  Contient uniquement des données. Aucune dépendance à ImGui ou SFML.
// ─────────────────────────────────────────────────────────────────────────────

struct Plant {
    // --- Identité ---
    std::string nom;
    std::string nomScientifique;
    std::vector<std::string> autresNoms;
    int         type               = 0;
    std::string famille;
    std::string origine;

    // --- Culture ---
    Rusticite       rusticite      = Rusticite::INCONNU;
    std::string     zoneClimat;
    ExpositionSoleil exposition    = ExpositionSoleil::INCONNU;
    ExpositionVent  expositionVent = ExpositionVent::INCONNU;
    std::string     volumePotMin;
    std::string     dimensionsAdulte;
    NiveauDifficulte difficulte    = NiveauDifficulte::INCONNU;

    // --- Arrosage ---
    int         besoinEau          = 0;   // 0-5
    std::string frequenceEte;
    std::string frequenceHiver;

    // --- Calendrier (mois 1-12, 0 = inconnu) ---
    int floraisonDebut = 0;
    int floraisonFin   = 0;
    int recolteDebut   = 0;
    int recolteFin     = 0;

    // --- Conseils texte ---
    std::string feuillage;
    std::string conseilTerre;
    std::string conseilArrosage;
    std::string rempotage;
    std::string conseilEntretien;
    std::string maladies;
    std::string compagnonnage;
    std::string vertus;
    std::string precautions;
    std::string notes;
    std::string astucePro;
    std::string phSol;
    std::string multiplication;

    // --- Caractéristiques ---
    std::string toleranceSecheresse;
    std::string sensibiliteExcesEau;
    std::string vitesseCroissance;
    std::string typeRacinaireTexte;

    // --- Toxicité ---
    bool toxiciteAnimaux = false;

    // --- Score balcon ---
    int scoreBalcon = 0;

    // --- Relations (clés JSON) ---
    std::string            solRecommande;
    std::string            solAlternatif;
    TypeRacinaireEnum      typeRacinairePot = TypeRacinaireEnum::INCONNU;
    std::vector<std::string> bouturesCompatibles;
};