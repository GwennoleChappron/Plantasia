#pragma once
#include <string>
#include <unordered_map>
#include <imgui.h>
#include "UI/ColorTheme.hpp"

// ═════════════════════════════════════════════════════════════════════════════
//  EnumInfo.hpp — Source unique de vérité pour tous les enums de Plantasia
//
//  SOMMAIRE DE L'ARCHITECTURE :
//  Ce fichier fait le pont entre les données textuelles (JSON) et l'UI (ImGui).
//  
//  1. METADATA           : Structure de base et fallback.
//  2. ENUMS COMMUNS      : Partagés par plusieurs entités (Difficulté, Saisons, Racines).
//  3. ENCYCLOPEDIA.JSON  : Enums spécifiques aux plantes (Rusticité, Expo, etc.).
//  4. SOLS.JSON          : Enums liés aux substrats (Texture, CEC, Drainage...).
//  5. POTS.JSON          : Enums liés aux contenants (Forme, Matériau...).
//  6. BOUTURES.JSON      : Enums liés à la multiplication (Taux réussite, Tige...).
//  7. PARSERS (Helpers)  : Fonctions String -> Enum utilisées par le DataLoader.
//
//  Usage UI :
//    const auto& m = EnumInfo::get(exposition);
//    ImGui::TextColored(m.color, "%s", m.label.c_str());
// ═════════════════════════════════════════════════════════════════════════════

// ─────────────────────────────────────────────────────────────────────────────
//  1. STRUCTURE DE MÉTADONNÉES & FALLBACK
// ─────────────────────────────────────────────────────────────────────────────

struct EnumMetadata {
    std::string label;        // Court, affiché dans les listes et tags
    std::string description;  // Phrase complète, utilisée dans les tooltips
    ImVec4      color;        // Couleur sémantique pour tag / icône
};

// Fallback global (jamais affiché en production si les enums sont complets)
inline const EnumMetadata& unknownMetadata() {
    static EnumMetadata u{ "?", "Valeur inconnue", Theme::TextMuted };
    return u;
}

// Macro helper — évite la répétition dans les fonctions get()
#define ENUM_MAP_LOOKUP(map, key) \
    { auto it = (map).find(key); \
      if (it != (map).end()) return it->second; \
      return unknownMetadata(); }


// ═════════════════════════════════════════════════════════════════════════════
//  2. ENUMS COMMUNS (Partagés entre plusieurs entités)
// ═════════════════════════════════════════════════════════════════════════════

enum class NiveauDifficulte { FACILE, MOYEN, DIFFICILE, INCONNU };
enum class TypeRacinaireEnum { FASCICULE, PIVOTANT, TRACANT, LIGNEUX, TUBEREUX, INCONNU }; 
enum class Saison { PRINTEMPS, ETE, AUTOMNE, HIVER, INTERIEUR, INCONNU };

namespace EnumInfo {
    // Niveau Difficulté
    inline const EnumMetadata& get(NiveauDifficulte v) {
        using N = NiveauDifficulte;
        static const std::unordered_map<N, EnumMetadata> map {
            { N::FACILE,    { "Facile",    "Convient aux débutants, pardonne facilement les erreurs.",  Theme::PlantGreen } },
            { N::MOYEN,     { "Moyen",     "Demande un peu d'attention et de régularité.",              Theme::Amber } },
            { N::DIFFICILE, { "Difficile", "Exige de l'expérience et des conditions précises.",         Theme::DangerRed } }
        };
        ENUM_MAP_LOOKUP(map, v)
    }
    inline const char* label(NiveauDifficulte v) { return get(v).label.c_str(); }

    // Type Racinaire
    inline const EnumMetadata& get(TypeRacinaireEnum v) {
        using T = TypeRacinaireEnum;
        static const std::unordered_map<T, EnumMetadata> map {
            { T::FASCICULE, { "Fasciculé", "Racines fines et denses en surface.",                       Theme::PlantMid } },
            { T::PIVOTANT,  { "Pivotant",  "Racine principale profonde.",                               Theme::darken(Theme::WarningOrange, 0.5f) } },
            { T::TRACANT,   { "Traçant",   "Stolons horizontaux envahissants.",                         Theme::WarningOrange } },
            { T::LIGNEUX,   { "Ligneux",   "Système racinaire fort et permanent.",                      Theme::darken(Theme::Amber, 0.5f) } },
            { T::TUBEREUX,  { "Tubéreux",  "Racines renflées stockant des réserves (tubercules).",      Theme::Floraison } }
        };
        ENUM_MAP_LOOKUP(map, v)
    }
    inline const char* label(TypeRacinaireEnum v) { return get(v).label.c_str(); }

    // Saison
    inline const EnumMetadata& get(Saison v) {
        using S = Saison;
        static const std::unordered_map<S, EnumMetadata> map {
            { S::PRINTEMPS, { "Printemps", "Mars à mai — montée de sève.",                              Theme::PlantGreen } },
            { S::ETE,       { "Été",       "Juin à août — pleine croissance.",                          Theme::Amber } },
            { S::AUTOMNE,   { "Automne",   "Septembre à novembre — ralentissement.",                    Theme::WarningOrange } },
            { S::HIVER,     { "Hiver",     "Décembre à février — dormance.",                            Theme::InfoBlue } },
            { S::INTERIEUR, { "Intérieur", "Possible toute l'année en intérieur chauffé.",              Theme::Violet } }
        };
        ENUM_MAP_LOOKUP(map, v)
    }
    inline const char* label(Saison v) { return get(v).label.c_str(); }

    // Mois (Spécial, basé sur int 1-12)
    struct MoisMetadata { const char* court; const char* long_; ImVec4 color; };
    inline const MoisMetadata& getMois(int mois) {
        static const MoisMetadata table[13] = {
            { "?",   "Inconnu",    Theme::TextMuted },
            { "Jan", "Janvier",    Theme::InfoBlue }, 
            { "Fév", "Février",    Theme::withAlpha(Theme::InfoBlue, 0.8f) },
            { "Mar", "Mars",       Theme::TextSecondary }, 
            { "Avr", "Avril",      Theme::PlantMid },
            { "Mai", "Mai",        Theme::PlantGreen }, 
            { "Juin","Juin",       Theme::Amber },
            { "Juil","Juillet",    Theme::WarningOrange }, 
            { "Août","Août",       Theme::withAlpha(Theme::WarningOrange, 0.8f) },
            { "Sep", "Septembre",  Theme::darken(Theme::Amber, 0.6f) }, 
            { "Oct", "Octobre",    Theme::darken(Theme::WarningOrange, 0.5f) },
            { "Nov", "Novembre",   Theme::TextDisabled }, 
            { "Déc", "Décembre",   Theme::InfoBlue }
        };
        return (mois >= 1 && mois <= 12) ? table[mois] : table[0];
    }
} // namespace EnumInfo


// ═════════════════════════════════════════════════════════════════════════════
//  3. ENCYCLOPEDIA.JSON (Spécifique aux Plantes)
// ═════════════════════════════════════════════════════════════════════════════

enum class Rusticite { RUSTIQUE, SEMI_RUSTIQUE, FRAGILE, INCONNU };
enum class ExpositionSoleil { PLEIN_SOLEIL, MI_OMBRE, INCONNU };
enum class ExpositionVent { MODERE, ABRITE, INCONNU };
enum class VitesseCroissance { LENTE, MODEREE, RAPIDE, TRES_RAPIDE, INCONNU};
enum class ToleranceSecheresse { EXCELLENTE, BONNE, MOYENNE, FAIBLE, TRES_FAIBLE, INCONNU };
enum class SensibiliteEau { TRES_ELEVEE, ELEVEE, MOYENNE, FAIBLE, INCONNU };
enum class TypeFeuillage { PERSISTANT, CADUC, SEMI_PERSISTANT, ANNUEL, BISANNUEL, INCONNU };
enum class TypeBouture { BOUTURE_HERBACEE, BOUTURE_SEMI_LIGNEUSE, BOUTURE_LIGNEUSE, BOUTURE_EAU, BOUTURE_FEUILLE, BOUTURE_RACINE, INCONNU };

namespace EnumInfo {

    // Rusticité
    inline const EnumMetadata& get(Rusticite v) {
        using R = Rusticite;
        static const std::unordered_map<R, EnumMetadata> map {
            { R::RUSTIQUE,      { "Rustique",      "Résiste au gel et aux hivers rigoureux.",          Theme::PlantGreen } },
            { R::SEMI_RUSTIQUE, { "Semi-rustique", "Supporte des gelées légères, à protéger.",         Theme::Amber } },
            { R::FRAGILE,       { "Fragile",       "Craint le moindre gel, rentrée indispensable.",    Theme::DangerRed } }
        };
        ENUM_MAP_LOOKUP(map, v)
    }
    inline const char* label(Rusticite v) { return get(v).label.c_str(); }

    // Exposition au soleil
    inline const EnumMetadata& get(ExpositionSoleil v) {
        using E = ExpositionSoleil;
        static const std::unordered_map<E, EnumMetadata> map {
            { E::PLEIN_SOLEIL, { "Plein soleil", "Nécessite au minimum 6h de soleil direct/jour.",    Theme::Amber } },
            { E::MI_OMBRE,     { "Mi-ombre",     "Préfère 3 à 5h de soleil, supporte l'ombre.",       Theme::PlantMid } }
        };
        ENUM_MAP_LOOKUP(map, v)
    }
    inline const char* label(ExpositionSoleil v) { return get(v).label.c_str(); }

    // Exposition au vent
    inline const EnumMetadata& get(ExpositionVent v) {
        using E = ExpositionVent;
        static const std::unordered_map<E, EnumMetadata> map {
            { E::MODERE, { "Vent modéré", "Tolère une exposition venteuse normale.",                  Theme::withAlpha(Theme::InfoBlue, 0.8f) } },
            { E::ABRITE, { "Abrité",      "Fragile au vent, placer à l'abri d'un mur.",               Theme::InfoBlue } }
        };
        ENUM_MAP_LOOKUP(map, v)
    }
    inline const char* label(ExpositionVent v) { return get(v).label.c_str(); }

    // Besoin en eau (Spécial, basé sur int 0-5)
    struct BesoinEauMetadata { const char* label; const char* description; ImVec4 color; int gouttes; };
    inline const BesoinEauMetadata& getBesoinEau(int niveau) {
        static const BesoinEauMetadata table[6] = {
            { "Nul",         "Arrosage quasi-inexistant.",                               Theme::withAlpha(Theme::Amber, 0.7f),  0 },
            { "Très faible", "Arrosage très rare, forte sécheresse.",                    Theme::withAlpha(Theme::WarningOrange, 0.8f), 1 },
            { "Faible",      "Arrosage peu fréquent, tolère la sécheresse.",             Theme::Amber, 2 },
            { "Moyen",       "Arrosage régulier, laisser sécher entre deux.",            Theme::TextSecondary, 3 },
            { "Élevé",       "Arrosage fréquent, substrat légèrement humide.",           Theme::withAlpha(Theme::InfoBlue, 0.8f), 4 },
            { "Très élevé",  "Arrosage très régulier, craint la sécheresse.",            Theme::InfoBlue, 5 }
        };
        return (niveau >= 0 && niveau <= 5) ? table[niveau] : table[0];
    }

    inline const EnumMetadata& get(VitesseCroissance v) {
        using V = VitesseCroissance;
        static const std::unordered_map<V, EnumMetadata> map {
            { V::LENTE,       { "Lente",       "Croissance lente, peu de rempotages nécessaires.",         Theme::TextSecondary } },
            { V::MODEREE,     { "Modérée",     "Croissance régulière et prévisible.",                      Theme::PlantMid } },
            { V::RAPIDE,      { "Rapide",      "Croît vite, surveiller le rempotage et la taille.",        Theme::withAlpha(Theme::WarningOrange, 0.8f) } },
            { V::TRES_RAPIDE, { "Très rapide", "Envahissante possible, contenir dans un pot isolé.",       Theme::WarningOrange } },
            { V::INCONNU,     { "?",           "Non renseignée.",                                          Theme::TextMuted } },
        };
        ENUM_MAP_LOOKUP(map, v)
    }
    inline const char* label(VitesseCroissance v) { return get(v).label.c_str();}

    inline const EnumMetadata& get(ToleranceSecheresse v) {
        using T = ToleranceSecheresse;
        static const std::unordered_map<T, EnumMetadata> map {
            { T::EXCELLENTE,  { "Excellente",  "Supporte de longues périodes sans eau.",                   Theme::withAlpha(Theme::Amber, 0.7f) } },
            { T::BONNE,       { "Bonne",       "Tolère quelques oublis d'arrosage sans dommage.",          Theme::withAlpha(Theme::WarningOrange, 0.8f) } },
            { T::MOYENNE,     { "Moyenne",     "Peut supporter 1-2 semaines de sécheresse en été.",        Theme::Amber } },
            { T::FAIBLE,      { "Faible",      "Flétrit rapidement sans arrosage régulier.",               Theme::withAlpha(Theme::InfoBlue, 0.8f) } },
            { T::TRES_FAIBLE, { "Très faible", "Exige un substrat constamment frais, flétrit en 24-48h.",  Theme::InfoBlue } },
            { T::INCONNU,     { "?",           "Non renseignée.",                                          Theme::TextMuted } },
        };
        ENUM_MAP_LOOKUP(map, v)
    }
    inline const char* label(ToleranceSecheresse v) { return get(v).label.c_str(); }

    inline const EnumMetadata& get(SensibiliteEau v) {
        using S = SensibiliteEau;
        static const std::unordered_map<S, EnumMetadata> map {
            { S::TRES_ELEVEE, { "Très sensible",  "Le moindre excès provoque la pourriture des racines.",  Theme::DangerRed } },
            { S::ELEVEE,      { "Sensible",       "Réduire fortement l'arrosage en hiver.",                Theme::DangerHover } },
            { S::MOYENNE,     { "Moyenne",        "Tolère quelques excès ponctuels sans dommage grave.",   Theme::withAlpha(Theme::WarningOrange, 0.8f) } },
            { S::FAIBLE,      { "Résistante",     "Apprécie un substrat constamment frais.",               Theme::withAlpha(Theme::InfoBlue, 0.8f) } },
            { S::INCONNU,     { "?",              "Non renseignée.",                                       Theme::TextMuted } },
        };
        ENUM_MAP_LOOKUP(map, v)
    }
    inline const char* label(SensibiliteEau v) { return get(v).label.c_str(); }

    inline const EnumMetadata& get(TypeFeuillage v) {
        using F = TypeFeuillage;
        static const std::unordered_map<F, EnumMetadata> map {
            { F::PERSISTANT,       { "Persistant",       "Garde ses feuilles toute l'année.",                   Theme::PlantGreen } },
            { F::CADUC,            { "Caduc",            "Perd ses feuilles en hiver, repos végétatif.",        Theme::darken(Theme::Amber, 0.5f) } },
            { F::SEMI_PERSISTANT,  { "Semi-persistant",  "Garde une partie du feuillage selon les hivers.",     Theme::PlantMid } },
            { F::ANNUEL,           { "Annuel",           "Cycle d'un an — repartir de semis ou boutures.",      Theme::withAlpha(Theme::WarningOrange, 0.8f) } },
            { F::BISANNUEL,        { "Bisannuel",        "Cultivé comme annuel, fleurit la deuxième année.",    Theme::Amber } },
            { F::INCONNU,          { "?",                "Non renseigné.",                                      Theme::TextMuted } },
        };
        ENUM_MAP_LOOKUP(map, v)
    }
    inline const char* label(TypeFeuillage v) { return get(v).label.c_str(); }

    inline const EnumMetadata& get(TypeBouture v) {
        using B = TypeBouture;
        static const std::unordered_map<B, EnumMetadata> map {
            { B::BOUTURE_HERBACEE,      { "Herbacée",      "Tige souple, enracinement rapide 1-3 sem.",         Theme::PlantGreen } },
            { B::BOUTURE_SEMI_LIGNEUSE, { "Semi-ligneuse", "Tige aoûtée, enracinement 1-2 mois.",               Theme::PlantMid } },
            { B::BOUTURE_LIGNEUSE,      { "Ligneuse",      "Bois dormant, enracinement lent 2-4 mois.",         Theme::darken(Theme::WarningOrange, 0.5f) } },
            { B::BOUTURE_EAU,           { "Dans l'eau",    "Dans un verre d'eau, sans substrat.",               Theme::InfoBlue } },
            { B::BOUTURE_FEUILLE,       { "Par feuille",   "Feuille entière posée sur substrat drainant.",      Theme::TextSecondary } },
            { B::BOUTURE_RACINE,        { "Par racine",    "Segment de racine en hiver — technique avancée.",   Theme::darken(Theme::Amber, 0.5f) } },
            { B::INCONNU,               { "?",             "Type non renseigné.",                               Theme::TextMuted } },
        };
        ENUM_MAP_LOOKUP(map, v)
    }
    inline const char* label(TypeBouture v) { return get(v).label.c_str(); }

}


// ═════════════════════════════════════════════════════════════════════════════
//  4. SOLS.JSON (Spécifique aux Substrats)
// ═════════════════════════════════════════════════════════════════════════════

enum class TypeSol { TERREAU_UNIVERSEL, TERREAU_MEDITERRANEEN, TERRE_BRUYERE, TERREAU_HORTICOLE_ENRICHI, SUBSTRAT_AGRUMES, TERREAU_SEMIS, SOL_ARGILEUX, SOL_SABLEUX, EPIPHYTE, DRAINANT, INCONNU };
enum class DrainageSol { TRES_RAPIDE, EXCELLENT, TRES_BON, BON, MOYEN, FAIBLE, INCONNU };
enum class RetentionEau { TRES_FORTE, BONNE, MOYENNE, FAIBLE, INCONNU };
enum class RichesseSol { ELEVEE, MOYENNE_ELEVEE, MOYENNE, FAIBLE, INCONNU };
enum class TextureSol { TRES_LEGER, LEGER_CAILLOUTEUX, LEGERE, AERE, SOUPLE, EQUILIBREE, TRES_FINE, LOURDE, INCONNU };
enum class CecSol { TRES_ELEVEE, ELEVEE, MOYENNE, FAIBLE, TRES_FAIBLE, INCONNU };
enum class AerationSol { EXCELLENTE, TRES_ELEVEE, BONNE, MOYENNE, TRES_FAIBLE, INCONNU };
enum class DensiteSol { TRES_LEGERE, LEGERE, MOYENNE, LOURDE, TRES_LOURDE, INCONNU };
enum class tamponPh { TRES_FORT, FORT, MOYEN, FAIBLE, TRES_FAIBLE, INCONNU };
enum class VieMicrobienne { TRES_ELEVEE, MOYENNE, FAIBLE, INCONNU };
enum class Mineralisation { RAPIDE, MODEREE, LENTE, INCONNU };
enum class CompatibiliteCalcaire { EXCELLENTE, MOYENNE, TRES_FAIBLE, INCONNU };
enum class CorrectionAcidite { CHAUX, CENDRES, CALCAIRE, EAU_CALCAIRE, TERREAU_UNIVERSEL, COMPOST, INCONNU };
enum class CorrectionAlcalinite { TERRE_BRUYERE, SOUFRE, EAU_PLUIE, COMPOST, TERREAU_UNIVERSEL, INCONNU };
enum class CompositionSol { TOURBE, COMPOST, FIBRES_VEGETALES, SABLE, POUZZOLANE, CALCAIRE, MATIERE_ORGANIQUE_ACIDE, COMPOST_ENRICHI, ENGRAIS_ORGANIQUE, ECORCES, ARGILE, INCONNU };
enum class risquesSol { TASSEMENT, APPAUVRISSEMENT, COMPACTION, LESSIVAGE, EXCES_AZOTE, CARENCES, PAUVRETE_LONG_TERME, INCONNU };
enum class frequenceRenouvellement { TOUS_1_2_ANS, TOUS_2_3_ANS, TOUS_3_5_ANS, TOUS_5_10_ANS, TOUS_10_20_ANS, JAMAIS, INCONNU };
enum class compatibiliteCalcaire { EXCELLENTE, MOYENNE, TRES_FAIBLE, INCONNU };

namespace EnumInfo {

    // Type Sol
    inline const EnumMetadata& get(TypeSol v) {
        static const std::unordered_map<TypeSol, EnumMetadata> map {
            { TypeSol::TERREAU_UNIVERSEL,         { "Universel",      "Polyvalent pour aromatiques.",      Theme::PlantMid } },
            { TypeSol::TERREAU_MEDITERRANEEN,     { "Méditerranéen",  "Très drainant, pauvre.",            Theme::withAlpha(Theme::WarningOrange, 0.8f) } },
            { TypeSol::TERRE_BRUYERE,             { "Terre Bruyère",  "Sol acide.",                        Theme::PhAcide } },
            { TypeSol::TERREAU_HORTICOLE_ENRICHI, { "Horticole",      "Très fertile, gourmandes.",         Theme::PlantGreen } },
            { TypeSol::SUBSTRAT_AGRUMES,          { "Agrumes",        "Spécial Yuzu/Agrumes.",             Theme::Amber } },
            { TypeSol::TERREAU_SEMIS,             { "Semis",          "Très fin et stérile.",              Theme::TextDisabled } },
            { TypeSol::SOL_ARGILEUX,              { "Argileux",       "Lourd et fertile.",                 Theme::darken(Theme::WarningOrange, 0.5f) } },
            { TypeSol::EPIPHYTE,                  { "Épiphyte",       "Mélange d'écorces et sphaigne.",    Theme::darken(Theme::Amber, 0.5f) } },
            { TypeSol::DRAINANT,                  { "Drainant",       "Spécial succulentes et cactées.",   Theme::TextMuted } },
            { TypeSol::SOL_SABLEUX,               { "Sableux",        "Très drainant.",                    Theme::withAlpha(Theme::Amber, 0.7f) } }
        }; ENUM_MAP_LOOKUP(map, v)
    }
    inline const char* label(TypeSol v) { return get(v).label.c_str(); }

    // Texture Sol
    inline const EnumMetadata& get(TextureSol v) {
        static const std::unordered_map<TextureSol, EnumMetadata> map {
            { TextureSol::TRES_LEGER,        { "Très léger",        "Majoritairement sableux.",                Theme::withAlpha(Theme::Amber, 0.7f) } },
            { TextureSol::LEGER_CAILLOUTEUX, { "Caillouteux",       "Mélange sable + graviers.",               Theme::darken(Theme::Amber, 0.5f) } },
            { TextureSol::LEGERE,            { "Légère",            "Sol meuble et aéré.",                     Theme::TextSecondary } },
            { TextureSol::AERE,              { "Aéré",              "Structure ouverte.",                      Theme::withAlpha(Theme::InfoBlue, 0.8f) } },
            { TextureSol::SOUPLE,            { "Souple",            "Ni trop compacte ni trop meuble.",        Theme::PlantMid } },
            { TextureSol::EQUILIBREE,        { "Équilibrée",        "Bonne structure limon-argile-sable.",     Theme::PlantGreen } },
            { TextureSol::TRES_FINE,         { "Très fine",         "Idéale pour la germination.",             Theme::TextDisabled } },
            { TextureSol::LOURDE,            { "Lourde",            "Sol argileux collant.",                   Theme::darken(Theme::WarningOrange, 0.5f) } }
        }; ENUM_MAP_LOOKUP(map, v)
    }
    inline const char* label(TextureSol v) { return get(v).label.c_str(); }

    // Drainage Sol
    inline const EnumMetadata& get(DrainageSol v) {
        static const std::unordered_map<DrainageSol, EnumMetadata> map {
            { DrainageSol::TRES_RAPIDE, { "Très rapide", "L'eau traverse immédiatement.",                  Theme::withAlpha(Theme::Amber, 0.7f) } },
            { DrainageSol::EXCELLENT,   { "Excellent",   "Évacuation rapide, jamais détrempé.",            Theme::withAlpha(Theme::WarningOrange, 0.8f) } },
            { DrainageSol::TRES_BON,    { "Très bon",    "Bonne évacuation, légère rétention utile.",      Theme::PlantMid } },
            { DrainageSol::BON,         { "Bon",         "Drainage satisfaisant.",                         Theme::PlantGreen } },
            { DrainageSol::MOYEN,       { "Moyen",       "Rétention modérée.",                             Theme::withAlpha(Theme::WarningOrange, 0.8f) } },
            { DrainageSol::FAIBLE,      { "Faible",      "Risque d'asphyxie racinaire.",                   Theme::DangerRed } }
        }; ENUM_MAP_LOOKUP(map, v)
    }
    inline const char* label(DrainageSol v) { return get(v).label.c_str(); }

    // Rétention Eau Sol
    inline const EnumMetadata& get(RetentionEau v) {
        static const std::unordered_map<RetentionEau, EnumMetadata> map {
            { RetentionEau::TRES_FORTE, { "Très forte", "Sol saturant — espacement arrosages.",            Theme::InfoBlue } },
            { RetentionEau::BONNE,      { "Bonne",      "Retient bien l'humidité.",                        Theme::withAlpha(Theme::InfoBlue, 0.8f) } },
            { RetentionEau::MOYENNE,    { "Moyenne",    "Équilibre correct drainage/rétention.",           Theme::PlantMid } },
            { RetentionEau::FAIBLE,     { "Faible",     "Sèche vite, arrosage fréquent.",                  Theme::withAlpha(Theme::WarningOrange, 0.8f) } }
        }; ENUM_MAP_LOOKUP(map, v)
    }
    inline const char* label(RetentionEau v) { return get(v).label.c_str(); }

    // Richesse Sol
    inline const EnumMetadata& get(RichesseSol v) {
        static const std::unordered_map<RichesseSol, EnumMetadata> map {
            { RichesseSol::ELEVEE,         { "Élevée",         "Sol très fertile.",                        Theme::PlantGreen } },
            { RichesseSol::MOYENNE_ELEVEE, { "Moyenne-élevée", "Bonne fertilité.",                         Theme::PlantMid } },
            { RichesseSol::MOYENNE,        { "Moyenne",        "Fertilité standard.",                      Theme::withAlpha(Theme::WarningOrange, 0.8f) } },
            { RichesseSol::FAIBLE,         { "Faible",         "Sol pauvre — idéal méditerranéennes.",     Theme::withAlpha(Theme::Amber, 0.7f) } }
        }; ENUM_MAP_LOOKUP(map, v)
    }
    inline const char* label(RichesseSol v) { return get(v).label.c_str(); }

    // CEC Sol
    inline const EnumMetadata& get(CecSol v) {
        static const std::unordered_map<CecSol, EnumMetadata> map {
            { CecSol::TRES_ELEVEE, { "Très élevée", "Rétention maximale des engrais.",                     Theme::PlantGreen } },
            { CecSol::ELEVEE,      { "Élevée",      "Bonne capacité à retenir les nutriments.",            Theme::PlantMid } },
            { CecSol::MOYENNE,     { "Moyenne",     "Apports réguliers recommandés.",                      Theme::withAlpha(Theme::WarningOrange, 0.8f) } },
            { CecSol::FAIBLE,      { "Faible",      "Lessivage rapide des nutriments.",                    Theme::WarningOrange } },
            { CecSol::TRES_FAIBLE, { "Très faible", "Sol quasi-inerte.",                                   Theme::DangerRed } }
        }; ENUM_MAP_LOOKUP(map, v)
    }
    inline const char* label(CecSol v) { return get(v).label.c_str(); }

    // Aération Sol
    inline const EnumMetadata& get(AerationSol v) {
        static const std::unordered_map<AerationSol, EnumMetadata> map {
            { AerationSol::EXCELLENTE,  { "Excellente",  "Circulation d'air optimale.",                    Theme::PlantGreen } },
            { AerationSol::TRES_ELEVEE, { "Très élevée", "Très bonne oxygénation.",                        Theme::PlantMid } },
            { AerationSol::BONNE,       { "Bonne",       "Aération correcte.",                             Theme::TextSecondary } },
            { AerationSol::MOYENNE,     { "Moyenne",     "Aération acceptable, attention au tassement.",   Theme::withAlpha(Theme::WarningOrange, 0.8f) } },
            { AerationSol::TRES_FAIBLE, { "Très faible", "Sol compact — risque de pourriture.",            Theme::DangerRed } }
        }; ENUM_MAP_LOOKUP(map, v)
    }
    inline const char* label(AerationSol v) { return get(v).label.c_str(); }

    // Vie Microbienne Sol
    inline const EnumMetadata& get(VieMicrobienne v) {
        static const std::unordered_map<VieMicrobienne, EnumMetadata> map {
            { VieMicrobienne::TRES_ELEVEE, { "Très élevée", "Sol vivant et actif.",                        Theme::PlantGreen } },
            { VieMicrobienne::MOYENNE,     { "Moyenne",     "Activité microbienne correcte.",              Theme::PlantMid } },
            { VieMicrobienne::FAIBLE,      { "Faible",      "Peu de vie biologique.",                      Theme::withAlpha(Theme::WarningOrange, 0.8f) } }
        }; ENUM_MAP_LOOKUP(map, v)
    }
    inline const char* label(VieMicrobienne v) { return get(v).label.c_str(); }

    // Densité Sol
    inline const EnumMetadata& get(DensiteSol v) {
        static const std::unordered_map<DensiteSol, EnumMetadata> map {
            { DensiteSol::TRES_LEGERE, { "Très légère", "Sol très aéré, idéal pour semis et boutures.",    Theme::withAlpha(Theme::Amber, 0.7f) } },
            { DensiteSol::LEGERE,      { "Légère",      "Sol meuble, facile à travailler.",                Theme::TextSecondary } },
            { DensiteSol::MOYENNE,     { "Moyenne",     "Densité standard, bonne rétention d'eau.",        Theme::PlantMid } },
            { DensiteSol::LOURDE,      { "Lourde",      "Sol compact, riche en argile.",                   Theme::darken(Theme::WarningOrange, 0.5f) } },
            { DensiteSol::TRES_LOURDE, { "Très lourde", "Sol très compact et collant.",                    Theme::darken(Theme::Amber, 0.5f) } }
        }; ENUM_MAP_LOOKUP(map, v)
    }
    inline const char* label(DensiteSol v) { return get(v).label.c_str(); }

    // Tampon pH Sol
    inline const EnumMetadata& get(tamponPh v) {
        static const std::unordered_map<tamponPh, EnumMetadata> map {
            { tamponPh::TRES_FORT,  { "Très fort",  "Sol très résistant aux variations de pH.",            Theme::PlantGreen } },
            { tamponPh::FORT,       { "Fort",       "Bonne résistance aux changements de pH.",             Theme::PlantMid } },
            { tamponPh::MOYEN,      { "Moyenne",    "Résistance modérée aux variations de pH.",            Theme::withAlpha(Theme::WarningOrange, 0.8f) } },
            { tamponPh::FAIBLE,     { "Faible",     "Sol sensible aux changements de pH.",                 Theme::WarningOrange } },
            { tamponPh::TRES_FAIBLE, { "Très faible", "Sol très sensible, risque de stress pour la plante.",Theme::DangerRed } }
        }; ENUM_MAP_LOOKUP(map, v)
    }
    inline const char* label(tamponPh v) { return get(v).label.c_str(); }

    // Mineralisation
    inline const EnumMetadata& get(Mineralisation v) {
        static const std::unordered_map<Mineralisation, EnumMetadata> map {
            { Mineralisation::RAPIDE,   { "Rapide",   "Nutriments disponibles rapidement.",                Theme::PlantGreen } },
            { Mineralisation::MODEREE,  { "Modérée",  "Libération progressive et équilibrée.",             Theme::PlantMid } },
            { Mineralisation::LENTE,    { "Lente",    "Réserves sur le long terme.",                       Theme::withAlpha(Theme::WarningOrange, 0.8f) } }
        }; ENUM_MAP_LOOKUP(map, v)
    }
    inline const char* label(Mineralisation v) { return get(v).label.c_str(); }

    // Compatibilité au calcaire
    inline const EnumMetadata& get(CompatibiliteCalcaire v) {
        static const std::unordered_map<CompatibiliteCalcaire, EnumMetadata> map {
            { CompatibiliteCalcaire::EXCELLENTE,  { "Excellente",  "Tolère et apprécie les sols calcaires.",       Theme::PlantGreen } },
            { CompatibiliteCalcaire::MOYENNE,     { "Moyenne",     "Tolère une légère présence de calcaire.",      Theme::withAlpha(Theme::WarningOrange, 0.8f) } },
            { CompatibiliteCalcaire::TRES_FAIBLE, { "Très faible", "Le calcaire provoque une chlorose.",           Theme::DangerRed } }
        }; ENUM_MAP_LOOKUP(map, v)
    }
    inline const char* label(CompatibiliteCalcaire v) { return get(v).label.c_str(); }

    // Correction Acidité
    inline const EnumMetadata& get(CorrectionAcidite v) {
        static const std::unordered_map<CorrectionAcidite, EnumMetadata> map {
            { CorrectionAcidite::CHAUX,             { "Chaux",             "Élimine l'acidité du sol.",                         Theme::PhBasique } },
            { CorrectionAcidite::CENDRES,           { "Cendres",           "Légère correction de l'acidité.",                   Theme::darken(Theme::Amber, 0.5f) } },
            { CorrectionAcidite::CALCAIRE,          { "Calcaire",          "Corrige l'acidité et enrichit en calcium.",         Theme::PhBasique } },
            { CorrectionAcidite::EAU_CALCAIRE,      { "Eau calcaire",      "Arrosage avec eau dure pour réduire l'acidité.",    Theme::TextDisabled } },
            { CorrectionAcidite::TERREAU_UNIVERSEL, { "Terreau universel", "Substrat équilibré pour corriger l'acidité.",       Theme::PlantMid } },
            { CorrectionAcidite::COMPOST,           { "Compost",           "Amendement organique qui peut réduire l'acidité.",  Theme::PlantGreen } }
        }; ENUM_MAP_LOOKUP(map, v)
    }
    inline const char* label(CorrectionAcidite v) { return get(v).label.c_str(); }

    // Correction Alcalinité
    inline const EnumMetadata& get(CorrectionAlcalinite v) {
        static const std::unordered_map<CorrectionAlcalinite, EnumMetadata> map {
            { CorrectionAlcalinite::TERRE_BRUYERE,      { "Terre de bruyère",   "Substrat acide pour corriger l'alcalinité.",           Theme::PhAcide } },
            { CorrectionAlcalinite::SOUFRE,             { "Soufre",             "Amendement qui acidifie le sol.",                      Theme::PhAcide } },
            { CorrectionAlcalinite::EAU_PLUIE,          { "Eau de pluie",       "Arrosage avec eau douce pour réduire l'alcalinité.",   Theme::withAlpha(Theme::InfoBlue, 0.8f) } },
            { CorrectionAlcalinite::COMPOST,            { "Compost",            "Amendement organique qui peut acidifier le sol.",      Theme::PlantGreen } },
            { CorrectionAlcalinite::TERREAU_UNIVERSEL,  { "Terreau universel",  "Substrat équilibré pour corriger l'alcalinité.",       Theme::PlantMid } }
        }; ENUM_MAP_LOOKUP(map, v)
    }
    inline const char* label(CorrectionAlcalinite v) { return get(v).label.c_str(); }

    // Composition Sol
    inline const EnumMetadata& get(CompositionSol v) {
        static const std::unordered_map<CompositionSol, EnumMetadata> map {
            { CompositionSol::TOURBE,                   { "Tourbe",                 "Matière organique acide, retient bien l'eau.",     Theme::darken(Theme::Amber, 0.5f) } },
            { CompositionSol::COMPOST,                  { "Compost",                "Matière organique riche en nutriments.",           Theme::PlantGreen } },
            { CompositionSol::FIBRES_VEGETALES,         { "Fibres végétales",       "Amendement organique léger et aéré.",              Theme::PlantMid } },
            { CompositionSol::SABLE,                    { "Sable",                  "Particule minérale drainante.",                    Theme::withAlpha(Theme::Amber, 0.7f) } },
            { CompositionSol::POUZZOLANE,               { "Pouzzolane",             "Gravier volcanique léger et drainant.",            Theme::TextMuted } },
            { CompositionSol::CALCAIRE,                 { "Calcaire",               "Particule minérale alcaline.",                     Theme::PhBasique } },
            { CompositionSol::MATIERE_ORGANIQUE_ACIDE,  { "Matière organique acide","Amendement pour sols acides.",                     Theme::PhAcide } },
            { CompositionSol::COMPOST_ENRICHI,          { "Compost enrichi",        "Compost très riche en nutriments.",                Theme::PlantGreen } },
            { CompositionSol::ENGRAIS_ORGANIQUE,        { "Engrais organique",      "Source concentrée de nutriments naturels.",        Theme::PlantMid } },
            { CompositionSol::ECORCES,                  { "Écorces",                "Amendement organique grossier pour aérer le sol.", Theme::darken(Theme::Amber, 0.5f) } },
            { CompositionSol::ARGILE,                   { "Argile",                 "Particule minérale lourde et collante.",           Theme::darken(Theme::WarningOrange, 0.5f) } }
        }; ENUM_MAP_LOOKUP(map, v)
    }
    inline const char* label(CompositionSol v) { return get(v).label.c_str(); }

    // Risques Sol
    inline const EnumMetadata& get(risquesSol v) {
        static const std::unordered_map<risquesSol, EnumMetadata> map {
            { risquesSol::TASSEMENT,            { "Tassement",           "Risque de sol compacté, réduire les passages.",                            Theme::DangerRed } },
            { risquesSol::APPAUVRISSEMENT,      { "Appauvrissement",     "Perte de nutriments, amendement régulier conseillé.",                      Theme::withAlpha(Theme::WarningOrange, 0.8f) } },
            { risquesSol::COMPACTION,           { "Compaction",          "Risque de sol durci, éviter de travailler à la mauvaise période.",         Theme::DangerRed } },
            { risquesSol::LESSIVAGE,            { "Lessivage",           "Risque de nutriments emportés par les eaux d'arrosage.",                   Theme::withAlpha(Theme::InfoBlue, 0.8f) } },
            { risquesSol::EXCES_AZOTE,          { "Excès d'azote",       "Favorise la croissance au détriment de la floraison.",                     Theme::WarningOrange } },
            { risquesSol::CARENCES,             { "Carences",            "Manque de nutriments essentiels, surveiller la fertilité.",                Theme::DangerRed } },
            { risquesSol::PAUVRETE_LONG_TERME,  { "Pauvreté long terme", "Dégradation progressive du sol, rotation et amendement nécessaires.",      Theme::DangerRed } }
        }; ENUM_MAP_LOOKUP(map, v)
    }
    inline const char* label(risquesSol v) { return get(v).label.c_str(); }
}


// ═════════════════════════════════════════════════════════════════════════════
//  5. RACINES.JSON (Spécifique aux Contenants)
// ═════════════════════════════════════════════════════════════════════════════

enum class FormePot { BAC, COLONNE, JARDINIERE, STANDARD, INCONNU };
enum class TypePot { TERRE_CUITE, PLASTIQUE, CERAMIQUE_EMAILLEE, GEOTEXTILE, VERRE, BOIS, INCONNU };
enum class profondeurPot {FAIBLE, MOYEN, PROFOND, INCONNU};
enum class LargeurPot { ETROIT, MOYEN, LARGE, TRES_LARGE,INCONNU };
enum class FrequenceRempotage { FREQUENT, ANNUEL, BI_ANNUEL, RARE, INCONNU };
enum class SensibiliteRempotage { FAIBLE, MOYENNE, ELEVEE, INCONNU };
enum class BesoinsDrainage { FAIBLE, MOYEN, ELEVEE, ESSENTIEL, INCONNU }; 

namespace EnumInfo {

    // Forme du pot
    inline const EnumMetadata& get(FormePot v) {
        static const std::unordered_map<FormePot, EnumMetadata> map {
            { FormePot::BAC,        { "Bac",         "Grand contenant carré ou rectangulaire.",  Theme::darken(Theme::Amber, 0.5f) } },
            { FormePot::COLONNE,    { "Colonne",     "Pot haut et étroit.",                      Theme::darken(Theme::WarningOrange, 0.5f) } },
            { FormePot::JARDINIERE, { "Jardinière",  "Contenant long et peu profond.",           Theme::PlantMid } },
            { FormePot::STANDARD,   { "Standard",    "Pot classique arrondi.",                   Theme::TextDisabled } }
        }; ENUM_MAP_LOOKUP(map, v)
    }
    inline const char* label(FormePot v) { return get(v).label.c_str(); }

    // Type de pot
    inline const EnumMetadata& get(TypePot v) {
        using T = TypePot;
        static const std::unordered_map<T, EnumMetadata> map {
            { T::TERRE_CUITE,        { "Terre cuite",        "Poreux, favorise l'aération des racines.",     Theme::darken(Theme::Amber, 0.5f) } },
            { T::PLASTIQUE,          { "Plastique",          "Garde l'humidité plus longtemps.",             Theme::withAlpha(Theme::InfoBlue, 0.8f) } },
            { T::CERAMIQUE_EMAILLEE, { "Céramique émaillée", "Esthétique et imperméable.",                   Theme::TextSecondary } },
            { T::GEOTEXTILE,         { "Géotextile",         "Excellente oxygénation (Air-Pruning).",        Theme::TextDisabled } },
            { T::VERRE,              { "Verre",              "Pour terrariums ou bouturage à l'eau.",        Theme::InfoBlue } },
            { T::BOIS,               { "Bois",               "Isolant naturel, aspect rustique.",            Theme::darken(Theme::WarningOrange, 0.5f) } }
        };
        ENUM_MAP_LOOKUP(map, v)
    }
    inline const char* label(TypePot v) { return get(v).label.c_str(); }

    // Profondeur du pot
    inline const EnumMetadata& get(profondeurPot v) {
        using P = profondeurPot;
        static const std::unordered_map<P, EnumMetadata> map {
            { P::FAIBLE,   { "Faible",   "Moins de 20 cm de profondeur.",            Theme::PlantMid } },
            { P::MOYEN,    { "Moyen",    "Entre 20 et 40 cm de profondeur.",         Theme::PlantGreen } },
            { P::PROFOND,  { "Profond",  "Plus de 40 cm de profondeur.",             Theme::withAlpha(Theme::InfoBlue, 0.8f) } }
        };
        ENUM_MAP_LOOKUP(map, v)
    }
    inline const char* label(profondeurPot v) { return get(v).label.c_str(); }

    // Largeur du pot
    inline const EnumMetadata& get(LargeurPot v) {
        using L = LargeurPot;
        static const std::unordered_map<L, EnumMetadata> map {
            { L::ETROIT,      { "Étroit",      "Moins de 20 cm de largeur.",         Theme::PlantMid } },
            { L::MOYEN,       { "Moyen",       "Entre 20 et 40 cm de largeur.",      Theme::PlantGreen } },
            { L::LARGE,       { "Large",       "Entre 40 et 60 cm de largeur.",      Theme::withAlpha(Theme::InfoBlue, 0.8f) } },
            { L::TRES_LARGE,  { "Très large",  "Plus de 60 cm de largeur.",          Theme::InfoBlue } }
        };
        ENUM_MAP_LOOKUP(map, v)
    }
    inline const char* label(LargeurPot v) { return get(v).label.c_str(); }

    // Fréquence de rempotage
    inline const EnumMetadata& get(FrequenceRempotage v) {
        using F = FrequenceRempotage;
        static const std::unordered_map<F, EnumMetadata> map {
            { F::FREQUENT,   { "Fréquent",   "Rempotage recommandé tous les 6 mois.",    Theme::DangerRed } },
            { F::ANNUEL,     { "Annuel",     "Rempotage recommandé chaque année.",       Theme::WarningOrange } },
            { F::BI_ANNUEL,  { "Bi-annuel",  "Rempotage recommandé tous les 2 ans.",     Theme::PlantMid } },
            { F::RARE,       { "Rare",       "Rempotage peu fréquent, tous les 3-5 ans.",Theme::PlantGreen } }
        };
        ENUM_MAP_LOOKUP(map, v)
    }
    inline const char* label(FrequenceRempotage v) { return get(v).label.c_str(); }

    // Sensibilité au rempotage
    inline const EnumMetadata& get(SensibiliteRempotage v) {
        using S = SensibiliteRempotage;
        static const std::unordered_map<S, EnumMetadata> map {
            { S::FAIBLE,     { "Faible",     "Plante robuste, supporte bien le rempotage.",   Theme::PlantGreen } },
            { S::MOYENNE,    { "Moyenne",    "Plante modérément sensible, rempotage à faire avec précaution.", Theme::PlantMid } },
            { S::ELEVEE,     { "Élevée",     "Plante très sensible, rempotage à éviter ou à faire très délicatement.", Theme::DangerRed } }
        };
        ENUM_MAP_LOOKUP(map, v)
    }
    inline const char* label(SensibiliteRempotage v) { return get(v).label.c_str(); }

    // Besoins en drainage
    inline const EnumMetadata& get(BesoinsDrainage v) {
        using B = BesoinsDrainage;
        static const std::unordered_map<B, EnumMetadata> map {
            { B::FAIBLE,     { "Faible",     "Plante tolérante à un drainage limité.",   Theme::PlantMid } },
            { B::MOYEN,      { "Moyen",      "Plante nécessitant un drainage modéré.",   Theme::PlantGreen } },
            { B::ELEVEE,     { "Élevé",      "Plante nécessitant un bon drainage.",      Theme::withAlpha(Theme::InfoBlue, 0.8f) } },
            { B::ESSENTIEL,  { "Essentiel",  "Plante nécessitant un drainage optimal.",  Theme::DangerRed } }
        };
        ENUM_MAP_LOOKUP(map, v)
    }
    inline const char* label(BesoinsDrainage v) { return get(v).label.c_str(); }

}


// ═════════════════════════════════════════════════════════════════════════════
//  6. BOUTURES.JSON (Spécifique à la multiplication)
// ═════════════════════════════════════════════════════════════════════════════

enum class TauxReussite { TRES_ELEVE, ELEVE, BON, MOYEN, VARIABLE, FAIBLE, INCONNU };
enum class TypeTige { HERBACEE, SEMI_LIGNEUSE, LIGNEUSE, FEUILLE, RACINE, INCONNU };
enum class HormoneBouturage { NON, OPTIONNEL, RECOMMANDE, INCONNU };
enum class HumiditeBouture { ELEVEE, MOYENNE, FAIBLE, IMMERGEE, INCONNU };
enum class SubstratBouture { EAU, LEGER, LEGER_DRAINANT, TRES_DRAINANT, MINERAL, INCONNU };
enum class EtapeBouture { COUPER_SOUS_NOEUD, RETRAIT_FEUILLES_BASSES, PLANTER_SUBSTRAT, PLACER_LUMIERE_INDIRECTE, PRELEVER_TIGE_SEMI_RIGIDE, ARROSAGE_LEGER, COUPER_BOIS_DORMANT, PLANTER_ENTERRES, RETIRER_FEUILLES_EAU, CHANGER_EAU_REGULIEREMENT, PRELEVER_FEUILLE, SECHAGE, POSER_SUBSTRAT, COUPER_RACINE, PLANTER_HORIZONTAL, COUVRIR, INCONNU };

namespace EnumInfo {
    // ── Taux de Réussite ──
    inline const EnumMetadata& get(TauxReussite v) {
        using T = TauxReussite;
        static const std::unordered_map<T, EnumMetadata> map {
            { T::TRES_ELEVE, { "Très élevé", "Réussit dans presque tous les cas, même pour les débutants.",  Theme::PlantGreen } },
            { T::ELEVE,      { "Élevé",      "Bonne probabilité de succès avec des conditions correctes.",   Theme::PlantMid } },
            { T::BON,        { "Bon",        "Réussit bien en suivant les étapes recommandées.",             Theme::TextSecondary } },
            { T::MOYEN,      { "Moyen",      "Demande de la patience, plusieurs tentatives parfois utiles.",  Theme::withAlpha(Theme::WarningOrange, 0.8f) } },
            { T::VARIABLE,   { "Variable",   "Résultats inégaux selon la plante et la saison.",              Theme::Amber } },
            { T::FAIBLE,     { "Faible",     "Technique délicate, taux d'échec élevé, réservé aux experts.", Theme::DangerRed } }
        };
        ENUM_MAP_LOOKUP(map, v)
    }
    inline const char* label(TauxReussite v) { return get(v).label.c_str(); }


    // ── Type de Tige ──
    inline const EnumMetadata& get(TypeTige v) {
        using T = TypeTige;
        static const std::unordered_map<T, EnumMetadata> map {
            { T::HERBACEE,      { "Herbacée",      "Tige souple et verte, enracinement rapide (1-3 sem).",       Theme::PlantGreen } },
            { T::SEMI_LIGNEUSE, { "Semi-ligneuse", "Tige partiellement aoûtée, enracinement moyen (1-2 mois).",  Theme::PlantMid } },
            { T::LIGNEUSE,      { "Ligneuse",      "Bois dur dormant, enracinement lent (2-4 mois).",            Theme::darken(Theme::WarningOrange, 0.5f) } },
            { T::FEUILLE,       { "Feuille",       "Propagation par feuille entière, sans tige.",                Theme::TextSecondary } },
            { T::RACINE,        { "Racine",        "Bouture de racine, technique hivernale spécialisée.",        Theme::darken(Theme::Amber, 0.5f) } }
        };
        ENUM_MAP_LOOKUP(map, v)
    }
    inline const char* label(TypeTige v) { return get(v).label.c_str(); }


    // ── Hormone de Bouturage ──
    inline const EnumMetadata& get(HormoneBouturage v) {
        using H = HormoneBouturage;
        static const std::unordered_map<H, EnumMetadata> map {
            { H::NON,        { "Non",         "Hormone inutile, peut même nuire à cette espèce.",             Theme::TextSecondary } },
            { H::OPTIONNEL,  { "Optionnel",   "Améliore légèrement le résultat, non indispensable.",          Theme::withAlpha(Theme::WarningOrange, 0.8f) } },
            { H::RECOMMANDE, { "Recommandé",  "Augmente significativement le taux de réussite.",              Theme::WarningOrange } }
        };
        ENUM_MAP_LOOKUP(map, v)
    }
    inline const char* label(HormoneBouturage v) { return get(v).label.c_str(); }


    // ── Humidité ──
    inline const EnumMetadata& get(HumiditeBouture v) {
        using H = HumiditeBouture;
        static const std::unordered_map<H, EnumMetadata> map {
            { H::ELEVEE,   { "Élevée",   "Brumisation fréquente ou mini-serre recommandée.",        Theme::InfoBlue } },
            { H::MOYENNE,  { "Moyenne",  "Arrosages réguliers, substrat légèrement humide.",        Theme::withAlpha(Theme::InfoBlue, 0.8f) } },
            { H::FAIBLE,   { "Faible",   "Substrat quasi-sec, arrosage très occasionnel.",          Theme::withAlpha(Theme::Amber, 0.7f) } },
            { H::IMMERGEE, { "Immergée", "Tige plongée directement dans l'eau, changée 2×/sem.",    Theme::InfoBlue } }
        };
        ENUM_MAP_LOOKUP(map, v)
    }
    inline const char* label(HumiditeBouture v) { return get(v).label.c_str(); }


    // ── Substrat ──
    inline const EnumMetadata& get(SubstratBouture v) {
        using S = SubstratBouture;
        static const std::unordered_map<S, EnumMetadata> map {
            { S::EAU,            { "Eau",            "Verre d'eau claire, renouveler tous les 2-3 jours.",         Theme::InfoBlue } },
            { S::LEGER,          { "Léger",          "Terreau fin ou terreau de semis peu compacté.",              Theme::TextSecondary } },
            { S::LEGER_DRAINANT, { "Léger drainant", "Mélange terreau + perlite (50/50) ou vermiculite.",          Theme::PlantMid } },
            { S::TRES_DRAINANT,  { "Très drainant",  "Majoritairement sable horticole ou perlite (70%+).",         Theme::withAlpha(Theme::Amber, 0.7f) } },
            { S::MINERAL,        { "Minéral",        "Sable grossier pur ou pouzzolane, sans matière organique.",  Theme::TextMuted } }
        };
        ENUM_MAP_LOOKUP(map, v)
    }
    inline const char* label(SubstratBouture v) { return get(v).label.c_str(); }

    // ── Étapes clés du bouturage ──
    inline const EnumMetadata& get(EtapeBouture v) {
        using E = EtapeBouture;
        static const std::unordered_map<E, EnumMetadata> map {
            { E::COUPER_SOUS_NOEUD,             { "Couper sous nœud",           "Faire une coupe nette juste en dessous d'un nœud pour favoriser l'enracinement.",                       Theme::PlantGreen } },
            { E::RETRAIT_FEUILLES_BASSES,       { "Retrait feuilles basses",    "Enlever les feuilles proches de la base pour éviter la pourriture.",                                    Theme::PlantMid } },
            { E::PLANTER_SUBSTRAT,              { "Planter substrat",           "Enfoncer la tige dans le substrat jusqu'à au moins 2-3 cm de profondeur.",                              Theme::withAlpha(Theme::InfoBlue, 0.8f) } },
            { E::PLACER_LUMIERE_INDIRECTE,      { "Placer lumière indirecte",   "Éviter la lumière directe du soleil qui peut dessécher la bouture.",                                    Theme::InfoBlue } },
            { E::PRELEVER_TIGE_SEMI_RIGIDE,     { "Prélever tige semi-rigide",  "Utiliser une tige semi-ligneuse pour un meilleur taux de réussite.",                                    Theme::darken(Theme::WarningOrange, 0.5f) } },
            { E::ARROSAGE_LEGER,                { "Arrosage léger",             "Maintenir le substrat légèrement humide sans le détremper.",                                            Theme::withAlpha(Theme::Amber, 0.7f) } },
            { E::COUPER_BOIS_DORMANT,           { "Couper bois dormant",        "Utiliser une tige ligneuse prélevée en période de dormance pour certaines espèces.",                    Theme::darken(Theme::Amber, 0.5f) } },
            { E::PLANTER_ENTERRES,              { "Planter enterrés",           "Enfouir une partie de la tige sous le substrat pour favoriser l'enracinement.",                         Theme::TextSecondary } },
            { E::RETIRER_FEUILLES_EAU,          { "Retirer feuilles eau",       "Enlever les feuilles immergées pour éviter la pourriture dans les boutures à l'eau.",                   Theme::InfoBlue } },
            { E::CHANGER_EAU_REGULIEREMENT,     { "Changer eau régulièrement",  "Renouveler l'eau tous les 2-3 jours pour les boutures à l'eau afin d'éviter les algues.",               Theme::withAlpha(Theme::InfoBlue, 0.8f) } },
            { E::PRELEVER_FEUILLE,              { "Prélever feuille",           "Bouturer à partir d'une feuille entière pour les espèces qui le permettent.",                           Theme::TextSecondary } },
            { E::SECHAGE,                       { "Séchage",                    "Laisser sécher la coupe à l'air libre pendant quelques heures avant de planter.",                       Theme::withAlpha(Theme::Amber, 0.7f) } },
            { E::POSER_SUBSTRAT,                { "Poser substrat",             "Placer la bouture sur le substrat sans l'enfoncer pour les espèces sensibles.",                         Theme::PlantMid } },
            { E::COUPER_RACINE,                 { "Couper racine",              "Tailler légèrement les racines avant de rempoter pour stimuler la croissance.",                         Theme::darken(Theme::WarningOrange, 0.5f) } },
            { E::PLANTER_HORIZONTAL,            { "Planter horizontal",         "Placer la tige à l'horizontale sous le substrat pour favoriser l'apparition de nouvelles pousses.",     Theme::PlantGreen } },
            { E::COUVRIR,                       { "Couvrir",                    "Recouvrir la bouture d'un sac plastique ou d'une mini-serre pour maintenir l'humidité.",                Theme::InfoBlue } }
        };
        ENUM_MAP_LOOKUP(map, v)
    }
    inline const char* label(EtapeBouture v) { return get(v).label.c_str(); }

} // namespace EnumInfo

// ═════════════════════════════════════════════════════════════════════════════
//  7. HELPERS GLOBAUX : PARSERS STRING -> ENUM (Optimisés O(1))
//  Utilisés par DataLoader.cpp pour la conversion des JSON.
// ═════════════════════════════════════════════════════════════════════════════

namespace EnumInfo {

    // --- COMMUNS & PLANTES ---
    inline NiveauDifficulte parseDifficulte(const std::string& s) {
        static const std::unordered_map<std::string, NiveauDifficulte> m {
            {"FACILE", NiveauDifficulte::FACILE}, {"MOYEN", NiveauDifficulte::MOYEN}, {"DIFFICILE", NiveauDifficulte::DIFFICILE}};
        auto it = m.find(s); return it != m.end() ? it->second : NiveauDifficulte::INCONNU;
    }

    inline TypeRacinaireEnum parseTypeRacinaire(const std::string& s) {
        static const std::unordered_map<std::string, TypeRacinaireEnum> m {
            {"FASCICULE", TypeRacinaireEnum::FASCICULE}, {"PIVOTANT", TypeRacinaireEnum::PIVOTANT}, 
            {"TRACANT", TypeRacinaireEnum::TRACANT}, {"LIGNEUX", TypeRacinaireEnum::LIGNEUX}, 
            {"TUBEREUX", TypeRacinaireEnum::TUBEREUX}};
        auto it = m.find(s); return it != m.end() ? it->second : TypeRacinaireEnum::INCONNU;
    }

    inline Saison parseSaison(const std::string& s) {
        static const std::unordered_map<std::string, Saison> m {
            {"PRINTEMPS", Saison::PRINTEMPS}, {"ETE", Saison::ETE}, {"AUTOMNE", Saison::AUTOMNE}, 
            {"HIVER", Saison::HIVER}, {"INTERIEUR", Saison::INTERIEUR}};
        auto it = m.find(s); return it != m.end() ? it->second : Saison::INCONNU;
    }

    inline Rusticite parseRusticite(const std::string& s) {
        static const std::unordered_map<std::string, Rusticite> m {
            {"RUSTIQUE", Rusticite::RUSTIQUE}, {"SEMI_RUSTIQUE", Rusticite::SEMI_RUSTIQUE}, {"FRAGILE", Rusticite::FRAGILE}};
        auto it = m.find(s); return it != m.end() ? it->second : Rusticite::INCONNU;
    }

    inline ExpositionSoleil parseExpositionSoleil(const std::string& s) {
        static const std::unordered_map<std::string, ExpositionSoleil> m {
            {"PLEIN_SOLEIL", ExpositionSoleil::PLEIN_SOLEIL}, {"MI_OMBRE", ExpositionSoleil::MI_OMBRE}};
        auto it = m.find(s); return it != m.end() ? it->second : ExpositionSoleil::INCONNU;
    }

    inline ExpositionVent parseExpositionVent(const std::string& s){
        static const std::unordered_map<std::string, ExpositionVent> m {
            {"MODERE", ExpositionVent::MODERE}, {"ABRITE", ExpositionVent::ABRITE}};
        auto it = m.find(s); return it != m.end() ? it->second : ExpositionVent::INCONNU;
    }

    inline VitesseCroissance parseVitesseCroissance(const std::string& s) {
        static const std::unordered_map<std::string, VitesseCroissance> m {
            {"Lente", VitesseCroissance::LENTE}, { "Modérée", VitesseCroissance::MODEREE}, 
            { "Rapide", VitesseCroissance::RAPIDE}, { "Très rapide", VitesseCroissance::TRES_RAPIDE}};
        auto it = m.find(s); return it != m.end() ? it->second : VitesseCroissance::INCONNU;
    }

    inline ToleranceSecheresse parseToleranceSecheresse(const std::string& s) {
        static const std::unordered_map<std::string, ToleranceSecheresse> m {
            {"Excellente", ToleranceSecheresse::EXCELLENTE}, {"Bonne", ToleranceSecheresse::BONNE}, {"Moyenne", ToleranceSecheresse::MOYENNE},
            {"Faible", ToleranceSecheresse::FAIBLE}, {"Très faible", ToleranceSecheresse::TRES_FAIBLE}};
        auto it = m.find(s); return it != m.end() ? it->second : ToleranceSecheresse::INCONNU;
    }

    inline SensibiliteEau parseSensibiliteEau(const std::string& s) {
        static const std::unordered_map<std::string, SensibiliteEau> m {
            {"Très élevée", SensibiliteEau::TRES_ELEVEE}, {"Élevée", SensibiliteEau::ELEVEE}, 
            {"Moyenne", SensibiliteEau::MOYENNE}, {"Faible", SensibiliteEau::FAIBLE}};
        auto it = m.find(s); return it != m.end() ? it->second : SensibiliteEau::INCONNU;
    }

    inline TypeFeuillage parseTypeFeuillage(const std::string& s) {
        static const std::unordered_map<std::string, TypeFeuillage> m {
            {"Persistant", TypeFeuillage::PERSISTANT}, {"Caduc", TypeFeuillage::CADUC}, {"Caduc (perd ses feuilles en hiver)", TypeFeuillage::CADUC}, 
            {"Caduc (disparaît totalement en hiver)", TypeFeuillage::CADUC}, {"Semi-persistant", TypeFeuillage::SEMI_PERSISTANT}, 
            {"Caduc à semi-persistant", TypeFeuillage::SEMI_PERSISTANT}, {"Persistant à semi-persistant", TypeFeuillage::SEMI_PERSISTANT},
            {"Annuel", TypeFeuillage::ANNUEL}, {"Bisannuel", TypeFeuillage::BISANNUEL}, {"Bisannuel (cultivé comme annuel)", TypeFeuillage::BISANNUEL}};
        auto it = m.find(s); return it != m.end() ? it->second : TypeFeuillage::INCONNU;
    }

    inline TypeBouture parseTypeBouture(const std::string& s) {
        static const std::unordered_map<std::string, TypeBouture> m {
            {"BOUTURE_HERBACEE", TypeBouture::BOUTURE_HERBACEE}, {"BOUTURE_SEMI_LIGNEUSE", TypeBouture::BOUTURE_SEMI_LIGNEUSE}, 
            {"BOUTURE_LIGNEUSE", TypeBouture::BOUTURE_LIGNEUSE}, {"BOUTURE_EAU", TypeBouture::BOUTURE_EAU}, 
            {"BOUTURE_FEUILLE", TypeBouture::BOUTURE_FEUILLE}, {"BOUTURE_RACINE", TypeBouture::BOUTURE_RACINE}};
        auto it = m.find(s); return it != m.end() ? it->second : TypeBouture::INCONNU;
    }

    // --- SOLS.JSON (Nouveaux parsers générés) ---
    inline TypeSol typeSolFromString(const std::string& s) {
        static const std::unordered_map<std::string, TypeSol> m {
            {"TERREAU_UNIVERSEL", TypeSol::TERREAU_UNIVERSEL}, {"TERREAU_MEDITERRANEEN", TypeSol::TERREAU_MEDITERRANEEN},
            {"TERRE_BRUYERE", TypeSol::TERRE_BRUYERE}, {"TERREAU_HORTICOLE_ENRICHI", TypeSol::TERREAU_HORTICOLE_ENRICHI},
            {"SUBSTRAT_AGRUMES", TypeSol::SUBSTRAT_AGRUMES}, {"TERREAU_SEMIS", TypeSol::TERREAU_SEMIS},
            {"SOL_ARGILEUX", TypeSol::SOL_ARGILEUX}, {"SOL_SABLEUX", TypeSol::SOL_SABLEUX}};
        auto it = m.find(s); return it != m.end() ? it->second : TypeSol::INCONNU;
    }

    inline TextureSol parseTexture(const std::string& s) {
        static const std::unordered_map<std::string, TextureSol> m {
            {"EQUILIBREE", TextureSol::EQUILIBREE}, {"LEGER_CAILLOUTEUX", TextureSol::LEGER_CAILLOUTEUX},
            {"LEGERE", TextureSol::LEGERE}, {"SOUPLE", TextureSol::SOUPLE}, {"AERE", TextureSol::AERE},
            {"TRES_FINE", TextureSol::TRES_FINE}, {"LOURDE", TextureSol::LOURDE}, {"TRES_LEGER", TextureSol::TRES_LEGER}};
        auto it = m.find(s); return it != m.end() ? it->second : TextureSol::INCONNU;
    }

    inline DensiteSol parseDensite(const std::string& s){
        static const std::unordered_map<std::string, DensiteSol> m {
            {"TRES_LEGERE", DensiteSol::TRES_LEGERE}, {"LEGERE", DensiteSol::LEGERE}, {"MOYENNE", DensiteSol::MOYENNE},
            {"LOURDE", DensiteSol::LOURDE}, {"TRES_LOURDE", DensiteSol::TRES_LOURDE}};
        auto it = m.find(s); return it!= m.end() ? it->second : DensiteSol::INCONNU;
    }

    inline DrainageSol parseDrainage(const std::string& s) {
        static const std::unordered_map<std::string, DrainageSol> m {
            {"MOYEN", DrainageSol::MOYEN}, {"EXCELLENT", DrainageSol::EXCELLENT}, {"BON", DrainageSol::BON},
            {"TRES_BON", DrainageSol::TRES_BON}, {"FAIBLE", DrainageSol::FAIBLE}, {"TRES_RAPIDE", DrainageSol::TRES_RAPIDE}};
        auto it = m.find(s); return it != m.end() ? it->second : DrainageSol::INCONNU;
    }

    inline RetentionEau parseRetentionEau(const std::string& s) {
        static const std::unordered_map<std::string, RetentionEau> m {
            {"MOYENNE", RetentionEau::MOYENNE}, {"FAIBLE", RetentionEau::FAIBLE}, 
            {"BONNE", RetentionEau::BONNE}, {"TRES_FORTE", RetentionEau::TRES_FORTE}};
        auto it = m.find(s); return it != m.end() ? it->second : RetentionEau::INCONNU;
    }

    inline RichesseSol parseRichesse(const std::string& s) {
        static const std::unordered_map<std::string, RichesseSol> m {
            {"MOYENNE", RichesseSol::MOYENNE}, {"FAIBLE", RichesseSol::FAIBLE}, 
            {"ELEVEE", RichesseSol::ELEVEE}, {"MOYENNE_ELEVEE", RichesseSol::MOYENNE_ELEVEE}};
        auto it = m.find(s); return it != m.end() ? it->second : RichesseSol::INCONNU;
    }

    inline CecSol parseCec(const std::string& s) {
        static const std::unordered_map<std::string, CecSol> m {
            {"MOYENNE", CecSol::MOYENNE}, {"FAIBLE", CecSol::FAIBLE}, {"ELEVEE", CecSol::ELEVEE}, 
            {"TRES_ELEVEE", CecSol::TRES_ELEVEE}, {"TRES_FAIBLE", CecSol::TRES_FAIBLE}};
        auto it = m.find(s); return it != m.end() ? it->second : CecSol::INCONNU;
    }

    inline AerationSol parseAeration(const std::string& s) {
        static const std::unordered_map<std::string, AerationSol> m {
            {"MOYENNE", AerationSol::MOYENNE}, {"EXCELLENTE", AerationSol::EXCELLENTE}, {"BONNE", AerationSol::BONNE}, 
            {"TRES_ELEVEE", AerationSol::TRES_ELEVEE}, {"TRES_FAIBLE", AerationSol::TRES_FAIBLE}};
        auto it = m.find(s); return it != m.end() ? it->second : AerationSol::INCONNU;
    }

    inline tamponPh parseTamponPh(const std::string& s) {
        static const std::unordered_map<std::string, tamponPh> m {
            {"TRES_FORT", tamponPh::TRES_FORT}, {"FORT", tamponPh::FORT}, {"MOYEN", tamponPh::MOYEN},
            {"FAIBLE", tamponPh::FAIBLE}, {"TRES_FAIBLE", tamponPh::TRES_FAIBLE}};
        auto it = m.find(s); return it != m.end() ? it->second : tamponPh::INCONNU;
    }

    inline VieMicrobienne parseVieMicrobienne(const std::string& s) {
        static const std::unordered_map<std::string, VieMicrobienne> m {
            {"MOYENNE", VieMicrobienne::MOYENNE}, {"FAIBLE", VieMicrobienne::FAIBLE}, {"TRES_ELEVEE", VieMicrobienne::TRES_ELEVEE}};
        auto it = m.find(s); return it != m.end() ? it->second : VieMicrobienne::INCONNU;
    }

    inline Mineralisation parseMineralisation(const std::string& s) {
        static const std::unordered_map<std::string, Mineralisation> m {
            {"MODEREE", Mineralisation::MODEREE}, {"LENTE", Mineralisation::LENTE}, {"RAPIDE", Mineralisation::RAPIDE}};
        auto it = m.find(s); return it != m.end() ? it->second : Mineralisation::INCONNU;
    }

    inline CompatibiliteCalcaire parseCompatibiliteCalcaire(const std::string& s) {
        static const std::unordered_map<std::string, CompatibiliteCalcaire> m {
            {"MOYENNE", CompatibiliteCalcaire::MOYENNE}, {"EXCELLENTE", CompatibiliteCalcaire::EXCELLENTE}, {"TRES_FAIBLE", CompatibiliteCalcaire::TRES_FAIBLE}};
        auto it = m.find(s); return it != m.end() ? it->second : CompatibiliteCalcaire::INCONNU;
    }

    inline CorrectionAcidite parseCorrectionAcidite(const std::string& s) {
        static const std::unordered_map<std::string, CorrectionAcidite> m {
            {"CHAUX", CorrectionAcidite::CHAUX}, {"CENDRES", CorrectionAcidite::CENDRES}, {"CALCAIRE", CorrectionAcidite::CALCAIRE},
            {"EAU_CALCAIRE", CorrectionAcidite::EAU_CALCAIRE}, {"TERREAU_UNIVERSEL", CorrectionAcidite::TERREAU_UNIVERSEL}, {"COMPOST", CorrectionAcidite::COMPOST}};
        auto it = m.find(s); return it != m.end() ? it->second : CorrectionAcidite::INCONNU;
    }

    inline CorrectionAlcalinite parseCorrectionAlcalinite(const std::string& s) {
        static const std::unordered_map<std::string, CorrectionAlcalinite> m {
            {"TERRE_BRUYERE", CorrectionAlcalinite::TERRE_BRUYERE}, {"SOUFRE", CorrectionAlcalinite::SOUFRE}, {"EAU_PLUIE", CorrectionAlcalinite::EAU_PLUIE},
            {"COMPOST", CorrectionAlcalinite::COMPOST}, {"TERREAU_UNIVERSEL", CorrectionAlcalinite::TERREAU_UNIVERSEL}, {"TERRE_BRUYERE", CorrectionAlcalinite::TERRE_BRUYERE}};
        auto it = m.find(s); return it != m.end() ? it->second : CorrectionAlcalinite::INCONNU;
    }

    inline CompositionSol parseCompositionSol(const std::string& s) {
        static const std::unordered_map<std::string, CompositionSol> m {
            {"TOURBE", CompositionSol::TOURBE}, {"COMPOST", CompositionSol::COMPOST}, {"FIBRES_VEGETALES", CompositionSol::FIBRES_VEGETALES},
            {"SABLE", CompositionSol::SABLE}, {"POUZZOLANE", CompositionSol::POUZZOLANE}, {"CALCAIRE", CompositionSol::CALCAIRE},
            {"MATIERE_ORGANIQUE_ACIDE", CompositionSol::MATIERE_ORGANIQUE_ACIDE}, {"COMPOST_ENRICHI", CompositionSol::COMPOST_ENRICHI},
            {"ENGRAIS_ORGANIQUE", CompositionSol::ENGRAIS_ORGANIQUE}, {"ECORCES", CompositionSol::ECORCES}, {"ARGILE", CompositionSol::ARGILE}};
        auto it = m.find(s); return it != m.end() ? it->second : CompositionSol::INCONNU;
    }

    inline risquesSol parseRisquesSol(const std::string& s) {
        static const std::unordered_map<std::string, risquesSol> m {
            {"TASSEMENT", risquesSol::TASSEMENT}, {"APPAUVRISSEMENT", risquesSol::APPAUVRISSEMENT}, {"COMPACTION", risquesSol::COMPACTION},
            {"LESSIVAGE", risquesSol::LESSIVAGE}, {"EXCES_AZOTE", risquesSol::EXCES_AZOTE}, {"CARENCES", risquesSol::CARENCES},
            {"PAUVRETE_LONG_TERME", risquesSol::PAUVRETE_LONG_TERME}};
        auto it = m.find(s); return it != m.end() ? it->second : risquesSol::INCONNU;
    }

    // --- RACINES.JSON ---
    inline FormePot parseFormePot(const std::string& s) {
        static const std::unordered_map<std::string, FormePot> m {
            {"BAC", FormePot::BAC}, {"COLONNE", FormePot::COLONNE}, {"JARDINIERE", FormePot::JARDINIERE}, {"STANDARD", FormePot::STANDARD}};
        auto it = m.find(s); return it != m.end() ? it->second : FormePot::INCONNU;
    }

    inline TypePot parseTypePot(const std::string& s) {
        static const std::unordered_map<std::string, TypePot> m {
            {"TERRE_CUITE", TypePot::TERRE_CUITE}, {"PLASTIQUE", TypePot::PLASTIQUE}, 
            {"BOIS", TypePot::BOIS}, {"GEOTEXTILE", TypePot::GEOTEXTILE}};
        auto it = m.find(s); return it != m.end() ? it->second : TypePot::INCONNU;
    }

    inline profondeurPot parseProfondeurPot(const std::string& s) {
        static const std::unordered_map<std::string, profondeurPot> m {
            {"FAIBLE", profondeurPot::FAIBLE}, {"MOYEN", profondeurPot::MOYEN}, {"PROFOND", profondeurPot::PROFOND}};
        auto it = m.find(s); return it != m.end() ? it->second : profondeurPot::INCONNU;
    }

    inline LargeurPot parseLargeurPot(const std::string& s) {
        static const std::unordered_map<std::string, LargeurPot> m {
            {"ETROIT", LargeurPot::ETROIT}, {"MOYEN", LargeurPot::MOYEN}, {"LARGE", LargeurPot::LARGE}, {"TRES_LARGE", LargeurPot::TRES_LARGE}};
        auto it = m.find(s); return it != m.end() ? it->second : LargeurPot::INCONNU;
    }

    inline FrequenceRempotage parseFrequenceRempotage(const std::string& s) {
        static const std::unordered_map<std::string, FrequenceRempotage> m {
            {"FREQUENT", FrequenceRempotage::FREQUENT}, {"ANNUEL", FrequenceRempotage::ANNUEL}, 
            {"BI_ANNUEL", FrequenceRempotage::BI_ANNUEL}, {"RARE", FrequenceRempotage::RARE}};
        auto it = m.find(s); return it != m.end() ? it->second : FrequenceRempotage::INCONNU;
    }

    inline SensibiliteRempotage parseSensibiliteRempotage(const std::string& s) {
        static const std::unordered_map<std::string, SensibiliteRempotage> m {
            {"FAIBLE", SensibiliteRempotage::FAIBLE}, {"MOYENNE", SensibiliteRempotage::MOYENNE}, {"ELEVEE", SensibiliteRempotage::ELEVEE}};
        auto it = m.find(s); return it != m.end() ? it->second : SensibiliteRempotage::INCONNU;
    }

    inline BesoinsDrainage parseBesoinsDrainage(const std::string& s) {
        static const std::unordered_map<std::string, BesoinsDrainage> m {
            {"FAIBLE", BesoinsDrainage::FAIBLE}, {"MOYEN", BesoinsDrainage::MOYEN}, {"ELEVEE", BesoinsDrainage::ELEVEE}, {"ESSENTIEL", BesoinsDrainage::ESSENTIEL}};
        auto it = m.find(s); return it != m.end() ? it->second : BesoinsDrainage::INCONNU;
    }

    // --- BOUTURES.JSON ---
    inline TauxReussite parseTauxReussite(const std::string& s) {
        static const std::unordered_map<std::string, TauxReussite> m {
            {"TRES_ELEVE", TauxReussite::TRES_ELEVE}, {"ELEVE", TauxReussite::ELEVE}, {"BON", TauxReussite::BON},
            {"MOYEN", TauxReussite::MOYEN}, {"VARIABLE", TauxReussite::VARIABLE}, {"FAIBLE", TauxReussite::FAIBLE}};
        auto it = m.find(s); return it != m.end() ? it->second : TauxReussite::INCONNU;
    }

    inline TypeTige parseTypeTige(const std::string& s) {
        static const std::unordered_map<std::string, TypeTige> m {
            {"HERBACEE", TypeTige::HERBACEE}, {"SEMI_LIGNEUSE", TypeTige::SEMI_LIGNEUSE}, 
            {"LIGNEUSE", TypeTige::LIGNEUSE}, {"FEUILLE", TypeTige::FEUILLE}, {"RACINE", TypeTige::RACINE}};
        auto it = m.find(s); return it != m.end() ? it->second : TypeTige::INCONNU;
    }

    inline HormoneBouturage parseHormone(const std::string& s) {
        static const std::unordered_map<std::string, HormoneBouturage> m {
            {"NON", HormoneBouturage::NON}, {"OPTIONNEL", HormoneBouturage::OPTIONNEL}, {"RECOMMANDE", HormoneBouturage::RECOMMANDE}};
        auto it = m.find(s); return it != m.end() ? it->second : HormoneBouturage::INCONNU;
    }

    inline HumiditeBouture parseHumidite(const std::string& s) {
        static const std::unordered_map<std::string, HumiditeBouture> m {
            {"ELEVEE", HumiditeBouture::ELEVEE}, {"MOYENNE", HumiditeBouture::MOYENNE}, 
            {"FAIBLE", HumiditeBouture::FAIBLE}, {"IMMERGEE", HumiditeBouture::IMMERGEE}};
        auto it = m.find(s); return it != m.end() ? it->second : HumiditeBouture::INCONNU;
    }

    inline SubstratBouture parseSubstrat(const std::string& s) {
        static const std::unordered_map<std::string, SubstratBouture> m {
            {"EAU", SubstratBouture::EAU}, {"LEGER", SubstratBouture::LEGER}, {"LEGER_DRAINANT", SubstratBouture::LEGER_DRAINANT},
            {"TRES_DRAINANT", SubstratBouture::TRES_DRAINANT}, {"MINERAL", SubstratBouture::MINERAL}};
        auto it = m.find(s); return it != m.end() ? it->second : SubstratBouture::INCONNU;
    }

    inline EtapeBouture parseEtape(const std::string& s) { 
        static const std::unordered_map<std::string, EtapeBouture> m {
            {"COUPER_SOUS_NOEUD", EtapeBouture::COUPER_SOUS_NOEUD}, {"RETRAIT_FEUILLES_BASSES", EtapeBouture::RETRAIT_FEUILLES_BASSES},
            {"PLANTER_SUBSTRAT", EtapeBouture::PLANTER_SUBSTRAT}, {"PLACER_LUMIERE_INDIRECTE", EtapeBouture::PLACER_LUMIERE_INDIRECTE},
            {"PRELEVER_TIGE_SEMI_RIGIDE", EtapeBouture::PRELEVER_TIGE_SEMI_RIGIDE}, {"ARROSAGE_LEGER", EtapeBouture::ARROSAGE_LEGER},
            {"COUPER_BOIS_DORMANT", EtapeBouture::COUPER_BOIS_DORMANT}, {"PLANTER_ENTERRES", EtapeBouture::PLANTER_ENTERRES},
            {"RETIRER_FEUILLES_EAU", EtapeBouture::RETIRER_FEUILLES_EAU}, {"CHANGER_EAU_REGULIEREMENT", EtapeBouture::CHANGER_EAU_REGULIEREMENT},
            {"PRELEVER_FEUILLE", EtapeBouture::PRELEVER_FEUILLE}, {"SECHAGE", EtapeBouture::SECHAGE},
            {"POSER_SUBSTRAT", EtapeBouture::POSER_SUBSTRAT}, {"COUPER_RACINE", EtapeBouture::COUPER_RACINE},
            {"PLANTER_HORIZONTAL", EtapeBouture::PLANTER_HORIZONTAL}, {"COUVRIR", EtapeBouture::COUVRIR}};
        auto it = m.find(s); return it != m.end() ? it->second : EtapeBouture::INCONNU;
    }

} // namespace EnumInfo

#undef ENUM_MAP_LOOKUP