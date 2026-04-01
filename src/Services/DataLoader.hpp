#pragma once
#include <vector>
#include <string>
#include "Data/Plante.hpp"
#include "Data/Soil.hpp"
#include "Data/RacineBouture.hpp"

// ─────────────────────────────────────────────────────────────────────────────
//  DataLoader
//
//  Rôle unique : lire les fichiers JSON et remplir les vecteurs de structs.
//  Aucune logique métier ici, aucune dépendance à ImGui ou SFML.
//
//  Usage :
//    std::vector<Plant>   plantes;
//    std::vector<Soil>    sols;
//    std::vector<Pot>     pots;
//    std::vector<Bouture> boutures;
//
//    DataLoader::chargerPlantes ("data/encyclopedia.json", plantes);
//    DataLoader::chargerSols    ("data/sols.json",         sols);
//    DataLoader::chargerPots    ("data/pots.json",         pots);
//    DataLoader::chargerBoutures("data/boutures.json",     boutures);
// ─────────────────────────────────────────────────────────────────────────────

namespace DataLoader {

bool chargerPlantes (const std::string& chemin, std::vector<Plant>&   out);
bool chargerSols    (const std::string& chemin, std::vector<Soil>&    out);
bool chargerRacines    (const std::string& chemin, std::vector<Racine>&     out);
bool chargerBoutures(const std::string& chemin, std::vector<Bouture>& out);

}