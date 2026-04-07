#pragma once
#include "EnumInfo.hpp"
#include <string>
#include <vector>

// ─────────────────────────────────────────────────────────────────────────────
//  STRUCT RACINE
//  Correspond à pots.json — une entrée par type racinaire.
// ─────────────────────────────────────────────────────────────────────────────

struct Racine {
    TypeRacinaireEnum        typeRacinaire        = TypeRacinaireEnum::INCONNU;
    profondeurPot            profondeur           = profondeurPot::INCONNU;
    LargeurPot               largeur              = LargeurPot::INCONNU;               
    int                      volumeMin            = 0;
    int                      volumeMax            = 0;
    FormePot                 formePot             = FormePot::INCONNU;
    std::vector<TypePot>     materiaux;
    std::vector<std::string> plantesExemples;    // Reste string (noms propres)
    BesoinsDrainage          drainage             = BesoinsDrainage::INCONNU;
    FrequenceRempotage       frequenceRempotage   = FrequenceRempotage::INCONNU;
    SensibiliteRempotage     sensibiliteRempotage = SensibiliteRempotage::INCONNU;
};

// ─────────────────────────────────────────────────────────────────────────────
//  STRUCT BOUTURE
//  Correspond à boutures.json.
// ─────────────────────────────────────────────────────────────────────────────

struct Bouture {
    TypeBouture               nom;               // clé — ex: "BOUTURE_HERBACEE"
    NiveauDifficulte          niveauDifficulte = NiveauDifficulte::INCONNU;
    std::vector<Saison>       periode;
    TypeTige                  typeTige         = TypeTige::INCONNU;
    std::vector<std::string>  plantesConcernees;

    int                       longueurMin      = 0;  // cm (-1 si null)
    int                       longueurMax      = 0;

    SubstratBouture           substrat         = SubstratBouture::INCONNU;
    HormoneBouturage          hormoneBouturage = HormoneBouturage::INCONNU;
    HumiditeBouture           humidite         = HumiditeBouture::INCONNU;

    int                       temperatureMin   = 0;
    int                       temperatureMax   = 0;

    int                       enracinementMin  = 0;  // jours
    int                       enracinementMax  = 0;

    std::vector<EtapeBouture> etapes;
    TauxReussite              tauxReussite     = TauxReussite::INCONNU;
};