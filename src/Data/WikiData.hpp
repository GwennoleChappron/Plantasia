#pragma once
#include <string>
#include <vector>
#include <optional>
#include <map>
#include <nlohmann/json.hpp>

// ============================================================
// 1. ÉNUMÉRATIONS (Basées sur tes listes)
// ============================================================

enum class NiveauDifficulte { FACILE, MOYEN, DIFFICILE };
enum class Humidite       { FAIBLE, MOYENNE, ELEVEE, IMMERGEE };
enum class TypeTige       { HERBACEE, SEMI_LIGNEUSE, LIGNEUSE, FEUILLE, RACINE };
enum class Periode        { PRINTEMPS, ETE, AUTOMNE, HIVER, INTERIEUR };
enum class Hormone        { NON, OPTIONNEL, RECOMMANDE };
enum class FormePot       { COLONNE, BAC, JARDINIERE, STANDARD };
enum class FreqRempotage  { RARE, ANNUEL, FREQUENT, TOUS_2_4_ANS };

// Fusion des drainages Pot + Sol
enum class Drainage { 
    FAIBLE, MOYEN, BON, TRES_BON, EXCELLENT, TRES_RAPIDE, 
    IMPORTANT, TRES_IMPORTANT, ESSENTIEL 
};

// Sols
enum class TextureSol { 
    TRES_FINE, FINE, EQUILIBREE, SOUPLE, LEGER, LOURDE, TRES_LEGER, 
    LEGER_CAILLOUTEUX, LEGERE, AERE // Ajouts basés sur ton JSON
};
enum class RetentionEau { FAIBLE, MOYENNE, BONNE, TRES_FORTE };
enum class Richesse     { FAIBLE, MOYENNE, ELEVEE, MOYENNE_ELEVEE };
enum class Cec          { TRES_FAIBLE, FAIBLE, MOYENNE, ELEVEE, TRES_ELEVEE };
enum class Aeration     { TRES_FAIBLE, FAIBLE, MOYENNE, BONNE, EXCELLENTE, TRES_ELEVEE };

// ============================================================
// 2. MACROS NLOHMANN (Conversion Magique String <-> Enum)
// ============================================================
// Ces macros permettent à nlohmann::json de convertir tout seul
// "FACILE" en NiveauDifficulte::FACILE et inversement !

NLOHMANN_JSON_SERIALIZE_ENUM(NiveauDifficulte, {
    {NiveauDifficulte::FACILE, "FACILE"}, {NiveauDifficulte::MOYEN, "MOYEN"}, {NiveauDifficulte::DIFFICILE, "DIFFICILE"}
})
NLOHMANN_JSON_SERIALIZE_ENUM(Humidite, {
    {Humidite::FAIBLE, "FAIBLE"}, {Humidite::MOYENNE, "MOYENNE"}, {Humidite::ELEVEE, "ELEVEE"}, {Humidite::IMMERGEE, "IMMERGEE"}
})
NLOHMANN_JSON_SERIALIZE_ENUM(TypeTige, {
    {TypeTige::HERBACEE, "HERBACEE"}, {TypeTige::SEMI_LIGNEUSE, "SEMI_LIGNEUSE"}, {TypeTige::LIGNEUSE, "LIGNEUSE"}, {TypeTige::FEUILLE, "FEUILLE"}, {TypeTige::RACINE, "RACINE"}
})
NLOHMANN_JSON_SERIALIZE_ENUM(Periode, {
    {Periode::PRINTEMPS, "PRINTEMPS"}, {Periode::ETE, "ETE"}, {Periode::AUTOMNE, "AUTOMNE"}, {Periode::HIVER, "HIVER"}, {Periode::INTERIEUR, "INTERIEUR"}
})
NLOHMANN_JSON_SERIALIZE_ENUM(Hormone, {
    {Hormone::NON, "NON"}, {Hormone::OPTIONNEL, "OPTIONNEL"}, {Hormone::RECOMMANDE, "RECOMMANDE"}
})
NLOHMANN_JSON_SERIALIZE_ENUM(FormePot, {
    {FormePot::COLONNE, "COLONNE"}, {FormePot::BAC, "BAC"}, {FormePot::JARDINIERE, "JARDINIERE"}, {FormePot::STANDARD, "STANDARD"}
})
NLOHMANN_JSON_SERIALIZE_ENUM(FreqRempotage, {
    {FreqRempotage::RARE, "RARE"}, {FreqRempotage::ANNUEL, "ANNUEL"}, {FreqRempotage::FREQUENT, "FREQUENT"}, {FreqRempotage::TOUS_2_4_ANS, "TOUS_2_4_ANS"}
})
NLOHMANN_JSON_SERIALIZE_ENUM(Drainage, {
    {Drainage::FAIBLE, "FAIBLE"}, {Drainage::MOYEN, "MOYEN"}, {Drainage::BON, "BON"}, {Drainage::TRES_BON, "TRES_BON"}, 
    {Drainage::EXCELLENT, "EXCELLENT"}, {Drainage::TRES_RAPIDE, "TRES_RAPIDE"}, {Drainage::IMPORTANT, "IMPORTANT"}, 
    {Drainage::TRES_IMPORTANT, "TRES_IMPORTANT"}, {Drainage::ESSENTIEL, "ESSENTIEL"}
})
NLOHMANN_JSON_SERIALIZE_ENUM(TextureSol, {
    {TextureSol::TRES_FINE, "TRES_FINE"}, {TextureSol::FINE, "FINE"}, {TextureSol::EQUILIBREE, "EQUILIBREE"}, 
    {TextureSol::SOUPLE, "SOUPLE"}, {TextureSol::LEGER, "LEGER"}, {TextureSol::LOURDE, "LOURDE"}, 
    {TextureSol::TRES_LEGER, "TRES_LEGER"}, {TextureSol::LEGER_CAILLOUTEUX, "LEGER_CAILLOUTEUX"}, 
    {TextureSol::LEGERE, "LEGERE"}, {TextureSol::AERE, "AERE"}
})
NLOHMANN_JSON_SERIALIZE_ENUM(RetentionEau, {
    {RetentionEau::FAIBLE, "FAIBLE"}, {RetentionEau::MOYENNE, "MOYENNE"}, {RetentionEau::BONNE, "BONNE"}, {RetentionEau::TRES_FORTE, "TRES_FORTE"}
})
NLOHMANN_JSON_SERIALIZE_ENUM(Richesse, {
    {Richesse::FAIBLE, "FAIBLE"}, {Richesse::MOYENNE, "MOYENNE"}, {Richesse::ELEVEE, "ELEVEE"}, {Richesse::MOYENNE_ELEVEE, "MOYENNE_ELEVEE"}
})
NLOHMANN_JSON_SERIALIZE_ENUM(Cec, {
    {Cec::TRES_FAIBLE, "TRES_FAIBLE"}, {Cec::FAIBLE, "FAIBLE"}, {Cec::MOYENNE, "MOYENNE"}, {Cec::ELEVEE, "ELEVEE"}, {Cec::TRES_ELEVEE, "TRES_ELEVEE"}
})
NLOHMANN_JSON_SERIALIZE_ENUM(Aeration, {
    {Aeration::TRES_FAIBLE, "TRES_FAIBLE"}, {Aeration::FAIBLE, "FAIBLE"}, {Aeration::MOYENNE, "MOYENNE"}, 
    {Aeration::BONNE, "BONNE"}, {Aeration::EXCELLENTE, "EXCELLENTE"}, {Aeration::TRES_ELEVEE, "TRES_ELEVEE"}
})


// ============================================================
// 3. STRUCTURES DE SOUTIEN (Min/Max)
// ============================================================

struct MinMaxInt {
    int min = 0;
    int max = 0;
};
inline void from_json(const nlohmann::json& j, MinMaxInt& m) {
    if(j.is_object()) {
        m.min = j.value("min", 0);
        m.max = j.value("max", 0);
    }
}

struct MinMaxFloat {
    float min = 0.0f;
    float max = 0.0f;
};
inline void from_json(const nlohmann::json& j, MinMaxFloat& m) {
    if(j.is_object()) {
        m.min = j.value("min", 0.0f);
        m.max = j.value("max", 0.0f);
    }
}


// ============================================================
// 4. LES STRUCTURES PRINCIPALES
// ============================================================

// --- A. BOUTURES ---
struct BoutureData {
    std::string nom;
    NiveauDifficulte niveau_difficulte = NiveauDifficulte::MOYEN;
    std::vector<Periode> periode;
    TypeTige type_tige = TypeTige::HERBACEE;
    std::vector<std::string> plantes_concernees;
    
    // Optional car le JSON peut avoir "null" (ex: Bouture Feuille)
    std::optional<MinMaxInt> longueur_coupe_cm; 
    
    std::string substrat;
    Hormone hormone_bouturage = Hormone::NON;
    Humidite humidite = Humidite::MOYENNE;
    MinMaxInt temperature_c;
    MinMaxInt temps_enracinement_jours;
    std::vector<std::string> etapes;
    std::string taux_reussite;
};

inline void from_json(const nlohmann::json& j, BoutureData& b) {
    b.nom = j.value("nom", "");
    b.niveau_difficulte = j.value("niveau_difficulte", NiveauDifficulte::MOYEN);
    b.periode = j.value("periode", std::vector<Periode>());
    b.type_tige = j.value("type_tige", TypeTige::HERBACEE);
    b.plantes_concernees = j.value("plantes_concernees", std::vector<std::string>());
    
    if (j.contains("longueur_coupe_cm") && !j["longueur_coupe_cm"].is_null()) {
        b.longueur_coupe_cm = j["longueur_coupe_cm"].get<MinMaxInt>();
    }
    
    b.substrat = j.value("substrat", "");
    b.hormone_bouturage = j.value("hormone_bouturage", Hormone::NON);
    b.humidite = j.value("humidite", Humidite::MOYENNE);
    b.temperature_c = j.value("temperature_c", MinMaxInt{0,0});
    b.temps_enracinement_jours = j.value("temps_enracinement_jours", MinMaxInt{0,0});
    b.etapes = j.value("etapes", std::vector<std::string>());
    b.taux_reussite = j.value("taux_reussite", "");
}

// --- B. POTS & RACINES ---
struct PotData {
    std::string type_racinaire;
    std::string profondeur_pot;
    std::string largeur_pot;
    MinMaxInt volume_litres;
    FormePot forme_pot = FormePot::STANDARD;
    std::vector<std::string> materiaux;
    std::vector<std::string> plantes_exemples;
    Drainage drainage = Drainage::MOYEN;
    FreqRempotage frequence_rempotage = FreqRempotage::ANNUEL;
    std::string sensibilite_rempotage;
};

inline void from_json(const nlohmann::json& j, PotData& p) {
    p.type_racinaire = j.value("type_racinaire", "");
    p.profondeur_pot = j.value("profondeur_pot", "");
    p.largeur_pot = j.value("largeur_pot", "");
    p.volume_litres = j.value("volume_litres", MinMaxInt{0,0});
    p.forme_pot = j.value("forme_pot", FormePot::STANDARD);
    p.materiaux = j.value("materiaux", std::vector<std::string>());
    p.plantes_exemples = j.value("plantes_exemples", std::vector<std::string>());
    p.drainage = j.value("drainage", Drainage::MOYEN);
    p.frequence_rempotage = j.value("frequence_rempotage", FreqRempotage::ANNUEL);
    p.sensibilite_rempotage = j.value("sensibilite_rempotage", "");
}

// --- C. SOLS & SUBSTRATS ---
struct CorrectionPH {
    std::vector<std::string> baisser;
    std::vector<std::string> augmenter;
};
inline void from_json(const nlohmann::json& j, CorrectionPH& c) {
    c.baisser = j.value("BAISSER", std::vector<std::string>());
    c.augmenter = j.value("AUGMENTER", std::vector<std::string>());
}

struct SolData {
    std::string type_sol;
    TextureSol texture = TextureSol::EQUILIBREE;
    Drainage drainage = Drainage::MOYEN;
    RetentionEau retention_eau = RetentionEau::MOYENNE;
    Richesse richesse = Richesse::MOYENNE;
    MinMaxFloat ph;
    
    std::vector<std::string> composition;
    std::string utilisation;
    std::vector<std::string> adapte_pour;
    std::vector<std::string> problemes;
    
    CorrectionPH correction_ph;
    
    Cec cec = Cec::MOYENNE;
    Aeration aeration = Aeration::MOYENNE;
    std::string densite;
    std::string tampon_ph;
    
    // Champs optionnels (Toutes les clés du JSON ne les ont pas)
    std::string mineralisation;
    std::string vie_microbienne;
    std::vector<std::string> risques;
    std::string frequence_renouvellement;
    std::string compatibilite_calcaire;
    std::vector<std::string> amelioration;
    std::string sterilite;
    std::string travail_sol;
    std::string besoin_amendement;
    std::string besoin_engrais;

    std::map<std::string, int> indices; // ex: {"polyvalence": 90}
};

inline void from_json(const nlohmann::json& j, SolData& s) {
    s.type_sol = j.value("type_sol", "");
    s.texture = j.value("texture", TextureSol::EQUILIBREE);
    s.drainage = j.value("drainage", Drainage::MOYEN);
    s.retention_eau = j.value("retention_eau", RetentionEau::MOYENNE);
    s.richesse = j.value("richesse", Richesse::MOYENNE);
    s.ph = j.value("ph", MinMaxFloat{6.0f, 7.0f});
    
    s.composition = j.value("composition", std::vector<std::string>());
    s.utilisation = j.value("utilisation", "");
    s.adapte_pour = j.value("adapte_pour", std::vector<std::string>());
    s.problemes = j.value("problemes", std::vector<std::string>());
    
    if (j.contains("correction_ph")) {
        s.correction_ph = j["correction_ph"].get<CorrectionPH>();
    }

    s.cec = j.value("cec", Cec::MOYENNE);
    s.aeration = j.value("aeration", Aeration::MOYENNE);
    s.densite = j.value("densite", "");
    s.tampon_ph = j.value("tampon_ph", "");

    s.mineralisation = j.value("mineralisation", "");
    s.vie_microbienne = j.value("vie_microbienne", "");
    s.risques = j.value("risques", std::vector<std::string>());
    s.frequence_renouvellement = j.value("frequence_renouvellement", "");
    s.compatibilite_calcaire = j.value("compatibilite_calcaire", "");
    s.amelioration = j.value("amelioration", std::vector<std::string>());
    s.sterilite = j.value("sterilite", "");
    s.travail_sol = j.value("travail_sol", "");
    s.besoin_amendement = j.value("besoin_amendement", "");
    s.besoin_engrais = j.value("besoin_engrais", "");

    s.indices = j.value("indices", std::map<std::string, int>());
}