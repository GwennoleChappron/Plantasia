#pragma once
#include <string>
#include <iostream>
#include <vector>
#include <nlohmann/json.hpp>

enum class TypePlante { AROMATIQUE = 0, FRUITIER = 1, FLEUR = 2, LEGUME = 3 };
enum class Rusticite { RUSTIQUE, SEMI_RUSTIQUE, FRAGILE, GELIVE, TROPICALE };
enum class ExpositionSoleil { PLEIN_SOLEIL, MI_OMBRE, OMBRE_CLAIRE, OMBRE_DENSE };
enum class ExpositionVent { ABRITE, MODERE, COULOIR_DE_VENT };

// Macros de sérialisation JSON
NLOHMANN_JSON_SERIALIZE_ENUM(TypePlante, {
    {TypePlante::AROMATIQUE, 0},
    {TypePlante::FRUITIER, 1},
    {TypePlante::FLEUR, 2},
    {TypePlante::LEGUME, 3}
})

NLOHMANN_JSON_SERIALIZE_ENUM(Rusticite, {
    {Rusticite::RUSTIQUE, "RUSTIQUE"},
    {Rusticite::SEMI_RUSTIQUE, "SEMI_RUSTIQUE"},
    {Rusticite::FRAGILE, "FRAGILE"},
    {Rusticite::GELIVE, "GELIVE"},
    {Rusticite::TROPICALE, "TROPICALE"}
})

NLOHMANN_JSON_SERIALIZE_ENUM(ExpositionSoleil, {
    {ExpositionSoleil::PLEIN_SOLEIL, "PLEIN_SOLEIL"},
    {ExpositionSoleil::MI_OMBRE, "MI_OMBRE"},
    {ExpositionSoleil::OMBRE_CLAIRE, "OMBRE_CLAIRE"},
    {ExpositionSoleil::OMBRE_DENSE, "OMBRE_DENSE"}
})

NLOHMANN_JSON_SERIALIZE_ENUM(ExpositionVent, {
    {ExpositionVent::ABRITE, "ABRITE"},
    {ExpositionVent::MODERE, "MODERE"},
    {ExpositionVent::COULOIR_DE_VENT, "COULOIR_DE_VENT"}
})

class Plante {
public:
    std::string nom;
    TypePlante type;
    Rusticite rusticite;
    ExpositionSoleil exposition_soleil;
    ExpositionVent exposition_vent;
    
    int floraison_debut;
    int floraison_fin;
    int recolte_debut;
    int recolte_fin;
    
    std::string conseil_terre;
    std::string conseil_arrosage;
    std::string conseil_entretien;
    std::string maladies;
    std::string notes;

    Plante() = default;
    
    std::string getNom() const { return nom; }
    void afficher() const;
};

// Fonctions JSON
inline void to_json(nlohmann::json& j, const Plante& p) {
    j = nlohmann::json{
        {"nom", p.nom},
        {"type", p.type},
        {"rusticite", p.rusticite},
        {"exposition_soleil", p.exposition_soleil},
        {"exposition_vent", p.exposition_vent},
        {"floraison_debut", p.floraison_debut},
        {"floraison_fin", p.floraison_fin},
        {"recolte_debut", p.recolte_debut},
        {"recolte_fin", p.recolte_fin},
        {"conseil_terre", p.conseil_terre},
        {"conseil_arrosage", p.conseil_arrosage},
        {"conseil_entretien", p.conseil_entretien},
        {"maladies", p.maladies},
        {"notes", p.notes}
    };
}

inline void from_json(const nlohmann::json& j, Plante& p) {
    j.at("nom").get_to(p.nom);
    j.at("type").get_to(p.type);
    j.at("rusticite").get_to(p.rusticite);
    j.at("exposition_soleil").get_to(p.exposition_soleil);
    j.at("exposition_vent").get_to(p.exposition_vent);
    j.at("floraison_debut").get_to(p.floraison_debut);
    j.at("floraison_fin").get_to(p.floraison_fin);
    j.at("recolte_debut").get_to(p.recolte_debut);
    j.at("recolte_fin").get_to(p.recolte_fin);
    j.at("conseil_terre").get_to(p.conseil_terre);
    j.at("conseil_arrosage").get_to(p.conseil_arrosage);
    j.at("conseil_entretien").get_to(p.conseil_entretien);
    j.at("maladies").get_to(p.maladies);
    j.at("notes").get_to(p.notes);
}

// Opérateurs d'affichage
std::ostream& operator<<(std::ostream& os, const TypePlante& t);
std::ostream& operator<<(std::ostream& os, const Rusticite& r);
std::ostream& operator<<(std::ostream& os, const ExpositionSoleil& e);
std::ostream& operator<<(std::ostream& os, const ExpositionVent& e);