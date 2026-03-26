#ifdef _WIN32
#include <windows.h>
#pragma comment(linker, "/SUBSYSTEM:console")
#endif

#include <SFML/Graphics.hpp>
#include "imgui.h"
#include "imgui-SFML.h"
#include "Plante.hpp"
#include "Catalogue.hpp"
#include <iostream>
#include <string>
#include <cstring>
#include <cmath>
#include <vector>
#include <algorithm>

// ─────────────────────────────────────────────
//  PALETTE
// ─────────────────────────────────────────────
namespace Couleurs {
    // On met un peu de transparence (0.90f au lieu de 1.00f) sur la surface pour voir les lucioles à travers !
    const ImVec4 Fond          = ImVec4(0.04f, 0.06f, 0.04f, 1.00f);
    const ImVec4 Surface       = ImVec4(0.07f, 0.10f, 0.07f, 0.90f); 
    const ImVec4 SurfaceHaute  = ImVec4(0.11f, 0.15f, 0.11f, 0.95f);
    const ImVec4 Bordure       = ImVec4(0.18f, 0.28f, 0.18f, 1.00f);
    const ImVec4 Accent        = ImVec4(0.20f, 0.75f, 0.40f, 1.00f);
    const ImVec4 AccentDoux    = ImVec4(0.15f, 0.55f, 0.30f, 1.00f);
    const ImVec4 AccentSombre  = ImVec4(0.10f, 0.35f, 0.18f, 1.00f);
    const ImVec4 Texte         = ImVec4(0.88f, 0.92f, 0.88f, 1.00f);
    const ImVec4 TexteDoux     = ImVec4(0.55f, 0.68f, 0.55f, 1.00f);
    const ImVec4 TexteInactif  = ImVec4(0.30f, 0.40f, 0.30f, 1.00f);
}

// ─────────────────────────────────────────────
//  STYLE IMGUI
// ─────────────────────────────────────────────
void AppliquerStyle() {
    ImGuiStyle& s = ImGui::GetStyle();
    s.WindowRounding    = 12.0f;
    s.FrameRounding     = 8.0f;
    s.PopupRounding     = 10.0f;
    s.ChildRounding     = 8.0f;
    s.WindowBorderSize  = 1.0f;
    s.ItemSpacing       = ImVec2(10.0f, 10.0f);
    s.FramePadding      = ImVec2(12.0f, 8.0f);
    s.WindowPadding     = ImVec2(18.0f, 16.0f);

    ImVec4* c = s.Colors;
    c[ImGuiCol_WindowBg]             = Couleurs::Surface;
    c[ImGuiCol_ChildBg]              = ImVec4(0.04f, 0.06f, 0.04f, 0.70f); // Fond des listes plus transparent
    c[ImGuiCol_Border]               = Couleurs::Bordure;
    c[ImGuiCol_FrameBg]              = Couleurs::SurfaceHaute;
    c[ImGuiCol_FrameBgHovered]       = ImVec4(0.15f, 0.22f, 0.15f, 1.0f);
    c[ImGuiCol_FrameBgActive]        = ImVec4(0.18f, 0.28f, 0.18f, 1.0f);
    c[ImGuiCol_Button]               = Couleurs::AccentSombre;
    c[ImGuiCol_ButtonHovered]        = Couleurs::AccentDoux;
    c[ImGuiCol_ButtonActive]         = Couleurs::Accent;
    c[ImGuiCol_Text]                 = Couleurs::Texte;
    c[ImGuiCol_TextDisabled]         = Couleurs::TexteInactif;
    
    // Couleurs pour les onglets
    c[ImGuiCol_Tab]                  = Couleurs::SurfaceHaute;
    c[ImGuiCol_TabHovered]           = Couleurs::AccentDoux;
    c[ImGuiCol_TabActive]            = Couleurs::AccentSombre;
    c[ImGuiCol_TabUnfocused]         = Couleurs::Surface;
    c[ImGuiCol_TabUnfocusedActive]   = Couleurs::SurfaceHaute;
}

// ─────────────────────────────────────────────
//  ANIMATIONS & DESSINS (Lianes + Lucioles)
// ─────────────────────────────────────────────
struct PointLiane { float x, y; };

std::vector<PointLiane> GenererLiane(float sx, float sy, float ex, float ey, float amp, float freq, float temps, bool vertical) {
    std::vector<PointLiane> pts;
    for (int i = 0; i <= 60; i++) {
        float t = (float)i / 60.0f;
        float onde = std::sin(t * freq + temps) * amp * (0.3f + 0.7f * t);
        pts.push_back({
            sx + (ex - sx) * t + (vertical ? onde : 0.0f),
            sy + (ey - sy) * t + (vertical ? 0.0f : onde)
        });
    }
    return pts;
}

void DessinerFeuille(ImDrawList* dl, float x, float y, float angle, float taille, ImU32 col) {
    std::vector<ImVec2> pts;
    for (int i = 0; i <= 8; i++) {
        float a = (float)i / 8.0f * 3.14159265f;
        float rx = std::cos(a) * taille * 0.4f;
        float ry = std::sin(a) * taille;
        pts.push_back(ImVec2(
            x + rx * std::cos(angle) - ry * std::sin(angle),
            y + rx * std::sin(angle) + ry * std::cos(angle)
        ));
    }
    pts.push_back(ImVec2(x, y));
    dl->AddConvexPolyFilled(pts.data(), (int)pts.size(), col);
}

void DessinerLianesEtLucioles(ImDrawList* dl, float W, float H, float temps) {
    // 1. Lucioles (Plus grosses, plus nombreuses, plus lumineuses)
    for(int i = 0; i < 80; i++) {
        float sx = std::fmod(i * 137.5f, W);
        float sy = std::fmod(i * 93.1f, H);
        float speed = 5.f + (i % 8);
        float x = sx + std::sin(temps * 0.4f + i) * 50.f;
        float y = sy - std::fmod(temps * speed, H);
        if (y < 0) y += H;
        float alpha = (std::sin(temps * 2.0f + i) * 0.5f + 0.5f) * 255.f; 
        // Tailles variables pour la profondeur
        float rayon = 2.0f + (i % 4) * 0.8f;
        dl->AddCircleFilled(ImVec2(x, y), rayon, IM_COL32(190, 255, 140, (int)alpha));
    }

    // 2. Lianes (Décalées vers l'intérieur pour ne pas être cachées par la marge)
    const ImU32 tige  = IM_COL32(30, 90, 45, 220);
    const ImU32 feuA  = IM_COL32(40, 130, 55, 200);
    const ImU32 feuB  = IM_COL32(25, 85, 40, 180);

    auto tracerTige = [&](const std::vector<PointLiane>& pts, float ep) {
        for (int i = 1; i < (int)pts.size(); i++)
            dl->AddLine(ImVec2(pts[i-1].x, pts[i-1].y), ImVec2(pts[i].x, pts[i].y), tige, ep);
    };

    // Liane gauche (Positionnée à X = 20 au lieu de 8)
    auto lg = GenererLiane(20, H*0.95f, 20, H*0.05f, 15.f, 3.5f, temps*0.4f, true);
    tracerTige(lg, 3.0f);
    for (int i = 5; i < (int)lg.size()-5; i += 7) {
        float t = (float)i/lg.size();
        float a = 3.141f*0.4f + std::sin(t*5.f+temps)*0.3f;
        DessinerFeuille(dl, lg[i].x+12, lg[i].y,  a, 12.f + std::sin(t*4.f)*4.f, (i%2==0)?feuB:feuA);
        DessinerFeuille(dl, lg[i].x-12, lg[i].y, -a, 9.f + std::sin(t*4.f)*3.f, feuA);
    }

    // Liane droite (Positionnée à X = W - 20)
    auto ld = GenererLiane(W-20, H*0.90f, W-20, H*0.08f, 15.f, 3.5f, temps*0.4f+1.5f, true);
    tracerTige(ld, 3.0f);
    for (int i = 5; i < (int)ld.size()-5; i += 7) {
        float t = (float)i/ld.size();
        float a = 3.141f*0.6f + std::sin(t*5.f+temps+1.f)*0.3f;
        DessinerFeuille(dl, ld[i].x-12, ld[i].y, -a, 11.f + std::sin(t*4.5f)*4.f, (i%2==0)?feuB:feuA);
        DessinerFeuille(dl, ld[i].x+12, ld[i].y,  a, 8.f + std::sin(t*4.5f)*3.f, feuA);
    }
}

// ─────────────────────────────────────────────
//  WIDGETS CUSTOM (Calendrier)
// ─────────────────────────────────────────────
void DessinerCalendrierVisuel(const char* label, int debut, int fin, ImVec4 col) {
    ImGui::TextColored(Couleurs::TexteDoux, "%s", label);
    ImVec2 p = ImGui::GetCursorScreenPos();
    float w = ImGui::GetContentRegionAvail().x;
    float step = w / 12.f;
    ImDrawList* dl = ImGui::GetWindowDrawList();
    const char* initMois[] = {"J","F","M","A","M","J","J","A","S","O","N","D"};
    
    for(int i = 0; i < 12; i++) {
        bool on = false;
        if (debut > 0 && fin > 0) {
            int d = debut - 1, f = fin - 1;
            if (d <= f) on = (i >= d && i <= f);
            else        on = (i >= d || i <= f);
        }
        ImVec2 pmin(p.x + i * step + 2, p.y);
        ImVec2 pmax(p.x + (i + 1) * step - 2, p.y + 20); // Légèrement plus haut
        
        dl->AddRectFilled(pmin, pmax, on ? ImGui::ColorConvertFloat4ToU32(col) : IM_COL32(35,45,35,255), 4.f);
        
        if (on) {
            ImVec2 tsz = ImGui::CalcTextSize(initMois[i]);
            dl->AddText(ImVec2(pmin.x + (step - 4 - tsz.x) * 0.5f, pmin.y + 3), IM_COL32(10,20,10,255), initMois[i]);
        }
    }
    ImGui::Dummy(ImVec2(0, 26)); 
}

const char* RusiciteLabel(Rusticite r) {
    switch (r) {
        case Rusticite::RUSTIQUE:      return "Rustique";
        case Rusticite::SEMI_RUSTIQUE: return "Semi-rustique";
        case Rusticite::FRAGILE:       return "Fragile";
        case Rusticite::GELIVE:        return "Gelive";
        case Rusticite::TROPICALE:     return "Tropicale";
    }
    return "?";
}

ImVec4 CouleurRusticite(Rusticite r) {
    switch (r) {
        case Rusticite::RUSTIQUE:      return ImVec4(0.30f, 0.80f, 0.45f, 1.0f);
        case Rusticite::SEMI_RUSTIQUE: return ImVec4(0.60f, 0.80f, 0.30f, 1.0f);
        case Rusticite::FRAGILE:       return ImVec4(0.90f, 0.70f, 0.20f, 1.0f);
        case Rusticite::GELIVE:        return ImVec4(0.50f, 0.80f, 0.95f, 1.0f);
        case Rusticite::TROPICALE:     return ImVec4(0.95f, 0.55f, 0.25f, 1.0f);
    }
    return Couleurs::Texte;
}

ImVec4 CouleurTypePlante(TypePlante t) {
    switch (t) {
        case TypePlante::AROMATIQUE: return ImVec4(0.4f, 0.8f, 0.5f, 1.0f); 
        case TypePlante::FRUITIER:   return ImVec4(1.0f, 0.6f, 0.2f, 1.0f); 
        case TypePlante::FLEUR:      return ImVec4(1.0f, 0.5f, 0.7f, 1.0f); 
        case TypePlante::LEGUME:     return ImVec4(0.3f, 0.7f, 1.0f, 1.0f); 
    }
    return Couleurs::TexteDoux;
}

// ─────────────────────────────────────────────
//  MAIN
// ─────────────────────────────────────────────
int main(int argc, char** argv) {
#ifdef _WIN32
    SetConsoleOutputCP(CP_UTF8);
#endif
    sf::RenderWindow window(sf::VideoMode(1100, 720), "Plantasia - Mon Jardin");
    window.setFramerateLimit(60);
    if (!ImGui::SFML::Init(window)) return -1;
    AppliquerStyle();

    Catalogue monBalcon;
    monBalcon.charger("balcon.json");

    int indexSelectionne   = -1;
    char filtreRecherche[128] = {};
    sf::Clock deltaClock;
    float temps = 0.f;

    const float LARGEUR_LISTE = 340.f;
    // On agrandit la marge pour dégager les bords et bien voir les lianes
    const float MARGE_LIANE   = 45.f; 

    while (window.isOpen()) {
        sf::Event event;
        while (window.pollEvent(event)) {
            ImGui::SFML::ProcessEvent(window, event);
            if (event.type == sf::Event::Closed) window.close();
        }
        sf::Time dt = deltaClock.restart();
        ImGui::SFML::Update(window, dt);
        temps += dt.asSeconds();

        float W = (float)window.getSize().x;
        float H = (float)window.getSize().y;

        // ── OVERLAY LIANES ET LUCIOLES ──
        ImGui::SetNextWindowPos({0,0});
        ImGui::SetNextWindowSize({W,H});
        ImGui::SetNextWindowBgAlpha(0.f);
        ImGui::Begin("##overlay", nullptr, ImGuiWindowFlags_NoTitleBar|ImGuiWindowFlags_NoResize|ImGuiWindowFlags_NoMove|ImGuiWindowFlags_NoScrollbar|ImGuiWindowFlags_NoInputs);
        DessinerLianesEtLucioles(ImGui::GetWindowDrawList(), W, H, temps);
        ImGui::End();

        // ── FENÊTRE PRINCIPALE CENTRALE ──
        ImGui::SetNextWindowPos({MARGE_LIANE, MARGE_LIANE});
        ImGui::SetNextWindowSize({W-MARGE_LIANE*2, H-MARGE_LIANE*2});
        ImGui::Begin("##principal", nullptr, ImGuiWindowFlags_NoTitleBar|ImGuiWindowFlags_NoResize|ImGuiWindowFlags_NoMove);

        // Header
        ImGui::Spacing();
        ImGui::TextColored(Couleurs::Accent, " PLANTASIA");
        ImGui::SameLine();
        ImGui::TextColored(Couleurs::TexteDoux, "- Mon petit jardin de balcon");
        ImGui::Dummy({0,6});

        float hauteurContenu = ImGui::GetContentRegionAvail().y;
        float largeurDetail  = ImGui::GetContentRegionAvail().x - LARGEUR_LISTE - 16.f;

        // ════════ PANNEAU GAUCHE (LISTE) ════════
        ImGui::BeginChild("##gauche", {LARGEUR_LISTE, hauteurContenu}, false);
        
        ImGui::SetNextItemWidth(-1.f);
        ImGui::InputTextWithHint("##rech", "Rechercher une plante...", filtreRecherche, sizeof(filtreRecherche));
        ImGui::Spacing();

        const auto& plantes = monBalcon.getPlantes();
        std::string filtre(filtreRecherche);
        std::transform(filtre.begin(), filtre.end(), filtre.begin(), ::tolower);

        ImGui::BeginChild("##scroll_liste", {-1, -1}, false);
        for (int i = 0; i < (int)plantes.size(); i++) {
            const Plante& p = plantes[i];
            
            std::string n = p.nom;
            std::transform(n.begin(), n.end(), n.begin(), ::tolower);
            if (!filtre.empty() && n.find(filtre) == std::string::npos) continue;

            ImGui::PushID(i);
            bool sel  = (indexSelectionne == i);
            ImVec2 pos0 = ImGui::GetCursorScreenPos();
            ImVec2 sz   = {ImGui::GetContentRegionAvail().x, 58.f}; 

            ImGui::InvisibleButton("##c", sz);
            bool hov = ImGui::IsItemHovered();
            if (ImGui::IsItemClicked()) indexSelectionne = sel ? -1 : i;

            ImDrawList* dlL = ImGui::GetWindowDrawList();
            
            if (sel)       dlL->AddRectFilled(pos0, {pos0.x+sz.x, pos0.y+sz.y}, IM_COL32(25,60,35,255), 8.f);
            else if (hov)  dlL->AddRectFilled(pos0, {pos0.x+sz.x, pos0.y+sz.y}, IM_COL32(20,40,25,200), 8.f);
            else           dlL->AddRectFilled(pos0, {pos0.x+sz.x, pos0.y+sz.y}, IM_COL32(15,22,15,150), 8.f);
            
            ImVec4 colType = CouleurTypePlante(p.type);
            dlL->AddRectFilled(pos0, {pos0.x + 6.f, pos0.y+sz.y}, ImGui::ColorConvertFloat4ToU32(colType), 8.f, ImDrawFlags_RoundCornersLeft);

            ImVec4 cNom = sel ? Couleurs::Accent : (hov ? Couleurs::Texte : ImVec4(0.85f,0.90f,0.85f,1.f));
            ImGui::SetCursorScreenPos({pos0.x+18, pos0.y+10});
            ImGui::TextColored(cNom, "%s", p.nom.c_str());
            
            ImGui::SetCursorScreenPos({pos0.x+18, pos0.y+32});
            // EMOJI FIX: Remplacé par un tiret simple pour éviter les "?"
            ImGui::TextColored(CouleurRusticite(p.rusticite), "- %s", RusiciteLabel(p.rusticite));

            ImGui::SetCursorScreenPos({pos0.x, pos0.y+sz.y + 4.f});
            ImGui::PopID();
        }
        ImGui::EndChild();
        ImGui::EndChild();

        ImGui::SameLine(0, 16.f);

        // ════════ PANNEAU DROIT (DETAILS) ════════
        ImGui::BeginChild("##detail", {largeurDetail, hauteurContenu}, true);

        if (indexSelectionne >= 0 && indexSelectionne < (int)plantes.size()) {
            const Plante& p = plantes[indexSelectionne];

            // EMOJI FIX & TITRE GROSSI : On utilise SetWindowFontScale pour grossir le texte au lieu de <h2>
            ImGui::SetWindowFontScale(1.4f);
            ImGui::TextColored(Couleurs::Accent, "%s", p.nom.c_str());
            ImGui::SetWindowFontScale(1.0f);
            
            ImGui::Separator();
            ImGui::Spacing();

            if (p.floraison_debut > 0 || p.recolte_debut > 0) {
                if (p.floraison_debut > 0) 
                    DessinerCalendrierVisuel("Période de Floraison", p.floraison_debut, p.floraison_fin, ImVec4(0.9f, 0.5f, 0.8f, 1.0f));
                
                if (p.recolte_debut > 0) 
                    DessinerCalendrierVisuel("Période de Récolte", p.recolte_debut, p.recolte_fin, ImVec4(0.5f, 0.8f, 0.4f, 1.0f));
            }

            // NOUVEAU : Système d'onglets pour les conseils
            if (ImGui::BeginTabBar("OngletsConseils")) {
                
                auto SectionInfo = [&](const char* titre, const char* contenu, ImVec4 color) {
                    if (!contenu || std::strlen(contenu) == 0) return;
                    ImGui::TextColored(color, "[ %s ]", titre);
                    ImGui::PushStyleColor(ImGuiCol_Text, Couleurs::TexteDoux);
                    ImGui::TextWrapped("%s", contenu);
                    ImGui::PopStyleColor();
                    ImGui::Dummy({0,10});
                };

                // Onglet 1 : Terre & Eau
                if (ImGui::BeginTabItem("Sol & Arrosage")) {
                    ImGui::Spacing();
                    SectionInfo("Terre recommandée", p.conseil_terre.c_str(), ImVec4(0.8f, 0.6f, 0.3f, 1.f));
                    SectionInfo("Besoins en eau", p.conseil_arrosage.c_str(), ImVec4(0.4f, 0.7f, 1.0f, 1.f));
                    ImGui::EndTabItem();
                }

                // Onglet 2 : Entretien
                if (ImGui::BeginTabItem("Taille & Entretien")) {
                    ImGui::Spacing();
                    SectionInfo("Entretien général", p.conseil_entretien.c_str(), ImVec4(0.5f, 0.8f, 0.5f, 1.f));
                    ImGui::EndTabItem();
                }

                // Onglet 3 : Santé
                if (ImGui::BeginTabItem("Santé & Maladies")) {
                    ImGui::Spacing();
                    SectionInfo("Parasites et maladies", p.maladies.c_str(), ImVec4(0.9f, 0.4f, 0.4f, 1.f));
                    ImGui::EndTabItem();
                }

                ImGui::EndTabBar();
            }
        } 
        else {
            ImVec2 avail = ImGui::GetContentRegionAvail();
            ImVec2 pos = ImGui::GetCursorScreenPos();
            ImDrawList* dl = ImGui::GetWindowDrawList();
            
            DessinerFeuille(dl, pos.x + avail.x*0.5f, pos.y + avail.y*0.4f, -3.14f/4.f, 70.f, IM_COL32(35, 60, 40, 255));
            
            const char* msg = "Sélectionnez une plante pour voir son tableau de bord";
            ImVec2 tsz = ImGui::CalcTextSize(msg);
            ImGui::SetCursorScreenPos(ImVec2(pos.x + (avail.x - tsz.x)*0.5f, pos.y + avail.y*0.4f + 90.f));
            ImGui::TextColored(Couleurs::TexteInactif, "%s", msg);
        }

        ImGui::EndChild(); 
        ImGui::End(); 

        // Important : Le fond de base est noir, c'est ce qui fait ressortir la transparence
        window.clear(sf::Color(5, 8, 5));
        ImGui::SFML::Render(window);
        window.display();
    }

    ImGui::SFML::Shutdown();
    return 0;
}