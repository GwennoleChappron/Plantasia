#include "UserBalcony.hpp"
#include <fstream>
#include <iostream>

bool UserBalcony::chargerProfil(const std::string& filepath) {
    std::ifstream file(filepath);
    if (!file.is_open()) return false; 

    try {
        nlohmann::json j;
        file >> j;
        
        m_myPlants.clear();
        m_myWalls.clear();

        // Chargement des plantes (comme avant)
        if (j.contains("plantes")) {
            for (const auto& item : j["plantes"]) {
                m_myPlants.push_back(item.get<UserPlant>());
            }
        }
        
        // --- NOUVEAU : Chargement des murs ---
        if (j.contains("murs")) {
            for (const auto& item : j["murs"]) {
                m_myWalls.push_back(item.get<sf::Vector2i>());
            }
        }
        
        return true;
    } catch (const std::exception& e) {
        std::cerr << "Erreur de lecture du profil : " << e.what() << "\n";
        return false;
    }
}

bool UserBalcony::sauvegarderProfil(const std::string& filepath) const {
    std::ofstream file(filepath);
    if (!file.is_open()) return false;

    // --- NOUVEAU : Structure de sauvegarde composite ---
    nlohmann::json j;
    j["plantes"] = m_myPlants;
    j["murs"] = m_myWalls; // On sauvegarde le vecteur de murs d'un coup !
    
    file << j.dump(4); // Indentation de 4 espaces pour la lisibilité
    return true;
}