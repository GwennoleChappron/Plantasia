#include "Plante.hpp"
#include <iostream>
#include <string>

Plante::Plante(const std::string& nom, Rusticite rusticite, ExpositionSoleil exposition_soleil, ExpositionVent exposition_vent)
    : nom(nom), rusticite(rusticite), exposition_soleil(exposition_soleil), exposition_vent(exposition_vent) 
{

}
void Plante::afficher (const Plante& p) const {
    std::cout << "----- 🌿 " << p.nom << " (" << p.rusticite << ") -----\n";
    std::cout << "Exposition : " << p.exposition_soleil << "\n";
    std::cout << "Exposition au vent : " << p.exposition_vent << "\n";
    std::cout << "---------------------------\n";
}

std::ostream& operator<<(std::ostream& os, const Rusticite& r) {
    switch (r) {
        case Rusticite::RUSTIQUE:      os << "Rustique"; break;
        case Rusticite::SEMI_RUSTIQUE: os << "Semi-rustique"; break;
        case Rusticite::FRAGILE:       os << "Fragile"; break;
        case Rusticite::GELIVE:        os << "Gélive"; break;
        case Rusticite::TROPICALE:     os << "Tropicale"; break;
    }
    return os;
}
std::ostream& operator<<(std::ostream& os, const ExpositionSoleil& e) {
    switch (e) {
        case ExpositionSoleil::PLEIN_SOLEIL:    os << "Plein soleil"; break;
        case ExpositionSoleil::MI_OMBRE:       os << "Mi-ombre"; break;
        case ExpositionSoleil::OMBRE_CLAIRE:   os << "Ombre claire"; break;
        case ExpositionSoleil::OMBRE_DENSE:    os << "Ombre dense"; break;
    }
    return os;
}

std::ostream& operator<<(std::ostream& os, const ExpositionVent& e) {
    switch (e) {
        case ExpositionVent::ABRITE:         os << "Abrité"; break;
        case ExpositionVent::MODERE:         os << "Modéré"; break;
        case ExpositionVent::COULOIR_DE_VENT: os << "Couloir de vent"; break;
    }
    return os;
}