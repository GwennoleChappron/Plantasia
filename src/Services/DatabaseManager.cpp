#include "DatabaseManager.hpp"
#include <fstream>
#include <iostream>

bool DatabaseManager::chargerEncyclopedie(const std::string& filepath) {
    std::ifstream file(filepath);
    if (!file.is_open()) {
        std::cerr << "Erreur: Impossible d'ouvrir " << filepath << "\n";
        return false;
    }

    try {
        nlohmann::json j;
        file >> j;
        m_encyclopedia.clear();

        for (const auto& item : j) {
            Plante p = item.get<Plante>();
            m_encyclopedia[p.nom] = p; // On indexe par le nom de la plante
        }
        return true;
    } catch (const std::exception& e) {
        std::cerr << "Erreur JSON dans l'encyclopédie : " << e.what() << "\n";
        return false;
    }
}

const Plante* DatabaseManager::getPlante(const std::string& nom) const {
    auto it = m_encyclopedia.find(nom);
    if (it != m_encyclopedia.end()) {
        return &(it->second);
    }
    return nullptr;
}