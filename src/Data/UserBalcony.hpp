#pragma once
#include <string>
#include <vector>
#include <SFML/System/Vector2.hpp>
#include <nlohmann/json.hpp>
#include "BalconyConfig.hpp" // <-- NOUVEAU : On inclut la structure du balcon

struct UserPlant {
    std::string nom_espece; 
    std::string surnom;     
    
    sf::Vector2f position_balcon;
    sf::Vector2f position_catalogue;
    
    int jour_achat = 1;
    int mois_achat = 3;
    int annee_achat = 2026;

    int volume_pot_actuel_L;

    // ── TAMAGOTCHI ──────────────────────────────
    float hydration = 1.0f;   // 0 → 1 (eau)
    float health    = 1.0f;   // 0 → 1 (santé)
    float happiness = 0.8f;   // 0 → 1 (bonheur)
};

inline void to_json(nlohmann::json& j, const UserPlant& p) {
    j = nlohmann::json{
        {"nom_espece", p.nom_espece}, {"surnom", p.surnom},
        {"pos_x", p.position_balcon.x}, {"pos_y", p.position_balcon.y},
        {"cat_x", p.position_catalogue.x}, {"cat_y", p.position_catalogue.y},
        {"jour_achat", p.jour_achat}, {"mois_achat", p.mois_achat}, {"annee_achat", p.annee_achat},
        {"volume_pot", p.volume_pot_actuel_L},
        {"hydration", p.hydration}, {"health", p.health}, {"happiness", p.happiness}
    };
}

inline void from_json(const nlohmann::json& j, UserPlant& p) {
    p.nom_espece = j.value("nom_espece", "Inconnu");
    p.surnom = j.value("surnom", "");
    p.position_balcon.x = j.value("pos_x", -100.0f); // -100 = pas encore placée sur le balcon
    p.position_balcon.y = j.value("pos_y", -100.0f);
    p.position_catalogue.x = j.value("cat_x", -100.0f); 
    p.position_catalogue.y = j.value("cat_y", -100.0f);
    p.jour_achat = j.value("jour_achat", 1);
    p.mois_achat = j.value("mois_achat", 3);
    p.annee_achat = j.value("annee_achat", 2026);
    p.volume_pot_actuel_L = j.value("volume_pot", 5);
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