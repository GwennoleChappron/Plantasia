#pragma once
#include <string>
#include <iostream>
#include <vector>

enum class Rusticite {
    RUSTIQUE,        // Supporte le gros gel
    SEMI_RUSTIQUE,   // Supporte un petit gel (-5°C)
    FRAGILE,         // Supporte 0°C mais pas moins
    GELIVE,          // Meurt au premier gel
    TROPICALE        // Doit rester au chaud (> 12°C)
};
enum class ExpositionSoleil {
    PLEIN_SOLEIL,    // > 6h direct
    MI_OMBRE,       // 3h à 6h direct
    OMBRE_CLAIRE,   // Lumineux mais pas de rayons
    OMBRE_DENSE     // Nord, sombre
};

enum class ExpositionVent {
    ABRITE,         // Protégé par des murs/vitres
    MODERE,         // Circulation d'air normale
    COULOIR_DE_VENT // Fortes rafales, dessèchement rapide
};

class Plante
{
    public:
    std::string nom;
    Rusticite rusticite;
    ExpositionSoleil exposition_soleil;
    ExpositionVent exposition_vent;
    Plante(const std::string& nom, Rusticite rusticite, ExpositionSoleil exposition_soleil, ExpositionVent exposition_vent);
    void afficher(const Plante& p) const;
};
std::ostream& operator<<(std::ostream& os, const Rusticite& r);
std::ostream& operator<<(std::ostream& os, const ExpositionSoleil& e);
std::ostream& operator<<(std::ostream& os, const ExpositionVent& e);