#pragma once
#include <string>
#include <unordered_map>
#include "Data/Plante.hpp"

class DatabaseManager {
private:
    std::unordered_map<std::string, Plante> m_encyclopedia;

public:
    DatabaseManager() = default;
    
    bool chargerEncyclopedie(const std::string& filepath);
    const Plante* getPlante(const std::string& nom) const;
    const std::unordered_map<std::string, Plante>& getAllPlantes() const { return m_encyclopedia; }
};