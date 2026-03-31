#pragma once
#include <string>
#include <vector>
#include <map>

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

    std::vector<CompositionSol> composition;
    std::string utilisation;
    std::vector<std::string> adaptePour;   // noms de plantes
    std::vector<risquesSol> risques;

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
    std::string indiceClé;
    int         indiceValeur = 0;
};