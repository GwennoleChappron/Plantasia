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

    inline const EnumMetadata& get(ExpositionSoleil v) {
        using E = ExpositionSoleil;
        static const std::unordered_map<E, EnumMetadata> map {
            { E::PLEIN_SOLEIL, { "Plein soleil", "Nécessite au minimum 6h de soleil direct/jour.",    Palette::Jaune    } },
            { E::MI_OMBRE,     { "Mi-ombre",     "Préfère 3 à 5h de soleil, supporte l'ombre.",       Palette::VertDoux } }
        };
        ENUM_MAP_LOOKUP(map, v)
    }
    inline const char* label(ExpositionSoleil v) { return get(v).label.c_str(); }

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
enum class VieMicrobienne { TRES_ELEVEE, MOYENNE, FAIBLE, INCONNU };
enum class Mineralisation { RAPIDE, MODEREE, LENTE, INCONNU };
enum class CompatibiliteCalcaire { EXCELLENTE, MOYENNE, TRES_FAIBLE, INCONNU };

namespace EnumInfo {

    // --- GETTERS POUR LES SOLS ---

    inline const EnumMetadata& get(TypeSol v) {
        static const std::unordered_map<TypeSol, EnumMetadata> map {
            { TypeSol::TERREAU_UNIVERSEL,         { "Universel",      "Polyvalent pour aromatiques.",      Palette::VertDoux  } },
            { TypeSol::TERREAU_MEDITERRANEEN,     { "Méditerranéen",  "Très drainant, pauvre.",            Palette::OrangeDoux} },
            { TypeSol::TERRE_BRUYERE,             { "Terre Bruyère",  "Sol acide.",                        Palette::Violet    } },
            { TypeSol::TERREAU_HORTICOLE_ENRICHI, { "Horticole",      "Très fertile, gourmandes.",         Palette::VertVif   } },
            { TypeSol::SUBSTRAT_AGRUMES,          { "Agrumes",        "Spécial Yuzu/Agrumes.",             Palette::Jaune     } },
            { TypeSol::TERREAU_SEMIS,             { "Semis",          "Très fin et stérile.",              Palette::GrisClair } },
            { TypeSol::SOL_ARGILEUX,              { "Argileux",       "Lourd et fertile.",                 Palette::Brun      } },
            { T::EPIPHYTE,                        { "Épiphyte",       "Mélange d'écorces et sphaigne.",    Palette::Ecorce    } },
            { T::DRAINANT,                        { "Drainant",       "Spécial succulentes et cactées.",   Palette::Gris      } },
            { TypeSol::SOL_SABLEUX,               { "Sableux",        "Très drainant.",                    Palette::Sable     } }
        }; ENUM_MAP_LOOKUP(map, v)
    }
    inline const char* label(TypeSol v) { return get(v).label.c_str(); }

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

    inline const EnumMetadata& get(RetentionEau v) {
        static const std::unordered_map<RetentionEau, EnumMetadata> map {
            { RetentionEau::TRES_FORTE, { "Très forte", "Sol saturant — espacement arrosages.",           Palette::BleuCiel  } },
            { RetentionEau::BONNE,      { "Bonne",      "Retient bien l'humidité.",                       Palette::BleuDoux  } },
            { RetentionEau::MOYENNE,    { "Moyenne",    "Équilibre correct drainage/rétention.",          Palette::VertDoux  } },
            { RetentionEau::FAIBLE,     { "Faible",     "Sèche vite, arrosage fréquent.",                 Palette::OrangeDoux} }
        }; ENUM_MAP_LOOKUP(map, v)
    }
    inline const char* label(RetentionEau v) { return get(v).label.c_str(); }

    inline const EnumMetadata& get(RichesseSol v) {
        static const std::unordered_map<RichesseSol, EnumMetadata> map {
            { RichesseSol::ELEVEE,         { "Élevée",         "Sol très fertile.",                       Palette::VertVif   } },
            { RichesseSol::MOYENNE_ELEVEE, { "Moyenne-élevée", "Bonne fertilité.",                        Palette::VertDoux  } },
            { RichesseSol::MOYENNE,        { "Moyenne",        "Fertilité standard.",                     Palette::OrangeDoux} },
            { RichesseSol::FAIBLE,         { "Faible",         "Sol pauvre — idéal méditerranéennes.",    Palette::Sable     } }
        }; ENUM_MAP_LOOKUP(map, v)
    }
    inline const char* label(RichesseSol v) { return get(v).label.c_str(); }

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

    inline const EnumMetadata& get(VieMicrobienne v) {
        static const std::unordered_map<VieMicrobienne, EnumMetadata> map {
            { VieMicrobienne::TRES_ELEVEE, { "Très élevée", "Sol vivant et actif.",                       Palette::VertVif   } },
            { VieMicrobienne::MOYENNE,     { "Moyenne",     "Activité microbienne correcte.",             Palette::VertDoux  } },
            { VieMicrobienne::FAIBLE,      { "Faible",      "Peu de vie biologique.",                     Palette::OrangeDoux} }
        }; ENUM_MAP_LOOKUP(map, v)
    }
    inline const char* label(VieMicrobienne v) { return get(v).label.c_str(); }

    inline const EnumMetadata& get(Mineralisation v) {
        static const std::unordered_map<Mineralisation, EnumMetadata> map {
            { Mineralisation::RAPIDE,   { "Rapide",   "Nutriments disponibles rapidement.",               Palette::VertVif   } },
            { Mineralisation::MODEREE,  { "Modérée",  "Libération progressive et équilibrée.",            Palette::VertDoux  } },
            { Mineralisation::LENTE,    { "Lente",    "Réserves sur le long terme.",                      Palette::OrangeDoux} }
        }; ENUM_MAP_LOOKUP(map, v)
    }
    inline const char* label(Mineralisation v) { return get(v).label.c_str(); }

    inline const EnumMetadata& get(CompatibiliteCalcaire v) {
        static const std::unordered_map<CompatibiliteCalcaire, EnumMetadata> map {
            { CompatibiliteCalcaire::EXCELLENTE,  { "Excellente",  "Tolère et apprécie les sols calcaires.",       Palette::VertVif   } },
            { CompatibiliteCalcaire::MOYENNE,     { "Moyenne",     "Tolère une légère présence de calcaire.",      Palette::OrangeDoux} },
            { CompatibiliteCalcaire::TRES_FAIBLE, { "Très faible", "Le calcaire provoque une chlorose.",           Palette::Rouge     } }
        }; ENUM_MAP_LOOKUP(map, v)
    }
    inline const char* label(CompatibiliteCalcaire v) { return get(v).label.c_str(); }
}


// ═════════════════════════════════════════════════════════════════════════════
//  5. POTS.JSON (Spécifique aux Contenants)
// ═════════════════════════════════════════════════════════════════════════════

enum class FormePot { BAC, COLONNE, JARDINIERE, STANDARD, INCONNU };
enum class MateriauPot { TERRE_CUITE, PLASTIQUE, BOIS, GEOTEXTILE, INCONNU };
enum class TypePot { TERRE_CUITE, PLASTIQUE, CERAMIQUE_EMAILLEE, GEOTEXTILE, VERRE, BOIS, INCONNU };

namespace EnumInfo {
    inline const EnumMetadata& get(FormePot v) {
        static const std::unordered_map<FormePot, EnumMetadata> map {
            { FormePot::BAC,       { "Bac",         "Grand contenant carré ou rectangulaire.",  Palette::BrunClair } },
            { FormePot::COLONNE,   { "Colonne",     "Pot haut et étroit.",                      Palette::Brun      } },
            { FormePot::JARDINIERE,{ "Jardinière",  "Contenant long et peu profond.",           Palette::VertDoux  } },
            { FormePot::STANDARD,  { "Standard",    "Pot classique arrondi.",                   Palette::GrisClair } }
        }; ENUM_MAP_LOOKUP(map, v)
    }
    inline const char* label(FormePot v) { return get(v).label.c_str(); }

    inline const EnumMetadata& get(MateriauPot v) {
        static const std::unordered_map<MateriauPot, EnumMetadata> map {
            { MateriauPot::TERRE_CUITE, { "Terre cuite", "Poreux, régule l'humidité, lourd.",      Palette::BrunClair } },
            { MateriauPot::PLASTIQUE,   { "Plastique",   "Léger, retient bien l'eau.",             Palette::BleuDoux  } },
            { MateriauPot::BOIS,        { "Bois",        "Naturel et isolant.",                    Palette::Brun      } },
            { MateriauPot::GEOTEXTILE,  { "Géotextile",  "Aération maximale, anti-spiralisation.", Palette::VertPale  } }
        }; ENUM_MAP_LOOKUP(map, v)
    }
    inline const char* label(MateriauPot v) { return get(v).label.c_str(); }

    namespace EnumInfo {
        inline const EnumMetadata& get(TypePot v) {
            using T = TypePot;
            static const std::unordered_map<T, EnumMetadata> map {
                { T::TERRE_CUITE,        { "Terre cuite",        "Poreux, favorise l'aération des racines.",    Palette::Ocre      } },
                { T::PLASTIQUE,          { "Plastique",          "Garde l'humidité plus longtemps.",            Palette::GrisFonce } },
                { T::CERAMIQUE_EMAILLEE, { "Céramique émaillée", "Esthétique et imperméable.",                  Palette::BleuNuit  } },
                { T::GEOTEXTILE,         { "Géotextile",         "Excellente oxygénation (Air-Pruning).",       Palette::Anthracite} },
                { T::VERRE,              { "Verre",              "Pour terrariums ou bouturage à l'eau.",       Palette::Cyan      } },
                { T::BOIS,               { "Bois",               "Isolant naturel, aspect rustique.",           Palette::Marron    } },
                { T::INCONNU,            { "Inconnu",            "Type de contenant non défini.",               Palette::Neutre    } }
            };
            ENUM_MAP_LOOKUP(map, v)
        }
        inline const char* label(TypePot v) { return get(v).label.c_str(); }
    }
}


// ═════════════════════════════════════════════════════════════════════════════
//  6. BOUTURES.JSON (Spécifique à la multiplication)
// ═════════════════════════════════════════════════════════════════════════════

enum class TauxReussite { TRES_ELEVE, ELEVE, BON, MOYEN, VARIABLE, FAIBLE, INCONNU };
enum class TypeTige { HERBACEE, SEMI_LIGNEUSE, LIGNEUSE, FEUILLE, RACINE, INCONNU };
enum class HormoneBouturage { NON, OPTIONNEL, RECOMMANDE, INCONNU };
enum class HumiditeBouture { ELEVEE, MOYENNE, FAIBLE, IMMERGEE, INCONNU };
enum class SubstratBouture { EAU, LEGER, LEGER_DRAINANT, TRES_DRAINANT, MINERAL, INCONNU };
enum class SubstratBouture { EAU, LEGER, LEGER_DRAINANT, TRES_DRAINANT, MINERAL, INCONNU };

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
            {"TUBEREUX", TypeRacinaireEnum::TUBEREUX}}; // <- AJOUT DE TUBEREUX ICI
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

    // --- POTS.JSON ---
    inline FormePot parseFormePot(const std::string& s) {
        static const std::unordered_map<std::string, FormePot> m {
            {"BAC", FormePot::BAC}, {"COLONNE", FormePot::COLONNE}, {"JARDINIERE", FormePot::JARDINIERE}, {"STANDARD", FormePot::STANDARD}};
        auto it = m.find(s); return it != m.end() ? it->second : FormePot::INCONNU;
    }

    inline MateriauPot parseMateriauPot(const std::string& s) {
        static const std::unordered_map<std::string, MateriauPot> m {
            {"TERRE_CUITE", MateriauPot::TERRE_CUITE}, {"PLASTIQUE", MateriauPot::PLASTIQUE}, 
            {"BOIS", MateriauPot::BOIS}, {"GEOTEXTILE", MateriauPot::GEOTEXTILE}};
        auto it = m.find(s); return it != m.end() ? it->second : MateriauPot::INCONNU;
    }

    // --- BOUTURES.JSON ---
    // (J'ai appliqué la même technique de unordered_map pour la performance !)
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

} // namespace EnumInfo

#undef ENUM_MAP_LOOKUP