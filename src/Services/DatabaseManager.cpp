#include "DatabaseManager.hpp"
#include "DataLoader.hpp"
#include <algorithm>
#include <cctype>

// ─────────────────────────────────────────────────────────────────────────────
//  chargerTout
// ─────────────────────────────────────────────────────────────────────────────

bool DatabaseManager::chargerTout(
    const std::string& cheminPlantes,
    const std::string& cheminSols,
    const std::string& cheminPots,
    const std::string& cheminBoutures)
{
    bool ok = true;
    ok &= DataLoader::chargerPlantes (cheminPlantes,  m_plantes);
    ok &= DataLoader::chargerSols    (cheminSols,     m_sols);
    ok &= DataLoader::chargerPots    (cheminPots,     m_pots);
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

const Soil* DatabaseManager::findSol(const std::string& typeSol) const {
    for (const auto& s : m_sols)
        if (s.typeSol == typeSol) return &s;
    return nullptr;
}

const Pot* DatabaseManager::findPot(const std::string& typeRacin) const {
    for (const auto& p : m_pots)
        if (p.typeRacinaire == typeRacin) return &p;
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

std::vector<const Plant*> DatabaseManager::filtrerParSol(const std::string& typeSol) const {
    std::vector<const Plant*> result;
    for (const auto& p : m_plantes)
        if (p.solRecommande == typeSol || p.solAlternatif == typeSol)
            result.push_back(&p);
    return result;
}