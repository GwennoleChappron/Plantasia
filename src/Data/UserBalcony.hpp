#pragma once
#include <string>
#include <vector>
#include <SFML/System/Vector2.hpp>
#include <nlohmann/json.hpp>
#include "BalconyConfig.hpp"
#include "UserPlant.hpp"




inline void to_json(nlohmann::json& j, const UserPlant& p) {
    j = nlohmann::json{
        {"nom_espece", p.nomEspece}, {"surnom", p.surnom},
        {"pos_x", p.positionBalcon.x}, {"pos_y", p.positionBalcon.y},
        {"cat_x", p.positionCatalogue.x}, {"cat_y", p.positionCatalogue.y},
        {"jour_achat", p.jourAchat}, {"mois_achat", p.moisAchat}, {"annee_achat", p.anneeAchat},
        {"volume_pot", p.volumePotActuel_L},
        {"hydration", p.hydration}, {"health", p.health}, {"happiness", p.happiness}
    };
}

inline void from_json(const nlohmann::json& j, UserPlant& p) {
    p.nomEspece = j.value("nom_espece", "Inconnu");
    p.surnom = j.value("surnom", "");
    p.positionBalcon.x = j.value("pos_x", -100.0f); // -100 = pas encore placée sur le balcon
    p.positionBalcon.y = j.value("pos_y", -100.0f);
    p.positionCatalogue.x = j.value("cat_x", -100.0f); 
    p.positionCatalogue.y = j.value("cat_y", -100.0f);
    p.jourAchat = j.value("jour_achat", 1);
    p.moisAchat = j.value("mois_achat", 3);
    p.anneeAchat = j.value("annee_achat", 2026);
    p.volumePotActuel_L = j.value("volume_pot", 5);
    p.hydration = j.value("hydration", 1.0f);
    p.health    = j.value("health",    1.0f);
    p.happiness = j.value("happiness", 0.8f);
}

class UserBalcony {
private:
    std::vector<UserPlant> m_myPlants;
    BalconyConfig m_balconyConfig; // <-- CORRECTION : On stocke toute l'architecture d'un coup

public:
    void ajouterPlante(const UserPlant& p) { m_myPlants.push_back(p); }
    const std::vector<UserPlant>& getMesPlantes() const { return m_myPlants; }
    std::vector<UserPlant>& getMesPlantesRef() { return m_myPlants; }
    
    // <-- CORRECTION : Accès à la configuration du balcon
    BalconyConfig& getBalconyConfigRef() { return m_balconyConfig; }

    bool chargerProfil(const std::string& filepath);
    bool sauvegarderProfil(const std::string& filepath) const;
};