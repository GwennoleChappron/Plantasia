#pragma once
#include <string>
#include <vector>
#include <nlohmann/json.hpp>

enum class TypePlante { AROMATIQUE = 0, FRUITIER = 1, FLEUR = 2, LEGUME = 3 };
enum class Rusticite { RUSTIQUE, SEMI_RUSTIQUE, FRAGILE, GELIVE, TROPICALE };
enum class ExpositionSoleil { PLEIN_SOLEIL, MI_OMBRE, OMBRE_CLAIRE, OMBRE_DENSE };
enum class ExpositionVent { ABRITE, MODERE, COULOIR_DE_VENT };
enum class NiveauDifficulte { FACILE, MOYEN, DIFFICILE, EXPERT };

// Macros de sérialisation JSON
NLOHMANN_JSON_SERIALIZE_ENUM(TypePlante, { {TypePlante::AROMATIQUE, 0}, {TypePlante::FRUITIER, 1}, {TypePlante::FLEUR, 2}, {TypePlante::LEGUME, 3} })
NLOHMANN_JSON_SERIALIZE_ENUM(Rusticite, { {Rusticite::RUSTIQUE, "RUSTIQUE"}, {Rusticite::SEMI_RUSTIQUE, "SEMI_RUSTIQUE"}, {Rusticite::FRAGILE, "FRAGILE"}, {Rusticite::GELIVE, "GELIVE"}, {Rusticite::TROPICALE, "TROPICALE"} })
NLOHMANN_JSON_SERIALIZE_ENUM(ExpositionSoleil, { {ExpositionSoleil::PLEIN_SOLEIL, "PLEIN_SOLEIL"}, {ExpositionSoleil::MI_OMBRE, "MI_OMBRE"}, {ExpositionSoleil::OMBRE_CLAIRE, "OMBRE_CLAIRE"}, {ExpositionSoleil::OMBRE_DENSE, "OMBRE_DENSE"} })
NLOHMANN_JSON_SERIALIZE_ENUM(ExpositionVent, { {ExpositionVent::ABRITE, "ABRITE"}, {ExpositionVent::MODERE, "MODERE"}, {ExpositionVent::COULOIR_DE_VENT, "COULOIR_DE_VENT"} })
NLOHMANN_JSON_SERIALIZE_ENUM(NiveauDifficulte, { {NiveauDifficulte::FACILE, "FACILE"}, {NiveauDifficulte::MOYEN, "MOYEN"}, {NiveauDifficulte::DIFFICILE, "DIFFICILE"}, {NiveauDifficulte::EXPERT, "EXPERT"} })

class Plante {
public:
    std::string nom;
    std::string nom_scientifique;  // NOUVEAU
    std::vector<std::string> autres_noms; // NOUVEAU
    std::string famille;           // NOUVEAU
    std::string origine;           // NOUVEAU
    
    TypePlante type;
    Rusticite rusticite;
    std::string zone_climat;       // NOUVEAU (ex: Zone 8)
    ExpositionSoleil exposition_soleil;
    ExpositionVent exposition_vent;
    
    NiveauDifficulte niveau_difficulte;
    std::string volume_pot_min;
    std::string dimensions_adulte; // NOUVEAU
    int besoin_eau; 

    int floraison_debut;
    int floraison_fin;
    int recolte_debut;
    int recolte_fin;
    
    std::string feuillage;         // NOUVEAU
    std::string conseil_terre;
    std::string conseil_arrosage;
    std::string frequence_arrosage_ete;
    std::string frequence_arrosage_hiver;
    std::string conseil_entretien;
    std::string rempotage;
    std::string maladies;
    std::string compagnonnage;
    std::string toxicite_animaux;
    std::string vertus_medicinales; // NOUVEAU
    std::string precautions;       // NOUVEAU
    std::string astuce_pro;
    std::string notes;
    std::string ph_sol;
    std::string type_racinaire;
    std::string vitesse_croissance;
    std::string multiplication;
    std::string tolerance_secheresse;
    std::string sensibilite_exces_eau;
    std::string contrainte_majeure;
    int score_balcon;

    Plante() = default;
};

inline void from_json(const nlohmann::json& j, Plante& p) {
    p.nom = j.value("nom", "Inconnu");
    p.nom_scientifique = j.value("nom_scientifique", "");
    p.autres_noms = j.value("autres_noms", std::vector<std::string>());
    p.famille = j.value("famille", "");
    p.origine = j.value("origine", "");
    p.type = j.value("type", TypePlante::AROMATIQUE);
    p.rusticite = j.value("rusticite", Rusticite::RUSTIQUE);
    p.zone_climat = j.value("zone_climat", "");
    p.exposition_soleil = j.value("exposition_soleil", ExpositionSoleil::MI_OMBRE);
    p.exposition_vent = j.value("exposition_vent", ExpositionVent::ABRITE);
    p.niveau_difficulte = j.value("niveau_difficulte", NiveauDifficulte::FACILE);
    p.volume_pot_min = j.value("volume_pot_min", "?");
    p.dimensions_adulte = j.value("dimensions_adulte", "");
    p.besoin_eau = j.value("besoin_eau", 2);
    p.floraison_debut = j.value("floraison_debut", 0);
    p.floraison_fin = j.value("floraison_fin", 0);
    p.recolte_debut = j.value("recolte_debut", 0);
    p.recolte_fin = j.value("recolte_fin", 0);
    p.feuillage = j.value("feuillage", "");
    p.conseil_terre = j.value("conseil_terre", "");
    p.conseil_arrosage = j.value("conseil_arrosage", "");
    p.frequence_arrosage_ete = j.value("frequence_arrosage_ete", "");
    p.frequence_arrosage_hiver = j.value("frequence_arrosage_hiver", "");
    p.conseil_entretien = j.value("conseil_entretien", "");
    p.rempotage = j.value("rempotage", "");
    p.maladies = j.value("maladies", "");
    p.compagnonnage = j.value("compagnonnage", "");
    p.toxicite_animaux = j.value("toxicite_animaux", "");
    p.vertus_medicinales = j.value("vertus_medicinales", "");
    p.precautions = j.value("precautions", "");
    p.astuce_pro = j.value("astuce_pro", "");
    p.notes = j.value("notes", "");
    p.ph_sol = j.value("ph_sol", "Inconnu");
    p.type_racinaire = j.value("type_racinaire", "Inconnu");
    p.vitesse_croissance = j.value("vitesse_croissance", "Moyenne");
    p.multiplication = j.value("multiplication", "Semis");
    p.tolerance_secheresse = j.value("tolerance_secheresse", "Moyenne");
    p.sensibilite_exces_eau = j.value("sensibilite_exces_eau", "Moyenne");
    p.contrainte_majeure = j.value("contrainte_majeure", "Aucune");
    p.score_balcon = j.value("score_balcon", 50);
}