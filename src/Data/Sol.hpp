#pragma once
#include <string>
#include <vector>
#include <nlohmann/json.hpp>

class Sol {
public:
    std::string type_sol;
    std::string texture;
    std::string drainage;
    std::string retention_eau;
    std::string richesse;
    std::string ph_typique;
    std::string utilisation;
    
    std::vector<std::string> composition;
    std::vector<std::string> adapte_pour;
    std::vector<std::string> problemes_frequents;
    std::vector<std::string> amelioration_structure;

    // Corrections pH
    std::vector<std::string> correction_baisser_ph;
    std::vector<std::string> correction_augmenter_ph;

    // Statistiques techniques
    std::string cec;
    std::string aeration;
    std::string densite;

    Sol() = default;
};

inline void from_json(const nlohmann::json& j, Sol& s) {
    s.type_sol = j.value("type_sol", "Inconnu");
    s.texture = j.value("texture", "?");
    s.drainage = j.value("drainage", "?");
    s.retention_eau = j.value("retention_eau", "?");
    s.richesse = j.value("richesse", "?");
    s.ph_typique = j.value("ph_typique", "?");
    s.utilisation = j.value("utilisation", "?");
    s.cec = j.value("cec", "?");
    s.aeration = j.value("aeration", "?");
    s.densite = j.value("densite", "?");

    if (j.contains("composition")) s.composition = j["composition"].get<std::vector<std::string>>();
    if (j.contains("adapte_pour")) s.adapte_pour = j["adapte_pour"].get<std::vector<std::string>>();
    if (j.contains("problemes_frequents")) s.problemes_frequents = j["problemes_frequents"].get<std::vector<std::string>>();
    if (j.contains("amelioration_structure")) s.amelioration_structure = j["amelioration_structure"].get<std::vector<std::string>>();

    // Gestion de la correction du pH (qui a une structure imbriquée ou simple selon les sols)
    if (j.contains("correction_ph")) {
        auto& ph = j["correction_ph"];
        if (ph.contains("baisser_ph")) {
            if (ph["baisser_ph"].contains("methodes")) s.correction_baisser_ph = ph["baisser_ph"]["methodes"].get<std::vector<std::string>>();
            else if (ph["baisser_ph"].is_array()) s.correction_baisser_ph = ph["baisser_ph"].get<std::vector<std::string>>();
        }
        if (ph.contains("augmenter_ph")) {
            if (ph["augmenter_ph"].contains("methodes")) s.correction_augmenter_ph = ph["augmenter_ph"]["methodes"].get<std::vector<std::string>>();
            else if (ph["augmenter_ph"].is_array()) s.correction_augmenter_ph = ph["augmenter_ph"].get<std::vector<std::string>>();
        }
    }
}