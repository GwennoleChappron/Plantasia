#include "Services/DataBaseManager.hpp"
#include "DataLoader.hpp"
#include "Data/EnumInfo.hpp" // Indispensable pour utiliser nos parseurs (typeSolFromString, etc.)
#include <algorithm>
#include <cctype>

// ─────────────────────────────────────────────────────────────────────────────
//  chargerTout
// ─────────────────────────────────────────────────────────────────────────────

bool DatabaseManager::chargerTout(
    const std::string& cheminPlantes,
    const std::string& cheminSols,
    const std::string& cheminRacines,
    const std::string& cheminBoutures)
{
    bool ok = true;
    ok &= DataLoader::chargerPlantes (cheminPlantes,  m_plantes);
    ok &= DataLoader::chargerSols    (cheminSols,     m_sols);
    ok &= DataLoader::chargerRacines (cheminRacines,  m_racines);
    ok &= DataLoader::chargerBoutures(cheminBoutures, m_boutures);

    if (ok) {
        construireCache();
        m_charge = true;
    }
    return ok;
}

// ─────────────────────────────────────────────────────────────────────────────
//  Cache — calculé une seule fois
// ─────────────────────────────────────────────────────────────────────────────

void DatabaseManager::construireCache() {
    // Pointeurs vers les plantes du vecteur principal
    m_plantesTrieesParNom.clear();
    m_plantesTrieesParScore.clear();
    m_plantesTrieesParNom.reserve(m_plantes.size());
    m_plantesTrieesParScore.reserve(m_plantes.size());

    for (const auto& p : m_plantes) {
        m_plantesTrieesParNom.push_back(&p);
        m_plantesTrieesParScore.push_back(&p);
    }

    std::sort(m_plantesTrieesParNom.begin(), m_plantesTrieesParNom.end(),
        [](const Plant* a, const Plant* b) {
            return a->nom < b->nom;
        });

    std::sort(m_plantesTrieesParScore.begin(), m_plantesTrieesParScore.end(),
        [](const Plant* a, const Plant* b) {
            return a->scoreBalcon > b->scoreBalcon;
        });
}

// ─────────────────────────────────────────────────────────────────────────────
//  Recherches par clé — O(n) acceptable sur 16-100 plantes
// ─────────────────────────────────────────────────────────────────────────────

const Plant* DatabaseManager::findPlante(const std::string& nom) const {
    for (const auto& p : m_plantes)
        if (p.nom == nom) return &p;
    return nullptr;
}

const Soil* DatabaseManager::findSol(const std::string& typeSolStr) const {
    // 1. On traduit le texte JSON en Enum C++
    TypeSol targetEnum = EnumInfo::typeSolFromString(typeSolStr);
    
    // 2. On cherche dans la liste
    for (const auto& s : m_sols) {
        if (s.typeSol == targetEnum) {
            return &s;
        }
    }
    return nullptr;
}

const Racine* DatabaseManager::findRacine(const std::string& typeRacineStr) const {
    TypeRacinaireEnum targetEnum = EnumInfo::parseTypeRacinaire(typeRacineStr);
    
    for (const auto& r : m_racines) {
        if (r.typeRacinaire == targetEnum) return &r; 
    }
    return nullptr;
}

const Bouture* DatabaseManager::findBouture(const std::string& nom) const {
    for (const auto& b : m_boutures)
        if (b.nom == nom) return &b;
    return nullptr;
}

// ─────────────────────────────────────────────────────────────────────────────
//  Filtres
// ─────────────────────────────────────────────────────────────────────────────

// Comparaison insensible à la casse (ASCII simple — suffisant pour noms latins)
static bool contientInsensible(const std::string& texte, const std::string& recherche) {
    if (recherche.empty()) return true;
    std::string t = texte, r = recherche;
    std::transform(t.begin(), t.end(), t.begin(), ::tolower);
    std::transform(r.begin(), r.end(), r.begin(), ::tolower);
    return t.find(r) != std::string::npos;
}

std::vector<const Plant*> DatabaseManager::filtrerParNom(const std::string& recherche) const {
    std::vector<const Plant*> result;
    for (const auto* p : m_plantesTrieesParNom)
        if (contientInsensible(p->nom, recherche) ||
            contientInsensible(p->nomScientifique, recherche))
            result.push_back(p);
    return result;
}

std::vector<const Plant*> DatabaseManager::filtrerParSol(const std::string& typeSolStr) const {
    std::vector<const Plant*> result;
    
    // 1. On transforme la chaîne passée en paramètre en énumération (ex: "TERREAU_UNIVERSEL" -> Enum::TERREAU_UNIVERSEL)
    TypeSol targetSol = EnumInfo::typeSolFromString(typeSolStr);
    
    if (targetSol == TypeSol::INCONNU) return result; // Sécurité si le texte est invalide

    // 2. On compare avec les nouveaux champs Enum de Plant !
    for (const auto& p : m_plantes) {
        if (p.solEnum == targetSol || p.solAlternatifEnum == targetSol) {
            result.push_back(&p);
        }
    }
    return result;
}