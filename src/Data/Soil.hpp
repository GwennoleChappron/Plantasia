#pragma once
#include <string>
#include <vector>
#include <map>

// ─────────────────────────────────────────────────────────────────────────────
//  ENUMS SOL
// ─────────────────────────────────────────────────────────────────────────────

enum class DrainageSol {
    TRES_RAPIDE,
    EXCELLENT,
    TRES_BON,
    BON,
    MOYEN,
    FAIBLE,
    INCONNU
};

enum class RichesseSol {
    TRES_ELEVEE,
    ELEVEE,
    MOYENNE_ELEVEE,
    MOYENNE,
    FAIBLE,
    INCONNU
};

// ─────────────────────────────────────────────────────────────────────────────
//  STRUCT SOL
// ─────────────────────────────────────────────────────────────────────────────

struct Soil {
    std::string typeSol;             // clé JSON — ex: "TERREAU_MEDITERRANEEN"
    std::string texture;
    std::string drainage;
    std::string retentionEau;
    std::string richesse;

    float phMin = 0.f;
    float phMax = 0.f;

    std::vector<std::string> composition;
    std::string utilisation;
    std::vector<std::string> adaptePour;   // noms de plantes
    std::vector<std::string> problemes;
    std::vector<std::string> risques;

    // Correction pH
    std::vector<std::string> correctionBaisser;
    std::vector<std::string> correctionAugmenter;

    // Propriétés techniques
    std::string cec;
    std::string aeration;
    std::string densite;
    std::string tamponPh;
    std::string mineralisation;
    std::string vieMicrobienne;
    std::string frequenceRenouvellement;
    std::string compatibiliteCalcaire;

    // Indice principal (ex: drainage: 95, fertilite: 85...)
    std::string indiceClé;
    int         indiceValeur = 0;
};