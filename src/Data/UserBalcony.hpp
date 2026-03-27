#pragma once
#include <string>
#include <vector>
#include <SFML/System/Vector2.hpp>
#include <nlohmann/json.hpp>
#include <SFML/System/Vector2.hpp>

struct UserPlant {
    std::string nom_espece; // Fait le lien avec l'encyclopédie (ex: "Thym")
    std::string surnom;     // Ex: "Thym de la cuisine"
    
    // Pour le futur simulateur 2D !
    sf::Vector2f position_balcon; 
    
    // Données perso
    int jour_achat = 1;
    int mois_achat = 3;
    int annee_achat = 2026;

    int volume_pot_actuel_L;
};

inline void to_json(nlohmann::json& j, const UserPlant& p) {
    j = nlohmann::json{
        {"nom_espece", p.nom_espece}, {"surnom", p.surnom},
        {"pos_x", p.position_balcon.x}, {"pos_y", p.position_balcon.y},
        {"jour_achat", p.jour_achat}, {"mois_achat", p.mois_achat}, {"annee_achat", p.annee_achat},
        {"volume_pot", p.volume_pot_actuel_L}
    };
}

inline void from_json(const nlohmann::json& j, UserPlant& p) {
    p.nom_espece = j.value("nom_espece", "Inconnu");
    p.surnom = j.value("surnom", "");
    p.position_balcon.x = j.value("pos_x", 0.0f);
    p.position_balcon.y = j.value("pos_y", 0.0f);
    p.jour_achat = j.value("jour_achat", 1);
    p.mois_achat = j.value("mois_achat", 3);
    p.annee_achat = j.value("annee_achat", 2026);
    p.volume_pot_actuel_L = j.value("volume_pot", 5);
}

// --- CONVERSIONS JSON POUR LES TYPES SFML (Dans leur namespace d'origine) ---
namespace sf {
    inline void to_json(nlohmann::json& j, const sf::Vector2i& v) {
        j = nlohmann::json{{"gx", v.x}, {"gy", v.y}};
    }

    inline void from_json(const nlohmann::json& j, sf::Vector2i& v) {
        v.x = j.value("gx", 0);
        v.y = j.value("gy", 0);
    }
}

class UserBalcony {
private:
    std::vector<UserPlant> m_myPlants;
    std::vector<sf::Vector2i> m_myWalls; // --- NOUVEAU : On stocke les murs ici ! ---

public:
    void ajouterPlante(const UserPlant& p) { m_myPlants.push_back(p); }
    const std::vector<UserPlant>& getMesPlantes() const { return m_myPlants; }
    std::vector<UserPlant>& getMesPlantesRef() { return m_myPlants; }
    
    // --- NOUVEAU : Accès aux murs ---
    std::vector<sf::Vector2i>& getMesMursRef() { return m_myWalls; }

    bool chargerProfil(const std::string& filepath);
    bool sauvegarderProfil(const std::string& filepath) const;
};