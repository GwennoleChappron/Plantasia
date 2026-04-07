#pragma once
#include <string>
#include <vector>
#include <optional>
#include "EnumInfo.hpp"
// ─────────────────────────────────────────────────────────────────────────────

struct Plant {

    // ── Identité ──────────────────────────────────────────────────────────────
    std::string              nom;
    std::string              nomScientifique;
    std::vector<std::string> autresNoms;
    TypePlante               type = TypePlante::INCONNU;       // 0=aromatique, 1=fruitier, etc.
    std::string              famille;
    std::string              origine;

    // ── Culture ───────────────────────────────────────────────────────────────
    Rusticite        rusticite      = Rusticite::INCONNU;
    std::string      zoneClimat;
    ExpositionSoleil expositionSoleil     = ExpositionSoleil::INCONNU;
    ExpositionVent   expositionVent = ExpositionVent::INCONNU;
    std::string      volumePotMin;
    std::string      dimensionsAdulte;
    NiveauDifficulte difficulte     = NiveauDifficulte::INCONNU;

    // ── Arrosage ──────────────────────────────────────────────────────────────
    int         besoinEau    = 0;   // 0-5
    std::string frequenceEte;
    std::string frequenceHiver;

    // ── Calendrier (mois 1-12, 0 = non renseigné) ────────────────────────────
    int floraisonDebut = 0;
    int floraisonFin   = 0;
    int recolteDebut   = 0;
    int recolteFin     = 0;

    // ── Caractéristiques — enums propres ─────────────────────────────────────
    //
    //  Ces 4 champs étaient des strings incohérentes dans l'ancienne version
    //  (remplis seulement pour 3-5 plantes sur 16, valeurs non normalisées).
    //  Maintenant tous les 16 peuvent avoir une valeur, et l'UI peut filtrer.

    VitesseCroissance   vitesseCroissance   = VitesseCroissance::INCONNU;
    ToleranceSecheresse toleranceSecheresse = ToleranceSecheresse::INCONNU;
    SensibiliteEau      sensibiliteExcesEau = SensibiliteEau::INCONNU;
    TypeFeuillage       feuillage           = TypeFeuillage::INCONNU;

    // ── Sol — DUAL FIELD  ──────────────────────────────────────────────────────
    //
    //  POURQUOI DEUX CHAMPS ?
    //  solEnum  → lie la plante à une fiche sol complète (pH 7-8.5, CEC faible…)
    //             utilisé par : le bouton "Voir le sol", les filtres, la compat
    //  solTexte → nuance propre à CETTE plante que le sol générique ne peut pas
    //             exprimer. Exemples réels dans tes données :
    //             "Supporte mal les sols lourds et argileux" (Romarin)
    //             "Elle a besoin de place pour sa grosse racine" (Livèche)
    //             "De préférence alcalin" (Verveine — alors que son sol est neutre)
    //
    //  RÈGLE : solTexte ne répète pas les infos de solEnum.
    //          Il ajoute uniquement la nuance spécifique à la plante.

    TypeSol     solEnum         = TypeSol::INCONNU;  // → lie à sols.json
    TypeSol     solAlternatifEnum = TypeSol::INCONNU; // optionnel
    std::string solTexte;       // conseil narratif spécifique à la plante

    //  Note sur le pH : phMin/phMax sont dans sols.json via solEnum.
    //  phNote sert uniquement si la plante a une préférence pH
    //  différente de son sol recommandé (cas rare : Verveine "préfère alcalin"
    //  alors que TERREAU_UNIVERSEL est neutre 6-7).
    std::string phNote;         // ex: "De préférence alcalin" — vide si redondant

    // ── Type racinaire — DUAL FIELD ──────────────────────────────────────────

    TypeRacinaireEnum typeRacinaireEnum  = TypeRacinaireEnum::INCONNU; // → lie à pots.json
    std::string       typeRacinaireTexte; // ex: "Superficiel fasciculé"

    // ── Toxicité — DUAL FIELD ──────────────────────────────────────────────────

    bool        toxiciteAnimaux = false;   // true si dangereux
    std::string toxiciteNote;              // ex: "Toxique si ingéré en grande quantité"
    // ── Score ─────────────────────────────────────────────────────────────────
    int scoreBalcon = 0;   // 0-100

    // ── Relations ─────────────────────────────────────────────────────────────
    std::vector<TypeBouture> bouturesCompatibles;  // liste d'enums, pas de strings

    // ── Conseils texte (narratifs, irréductibles à des enums) ────────────────
    std::string conseilArrosage;
    std::string rempotage;
    std::string conseilEntretien;
    std::string maladies;
    std::string compagnonnage;
    std::string vertus;
    std::string precautions;
    std::string notes;
    std::string astucePro;
    std::string multiplication;
};