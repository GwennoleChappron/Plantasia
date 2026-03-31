#include "DataLoader.hpp"
#include "Data/EnumInfo"
#include <fstream>
#include <iostream>
#include <nlohmann/json.hpp>

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

} // namespace (anonyme)

// ─────────────────────────────────────────────────────────────────────────────
//  chargerPlantes — encyclopedia.json
// ─────────────────────────────────────────────────────────────────────────────

bool DataLoader::chargerPlantes(const std::string& chemin, std::vector<Plant>& out) {
    json j;
    if (!ouvrirJson(chemin, j)) return false;
    if (!j.is_array()) {
        std::cerr << "[DataLoader] encyclopedia.json : tableau attendu à la racine.\n";
        return false;
    }

    out.clear();
    out.reserve(j.size());

    for (const auto& item : j) {
        Plant p;

        // Identité
        p.nom             = str(item, "nom");
        p.nomScientifique = str(item, "nom_scientifique");
        p.autresNoms      = tableau(item, "autres_noms");
        p.type            = entier(item, "type");
        p.famille         = str(item, "famille");
        p.origine         = str(item, "origine");

        // Culture
        p.rusticite       = EnumInfo::parseRusticite(str(item, "rusticite"));
        p.zoneClimat      = str(item, "zone_climat");
        p.exposition      = EnumInfo::parseExposition(str(item, "exposition_soleil"));
        p.expositionVent  = EnumInfo::parseVent(str(item, "exposition_vent"));
        p.volumePotMin    = str(item, "volume_pot_min");
        p.dimensionsAdulte= str(item, "dimensions_adulte");
        p.difficulte      = EnumInfo::parseDifficulte(str(item, "niveau_difficulte"));

        // Arrosage
        p.besoinEau       = entier(item, "besoin_eau");
        p.frequenceEte    = str(item, "frequence_arrosage_ete");
        p.frequenceHiver  = str(item, "frequence_arrosage_hiver");

        // Calendrier
        p.floraisonDebut  = entier(item, "floraison_debut");
        p.floraisonFin    = entier(item, "floraison_fin");
        p.recolteDebut    = entier(item, "recolte_debut");
        p.recolteFin      = entier(item, "recolte_fin");

        // Conseils
        p.feuillage        = str(item, "feuillage");
        p.conseilTerre     = str(item, "conseil_terre");
        p.conseilArrosage  = str(item, "conseil_arrosage");
        p.rempotage        = str(item, "rempotage");
        p.conseilEntretien = str(item, "conseil_entretien");
        p.maladies         = str(item, "maladies");
        p.compagnonnage    = str(item, "compagnonnage");
        p.vertus           = str(item, "vertus_medicinales");
        p.precautions      = str(item, "precautions");
        p.notes            = str(item, "notes");
        p.astucePro        = str(item, "astuce_pro");
        p.phSol            = str(item, "ph_sol");
        p.multiplication   = str(item, "multiplication");

        // Caractéristiques
        p.toleranceSecheresse = str(item, "tolerance_secheresse");
        p.sensibiliteExcesEau = str(item, "sensibilite_exces_eau");
        p.vitesseCroissance   = str(item, "vitesse_croissance");
        p.typeRacinaireTexte  = str(item, "type_racinaire");

        // Toxicité
        p.toxiciteAnimaux = (str(item, "toxicite_animaux") == "OUI");

        // Score
        p.scoreBalcon = entier(item, "score_balcon");

        // Relations (clés JSON brutes — la résolution se fait dans DatabaseManager)
        p.solRecommande      = str(item, "sol_recommande");
        p.solAlternatif      = str(item, "sol_alternatif");
        p.typeRacinairePot   = EnumInfo::parseTypeRacinaire(str(item, "type_racinaire_pot"));
        p.bouturesCompatibles= tableau(item, "boutures_compatibles");

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
        s.typeSol      = str(item, "type_sol");
        s.texture      = str(item, "texture");
        s.drainage     = str(item, "drainage");
        s.retentionEau = str(item, "retention_eau");
        s.richesse     = str(item, "richesse");

        if (item.contains("ph") && item["ph"].is_object()) {
            s.phMin = flottant(item["ph"], "min");
            s.phMax = flottant(item["ph"], "max");
        }

        s.composition  = tableau(item, "composition");
        s.utilisation  = str(item, "utilisation");
        s.adaptePour   = tableau(item, "adapte_pour");
        s.problemes    = tableau(item, "problemes");
        s.risques      = tableau(item, "risques");

        if (item.contains("correction_ph") && item["correction_ph"].is_object()) {
            s.correctionBaisser   = tableau(item["correction_ph"], "BAISSER");
            s.correctionAugmenter = tableau(item["correction_ph"], "AUGMENTER");
        }

        s.cec                   = str(item, "cec");
        s.aeration              = str(item, "aeration");
        s.densite               = str(item, "densite");
        s.tamponPh              = str(item, "tampon_ph");
        s.mineralisation        = str(item, "mineralisation");
        s.vieMicrobienne        = str(item, "vie_microbienne");
        s.frequenceRenouvellement = str(item, "frequence_renouvellement");
        s.compatibiliteCalcaire = str(item, "compatibilite_calcaire");

        // Indice principal : on cherche la première clé non-nulle dans "indices"
        if (item.contains("indices") && item["indices"].is_object()) {
            for (const auto& [key, val] : item["indices"].items()) {
                if (!val.is_null()) {
                    s.indiceClé   = key;
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
//  chargerPots — pots.json
// ─────────────────────────────────────────────────────────────────────────────

bool DataLoader::chargerPots(const std::string& chemin, std::vector<Pot>& out) {
    json j;
    if (!ouvrirJson(chemin, j)) return false;

    out.clear();
    out.reserve(j.size());

    for (const auto& item : j) {
        Pot p;
        p.typeRacinaire       = EnumInfo::parseTypeRacinaire(str(item, "type_racinaire"));
        p.profondeurPot       = str(item, "profondeur_pot");
        p.largeurPot          = str(item, "largeur_pot");
        p.formePot            = str(item, "forme_pot");
        p.drainage            = str(item, "drainage");
        p.frequenceRempotage  = str(item, "frequence_rempotage");
        p.sensibiliteRempotage= str(item, "sensibilite_rempotage");
        p.materiaux           = tableau(item, "materiaux");
        p.plantesExemples     = tableau(item, "plantes_exemples");

        if (item.contains("volume_litres") && item["volume_litres"].is_object()) {
            p.volumeMin = entier(item["volume_litres"], "min");
            p.volumeMax = entier(item["volume_litres"], "max");
        }

        out.push_back(std::move(p));
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
        b.nom              = str(item, "nom");
        b.niveauDifficulte = EnumInfo::parseDifficulte(str(item, "niveau_difficulte"));
        b.typeTige         = EnumInfo::parseTypeTige(str(item, "type_tige"));
        b.substrat         = EnumInfo::parseSubstrat(str(item, "substrat"));
        b.hormoneBouturage = EnumInfo::parseHormone(str(item, "hormone_bouturage"));
        b.humidite         = EnumInfo::parseHumidite(str(item, "humidite"));
        b.tauxReussite     = EnumInfo::parseTauxReussite(str(item, "taux_reussite"));

        b.periode = tableauEnum<Saison>(item, "periode", EnumInfo::parseSaison);
        b.plantesConcernees = tableau(item, "plantes_concernees");
        b.etapes            = tableau(item, "etapes");

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