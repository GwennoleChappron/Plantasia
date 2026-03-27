#include "StateBalconySim.hpp"
#include "Core/Application.hpp"
#include "Services/SunCalculator.hpp"
#include <imgui.h>
#include <cmath>
#include <algorithm>

StateBalconySim::StateBalconySim(Application* app)
    : State(app)
{
    // Initialisation de la grille (2.0m x 1.5m @ 5cm/cellule)
    m_cfg.width = 40;
    m_cfg.height = 30;
    m_cfg.grid.resize(m_cfg.height, std::vector<GridCell>(m_cfg.width));
}

void StateBalconySim::onEnter() {
    m_cfg.syncRealDate();
    recalcSun();
    markSunMapDirty();
}

void StateBalconySim::onExit() {}

void StateBalconySim::update(float dt) {
    if (m_sunMapDirty) {
        SunCalculator::computeDailySunMap(m_cfg);
        m_sunMapDirty = false;
    }
    handleInput();
}

void StateBalconySim::handleInput() {
    if (ImGui::GetIO().WantCaptureMouse) return;

    // Calcul de l'offset de centrage
    float gridW = m_cfg.width * CELL_PX;
    float gridH = m_cfg.height * CELL_PX;
    sf::Vector2f offset((m_app->getWindow().getSize().x - gridW) / 2.f, 
                        (m_app->getWindow().getSize().y - gridH) / 2.f);

    if (sf::Mouse::isButtonPressed(sf::Mouse::Left) || sf::Mouse::isButtonPressed(sf::Mouse::Right)) {
        sf::Vector2i mPos = sf::Mouse::getPosition(m_app->getWindow());
        int gx = static_cast<int>((mPos.x - offset.x) / CELL_PX);
        int gy = static_cast<int>((mPos.y - offset.y) / CELL_PX);

        if (gx >= 0 && gx < m_cfg.width && gy >= 0 && gy < m_cfg.height) {
            if (m_editMode == 0) { // Mode Construction Mur
                if (sf::Mouse::isButtonPressed(sf::Mouse::Left)) {
                    m_cfg.grid[gy][gx].isWall = true;
                    m_cfg.grid[gy][gx].wallHeight = m_cfg.defaultWallHeight;
                } else {
                    m_cfg.grid[gy][gx].isWall = false;
                }
            } else if (m_editMode == 2 && m_selectedPlantIndex >= 0) { // Mode Jardinage
                 auto& plantes = m_app->getUserBalcony().getMesPlantesRef();
                 plantes[m_selectedPlantIndex].position_balcon = sf::Vector2f(gx * CELL_PX, gy * CELL_PX);
                 m_app->getUserBalcony().sauvegarderProfil("mon_balcon.json");
            }
            markSunMapDirty();
        }
    }
}

// ============================================================
//  RENDU
// ============================================================

void StateBalconySim::draw(sf::RenderWindow& window) {
    float gridW = m_cfg.width * CELL_PX;
    float gridH = m_cfg.height * CELL_PX;
    sf::Vector2f offset((window.getSize().x - gridW) / 2.f, (window.getSize().y - gridH) / 2.f);

    renderGrid(window, offset);
    if (m_showShadows) renderShadowLayer(window, offset);
    if (m_showSunMap)  renderSunMapLayer(window, offset);
    renderPlants(window, offset);
}

void StateBalconySim::renderGrid(sf::RenderWindow& window, sf::Vector2f offset) {
    sf::RectangleShape cell(sf::Vector2f(CELL_PX - 1.f, CELL_PX - 1.f));

    for (int r = 0; r < m_cfg.height; ++r) {
        for (int c = 0; c < m_cfg.width; ++c) {
            cell.setPosition(offset.x + c * CELL_PX, offset.y + r * CELL_PX);
            const auto& gc = m_cfg.grid[r][c];
            
            if (gc.isWall) {
                float hT = std::clamp((gc.wallHeight - 0.5f) / 3.5f, 0.f, 1.f);
                cell.setFillColor(sf::Color(80 + (int)(hT * 60), 60, 30));
            } else {
                cell.setFillColor(sf::Color(45, 50, 45));
            }
            window.draw(cell);
        }
    }
}

void StateBalconySim::renderShadowLayer(sf::RenderWindow& window, sf::Vector2f offset) {
    if (!m_sun.isAboveHorizon()) return;

    sf::RectangleShape shadow(sf::Vector2f(CELL_PX, CELL_PX));
    shadow.setFillColor(sf::Color(0, 0, 0, 150));

    for (int r = 0; r < m_cfg.height; ++r) {
        for (int c = 0; c < m_cfg.width; ++c) {
            if (!m_cfg.grid[r][c].isWall && SunCalculator::isInShadow(m_cfg, c, r, m_sun)) {
                shadow.setPosition(offset.x + c * CELL_PX, offset.y + r * CELL_PX);
                window.draw(shadow);
            }
        }
    }
}

// ============================================================
//  INTERFACE
// ============================================================

void StateBalconySim::drawImGui() {
    ImGui::Begin("🛠️ Atelier du Balcon", nullptr, ImGuiWindowFlags_AlwaysAutoResize);

    if (ImGui::Button("< Menu")) m_app->getStateMachine().removeState();
    
    if (ImGui::CollapsingHeader("🧭 Boussole (Nord en haut)", ImGuiTreeNodeFlags_DefaultOpen)) {
        drawCompass();
    }

    if (ImGui::CollapsingHeader("🔨 Outils", ImGuiTreeNodeFlags_DefaultOpen)) {
        const char* modes[] = { "Murs", "Observation", "Plantes" };
        ImGui::Combo("Mode", &m_editMode, modes, IM_ARRAYSIZE(modes));
        
        if (m_editMode == 0) {
            ImGui::SliderFloat("H. Mur", &m_cfg.defaultWallHeight, 0.5f, 4.0f, "%.1f m");
        } else if (m_editMode == 2) {
            auto& plantes = m_app->getUserBalcony().getMesPlantesRef();
            for (int i = 0; i < (int)plantes.size(); i++) {
                if (ImGui::Selectable((plantes[i].surnom + "##" + std::to_string(i)).c_str(), m_selectedPlantIndex == i))
                    m_selectedPlantIndex = i;
            }
        }
    }

    if (ImGui::CollapsingHeader("🏠 Structure")) {
        if (ImGui::Checkbox("Toit (Balcon du haut)", &m_cfg.hasRoof)) markSunMapDirty();
    }

    if (ImGui::CollapsingHeader("☀️ Simulation")) {
        if (ImGui::SliderFloat("Heure", &m_cfg.hour, 0.f, 23.9f, "%.1f h")) recalcSun();
        ImGui::Checkbox("Afficher Ombres", &m_showShadows);
        if (ImGui::Checkbox("Afficher Heatmap", &m_showSunMap)) markSunMapDirty();
    }

    ImGui::End();
}

void StateBalconySim::drawCompass() {
    ImDrawList* draw = ImGui::GetWindowDrawList();
    ImVec2 center = ImGui::GetCursorScreenPos();
    center.x += 60; center.y += 60;
    float radius = 50.0f;

    draw->AddCircle(center, radius, IM_COL32(200, 200, 200, 255), 32, 2.0f);
    
    const char* labels[] = {"N","NE","E","SE","S","SW","W","NW"};
    for (int i = 0; i < 8; ++i) {
        float angle = (i * 45.0f - 90.0f) * (3.14159f / 180.0f);
        ImVec2 p(center.x + std::cos(angle) * radius, center.y + std::sin(angle) * radius);
        
        ImGui::SetCursorScreenPos(ImVec2(p.x - 12, p.y - 12));
        if (ImGui::Button((std::string(labels[i]) + "##c").c_str(), ImVec2(24, 24))) {
            m_cfg.orientation = static_cast<BalconyOrientation>(i);
            markSunMapDirty();
        }
    }
    ImGui::Dummy(ImVec2(120, 120));
}

void StateBalconySim::recalcSun() {
    m_sun = SunCalculator::compute(m_cfg);
}

void StateBalconySim::markSunMapDirty() {
    m_sunMapDirty = true;
}

void StateBalconySim::renderSunMapLayer(sf::RenderWindow& window, sf::Vector2f offset) {
    sf::RectangleShape heat(sf::Vector2f(CELL_PX, CELL_PX));

    for (int r = 0; r < m_cfg.height; ++r) {
        for (int c = 0; c < m_cfg.width; ++c) {
            if (m_cfg.grid[r][c].isWall) continue;
            
            float hours = m_cfg.grid[r][c].sunHours;
            sf::Color col;
            if (hours < 2.f)       col = sf::Color(40, 60, 180, 160); 
            else if (hours < 5.f)  col = sf::Color(60, 180, 80, 160); 
            else                   col = sf::Color(250, 220, 40, 160);

            heat.setFillColor(col);
            heat.setPosition(offset.x + c * CELL_PX, offset.y + r * CELL_PX);
            window.draw(heat);
        }
    }
}

void StateBalconySim::renderPlants(sf::RenderWindow& window, sf::Vector2f offset) {
    sf::CircleShape plantShape;
    const auto& plantes = m_app->getUserBalcony().getMesPlantes();
    
    for (int i = 0; i < (int)plantes.size(); ++i) {
        const auto& p = plantes[i];
        
        // On ne dessine que si la plante a une position valide
        if (p.position_balcon.x >= 0.0f) {
            float radius = std::max(CELL_PX * 0.4f, std::sqrt((float)p.volume_pot_actuel_L) * 2.0f);
            plantShape.setRadius(radius);
            plantShape.setOrigin(radius, radius); 
            
            // Position relative au balcon centré
            plantShape.setPosition(
                offset.x + p.position_balcon.x + CELL_PX * 0.5f, 
                offset.y + p.position_balcon.y + CELL_PX * 0.5f
            );
            
            if (m_editMode == 2 && m_selectedPlantIndex == i) {
                plantShape.setFillColor(sf::Color(100, 255, 120));
                plantShape.setOutlineThickness(2.f);
                plantShape.setOutlineColor(sf::Color::White);
            } else {
                plantShape.setFillColor(sf::Color(60, 150, 80));
                plantShape.setOutlineThickness(1.f);
                plantShape.setOutlineColor(sf::Color(30, 90, 40));
            }
            window.draw(plantShape);
        }
    }
}