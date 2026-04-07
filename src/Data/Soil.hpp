#pragma once
#include <string>
#include <vector>
#include <map>
#include <unordered_map>
#include "EnumInfo.hpp"

// ─────────────────────────────────────────────────────────────────────────────
//  STRUCT SOL
// ─────────────────────────────────────────────────────────────────────────────

struct Soil {
    TypeSol typeSol;             // clé JSON — ex: "TERREAU_MEDITERRANEEN"
    TextureSol texture;
    DrainageSol drainage;
    RetentionEau retentionEau;
    RichesseSol richesse;

    float phMin = 0.f;
    float phMax = 0.f;

    std::unordered_map<CompositionSol, float> composition;
    std::string utilisation;
    std::vector<std::string> adaptePour;   // noms de plantes
    std::vector<RisquesSol> risques;

    // Correction pH
    std::vector<CorrectionAcidite> correctionBaisser;
    std::vector<CorrectionAlcalinite> correctionAugmenter;

    // Propriétés techniques
    CecSol cec;
    AerationSol aeration;
    DensiteSol densite;
    tamponPh tamponPh;
    Mineralisation mineralisation;
    VieMicrobienne vieMicrobienne;
    std::string frequenceRenouvellement;
    std::string compatibiliteCalcaire;

    // Indice principal (ex: drainage: 95, fertilite: 85...)
    std::string indiceClef;
    int         indiceValeur = 0;
};