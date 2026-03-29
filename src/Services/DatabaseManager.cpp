#include "DatabaseManager.hpp"
#include <fstream>
#include <iostream>
#include <nlohmann/json.hpp>

void DatabaseManager::chargerEncyclopedie(const std::string& fichierPlantes) {
    // On s'assure de chercher dans le dossier assets/
    std::string cheminPlantes = "assets/" + fichierPlantes;

    // 1. --- CHARGEMENT DES PLANTES ---
    std::ifstream filePlantes(cheminPlantes);
    if (filePlantes.is_open()) {
        try {
            nlohmann::json j;
            filePlantes >> j;
            for (auto& item : j) {
                Plante p = item.get<Plante>();
                m_encyclopedia[p.nom] = p;
            }
            std::cout << "Succes : " << m_encyclopedia.size() << " plantes chargees (" << cheminPlantes << ")." << std::endl;
        } catch (const std::exception& e) {
            std::cerr << "ERREUR lecture " << cheminPlantes << " : " << e.what() << std::endl;
        }
    } else {
        std::cerr << "ERREUR CRITIQUE : Fichier " << cheminPlantes << " introuvable !" << std::endl;
    }

    // 2. --- CHARGEMENT DES SOLS ---
    std::ifstream fileSols("assets/sols.json");
    if (fileSols.is_open()) {
        try {
            nlohmann::json j;
            fileSols >> j;
            for (auto& item : j) {
                Sol s = item.get<Sol>();
                m_sols[s.type_sol] = s;
            }
            std::cout << "Succes : " << m_sols.size() << " sols charges (assets/sols.json)." << std::endl;
        } catch (const std::exception& e) {
            std::cerr << "ERREUR lecture sols.json : " << e.what() << std::endl;
        }
    } else {
        std::cerr << "ERREUR CRITIQUE : Fichier assets/sols.json introuvable !" << std::endl;
    }
}

const std::map<std::string, Plante>& DatabaseManager::getAllPlantes() const { 
    return m_encyclopedia; 
}

const Plante* DatabaseManager::getPlante(const std::string& nom) const {
    auto it = m_encyclopedia.find(nom);
    if (it != m_encyclopedia.end()) return &(it->second);
    return nullptr;
}

const std::map<std::string, Sol>& DatabaseManager::getAllSols() const { 
    return m_sols; 
}

const Sol* DatabaseManager::getSol(const std::string& type_sol) const {
    auto it = m_sols.find(type_sol);
    if (it != m_sols.end()) return &(it->second);
    return nullptr;
}