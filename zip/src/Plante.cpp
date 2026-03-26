#include "Plante.hpp"

void Plante::afficher() const {
    std::cout << "----- 🌿 " << nom << " (" << type << ") -----\n";
    std::cout << "Rusticité : " << rusticite << "\n";
    std::cout << "Exposition : " << exposition_soleil << " | Vent : " << exposition_vent << "\n";
    std::cout << "Arrosage : " << conseil_arrosage << "\n";
}

std::ostream& operator<<(std::ostream& os, const TypePlante& t) {
    switch (t) {
        case TypePlante::AROMATIQUE: os << "Aromatique"; break;
        case TypePlante::FRUITIER: os << "Fruitier"; break;
        case TypePlante::FLEUR: os << "Fleur"; break;
        case TypePlante::LEGUME: os << "Légume"; break;
    }
    return os;
}

std::ostream& operator<<(std::ostream& os, const Rusticite& r) {
    switch (r) {
        case Rusticite::RUSTIQUE: os << "Rustique"; break;
        case Rusticite::SEMI_RUSTIQUE: os << "Semi-rustique"; break;
        case Rusticite::FRAGILE: os << "Fragile"; break;
        case Rusticite::GELIVE: os << "Gélive"; break;
        case Rusticite::TROPICALE: os << "Tropicale"; break;
    }
    return os;
}

std::ostream& operator<<(std::ostream& os, const ExpositionSoleil& e) {
    switch (e) {
        case ExpositionSoleil::PLEIN_SOLEIL: os << "Plein soleil"; break;
        case ExpositionSoleil::MI_OMBRE: os << "Mi-ombre"; break;
        case ExpositionSoleil::OMBRE_CLAIRE: os << "Ombre claire"; break;
        case ExpositionSoleil::OMBRE_DENSE: os << "Ombre dense"; break;
    }
    return os;
}

std::ostream& operator<<(std::ostream& os, const ExpositionVent& e) {
    switch (e) {
        case ExpositionVent::ABRITE: os << "Abrité"; break;
        case ExpositionVent::MODERE: os << "Modéré"; break;
        case ExpositionVent::COULOIR_DE_VENT: os << "Couloir de vent"; break;
    }
    return os;
}