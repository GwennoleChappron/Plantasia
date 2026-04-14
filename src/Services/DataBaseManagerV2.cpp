#include "DataBaseManagerV2.hpp"
#include <iostream>
#include <stdexcept>

DatabaseManagerV2::DatabaseManagerV2(const std::string& dbPath) {
    try {
        // Ouverture en lecture seule (sécurisé pour l'UI)
        m_db = std::make_unique<SQLite::Database>(dbPath, SQLite::OPEN_READONLY);
        //m_db->exec("PRAGMA journal_mode=WAL;");

        // On charge tout dans la RAM au démarrage
        chargerUIMetadata();
        chargerPlantes();

        std::cout << "[DatabaseManagerV2] Base de donnees connectee avec succes !\n";
        std::cout << " -> " << m_plantes.size() << " plantes chargees en cache.\n";
    } catch (const SQLite::Exception& e) {
        std::cerr << "[DatabaseManagerV2] Erreur critique SQLite : " << e.what() << "\n";
        throw;
    }
}

// ─────────────────────────────────────────────────────────────
// Chargement du cache UI (Le fameux JOIN SQL)
// ─────────────────────────────────────────────────────────────
void DatabaseManagerV2::chargerUIMetadata() {
    // Liste des tables de dictionnaires à charger
    std::vector<std::string> refTables = {
        "ref_difficulte", "ref_type_plante", "ref_exposition_soleil",
        "ref_exposition_vent", "ref_rusticite", "ref_vitesse_croissance",
        "ref_tolerance_secheresse", "ref_sensibilite_eau", "ref_feuillage"
    };

    for (const auto& table : refTables) {
        chargerRefTable(table);
    }
}

void DatabaseManagerV2::chargerRefTable(const std::string& tableName) {
    // Cette requête récupère le texte ET va chercher la couleur dans ref_theme !
    std::string query = 
        "SELECT d.code, d.label_ui, d.description, t.r, t.g, t.b, t.a "
        "FROM " + tableName + " d "
        "JOIN ref_theme t ON d.theme_code = t.code";

    SQLite::Statement stmt(*m_db, query);

    while (stmt.executeStep()) {
        std::string code = stmt.getColumn(0).getText();
        
        UIMetadata meta;
        meta.label       = stmt.getColumn(1).getText();
        meta.description = stmt.getColumn(2).getText();
        meta.color.x     = static_cast<float>(stmt.getColumn(3).getDouble());
        meta.color.y     = static_cast<float>(stmt.getColumn(4).getDouble());
        meta.color.z     = static_cast<float>(stmt.getColumn(5).getDouble());
        meta.color.w     = static_cast<float>(stmt.getColumn(6).getDouble());

        m_uiCache[tableName][code] = meta;
    }
}

// ─────────────────────────────────────────────────────────────
// Accès O(1) aux couleurs et textes pour ImGui
// ─────────────────────────────────────────────────────────────
const UIMetadata& DatabaseManagerV2::uiOf(const std::string& refTable, const std::string& code) const {
    auto itTable = m_uiCache.find(refTable);
    if (itTable != m_uiCache.end()) {
        auto itCode = itTable->second.find(code);
        if (itCode != itTable->second.end()) {
            return itCode->second;
        }
    }
    return m_unknownMeta;
}

// ─────────────────────────────────────────────────────────────
// Chargement des Plantes
// ─────────────────────────────────────────────────────────────
void DatabaseManagerV2::chargerPlantes() {
    SQLite::Statement query(*m_db, 
        "SELECT id, nom, nom_scientifique, famille, origine, "
        "type_plante, rusticite, exposition_soleil, exposition_vent, "
        "difficulte, vitesse_croissance, tolerance_secheresse, "
        "sensibilite_eau, feuillage, type_racinaire, besoin_eau, score_balcon "
        "FROM plantes ORDER BY nom ASC"
    );

    while (query.executeStep()) {
        PlantRow p;
        p.id = query.getColumn(0).getInt();
        p.nom = query.getColumn(1).getText();
        p.nomScientifique = query.getColumn(2).getText();
        p.famille = query.getColumn(3).getText();
        p.origine = query.getColumn(4).getText();
        
        p.typePlante = query.getColumn(5).getText();
        p.rusticite = query.getColumn(6).getText();
        p.expositionSoleil = query.getColumn(7).getText();
        p.expositionVent = query.getColumn(8).getText();
        p.difficulte = query.getColumn(9).getText();
        p.vitesseCroissance = query.getColumn(10).getText();
        p.toleranceSecheresse = query.getColumn(11).getText();
        p.sensibiliteEau = query.getColumn(12).getText();
        p.feuillage = query.getColumn(13).getText();
        p.typeRacinaire = query.getColumn(14).getText();
        
        p.besoinEau = query.getColumn(15).getInt();
        p.scoreBalcon = query.getColumn(16).getInt();

        m_plantes.push_back(std::move(p));
    }
}

// ─────────────────────────────────────────────────────────────
// Exemple de requête de liaison complexe
// ─────────────────────────────────────────────────────────────
std::vector<std::string> DatabaseManagerV2::getCompagnons(int planteId) const {
    SQLite::Statement query(*m_db, 
        "SELECT p.nom FROM plantes p "
        "JOIN plante_compagnons c ON p.id = c.compagnon_id "
        "WHERE c.plante_id = ?"
    );
    query.bind(1, planteId); // Protection contre l'injection SQL

    std::vector<std::string> compagnons;
    while (query.executeStep()) {
        compagnons.push_back(query.getColumn(0).getText());
    }
    return compagnons;
}