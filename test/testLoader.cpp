// test_loader.cpp — à compiler séparément pour valider le chargement
// g++ test_loader.cpp DataLoader.cpp DatabaseManager.cpp -I.. -std=c++17 -o test_loader

#include <iostream>
#include <cassert>
#include "../src/Services/DataLoader.hpp"
#include "../src/Services/DatabaseManager.hpp"

int main() {
    DatabaseManager db;

    bool ok = db.chargerTout(
        "../../encyclopedia.json",   // adapte les chemins selon ton build
        "../../sols.json",
        "../../pots.json",
        "../../boutures.json"
    );

    if (!ok) {
        std::cerr << "ECHEC du chargement.\n";
        return 1;
    }

    // ── Vérifications plantes ──────────────────────────────────────────────
    assert(!db.getPlantes().empty());
    std::cout << "Plantes chargees : " << db.getPlantes().size() << "\n";

    const Plant* thym = db.findPlante("Thym");
    assert(thym != nullptr);
    assert(thym->exposition == ExpositionSoleil::PLEIN_SOLEIL);
    assert(thym->difficulte == NiveauDifficulte::FACILE);
    assert(thym->solRecommande == "TERREAU_MEDITERRANEEN");
    assert(!thym->bouturesCompatibles.empty());
    std::cout << "  Thym : OK (sol=" << thym->solRecommande
              << ", score=" << thym->scoreBalcon << ")\n";

    // Liste triée par nom
    const auto& triNom = db.getPlantesTrieesParNom();
    assert(triNom.size() == db.getPlantes().size());
    for (size_t i = 1; i < triNom.size(); ++i)
        assert(triNom[i-1]->nom <= triNom[i]->nom);
    std::cout << "  Tri alphabetique : OK\n";

    // Liste triée par score
    const auto& triScore = db.getPlantesTrieesParScore();
    for (size_t i = 1; i < triScore.size(); ++i)
        assert(triScore[i-1]->scoreBalcon >= triScore[i]->scoreBalcon);
    std::cout << "  Tri par score balcon : OK\n";

    // Filtre
    auto resultats = db.filtrerParNom("Menthe");
    std::cout << "  Filtre 'Menthe' -> " << resultats.size() << " résultats\n";

    // ── Vérifications sols ─────────────────────────────────────────────────
    assert(!db.getSols().empty());
    std::cout << "Sols charges : " << db.getSols().size() << "\n";

    const Soil* med = db.findSol("TERREAU_MEDITERRANEEN");
    assert(med != nullptr);
    assert(med->phMin > 6.f);
    std::cout << "  TERREAU_MEDITERRANEEN : ph " << med->phMin << "-" << med->phMax << " OK\n";

    // Filtre sol -> plantes
    auto plantsMed = db.filtrerParSol("TERREAU_MEDITERRANEEN");
    std::cout << "  Plantes pour TERREAU_MEDITERRANEEN : " << plantsMed.size() << "\n";

    // ── Vérifications pots ─────────────────────────────────────────────────
    assert(!db.getPots().empty());
    std::cout << "Pots charges : " << db.getPots().size() << "\n";

    const Pot* potTracant = db.findPot("TRACANT");
    assert(potTracant != nullptr);
    assert(potTracant->volumeMin > 0);
    std::cout << "  Pot TRACANT : " << potTracant->volumeMin << "-"
              << potTracant->volumeMax << " L OK\n";

    // ── Vérifications boutures ─────────────────────────────────────────────
    assert(!db.getBoutures().empty());
    std::cout << "Boutures chargees : " << db.getBoutures().size() << "\n";

    const Bouture* herb = db.findBouture("BOUTURE_HERBACEE");
    assert(herb != nullptr);
    assert(herb->enracinementMin == 7);
    assert(herb->longueurMin != -1);
    std::cout << "  BOUTURE_HERBACEE : enracinement " << herb->enracinementMin
              << "-" << herb->enracinementMax << " j OK\n";

    // Bouture feuille — longueur null dans JSON, doit être -1
    const Bouture* feuille = db.findBouture("BOUTURE_FEUILLE");
    assert(feuille != nullptr);
    assert(feuille->longueurMin == -1);
    std::cout << "  BOUTURE_FEUILLE : longueur null -> -1 OK\n";

    // Vérification bouture compatible depuis une plante
    assert(thym->bouturesCompatibles[0] == "BOUTURE_SEMI_LIGNEUSE");
    const Bouture* boutureThym = db.findBouture(thym->bouturesCompatibles[0]);
    assert(boutureThym != nullptr);
    std::cout << "  Bouture du Thym : " << boutureThym->nom
              << " (" << boutureThym->tauxReussite << ")\n";

    std::cout << "\nTous les tests passes.\n";
    return 0;
}