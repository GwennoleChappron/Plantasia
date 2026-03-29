#pragma once
#include "./Data/Plante.hpp"
#include "./Data/Sol.hpp" 
#include <map>
#include <string>

class DatabaseManager {
private:
    std::map<std::string, Plante> m_encyclopedia;
    std::map<std::string, Sol> m_sols;

public:
    // Le nom correspond maintenant EXACTEMENT à ton appel dans Application.cpp
    void chargerEncyclopedie(const std::string& fichierPlantes); 
    
    // Méthodes pour les plantes
    const std::map<std::string, Plante>& getAllPlantes() const;
    const Plante* getPlante(const std::string& nom) const;

    // Méthodes pour les sols
    const std::map<std::string, Sol>& getAllSols() const;
    const Sol* getSol(const std::string& type_sol) const;
};