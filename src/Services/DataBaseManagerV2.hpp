#pragma once
#include <SQLiteCpp/SQLiteCpp.h>
#include <vector>
#include <string>
#include <unordered_map>
#include <memory>
#include <imgui.h>

// ─────────────────────────────────────────────────────────────
// Structure pour l'UI ImGui (Remplace l'ancienne EnumMetadata)
// ─────────────────────────────────────────────────────────────
struct UIMetadata {
    std::string label;
    std::string description;
    ImVec4      color;
};

// ─────────────────────────────────────────────────────────────
// Structure allégée pour stocker une plante en mémoire
// On utilise des std::string pour stocker les clés (ex: "FACILE")
// ─────────────────────────────────────────────────────────────
struct PlantRow {
    int id;
    std::string nom;
    std::string nomScientifique;
    std::string famille;
    std::string origine;
    
    // Clés vers les dictionnaires
    std::string typePlante;
    std::string rusticite;
    std::string expositionSoleil;
    std::string expositionVent;
    std::string difficulte;
    std::string vitesseCroissance;
    std::string toleranceSecheresse;
    std::string sensibiliteEau;
    std::string feuillage;
    std::string typeRacinaire;

    int besoinEau;
    int scoreBalcon;
    
    // Tu pourras ajouter floraison_debut, conseil_entretien, etc.
};

class DatabaseManagerV2 {
public:
    // Le constructeur ouvre la DB et charge les caches
    explicit DatabaseManagerV2(const std::string& dbPath = "Data/assets/plantasia.db");

    // ── Accès aux données ────────────────────────────────────
    const std::vector<PlantRow>& getPlantes() const { return m_plantes; }

    // ── Accès à l'UI (La fonction Magique) ───────────────────
    // Exemple d'usage : uiOf("ref_difficulte", "FACILE")
    const UIMetadata& uiOf(const std::string& refTable, const std::string& code) const;

    // ── Requêtes de liaisons (Exécutées à la volée) ──────────
    // Récupère les noms des plantes compagnes pour un ID donné
    std::vector<std::string> getCompagnons(int planteId) const;

private:
    void chargerUIMetadata();
    void chargerRefTable(const std::string& tableName);
    void chargerPlantes();

    std::unique_ptr<SQLite::Database> m_db;
    
    std::vector<PlantRow> m_plantes;

    // Cache pour l'UI : m_uiCache["nom_table"]["CODE"] = UIMetadata
    std::unordered_map<std::string, std::unordered_map<std::string, UIMetadata>> m_uiCache;
    
    // Valeur de secours si la clé n'est pas trouvée
    UIMetadata m_unknownMeta{"?", "Non renseigné", ImVec4(0.4f, 0.5f, 0.4f, 1.0f)};
};