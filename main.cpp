#include "Plante.hpp"

int main()
{
    Plante p1("Lavande", Rusticite::RUSTIQUE, ExpositionSoleil::PLEIN_SOLEIL, ExpositionVent::ABRITE);
    Plante p2("Fougère", Rusticite::FRAGILE, ExpositionSoleil::OMBRE_CLAIRE, ExpositionVent::MODERE);
    Plante p3("Cactus", Rusticite::TROPICALE, ExpositionSoleil::PLEIN_SOLEIL, ExpositionVent::ABRITE);

    p1.afficher(p1);
    p2.afficher(p2);
    p3.afficher(p3);

    return 0;
}