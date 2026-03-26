#include "Catalogue.hpp"
#include "Plante.hpp"
#include <fstream>
#include <iostream>

using json = nlohmann::json;

void Catalogue::ajouterPlante(const Plante& p) {
    m_plantes.push_back(p);
}

void Catalogue::supprimerPlante(int index) {
    if (index >= 0 && index < (int)m_plantes.size())
        m_plantes.erase(m_plantes.begin() + index);
}

void Catalogue::modifierPlante(int index, const Plante& p) {
    if (index >= 0 && index < (int)m_plantes.size())
        m_plantes[index] = p;
}

void Catalogue::afficherTout() const {
    if (m_plantes.empty()) {
        std::cout << "Le catalogue est vide. 🌵" << std::endl;
        return;
    }
    for (const auto& p : m_plantes) p.afficher();
}

void Catalogue::sauvegarder(const std::string& nomFichier) const {
    json j = m_plantes; 
    std::ofstream f(nomFichier);
    if (f.is_open()) f << j.dump(4);
}

void Catalogue::charger(const std::string& nomFichier) {
    std::ifstream f(nomFichier);
    if (!f.is_open()) return;

    nlohmann::json j;
    f >> j;

    m_plantes.clear();
    if (j.is_array()) {
        for (const auto& item : j) {
            Plante p;
            from_json(item, p); 
            m_plantes.push_back(p);
        }
    }
    std::cout << "📂 " << m_plantes.size() << " plantes chargees." << std::endl;
}

int Catalogue::nombreDePlantes() const { return (int)m_plantes.size(); }

