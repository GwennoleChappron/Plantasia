#pragma once
#include <string>
#include <iostream>
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
    TypePlante type;
    Rusticite rusticite;
    ExpositionSoleil exposition_soleil;
    ExpositionVent exposition_vent;
    
    NiveauDifficulte niveau_difficulte;
    std::string volume_pot_min;
    int besoin_eau; // <-- NOUVEAU (1 à 4)

    int floraison_debut;
    int floraison_fin;
    int recolte_debut;
    int recolte_fin;
    
    std::string conseil_terre;
    std::string conseil_arrosage;
    std::string frequence_arrosage_ete;
    std::string frequence_arrosage_hiver;
    std::string conseil_entretien;
    std::string rempotage;
    std::string maladies;
    std::string compagnonnage;
    std::string toxicite_animaux;
    std::string astuce_pro;
    std::string notes;

    Plante() = default;
    std::string getNom() const { return nom; }
};

inline void to_json(nlohmann::json& j, const Plante& p) {
    j = nlohmann::json{
        {"nom", p.nom}, {"type", p.type}, {"rusticite", p.rusticite},
        {"exposition_soleil", p.exposition_soleil}, {"exposition_vent", p.exposition_vent},
        {"niveau_difficulte", p.niveau_difficulte}, {"volume_pot_min", p.volume_pot_min},
        {"besoin_eau", p.besoin_eau}, // <-- NOUVEAU
        {"floraison_debut", p.floraison_debut}, {"floraison_fin", p.floraison_fin},
        {"recolte_debut", p.recolte_debut}, {"recolte_fin", p.recolte_fin},
        {"conseil_terre", p.conseil_terre}, {"conseil_arrosage", p.conseil_arrosage},
        {"frequence_arrosage_ete", p.frequence_arrosage_ete}, {"frequence_arrosage_hiver", p.frequence_arrosage_hiver},
        {"conseil_entretien", p.conseil_entretien}, {"rempotage", p.rempotage},
        {"maladies", p.maladies}, {"compagnonnage", p.compagnonnage},
        {"toxicite_animaux", p.toxicite_animaux}, {"astuce_pro", p.astuce_pro}, {"notes", p.notes}
    };
}

inline void from_json(const nlohmann::json& j, Plante& p) {
    p.nom = j.value("nom", "Inconnu");
    p.type = j.value("type", TypePlante::AROMATIQUE);
    p.rusticite = j.value("rusticite", Rusticite::RUSTIQUE);
    p.exposition_soleil = j.value("exposition_soleil", ExpositionSoleil::MI_OMBRE);
    p.exposition_vent = j.value("exposition_vent", ExpositionVent::ABRITE);
    
    p.niveau_difficulte = j.value("niveau_difficulte", NiveauDifficulte::FACILE);
    p.volume_pot_min = j.value("volume_pot_min", "?");
    p.besoin_eau = j.value("besoin_eau", 2); // <-- NOUVEAU (Valeur par défaut à 2)

    p.floraison_debut = j.value("floraison_debut", 0);
    p.floraison_fin = j.value("floraison_fin", 0);
    p.recolte_debut = j.value("recolte_debut", 0);
    p.recolte_fin = j.value("recolte_fin", 0);

    p.conseil_terre = j.value("conseil_terre", "");
    p.conseil_arrosage = j.value("conseil_arrosage", "");
    p.frequence_arrosage_ete = j.value("frequence_arrosage_ete", "");
    p.frequence_arrosage_hiver = j.value("frequence_arrosage_hiver", "");
    p.conseil_entretien = j.value("conseil_entretien", "");
    p.rempotage = j.value("rempotage", "");
    p.maladies = j.value("maladies", "");
    p.compagnonnage = j.value("compagnonnage", "");
    p.toxicite_animaux = j.value("toxicite_animaux", "");
    p.astuce_pro = j.value("astuce_pro", "");
    p.notes = j.value("notes", "");
}