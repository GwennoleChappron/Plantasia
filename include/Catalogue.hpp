#pragma once
#include <vector>
#include <string>
#include "Plante.hpp"

class Catalogue {
private:
    std::vector<Plante> m_plantes;

public:
    const std::vector<Plante>& getPlantes() const { return m_plantes; }
    void ajouterPlante(const Plante& p);
    void modifierPlante(int index, const Plante& p);
    void supprimerPlante(int index);

    void afficherTout() const;
    int nombreDePlantes() const;
    void sauvegarder(const std::string& nomFichier) const;
    void charger(const std::string& nomFichier);
};