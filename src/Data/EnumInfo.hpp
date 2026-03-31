#pragma once
#include <string>
#include <unordered_map>
#include <imgui.h>

// ═════════════════════════════════════════════════════════════════════════════
//  EnumInfo.hpp — Source unique de vérité pour tous les enums de Plantasia
//
//  SOMMAIRE DE L'ARCHITECTURE :
//  Ce fichier fait le pont entre les données textuelles (JSON) et l'UI (ImGui).
//  
//  1. PALETTE & METADATA : Définition des couleurs et de la structure de base.
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
//  1. PALETTE & STRUCTURES DE BASE
// ─────────────────────────────────────────────────────────────────────────────

namespace Palette {
    // Verts (identité du projet)
    constexpr ImVec4 VertVif     = { 0.20f, 0.75f, 0.40f, 1.f };   // Accent
    constexpr ImVec4 VertDoux    = { 0.15f, 0.55f, 0.30f, 1.f };   // AccentDoux
    constexpr ImVec4 VertSombre  = { 0.10f, 0.35f, 0.18f, 1.f };   // AccentSombre
    constexpr ImVec4 VertPale    = { 0.55f, 0.68f, 0.55f, 1.f };   // TexteDoux

    // Sémantiques
    constexpr ImVec4 Rouge       = { 0.75f, 0.22f, 0.22f, 1.f };   // Danger
    constexpr ImVec4 RougeDoux   = { 0.85f, 0.38f, 0.30f, 1.f };
    constexpr ImVec4 Orange      = { 0.85f, 0.52f, 0.12f, 1.f };
    constexpr ImVec4 OrangeDoux  = { 0.90f, 0.65f, 0.25f, 1.f };
    constexpr ImVec4 Jaune       = { 0.88f, 0.80f, 0.20f, 1.f };
    constexpr ImVec4 BleuCiel    = { 0.30f, 0.60f, 0.90f, 1.f };
    constexpr ImVec4 BleuDoux    = { 0.40f, 0.68f, 0.85f, 1.f };
    constexpr ImVec4 Violet      = { 0.58f, 0.35f, 0.75f, 1.f };
    constexpr ImVec4 Rose        = { 0.85f, 0.42f, 0.62f, 1.f };
    constexpr ImVec4 Brun        = { 0.55f, 0.35f, 0.18f, 1.f };
    constexpr ImVec4 BrunClair   = { 0.72f, 0.52f, 0.30f, 1.f };
    constexpr ImVec4 Sable       = { 0.80f, 0.72f, 0.45f, 1.f };
    constexpr ImVec4 Gris        = { 0.50f, 0.50f, 0.50f, 1.f };
    constexpr ImVec4 GrisClair   = { 0.68f, 0.68f, 0.68f, 1.f };
    constexpr ImVec4 Blanc       = { 0.88f, 0.92f, 0.88f, 1.f };   // Texte
}

// ─────────────────────────────────────────────────────────────────────────────
//  Structure de métadonnée
// ─────────────────────────────────────────────────────────────────────────────

struct EnumMetadata {
    std::string label;        // Court, affiché dans les listes et tags
    std::string description;  // Phrase complète, utilisée dans les tooltips
    ImVec4      color;        // Couleur sémantique pour tag / icône
};

// ─────────────────────────────────────────────────────────────────────────────
//  Fallback global (jamais affiché en production si les enums sont complets)
// ─────────────────────────────────────────────────────────────────────────────

inline const EnumMetadata& unknownMetadata() {
    static EnumMetadata u{ "?", "Valeur inconnue", Palette::Gris };
    return u;
}

// ─────────────────────────────────────────────────────────────────────────────
//  Macro helper — évite la répétition dans les fonctions get()
// ─────────────────────────────────────────────────────────────────────────────

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
            { N::FACILE,    { "Facile",    "Convient aux débutants, pardonne facilement les erreurs.",  Palette::VertVif   } },
            { N::MOYEN,     { "Moyen",     "Demande un peu d'attention et de régularité.",              Palette::OrangeDoux} },
            { N::DIFFICILE, { "Difficile", "Exige de l'expérience et des conditions précises.",         Palette::Rouge     } }
        };
        ENUM_MAP_LOOKUP(map, v)
    }
    inline const char* label(NiveauDifficulte v) { return get(v).label.c_str(); }

    // Type Racinaire
    inline const EnumMetadata& get(TypeRacinaireEnum v) {
        using T = TypeRacinaireEnum;
        static const std::unordered_map<T, EnumMetadata> map {
            { T::FASCICULE, { "Fasciculé", "Racines fines et denses en surface.",                        Palette::VertDoux  } },
            { T::PIVOTANT,  { "Pivotant",  "Racine principale profonde.",                                Palette::Brun      } },
            { T::TRACANT,   { "Traçant",   "Stolons horizontaux envahissants.",                          Palette::OrangeDoux} },
            { T::LIGNEUX,   { "Ligneux",   "Système racinaire fort et permanent.",                       Palette::BrunClair } },
            { T::TUBEREUX,  { "Tubéreux",  "Racines renflées stockant des réserves (tubercules).",       Palette::Rose      } }
        };
        ENUM_MAP_LOOKUP(map, v)
    }
    inline const char* label(TypeRacinaireEnum v) { return get(v).label.c_str(); }

    // Saison
    inline const EnumMetadata& get(Saison v) {
        using S = Saison;
        static const std::unordered_map<S, EnumMetadata> map {
            { S::PRINTEMPS, { "Printemps", "Mars à mai — montée de sève.",                             Palette::VertVif   } },
            { S::ETE,       { "Été",       "Juin à août — pleine croissance.",                         Palette::Jaune     } },
            { S::AUTOMNE,   { "Automne",   "Septembre à novembre — ralentissement.",                   Palette::Orange    } },
            { S::HIVER,     { "Hiver",     "Décembre à février — dormance.",                           Palette::BleuCiel  } },
            { S::INTERIEUR, { "Intérieur", "Possible toute l'année en intérieur chauffé.",             Palette::Violet    } }
        };
        ENUM_MAP_LOOKUP(map, v)
    }
    inline const char* label(Saison v) { return get(v).label.c_str(); }

    // Mois (Spécial, basé sur int 1-12)
    struct MoisMetadata { const char* court; const char* long_; ImVec4 color; };
    inline const MoisMetadata& getMois(int mois) {
        static const MoisMetadata table[13] = {
            { "?",   "Inconnu",    Palette::Gris      },
            { "Jan", "Janvier",    Palette::BleuCiel  }, { "Fév", "Février",    Palette::BleuDoux   },
            { "Mar", "Mars",       Palette::VertPale  }, { "Avr", "Avril",      Palette::VertDoux   },
            { "Mai", "Mai",        Palette::VertVif   }, { "Juin","Juin",       Palette::Jaune      },
            { "Juil","Juillet",    Palette::Orange    }, { "Août","Août",       Palette::OrangeDoux },
            { "Sep", "Septembre",  Palette::BrunClair }, { "Oct", "Octobre",    Palette::Brun       },
            { "Nov", "Novembre",   Palette::GrisClair }, { "Déc", "Décembre",   Palette::BleuCiel   }
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

namespace EnumInfo {

    // Rusticité
    inline const EnumMetadata& get(Rusticite v) {
        using R = Rusticite;
        static const std::unordered_map<R, EnumMetadata> map {
            { R::RUSTIQUE,      { "Rustique",      "Résiste au gel et aux hivers rigoureux.",          Palette::VertVif   } },
            { R::SEMI_RUSTIQUE, { "Semi-rustique", "Supporte des gelées légères, à protéger.",         Palette::OrangeDoux} },
            { R::FRAGILE,       { "Fragile",       "Craint le moindre gel, rentrée indispensable.",    Palette::Rouge     } }
        };
        ENUM_MAP_LOOKUP(map, v)
    }
    inline const char* label(Rusticite v) { return get(v).label.c_str(); }

    // Exposition au soleil
    inline const EnumMetadata& get(ExpositionSoleil v) {
        using E = ExpositionSoleil;
        static const std::unordered_map<E, EnumMetadata> map {
            { E::PLEIN_SOLEIL, { "Plein soleil", "Nécessite au minimum 6h de soleil direct/jour.",    Palette::Jaune    } },
            { E::MI_OMBRE,     { "Mi-ombre",     "Préfère 3 à 5h de soleil, supporte l'ombre.",       Palette::VertDoux } }
        };
        ENUM_MAP_LOOKUP(map, v)
    }
    inline const char* label(ExpositionSoleil v) { return get(v).label.c_str(); }

    // Exposition au vent
    inline const EnumMetadata& get(ExpositionVent v) {
        using E = ExpositionVent;
        static const std::unordered_map<E, EnumMetadata> map {
            { E::MODERE, { "Vent modéré", "Tolère une exposition venteuse normale.",                  Palette::BleuDoux  } },
            { E::ABRITE, { "Abrité",      "Fragile au vent, placer à l'abri d'un mur.",               Palette::BleuCiel  } }
        };
        ENUM_MAP_LOOKUP(map, v)
    }
    inline const char* label(ExpositionVent v) { return get(v).label.c_str(); }

    // Besoin en eau (Spécial, basé sur int 0-5)
    struct BesoinEauMetadata { const char* label; const char* description; ImVec4 color; int gouttes; };
    inline const BesoinEauMetadata& getBesoinEau(int niveau) {
        static const BesoinEauMetadata table[6] = {
            { "Nul",         "Arrosage quasi-inexistant.",                               Palette::Sable,     0 },
            { "Très faible", "Arrosage très rare, forte sécheresse.",                    Palette::OrangeDoux,1 },
            { "Faible",      "Arrosage peu fréquent, tolère la sécheresse.",             Palette::Jaune,     2 },
            { "Moyen",       "Arrosage régulier, laisser sécher entre deux.",            Palette::VertPale,  3 },
            { "Élevé",       "Arrosage fréquent, substrat légèrement humide.",           Palette::BleuDoux,  4 },
            { "Très élevé",  "Arrosage très régulier, craint la sécheresse.",            Palette::BleuCiel,  5 }
        };
        return (niveau >= 0 && niveau <= 5) ? table[niveau] : table[0];
    }
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
enum class CorrectionAlcalinite { TERRE_BRUYERE, SOUFRE, EAU_PLUIE, COMPOST, TERREAU_UNIVERSEL, TERRE_BRUYERE, INCONNU };
enum class CompositionSol { TOURBE, COMPOST, FIBRES_VEGETALES, SABLE, POUZZOLANE, CALCAIRE, MATIERE_ORGANIQUE_ACIDE, COMPOST_ENRICHI, ENGRAIS_ORGANIQUE, ECORCES, ARGILE, INCONNU };
enum class risquesSol { TASSEMENT, APPAUVRISSEMENT, COMPACTION, LESSIVAGE, EXCES_AZOTE, CARENCES, PAUVRETE_LONG_TERME, INCONNU };
enum class frequenceRenouvellement { TOUS_1_2_ANS, TOUS_2_3_ANS, TOUS_3_5_ANS, TOUS_5_10_ANS, TOUS_10_20_ANS, JAMAIS, INCONNU };
enum class compatibiliteCalcaire { EXCELLENTE, MOYENNE, TRES_FAIBLE, INCONNU };

namespace EnumInfo {

    // Type Sol
    inline const EnumMetadata& get(TypeSol v) {
        static const std::unordered_map<TypeSol, EnumMetadata> map {
            { TypeSol::TERREAU_UNIVERSEL,         { "Universel",      "Polyvalent pour aromatiques.",      Palette::VertDoux  } },
            { TypeSol::TERREAU_MEDITERRANEEN,     { "Méditerranéen",  "Très drainant, pauvre.",            Palette::OrangeDoux} },
            { TypeSol::TERRE_BRUYERE,             { "Terre Bruyère",  "Sol acide.",                        Palette::Violet    } },
            { TypeSol::TERREAU_HORTICOLE_ENRICHI, { "Horticole",      "Très fertile, gourmandes.",         Palette::VertVif   } },
            { TypeSol::SUBSTRAT_AGRUMES,          { "Agrumes",        "Spécial Yuzu/Agrumes.",             Palette::Jaune     } },
            { TypeSol::TERREAU_SEMIS,             { "Semis",          "Très fin et stérile.",              Palette::GrisClair } },
            { TypeSol::SOL_ARGILEUX,              { "Argileux",       "Lourd et fertile.",                 Palette::Brun      } },
            { TypeSol::EPIPHYTE,                  { "Épiphyte",       "Mélange d'écorces et sphaigne.",    Palette::BrunClair } },
            { TypeSol::DRAINANT,                  { "Drainant",       "Spécial succulentes et cactées.",   Palette::Gris      } },
            { TypeSol::SOL_SABLEUX,               { "Sableux",        "Très drainant.",                    Palette::Sable     } }
        }; ENUM_MAP_LOOKUP(map, v)
    }
    inline const char* label(TypeSol v) { return get(v).label.c_str(); }

    // Texture Sol
    inline const EnumMetadata& get(TextureSol v) {
        static const std::unordered_map<TextureSol, EnumMetadata> map {
            { TextureSol::TRES_LEGER,        { "Très léger",        "Majoritairement sableux.",                Palette::Sable     } },
            { TextureSol::LEGER_CAILLOUTEUX, { "Caillouteux",       "Mélange sable + graviers.",               Palette::BrunClair } },
            { TextureSol::LEGERE,            { "Légère",            "Sol meuble et aéré.",                     Palette::VertPale  } },
            { TextureSol::AERE,              { "Aéré",              "Structure ouverte.",                      Palette::BleuDoux  } },
            { TextureSol::SOUPLE,            { "Souple",            "Ni trop compacte ni trop meuble.",        Palette::VertDoux  } },
            { TextureSol::EQUILIBREE,        { "Équilibrée",        "Bonne structure limon-argile-sable.",     Palette::VertVif   } },
            { TextureSol::TRES_FINE,         { "Très fine",         "Idéale pour la germination.",             Palette::GrisClair } },
            { TextureSol::LOURDE,            { "Lourde",            "Sol argileux collant.",                   Palette::Brun      } }
        }; ENUM_MAP_LOOKUP(map, v)
    }
    inline const char* label(TextureSol v) { return get(v).label.c_str(); }

    // Drainage Sol
    inline const EnumMetadata& get(DrainageSol v) {
        static const std::unordered_map<DrainageSol, EnumMetadata> map {
            { DrainageSol::TRES_RAPIDE, { "Très rapide", "L'eau traverse immédiatement.",                 Palette::Sable     } },
            { DrainageSol::EXCELLENT,   { "Excellent",   "Évacuation rapide, jamais détrempé.",           Palette::OrangeDoux} },
            { DrainageSol::TRES_BON,    { "Très bon",    "Bonne évacuation, légère rétention utile.",     Palette::VertDoux  } },
            { DrainageSol::BON,         { "Bon",         "Drainage satisfaisant.",                        Palette::VertVif   } },
            { DrainageSol::MOYEN,       { "Moyen",       "Rétention modérée.",                            Palette::OrangeDoux} },
            { DrainageSol::FAIBLE,      { "Faible",      "Risque d'asphyxie racinaire.",                  Palette::Rouge     } }
        }; ENUM_MAP_LOOKUP(map, v)
    }
    inline const char* label(DrainageSol v) { return get(v).label.c_str(); }

    // Rétention Eau Sol
    inline const EnumMetadata& get(RetentionEau v) {
        static const std::unordered_map<RetentionEau, EnumMetadata> map {
            { RetentionEau::TRES_FORTE, { "Très forte", "Sol saturant — espacement arrosages.",           Palette::BleuCiel  } },
            { RetentionEau::BONNE,      { "Bonne",      "Retient bien l'humidité.",                       Palette::BleuDoux  } },
            { RetentionEau::MOYENNE,    { "Moyenne",    "Équilibre correct drainage/rétention.",          Palette::VertDoux  } },
            { RetentionEau::FAIBLE,     { "Faible",     "Sèche vite, arrosage fréquent.",                 Palette::OrangeDoux} }
        }; ENUM_MAP_LOOKUP(map, v)
    }
    inline const char* label(RetentionEau v) { return get(v).label.c_str(); }

    // Richesse Sol
    inline const EnumMetadata& get(RichesseSol v) {
        static const std::unordered_map<RichesseSol, EnumMetadata> map {
            { RichesseSol::ELEVEE,         { "Élevée",         "Sol très fertile.",                       Palette::VertVif   } },
            { RichesseSol::MOYENNE_ELEVEE, { "Moyenne-élevée", "Bonne fertilité.",                        Palette::VertDoux  } },
            { RichesseSol::MOYENNE,        { "Moyenne",        "Fertilité standard.",                     Palette::OrangeDoux} },
            { RichesseSol::FAIBLE,         { "Faible",         "Sol pauvre — idéal méditerranéennes.",    Palette::Sable     } }
        }; ENUM_MAP_LOOKUP(map, v)
    }
    inline const char* label(RichesseSol v) { return get(v).label.c_str(); }

    // CEC Sol
    inline const EnumMetadata& get(CecSol v) {
        static const std::unordered_map<CecSol, EnumMetadata> map {
            { CecSol::TRES_ELEVEE, { "Très élevée", "Rétention maximale des engrais.",                    Palette::VertVif   } },
            { CecSol::ELEVEE,      { "Élevée",      "Bonne capacité à retenir les nutriments.",           Palette::VertDoux  } },
            { CecSol::MOYENNE,     { "Moyenne",     "Apports réguliers recommandés.",                     Palette::OrangeDoux} },
            { CecSol::FAIBLE,      { "Faible",      "Lessivage rapide des nutriments.",                   Palette::Orange    } },
            { CecSol::TRES_FAIBLE, { "Très faible", "Sol quasi-inerte.",                                  Palette::Rouge     } }
        }; ENUM_MAP_LOOKUP(map, v)
    }
    inline const char* label(CecSol v) { return get(v).label.c_str(); }

    // Aération Sol
    inline const EnumMetadata& get(AerationSol v) {
        static const std::unordered_map<AerationSol, EnumMetadata> map {
            { AerationSol::EXCELLENTE,  { "Excellente",  "Circulation d'air optimale.",                   Palette::VertVif   } },
            { AerationSol::TRES_ELEVEE, { "Très élevée", "Très bonne oxygénation.",                       Palette::VertDoux  } },
            { AerationSol::BONNE,       { "Bonne",       "Aération correcte.",                            Palette::VertPale  } },
            { AerationSol::MOYENNE,     { "Moyenne",     "Aération acceptable, attention au tassement.",  Palette::OrangeDoux} },
            { AerationSol::TRES_FAIBLE, { "Très faible", "Sol compact — risque de pourriture.",           Palette::Rouge     } }
        }; ENUM_MAP_LOOKUP(map, v)
    }
    inline const char* label(AerationSol v) { return get(v).label.c_str(); }

    // Vie Microbienne Sol
    inline const EnumMetadata& get(VieMicrobienne v) {
        static const std::unordered_map<VieMicrobienne, EnumMetadata> map {
            { VieMicrobienne::TRES_ELEVEE, { "Très élevée", "Sol vivant et actif.",                       Palette::VertVif   } },
            { VieMicrobienne::MOYENNE,     { "Moyenne",     "Activité microbienne correcte.",             Palette::VertDoux  } },
            { VieMicrobienne::FAIBLE,      { "Faible",      "Peu de vie biologique.",                     Palette::OrangeDoux} }
        }; ENUM_MAP_LOOKUP(map, v)
    }
    inline const char* label(VieMicrobienne v) { return get(v).label.c_str(); }

    // Densité Sol
    inline const EnumMetadata& get(DensiteSol v) {
        static const std::unordered_map<DensiteSol, EnumMetadata> map {
            { DensiteSol::TRES_LEGERE, { "Très légère", "Sol très aéré, idéal pour semis et boutures.", Palette::Sable     } },
            { DensiteSol::LEGERE,      { "Légère",      "Sol meuble, facile à travailler.",              Palette::VertPale  } },
            { DensiteSol::MOYENNE,     { "Moyenne",     "Densité standard, bonne rétention d'eau.",     Palette::VertDoux  } },
            { DensiteSol::LOURDE,      { "Lourde",      "Sol compact, riche en argile.",                Palette::Brun      } },
            { DensiteSol::TRES_LOURDE, { "Très lourde", "Sol très compact et collant.",               Palette::BrunClair } }
        }; ENUM_MAP_LOOKUP(map, v)
    }
    inline const char* label(DensiteSol v) { return get(v).label.c_str(); }

    // Tampon pH Sol
    inline const EnumMetadata& get(tamponPh v) {
        static const std::unordered_map<tamponPh, EnumMetadata> map {
            { tamponPh::TRES_FORT,  { "Très fort",  "Sol très résistant aux variations de pH.",   Palette::VertVif   } },
            { tamponPh::FORT,       { "Fort",       "Bonne résistance aux changements de pH.",     Palette::VertDoux  } },
            { tamponPh::MOYEN,      { "Moyen",      "Résistance modérée aux variations de pH.",    Palette::OrangeDoux} },
            { tamponPh::FAIBLE,     { "Faible",     "Sol sensible aux changements de pH.",         Palette::Orange    } },
            { tamponPh::TRES_FAIBLE, { "Très faible", "Sol très sensible, risque de stress pour la plante.", Palette::Rouge     } }
        }; ENUM_MAP_LOOKUP(map, v)
    }
    inline const char* label(tamponPh v) { return get(v).label.c_str(); }

    // Mineralisation
    inline const EnumMetadata& get(Mineralisation v) {
        static const std::unordered_map<Mineralisation, EnumMetadata> map {
            { Mineralisation::RAPIDE,   { "Rapide",   "Nutriments disponibles rapidement.",               Palette::VertVif   } },
            { Mineralisation::MODEREE,  { "Modérée",  "Libération progressive et équilibrée.",            Palette::VertDoux  } },
            { Mineralisation::LENTE,    { "Lente",    "Réserves sur le long terme.",                      Palette::OrangeDoux} }
        }; ENUM_MAP_LOOKUP(map, v)
    }
    inline const char* label(Mineralisation v) { return get(v).label.c_str(); }

    // Compatibilité au calcaire
    inline const EnumMetadata& get(CompatibiliteCalcaire v) {
        static const std::unordered_map<CompatibiliteCalcaire, EnumMetadata> map {
            { CompatibiliteCalcaire::EXCELLENTE,  { "Excellente",  "Tolère et apprécie les sols calcaires.",       Palette::VertVif   } },
            { CompatibiliteCalcaire::MOYENNE,     { "Moyenne",     "Tolère une légère présence de calcaire.",      Palette::OrangeDoux} },
            { CompatibiliteCalcaire::TRES_FAIBLE, { "Très faible", "Le calcaire provoque une chlorose.",           Palette::Rouge     } }
        }; ENUM_MAP_LOOKUP(map, v)
    }
    inline const char* label(CompatibiliteCalcaire v) { return get(v).label.c_str(); }

    // Correction Acidité
    inline const EnumMetadata& get(CorrectionAcidite v) {
        static const std::unordered_map<CorrectionAcidite, EnumMetadata> map {
            { CorrectionAcidite::CHAUX,             { "Chaux",             "Élimine l'acidité du sol.",                         Palette::Sable     } },
            { CorrectionAcidite::CENDRES,           { "Cendres",           "Légère correction de l'acidité.",                   Palette::BrunClair } },
            { CorrectionAcidite::CALCAIRE,          { "Calcaire",          "Corrige l'acidité et enrichit en calcium.",         Palette::Brun      } },
            { CorrectionAcidite::EAU_CALCAIRE,      { "Eau calcaire",      "Arrosage avec eau dure pour réduire l'acidité.",    Palette::GrisClair } },
            { CorrectionAcidite::TERREAU_UNIVERSEL, { "Terreau universel", "Substrat équilibré pour corriger l'acidité.",       Palette::VertDoux  } },
            { CorrectionAcidite::COMPOST,           { "Compost",           "Amendement organique qui peut réduire l'acidité.",  Palette::VertVif   } }
        }; ENUM_MAP_LOOKUP(map, v)
    }
    inline const char* label(CorrectionAcidite v) { return get(v).label.c_str(); }

    // Correction Alcalinité
    inline const EnumMetadata& get(CorrectionAlcalinite v) {
        static const std::unordered_map<CorrectionAlcalinite, EnumMetadata> map {
            { CorrectionAlcalinite::TERRE_BRUYERE,      { "Terre de bruyère",   "Substrat acide pour corriger l'alcalinité.",           Palette::Violet    } },
            { CorrectionAlcalinite::SOUFRE,             { "Soufre",             "Amendement qui acidifie le sol.",                      Palette::Rouge     } },
            { CorrectionAlcalinite::EAU_PLUIE,          { "Eau de pluie",       "Arrosage avec eau douce pour réduire l'alcalinité.",   Palette::BleuDoux  } },
            { CorrectionAlcalinite::COMPOST,            { "Compost",            "Amendement organique qui peut acidifier le sol.",      Palette::VertVif   } },
            { CorrectionAlcalinite::TERREAU_UNIVERSEL,  { "Terreau universel",  "Substrat équilibré pour corriger l'alcalinité.",       Palette::VertDoux  } },
            { CorrectionAlcalinite::TERRE_BRUYERE,      { "Terre de bruyère",   "Substrat acide pour corriger l'alcalinité.",           Palette::Violet    } }
        }; ENUM_MAP_LOOKUP(map, v)
    }
    inline const char* label(CorrectionAlcalinite v) { return get(v).label.c_str(); }

    // Composition Sol
    inline const EnumMetadata& get(CompositionSol v) {
        static const std::unordered_map<CompositionSol, EnumMetadata> map {
            { CompositionSol::TOURBE,                   { "Tourbe",                 "Matière organique acide, retient bien l'eau.",     Palette::BrunClair } },
            { CompositionSol::COMPOST,                  { "Compost",                "Matière organique riche en nutriments.",           Palette::VertVif   } },
            { CompositionSol::FIBRES_VEGETALES,         { "Fibres végétales",       "Amendement organique léger et aéré.",              Palette::VertDoux  } },
            { CompositionSol::SABLE,                    { "Sable",                  "Particule minérale drainante.",                    Palette::Sable     } },
            { CompositionSol::POUZZOLANE,               { "Pouzzolane",             "Gravier volcanique léger et drainant.",            Palette::Gris      } },
            { CompositionSol::CALCAIRE,                 { "Calcaire",               "Particule minérale alcaline.",                     Palette::Brun      } },
            { CompositionSol::MATIERE_ORGANIQUE_ACIDE,  { "Matière organique acide", "Amendement pour sols acides.",                    Palette::Violet    } },
            { CompositionSol::COMPOST_ENRICHI,          { "Compost enrichi",        "Compost très riche en nutriments.",                Palette::VertVif   } },
            { CompositionSol::ENGRAIS_ORGANIQUE,        { "Engrais organique",      "Source concentrée de nutriments naturels.",        Palette::VertDoux  } },
            { CompositionSol::ECORCES,                  { "Écorces",                "Amendement organique grossier pour aérer le sol.", Palette::BrunClair } },
            { CompositionSol::ARGILE,                   { "Argile",                 "Particule minérale lourde et collante.",           Palette::Brun      } }
        }; ENUM_MAP_LOOKUP(map, v)
    }
    inline const char* label(CompositionSol v) { return get(v).label.c_str(); }

    // Risques Sol
    inline const EnumMetadata& get(risquesSol v) {
        static const std::unordered_map<risquesSol, EnumMetadata> map {
            { risquesSol::TASSEMENT,            { "Tassement",          "Risque de sol compacté, réduire les passages.",                            Palette::Rouge     } },
            { risquesSol::APPAUVRISSEMENT,      { "Appauvrissement",    "Perte de nutriments, amendement régulier conseillé.",                      Palette::OrangeDoux} },
            { risquesSol::COMPACTION,           { "Compaction",         "Risque de sol durci, éviter de travailler à la mauvaise période.",         Palette::Rouge     } },
            { risquesSol::LESSIVAGE,            { "Lessivage",          "Risque de nutriments emportés par les eaux d'arrosage.",                   Palette::BleuDoux  } },
            { risquesSol::EXCES_AZOTE,          { "Excès d'azote",      "Favorise la croissance au détriment de la floraison.",                     Palette::Orange    } },
            { risquesSol::CARENCES,             { "Carences",           "Manque de nutriments essentiels, surveiller la fertilité.",                Palette::Rouge     } },
            { risquesSol::PAUVRETE_LONG_TERME,  { "Pauvreté long terme", "Dégradation progressive du sol, rotation et amendement nécessaires.",     Palette::Rouge     } }
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
            { FormePot::BAC,       { "Bac",         "Grand contenant carré ou rectangulaire.",  Palette::BrunClair } },
            { FormePot::COLONNE,   { "Colonne",     "Pot haut et étroit.",                      Palette::Brun      } },
            { FormePot::JARDINIERE,{ "Jardinière",  "Contenant long et peu profond.",           Palette::VertDoux  } },
            { FormePot::STANDARD,  { "Standard",    "Pot classique arrondi.",                   Palette::GrisClair } }
        }; ENUM_MAP_LOOKUP(map, v)
    }
    inline const char* label(FormePot v) { return get(v).label.c_str(); }

    // Type de pot
    inline const EnumMetadata& get(TypePot v) {
        using T = TypePot;
        static const std::unordered_map<T, EnumMetadata> map {
            { T::TERRE_CUITE,        { "Terre cuite",        "Poreux, favorise l'aération des racines.",    Palette::BrunClair      } },
            { T::PLASTIQUE,          { "Plastique",          "Garde l'humidité plus longtemps.",            Palette::BleuDoux } },
            { T::CERAMIQUE_EMAILLEE, { "Céramique émaillée", "Esthétique et imperméable.",                  Palette::VertPale  } },
            { T::GEOTEXTILE,         { "Géotextile",         "Excellente oxygénation (Air-Pruning).",       Palette::GrisClair} },
            { T::VERRE,              { "Verre",              "Pour terrariums ou bouturage à l'eau.",       Palette::BleuCiel      } },
            { T::BOIS,               { "Bois",               "Isolant naturel, aspect rustique.",           Palette::Brun    } }
        };
        ENUM_MAP_LOOKUP(map, v)
    }
    inline const char* label(TypePot v) { return get(v).label.c_str(); }

    // Profondeur du pot
    inline const EnumMetadata& get(profondeurPot v) {
        using P = profondeurPot;
        static const std::unordered_map<P, EnumMetadata> map {
            { P::FAIBLE,   { "Faible",   "Moins de 20 cm de profondeur.",           Palette::VertDoux  } },
            { P::MOYEN,    { "Moyen",    "Entre 20 et 40 cm de profondeur.",        Palette::VertVif   } },
            { P::PROFOND,  { "Profond",  "Plus de 40 cm de profondeur.",            Palette::BleuDoux  } }
        };
        ENUM_MAP_LOOKUP(map, v)
    }
    inline const char* label(profondeurPot v) { return get(v).label.c_str(); }

    // Largeur du pot
    inline const EnumMetadata& get(LargeurPot v) {
        using L = LargeurPot;
        static const std::unordered_map<L, EnumMetadata> map {
            { L::ETROIT,      { "Étroit",      "Moins de 20 cm de largeur.",         Palette::VertDoux  } },
            { L::MOYEN,       { "Moyen",       "Entre 20 et 40 cm de largeur.",      Palette::VertVif   } },
            { L::LARGE,       { "Large",       "Entre 40 et 60 cm de largeur.",      Palette::BleuDoux  } },
            { L::TRES_LARGE,  { "Très large",   "Plus de 60 cm de largeur.",         Palette::BleuCiel  } }
        };
        ENUM_MAP_LOOKUP(map, v)
    }
    inline const char* label(LargeurPot v) { return get(v).label.c_str(); }

    // Fréquence de rempotage
    inline const EnumMetadata& get(FrequenceRempotage v) {
        using F = FrequenceRempotage;
        static const std::unordered_map<F, EnumMetadata> map {
            { F::FREQUENT,   { "Fréquent",   "Rempotage recommandé tous les 6 mois.",    Palette::Rouge     } },
            { F::ANNUEL,     { "Annuel",     "Rempotage recommandé chaque année.",       Palette::Orange    } },
            { F::BI_ANNUEL,  { "Bi-annuel",  "Rempotage recommandé tous les 2 ans.",     Palette::VertDoux  } },
            { F::RARE,       { "Rare",       "Rempotage peu fréquent, tous les 3-5 ans.",Palette::VertVif   } }
        };
        ENUM_MAP_LOOKUP(map, v)
    }
    inline const char* label(FrequenceRempotage v) { return get(v).label.c_str(); }

    // Sensibilité au rempotage
    inline const EnumMetadata& get(SensibiliteRempotage v) {
        using S = SensibiliteRempotage;
        static const std::unordered_map<S, EnumMetadata> map {
            { S::FAIBLE,     { "Faible",     "Plante robuste, supporte bien le rempotage.",   Palette::VertVif   } },
            { S::MOYENNE,    { "Moyenne",    "Plante modérément sensible, rempotage à faire avec précaution.", Palette::VertDoux  } },
            { S::ELEVEE,     { "Élevée",     "Plante très sensible, rempotage à éviter ou à faire très délicatement.", Palette::Rouge     } }
        };
        ENUM_MAP_LOOKUP(map, v)
    }
    inline const char* label(SensibiliteRempotage v) { return get(v).label.c_str(); }

    // Besoins en drainage
    inline const EnumMetadata& get(BesoinsDrainage v) {
        using B = BesoinsDrainage;
        static const std::unordered_map<B, EnumMetadata> map {
            { B::FAIBLE,     { "Faible",     "Plante tolérante à un drainage limité.",   Palette::VertDoux  } },
            { B::MOYEN,      { "Moyen",      "Plante nécessitant un drainage modéré.",   Palette::VertVif   } },
            { B::ELEVEE,     { "Élevé",      "Plante nécessitant un bon drainage.",      Palette::BleuDoux  } },
            { B::ESSENTIEL,  { "Essentiel",  "Plante nécessitant un drainage optimal.",  Palette::Rouge     } }
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
            { T::TRES_ELEVE, { "Très élevé", "Réussit dans presque tous les cas, même pour les débutants.",  Palette::VertVif   } },
            { T::ELEVE,      { "Élevé",      "Bonne probabilité de succès avec des conditions correctes.",   Palette::VertDoux  } },
            { T::BON,        { "Bon",        "Réussit bien en suivant les étapes recommandées.",              Palette::VertPale  } },
            { T::MOYEN,      { "Moyen",      "Demande de la patience, plusieurs tentatives parfois utiles.",  Palette::OrangeDoux} },
            { T::VARIABLE,   { "Variable",   "Résultats inégaux selon la plante et la saison.",               Palette::Jaune     } },
            { T::FAIBLE,     { "Faible",     "Technique délicate, taux d'échec élevé, réservé aux experts.", Palette::Rouge     } }
        };
        ENUM_MAP_LOOKUP(map, v)
    }
    inline const char* label(TauxReussite v) { return get(v).label.c_str(); }


    // ── Type de Tige ──
    inline const EnumMetadata& get(TypeTige v) {
        using T = TypeTige;
        static const std::unordered_map<T, EnumMetadata> map {
            { T::HERBACEE,      { "Herbacée",      "Tige souple et verte, enracinement rapide (1-3 sem).",      Palette::VertVif   } },
            { T::SEMI_LIGNEUSE, { "Semi-ligneuse", "Tige partiellement aoûtée, enracinement moyen (1-2 mois).", Palette::VertDoux  } },
            { T::LIGNEUSE,      { "Ligneuse",      "Bois dur dormant, enracinement lent (2-4 mois).",           Palette::Brun      } },
            { T::FEUILLE,       { "Feuille",       "Propagation par feuille entière, sans tige.",               Palette::VertPale  } },
            { T::RACINE,        { "Racine",        "Bouture de racine, technique hivernale spécialisée.",       Palette::BrunClair } }
        };
        ENUM_MAP_LOOKUP(map, v)
    }
    inline const char* label(TypeTige v) { return get(v).label.c_str(); }


    // ── Hormone de Bouturage ──
    inline const EnumMetadata& get(HormoneBouturage v) {
        using H = HormoneBouturage;
        static const std::unordered_map<H, EnumMetadata> map {
            { H::NON,        { "Non",         "Hormone inutile, peut même nuire à cette espèce.",            Palette::VertPale  } },
            { H::OPTIONNEL,  { "Optionnel",   "Améliore légèrement le résultat, non indispensable.",         Palette::OrangeDoux} },
            { H::RECOMMANDE, { "Recommandé",  "Augmente significativement le taux de réussite.",             Palette::Orange    } }
        };
        ENUM_MAP_LOOKUP(map, v)
    }
    inline const char* label(HormoneBouturage v) { return get(v).label.c_str(); }


    // ── Humidité ──
    inline const EnumMetadata& get(HumiditeBouture v) {
        using H = HumiditeBouture;
        static const std::unordered_map<H, EnumMetadata> map {
            { H::ELEVEE,   { "Élevée",   "Brumisation fréquente ou mini-serre recommandée.",       Palette::BleuCiel  } },
            { H::MOYENNE,  { "Moyenne",  "Arrosages réguliers, substrat légèrement humide.",       Palette::BleuDoux  } },
            { H::FAIBLE,   { "Faible",   "Substrat quasi-sec, arrosage très occasionnel.",         Palette::Sable     } },
            { H::IMMERGEE, { "Immergée", "Tige plongée directement dans l'eau, changée 2×/sem.",   Palette::BleuCiel  } }
        };
        ENUM_MAP_LOOKUP(map, v)
    }
    inline const char* label(HumiditeBouture v) { return get(v).label.c_str(); }


    // ── Substrat ──
    inline const EnumMetadata& get(SubstratBouture v) {
        using S = SubstratBouture;
        static const std::unordered_map<S, EnumMetadata> map {
            { S::EAU,           { "Eau",            "Verre d'eau claire, renouveler tous les 2-3 jours.",         Palette::BleuCiel  } },
            { S::LEGER,         { "Léger",          "Terreau fin ou terreau de semis peu compacté.",              Palette::VertPale  } },
            { S::LEGER_DRAINANT,{ "Léger drainant", "Mélange terreau + perlite (50/50) ou vermiculite.",         Palette::VertDoux  } },
            { S::TRES_DRAINANT, { "Très drainant",  "Majoritairement sable horticole ou perlite (70%+).",        Palette::Sable     } },
            { S::MINERAL,       { "Minéral",        "Sable grossier pur ou pouzzolane, sans matière organique.", Palette::Gris      } }
        };
        ENUM_MAP_LOOKUP(map, v)
    }
    inline const char* label(SubstratBouture v) { return get(v).label.c_str(); }

    // ── Étapes clés du bouturage ──
    inline const EnumMetadata& get(EtapeBouture v) {
        using E = EtapeBouture;
        static const std::unordered_map<E, EnumMetadata> map {
            { E::COUPER_SOUS_NOEUD,             { "Couper sous nœud",           "Faire une coupe nette juste en dessous d'un nœud pour favoriser l'enracinement.",                      Palette::VertVif   } },
            { E::RETRAIT_FEUILLES_BASSES,       { "Retrait feuilles basses",    "Enlever les feuilles proches de la base pour éviter la pourriture.",                                   Palette::VertDoux  } },
            { E::PLANTER_SUBSTRAT,              { "Planter substrat",           "Enfoncer la tige dans le substrat jusqu'à au moins 2-3 cm de profondeur.",                             Palette::BleuDoux  } },
            { E::PLACER_LUMIERE_INDIRECTE,      { "Placer lumière indirecte",   "Éviter la lumière directe du soleil qui peut dessécher la bouture.",                                   Palette::BleuCiel  } },
            { E::PRELEVER_TIGE_SEMI_RIGIDE,     { "Prélever tige semi-rigide",  "Utiliser une tige semi-ligneuse pour un meilleur taux de réussite.",                                   Palette::Brun      } },
            { E::ARROSAGE_LEGER,                { "Arrosage léger",             "Maintenir le substrat légèrement humide sans le détremper.",                                           Palette::Sable     } },
            { E::COUPER_BOIS_DORMANT,           { "Couper bois dormant",        "Utiliser une tige ligneuse prélevée en période de dormance pour certaines espèces.",                   Palette::BrunClair } },
            { E::PLANTER_ENTERRES,              { "Planter enterrés",           "Enfouir une partie de la tige sous le substrat pour favoriser l'enracinement.",                        Palette::VertPale  } },
            { E::RETIRER_FEUILLES_EAU,          { "Retirer feuilles eau",       "Enlever les feuilles immergées pour éviter la pourriture dans les boutures à l'eau.",                  Palette::BleuCiel  } },
            { E::CHANGER_EAU_REGULIEREMENT,     { "Changer eau régulièrement",  "Renouveler l'eau tous les 2-3 jours pour les boutures à l'eau afin d'éviter les algues.",              Palette::BleuDoux  } },
            { E::PRELEVER_FEUILLE,              { "Prélever feuille",           "Bouturer à partir d'une feuille entière pour les espèces qui le permettent.",                          Palette::VertPale  } },
            { E::SECHAGE,                       { "Séchage",                    "Laisser sécher la coupe à l'air libre pendant quelques heures avant de planter.",                      Palette::Sable     } },
            { E::POSER_SUBSTRAT,                { "Poser substrat",             "Placer la bouture sur le substrat sans l'enfoncer pour les espèces sensibles.",                        Palette::VertDoux  } },
            { E::COUPER_RACINE,                 { "Couper racine",              "Tailler légèrement les racines avant de rempoter pour stimuler la croissance.",                        Palette::Brun      } },
            { E::PLANTER_HORIZONTAL,            { "Planter horizontal",         "Placer la tige à l'horizontale sous le substrat pour favoriser l'apparition de nouvelles pousses.",    Palette::VertVif   } },
            { E::COUVRIR,                       { "Couvrir",                    "Recouvrir la bouture d'un sac plastique ou d'une mini-serre pour maintenir l'humidité.",               Palette::BleuCiel  } }
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

    inline DrainageSol parseDrainage(const std::string& s) {
        static const std::unordered_map<std::string, DrainageSol> m {
            {"MOYEN", DrainageSol::MOYEN}, {"EXCELLENT", DrainageSol::EXCELLENT}, {"BON", DrainageSol::BON},
            {"TRES_BON", DrainageSol::TRES_BON}, {"FAIBLE", DrainageSol::FAIBLE}, {"TRES_RAPIDE", DrainageSol::TRES_RAPIDE}};
        auto it = m.find(s); return it != m.end() ? it->second : DrainageSol::INCONNU;
    }

    inline RetentionEau parseRetention(const std::string& s) {
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