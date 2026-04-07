#include "DataLoader.hpp"
#include "Data/EnumInfo.hpp"
#include <fstream>
#include <iostream>
#include <nlohmann/json.hpp>
#include <cmath>

using json = nlohmann::json;

// ─────────────────────────────────────────────────────────────────────────────
//  Helpers privés ouverture et parsing JSON
// ─────────────────────────────────────────────────────────────────────────────

namespace {

bool ouvrirJson(const std::string& chemin, json& out) {
    std::ifstream f(chemin);
    if (!f.is_open()) {
        std::cerr << "[DataLoader] Impossible d'ouvrir : " << chemin << "\n";
        return false;
    }
    try {
        f >> out;
    } catch (const json::parse_error& e) {
        std::cerr << "[DataLoader] Erreur de parsing JSON (" << chemin << ") : "
                  << e.what() << "\n";
        return false;
    }
    return true;
}
// Lit une string JSON avec une valeur par défaut si la clé est absente ou null.
std::string str(const json& j, const std::string& key, const std::string& def = "") {
    if (!j.contains(key) || j[key].is_null()) return def;
    return j[key].get<std::string>();
}
 
// Lit un int JSON avec une valeur par défaut.
int entier(const json& j, const std::string& key, int def = 0) {
    if (!j.contains(key) || j[key].is_null()) return def;
    return j[key].get<int>();
}
 
// Lit un float JSON avec une valeur par défaut.
float flottant(const json& j, const std::string& key, float def = 0.f) {
    if (!j.contains(key) || j[key].is_null()) return def;
    return j[key].get<float>();
}
 
// Lit un bool depuis une string "OUI"/"NON" ou un bool JSON.
bool booleen(const json& j, const std::string& key, bool def = false) {
    if (!j.contains(key) || j[key].is_null()) return def;
    if (j[key].is_boolean()) return j[key].get<bool>();
    if (j[key].is_string())  return j[key].get<std::string>() == "OUI";
    return def;
}

std::vector<std::string> tableau(const json& j, const std::string& key) {
    std::vector<std::string> result;
    if (!j.contains(key) || !j[key].is_array()) return result;
    for (const auto& item : j[key])
        if (item.is_string()) result.push_back(item.get<std::string>());
    return result;
}

template <typename EnumType, typename ParseFunction>
std::vector<EnumType> tableauEnum(const json& item, const std::string& key, ParseFunction parser) {
    std::vector<EnumType> resultat;
    
    if (item.contains(key) && item[key].is_array()) {
        for (const auto& val : item[key]) {
            if (val.is_string()) {
                // On appelle la fonction de parsing sur le texte
                resultat.push_back(parser(val.get<std::string>()));
            }
        }
    }
    return resultat;
}

template <typename EnumType, typename ParseFunction>
std::unordered_map<EnumType, float> dictionnaireEnum(const json& item, const std::string& key, ParseFunction parser) {
    std::unordered_map<EnumType, float> resultat;
    
    if (item.contains(key) && item[key].is_object()) {
        for (const auto& [json_key, json_val] : item[key].items()) {
            if (json_val.is_number()) {
                EnumType enumKey = parser(json_key);
                resultat[enumKey] = json_val.get<float>();
            }
        }
    }
    return resultat;
}

template <typename EnumType>
void normaliserPourcentages(std::unordered_map<EnumType, float>& proportions) {
    if (proportions.empty()) return;

    float somme_initiale = 0;
    for (const auto& [cle, valeur] : proportions) {
        somme_initiale += valeur;
    }

    if (std::abs(somme_initiale - 100.0f) < 0.1f) {return;}

    if (somme_initiale == 0) {
        std::cerr << "Avertissement : La somme totale est de 0." << std::endl;
        return;
    }

    float somme_finale = 0;
    for (auto& [cle, valeur] : proportions) {
        valeur = (valeur * 100) / somme_initiale; 
        somme_finale += valeur;
    }
}
} // namespace (anonyme)

// ─────────────────────────────────────────────────────────────────────────────
//  chargerPlantes — encyclopedia.json
// ─────────────────────────────────────────────────────────────────────────────

bool DataLoader::chargerPlantes(const std::string& chemin, std::vector<Plant>& out) {
    std::cerr << "[DataLoader] Chargement des plantes depuis : " << chemin << "\n";
    json j;
    if (!ouvrirJson(chemin, j)) return false;
 
    out.clear();
    out.reserve(j.size());
 
    for (const auto& item : j) {
        std::cerr << "[DataLoader] Chargement de la plante : " << item.value("nom", "Inconnu") << "\n";
        Plant p;
 
        // ── Identité ─────────────────────────────────────────────────────────
        p.nom              = str(item, "nom");
        p.nomScientifique  = str(item, "nom_scientifique");
        p.autresNoms       = tableau(item, "autres_noms");
        p.type             = EnumInfo::parseTypePlante(str(item, "type"));
        p.famille          = str(item, "famille");
        p.origine          = str(item, "origine");
 
        // ── Culture ──────────────────────────────────────────────────────────
        p.rusticite        = EnumInfo::parseRusticite(str(item, "rusticite"));
        p.zoneClimat       = str(item, "zone_climat");
        p.expositionSoleil = EnumInfo::parseExpositionSoleil(str(item, "exposition_soleil"));
        p.expositionVent   = EnumInfo::parseExpositionVent(str(item, "exposition_vent"));
        p.volumePotMin     = str(item, "volume_pot_min");
        p.dimensionsAdulte = str(item, "dimensions_adulte");
        p.difficulte       = EnumInfo::parseDifficulte(str(item, "niveau_difficulte"));
 
        // ── Arrosage ─────────────────────────────────────────────────────────
        p.besoinEau        = entier(item, "besoin_eau");
        p.frequenceEte     = str(item, "frequence_arrosage_ete");
        p.frequenceHiver   = str(item, "frequence_arrosage_hiver");

        // ── Calendrier ───────────────────────────────────────────────────────
        p.floraisonDebut   = entier(item, "floraison_debut");
        p.floraisonFin     = entier(item, "floraison_fin");
        p.recolteDebut     = entier(item, "recolte_debut");
        p.recolteFin       = entier(item, "recolte_fin");
        // ── Caractéristiques — nouvelles enums ───────────────────────────────
        p.vitesseCroissance   = EnumInfo::parseVitesseCroissance(str(item, "vitesse_croissance"));
        p.toleranceSecheresse = EnumInfo::parseToleranceSecheresse(str(item, "tolerance_secheresse"));
        p.sensibiliteExcesEau = EnumInfo::parseSensibiliteEau(str(item, "sensibilite_exces_eau"));
        p.feuillage           = EnumInfo::parseTypeFeuillage(str(item, "feuillage"));
 
        // ── Sol — DUAL FIELD ─────────────────────────────────────────────────
        //
        //  solEnum  ← "sol_recommande"  (clé vers sols.json)
        //  solTexte ← "conseil_terre"   (nuance narrative propre à la plante)
        //  phNote   ← "ph_sol"          (seulement si différent du sol général)
 
        p.solEnum            = EnumInfo::parseTypeSol(str(item, "sol_recommande"));
        p.solAlternatifEnum  = EnumInfo::parseTypeSol(str(item, "sol_alternatif"));
        p.solTexte           = str(item, "conseil_terre");
        p.phNote             = str(item, "ph_sol");   // vide pour 11/16 plantes → normal
 
        // ── Type racinaire — DUAL FIELD ──────────────────────────────────────
        //
        //  typeRacinaireEnum  ← "type_racinaire_pot"  (clé vers pots.json)
        //  typeRacinaireTexte ← "type_racinaire"      (description botanique)
 
        p.typeRacinaireEnum  = EnumInfo::parseTypeRacinaire(str(item, "type_racinaire_pot"));
        p.typeRacinaireTexte = str(item, "type_racinaire");   // "Superficiel fasciculé" etc.
 
        // ── Toxicité — SPLITÉE ───────────────────────────────────────────────
        //
        //  La valeur JSON est une string mixte : "OUI (Toxique si ingéré en grande quantité)"
        //  On split : bool = commence par "OUI", texte = tout le reste entre parenthèses
 
        {
            if (item.contains("toxicite_animaux")) {
                const auto& toxNode = item["toxicite_animaux"];

                // CAS 1 : C'est un vrai booléen dans le JSON (Nouveau format !)
                if (toxNode.is_boolean()) {
                    p.toxiciteAnimaux = toxNode.get<bool>();
                    p.toxiciteNote = item.value("toxicite_note", ""); 
                }
            } else {
                p.toxiciteAnimaux = false; // Par défaut si la clé n'existe pas
            }
        }
 
        // ── Score ─────────────────────────────────────────────────────────────
        p.scoreBalcon = entier(item, "score_balcon");
 
        // ── Relations — enum[] au lieu de string[] ────────────────────────────
        {
            const auto& bRaw = tableau(item, "boutures_compatibles");
            p.bouturesCompatibles.reserve(bRaw.size());
            for (const auto& s : bRaw)
                p.bouturesCompatibles.push_back(EnumInfo::parseTypeBouture(s));
        }
 
        // ── Conseils texte (irréductibles) ───────────────────────────────────
        p.conseilArrosage  = str(item, "conseil_arrosage");
        p.rempotage        = str(item, "rempotage");
        p.conseilEntretien = str(item, "conseil_entretien");
        p.maladies         = str(item, "maladies");
        p.compagnonnage    = str(item, "compagnonnage");
        p.vertus           = str(item, "vertus_medicinales");
        p.precautions      = str(item, "precautions");
        p.notes            = str(item, "notes");
        p.astucePro        = str(item, "astuce_pro");
        p.multiplication   = str(item, "multiplication");
 
        out.push_back(std::move(p));
    }
 
    std::cout << "[DataLoader] " << out.size() << " plantes chargées depuis " << chemin << "\n";
    return true;
}

// ─────────────────────────────────────────────────────────────────────────────
//  chargerSols — sols.json
// ─────────────────────────────────────────────────────────────────────────────

bool DataLoader::chargerSols(const std::string& chemin, std::vector<Soil>& out) {
    json j;
    if (!ouvrirJson(chemin, j)) return false;

    out.clear();
    out.reserve(j.size());

    for (const auto& item : j) {
        Soil s;
        s.typeSol      = EnumInfo::parseTypeSol(str(item, "type_sol"));
        s.texture      = EnumInfo::parseTexture(str(item, "texture"));
        s.drainage     = EnumInfo::parseDrainage(str(item, "drainage"));
        s.retentionEau = EnumInfo::parseRetentionEau(str(item, "retention_eau"));
        s.richesse     = EnumInfo::parseRichesse(str(item, "richesse"));

        if (item.contains("ph") && item["ph"].is_object()) {
            s.phMin = flottant(item["ph"], "min");
            s.phMax = flottant(item["ph"], "max");
        }

        s.composition  = dictionnaireEnum<CompositionSol>(item, "composition", EnumInfo::parseCompositionSol);
        normaliserPourcentages(s.composition);
        s.utilisation  = str(item, "utilisation");
        s.adaptePour   = tableau(item, "adapte_pour");
        s.risques      = tableauEnum<RisquesSol>(item, "risques", EnumInfo::parseRisquesSol);

        if (item.contains("correction_ph") && item["correction_ph"].is_object()) {
            s.correctionBaisser   = tableauEnum<CorrectionAcidite>(item["correction_ph"], "BAISSER", EnumInfo::parseCorrectionAcidite);
            s.correctionAugmenter = tableauEnum<CorrectionAlcalinite>(item["correction_ph"], "AUGMENTER", EnumInfo::parseCorrectionAlcalinite);
        }

        s.cec                   = EnumInfo::parseCec(str(item, "cec"));
        s.aeration              = EnumInfo::parseAeration(str(item, "aeration"));
        s.densite               = EnumInfo::parseDensite(str(item, "densite"));
        s.tamponPh              = EnumInfo::parseTamponPh(str(item, "tampon_ph"));
        s.mineralisation        = EnumInfo::parseMineralisation(str(item, "mineralisation"));
        s.vieMicrobienne        = EnumInfo::parseVieMicrobienne(str(item, "vie_microbienne"));
        s.frequenceRenouvellement = str(item, "frequence_renouvellement");
        s.compatibiliteCalcaire = str(item, "compatibilite_calcaire");

        // Indice principal : on cherche la première clé non-nulle dans "indices"
        if (item.contains("indices") && item["indices"].is_object()) {
            for (const auto& [key, val] : item["indices"].items()) {
                if (!val.is_null()) {
                    s.indiceClef   = key;
                    s.indiceValeur = val.get<int>();
                    break;
                }
            }
        }

        out.push_back(std::move(s));
    }

    std::cout << "[DataLoader] " << out.size() << " sols chargés depuis " << chemin << "\n";
    return true;
}

// ─────────────────────────────────────────────────────────────────────────────
//  chargerRacines — racines.json
// ─────────────────────────────────────────────────────────────────────────────

bool DataLoader::chargerRacines(const std::string& chemin, std::vector<Racine>& out) {
    json j;
    if (!ouvrirJson(chemin, j)) return false;

    out.clear();
    out.reserve(j.size());

    for (const auto& item : j) {
        Racine r;
        r.typeRacinaire       = EnumInfo::parseTypeRacinaire(str(item, "type_racinaire"));
        r.profondeur       = EnumInfo::parseProfondeurPot(str(item, "profondeur_pot"));
        r.largeur          = EnumInfo::parseLargeurPot(str(item, "largeur_pot"));
        r.formePot            = EnumInfo::parseFormePot(str(item, "forme_pot"));
        r.drainage            = EnumInfo::parseBesoinsDrainage(str(item, "drainage"));
        r.frequenceRempotage  = EnumInfo::parseFrequenceRempotage(str(item, "frequence_rempotage"));
        r.sensibiliteRempotage= EnumInfo::parseSensibiliteRempotage(str(item, "sensibilite_rempotage"));
        r.materiaux           = tableauEnum<TypePot> (item, "materiaux", EnumInfo::parseTypePot);
        r.plantesExemples     = tableau(item, "plantes_exemples");

        if (item.contains("volume_litres") && item["volume_litres"].is_object()) {
            r.volumeMin = entier(item["volume_litres"], "min");
            r.volumeMax = entier(item["volume_litres"], "max");
        }

        out.push_back(std::move(r));
    }

    std::cout << "[DataLoader] " << out.size() << " pots chargés depuis " << chemin << "\n";
    return true;
}

// ─────────────────────────────────────────────────────────────────────────────
//  chargerBoutures — boutures.json
// ─────────────────────────────────────────────────────────────────────────────

bool DataLoader::chargerBoutures(const std::string& chemin, std::vector<Bouture>& out) {
    json j;
    if (!ouvrirJson(chemin, j)) return false;

    out.clear();
    out.reserve(j.size());

    for (const auto& item : j) {
        Bouture b;
        b.nom              = EnumInfo::parseTypeBouture(str(item, "nom"));
        b.niveauDifficulte = EnumInfo::parseDifficulte(str(item, "niveau_difficulte"));
        b.typeTige         = EnumInfo::parseTypeTige(str(item, "type_tige"));
        b.substrat         = EnumInfo::parseSubstrat(str(item, "substrat"));
        b.hormoneBouturage = EnumInfo::parseHormone(str(item, "hormone_bouturage"));
        b.humidite         = EnumInfo::parseHumidite(str(item, "humidite"));
        b.tauxReussite     = EnumInfo::parseTauxReussite(str(item, "taux_reussite"));

        b.periode = tableauEnum<Saison>(item, "periode", EnumInfo::parseSaison);
        b.plantesConcernees = tableau(item, "plantes_concernees");
        b.etapes            = tableauEnum<EtapeBouture>(item, "etapes", EnumInfo::parseEtape);

        if (item.contains("longueur_coupe_cm") && item["longueur_coupe_cm"].is_object()) {
            b.longueurMin = entier(item["longueur_coupe_cm"], "min", -1);
            b.longueurMax = entier(item["longueur_coupe_cm"], "max", -1);
        } else {
            b.longueurMin = b.longueurMax = -1;  // null dans le JSON (ex: BOUTURE_FEUILLE)
        }

        if (item.contains("temperature_c") && item["temperature_c"].is_object()) {
            b.temperatureMin = entier(item["temperature_c"], "min");
            b.temperatureMax = entier(item["temperature_c"], "max");
        }

        if (item.contains("temps_enracinement_jours") && item["temps_enracinement_jours"].is_object()) {
            b.enracinementMin = entier(item["temps_enracinement_jours"], "min");
            b.enracinementMax = entier(item["temps_enracinement_jours"], "max");
        }

        out.push_back(std::move(b));
    }

    std::cout << "[DataLoader] " << out.size() << " boutures chargées depuis " << chemin << "\n";
    return true;
}