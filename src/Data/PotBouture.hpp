#pragma once
#include "EnumInfo.hpp"
#include <string>
#include <vector>

// ─────────────────────────────────────────────────────────────────────────────
//  STRUCT POT
//  Correspond à pots.json — une entrée par type racinaire.
// ─────────────────────────────────────────────────────────────────────────────

struct Pot {
    TypeRacinaireEnum     typeRacinaire;
    ProfondeurPot         profondeur;
    LargeurPot            largeur;               
    int                   volumeMin = 0;
    int                   volumeMax = 0;
    FormePot              formePot;
    std::vector<MateriauPot> materiaux;
    std::vector<std::string> plantesExemples;    // Reste string (noms propres)
    BesoinDrainagePot     drainage;
    FrequenceRempotage    frequenceRempotage;
    SensibiliteRempotage  sensibiliteRempotage;
};

// ─────────────────────────────────────────────────────────────────────────────
//  STRUCT BOUTURE
//  Correspond à boutures.json.
// ─────────────────────────────────────────────────────────────────────────────

struct Bouture {
    std::string nom;               // clé — ex: "BOUTURE_HERBACEE"
    std::string niveauDifficulte;
    std::vector<EnumInfo::Saison> periode;
    std::string typeTige;
    std::vector<std::string> plantesConcernees;

    int longueurMin = 0;           // cm (-1 si null)
    int longueurMax = 0;

    std::string substrat;
    std::string hormoneBouturage;
    std::string humidite;

    int temperatureMin = 0;
    int temperatureMax = 0;

    int enracinementMin = 0;       // jours
    int enracinementMax = 0;

    std::vector<std::string> etapes;
    std::string tauxReussite;
};