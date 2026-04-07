#include "CalendarManager.hpp"
#include "Services/DataBaseManager.hpp"

namespace {
    // Helper local pour ne pas dépasser le nombre de jours réel dans le mois
    int getJoursDansMois(int mois) {
        if (mois == 2) return 28; // On ignore les années bissextiles pour la simu
        if (mois == 4 || mois == 6 || mois == 9 || mois == 11) return 30;
        return 31;
    }
}

CalendarManager::CalendarManager() : m_currentMonth(3) {}

void CalendarManager::generateTasks(const UserBalcony& balcony, const DatabaseManager& db, int currentMonth) {
    m_currentTasks.clear();
    m_currentMonth = currentMonth;
    
    int joursMax = getJoursDansMois(m_currentMonth);

    for (const auto& up : balcony.getMesPlantes()) {
        // Remplacement de Plante par Plant, et utilisation de la bonne méthode (ex: findPlante)
        const Plant* refP = db.findPlante(up.nomEspece); 
        if (!refP) continue;

        // ── Événement 1 : Anniversaire d'Adoption (Rempotage) ──
        if (m_currentMonth == up.moisAchat && up.jourAchat <= joursMax) {
            m_currentTasks.push_back({
                "Anniversaire d'Adoption", 
                "Cela fait un an (ou plus). Vérifiez si votre " + up.surnom + " a besoin d'un plus grand pot !", 
                up.surnom, 
                TaskType::REMPOTAGE, 
                up.jourAchat, 
                false
            });
        }

        // ── Événement 2 : Les Arrosages ──
        // Utilisation de la nouvelle variable camelCase : besoinEau
        int intervalle;
        switch(refP->besoinEau) {
            case 5:  intervalle = 2;  break; // Très élevé : tous les 2 jours
            case 4:  intervalle = 3;  break; // Élevé : tous les 3 jours
            case 3:  intervalle = 5;  break; // Moyen : tous les 5 jours
            case 2:  intervalle = 10; break; // Faible : tous les 10 jours
            case 1:  intervalle = 15; break; // Très faible : tous les 15 jours
            default: intervalle = 30; break; // Nul : 1 fois par mois max
        }

        // Décalage pour ne pas arroser toutes les plantes le même jour de la semaine
        int decalage = (up.nomEspece.length() + up.jourAchat) % intervalle;
        int premierJour = 1 + decalage;

        // Déterminer la saison pour afficher la bonne consigne du JSON (nouvelles variables)
        bool estEnEte = (m_currentMonth >= 4 && m_currentMonth <= 9);
        std::string frequenceTexte = estEnEte ? refP->frequenceEte : refP->frequenceHiver;

        for (int j = premierJour; j <= joursMax; j += intervalle) {
            m_currentTasks.push_back({
                "Arrosage", 
                "Donnez à boire à votre " + up.surnom + " (" + frequenceTexte + ")", 
                up.surnom, 
                TaskType::ARROSAGE, 
                j, 
                false
            });
        }

        // ── Événement 3 : Récolte ──
        // Utilisation de recolteDebut au lieu de recolte_debut
        if (refP->recolteDebut > 0 && m_currentMonth == refP->recolteDebut) {
            m_currentTasks.push_back({
                "Début des récoltes", 
                "C'est la saison ! Observez votre " + up.surnom + " de près.", 
                up.surnom, 
                TaskType::RECOLTE, 
                15, // On le place symboliquement au milieu du mois (le 15)
                false 
            });
        }
    }
}