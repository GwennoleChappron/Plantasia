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

        // Chargement des plantes
        if (j.contains("plantes")) {
            for (const auto& item : j["plantes"]) {
                m_myPlants.push_back(item.get<UserPlant>());
            }
        }
        
        // --- CORRECTION : Chargement de toute l'architecture ---
        if (j.contains("architecture")) {
            m_balconyConfig = j["architecture"].get<BalconyConfig>();
        } else {
            // Profil vierge : on initialise une grille par défaut (40x30)
            m_balconyConfig.width = 40;
            m_balconyConfig.height = 30;
            m_balconyConfig.grid.assign(30, std::vector<GridCell>(40));
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

    nlohmann::json j;
    j["plantes"] = m_myPlants;
    j["architecture"] = m_balconyConfig; // <-- MAGIE : JSON sauvegarde tout l'objet et sa grille
    
    file << j.dump(4); 
    return true;
}