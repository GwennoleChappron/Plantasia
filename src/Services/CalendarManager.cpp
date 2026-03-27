#include "CalendarManager.hpp"

CalendarManager::CalendarManager() : m_currentMonth(3) {}

void CalendarManager::generateTasks(const UserBalcony& balcony, const DatabaseManager& db, int currentMonth) {
    m_currentTasks.clear();
    m_currentMonth = currentMonth;

    for (const auto& up : balcony.getMesPlantes()) {
        const Plante* refP = db.getPlante(up.nom_espece);
        if (!refP) continue;

        // Événement 1 : Anniversaire d'Adoption (Rempotage)
        // Si on simule le mois de l'achat, on place un rappel de rempotage au jour exact !
        if (m_currentMonth == up.mois_achat) {
            m_currentTasks.push_back({
                "Anniversaire d'Adoption", 
                "Cela fait un an (ou plus). Verifiez si votre " + up.surnom + " a besoin d'un plus grand pot !", 
                up.surnom, TaskType::REMPOTAGE, up.jour_achat, false
            });
        }

        // Événement 2 : Les Arrosages
        // On génère plusieurs arrosages dans le mois en fonction du besoin en eau de la plante
        int intervalle = (refP->besoin_eau >= 3) ? 4 : 8; // Tous les 4 jours si gourmande, sinon 8
        for (int j = (up.jour_achat % intervalle) + 1; j <= 31; j += intervalle) {
            m_currentTasks.push_back({
                "Arrosage", 
                "Donnez a boire a votre " + up.surnom + " (" + refP->frequence_arrosage_ete + ")", 
                up.surnom, TaskType::ARROSAGE, j, false
            });
        }

        // Événement 3 : Récolte
        if (m_currentMonth == refP->recolte_debut) {
            m_currentTasks.push_back({
                "Debut des recoltes", 
                "C'est la saison ! Observez votre " + up.surnom + " de pres.", 
                up.surnom, TaskType::RECOLTE, 15, false // On le place au milieu du mois (le 15)
            });
        }
    }
}