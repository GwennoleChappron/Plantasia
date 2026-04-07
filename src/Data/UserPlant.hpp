#pragma once

#include <string>
#include <SFML/System/Vector2.hpp>

// ─────────────────────────────────────────────────────────────────────────────
// UserPlant
// Représente une instance de plante possédée par l'utilisateur sur son balcon.
// Contient uniquement les données variables et l'état actuel, tandis que les
// données fixes (botanique, arrosage max, etc.) restent dans la struct 'Plant'.
// ─────────────────────────────────────────────────────────────────────────────

struct UserPlant {

    // ── Identité & Lien DB ───────────────────────────────────────────────────
    
    // Clé étrangère qui correspond au champ "nom" dans Plant.hpp
    // Permet de retrouver la fiche de référence via DatabaseManager::findPlante()
    std::string nomEspece; 
    
    std::string surnom;    // Le petit nom donné par l'utilisateur (ex: "Bébert le Ficus")

    // ── État "Tamagotchi" (de 0.0f à 1.0f) ───────────────────────────────────
    
    float health    = 1.0f;  // Gère la teinte (assombrissement) et l'inclinaison
    float hydration = 1.0f;  // Gère le clignotement bleu (alerte soif)
    float happiness = 1.0f;  // Gère l'animation de rebond et le halo vert

    // ── Historique & Culture actuelle ────────────────────────────────────────
    
    int jourAchat  = 1;
    int moisAchat  = 1;
    int anneeAchat = 2024;
    
    int volumePotActuel_L = 5; // En litres, modifiable via le DragInt dans l'UI

    // ── Interface Utilisateur (UI) ───────────────────────────────────────────
    // Position libre sur "l'étagère interactive" du mode catalogue vide
    // Initialisé en négatif pour forcer le recalcul au premier affichage
    sf::Vector2f positionCatalogue = {-1.f, -1.f};
    sf::Vector2f positionBalcon   = {-1.f, -1.f}; // Position actuelle sur le balcon (modifiable par glisser-déposer)

};