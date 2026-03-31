#pragma once
#include "EnumInfo.hpp"
#include <string>
#include <vector>

// ─────────────────────────────────────────────────────────────────────────────
//  STRUCT POT
//  Correspond à pots.json — une entrée par type racinaire.
// ─────────────────────────────────────────────────────────────────────────────

struct Racine {
    TypeRacinaireEnum     typeRacinaire;
    profondeurPot           profondeur;
    LargeurPot            largeur;               
    int                   volumeMin = 0;
    int                   volumeMax = 0;
    FormePot              formePot;
    std::vector<TypePot> materiaux;
    std::vector<std::string> plantesExemples;    // Reste string (noms propres)
    BesoinsDrainage     drainage;
    FrequenceRempotage    frequenceRempotage;
    SensibiliteRempotage  sensibiliteRempotage;
};

// ─────────────────────────────────────────────────────────────────────────────
//  STRUCT BOUTURE
//  Correspond à boutures.json.
// ─────────────────────────────────────────────────────────────────────────────

struct Bouture {
    std::string nom;               // clé — ex: "BOUTURE_HERBACEE"
    NiveauDifficulte niveauDifficulte;
    std::vector<Saison> periode;
    TypeTige typeTige;
    std::vector<std::string> plantesConcernees;

    int longueurMin = 0;           // cm (-1 si null)
    int longueurMax = 0;

    SubstratBouture substrat;
    HormoneBouturage hormoneBouturage;
    HumiditeBouture humidite;

    int temperatureMin = 0;
    int temperatureMax = 0;

    int enracinementMin = 0;       // jours
    int enracinementMax = 0;

    std::vector<EtapeBouture> etapes;
    TauxReussite tauxReussite;
};