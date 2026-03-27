#include "StateBalconySim.hpp"
#include "Core/Application.hpp"
#include "Services/SunCalculator.hpp"
#include <imgui.h>
#include <cmath>
#include <algorithm>

StateBalconySim::StateBalconySim(Application* app)
    : State(app)
{
    // Initialisation de la grille par défaut (2.0m x 1.5m @ 5cm/cellule)
    m_cfg.width = 40;
    m_cfg.height = 30;
    m_cfg.grid.resize(m_cfg.height, std::vector<GridCell>(m_cfg.width));
}

void StateBalconySim::onEnter() {
    m_cfg.syncRealDate(); //
    recalcSun();
    markSunMapDirty();
}

void StateBalconySim::onExit() {}

void StateBalconySim::update(float dt) {
    if (m_sunMapDirty) {
        SunCalculator::computeDailySunMap(m_cfg); //
        m_sunMapDirty = false;
    }
    handleInput();
}

void StateBalconySim::handleInput() {
    // On ignore si la souris est sur l'interface ImGui
    if (ImGui::GetIO().WantCaptureMouse) return;

    if (sf::Mouse::isButtonPressed(sf::Mouse::Left) || sf::Mouse::isButtonPressed(sf::Mouse::Right)) {
        sf::Vector2i mousePos = sf::Mouse::getPosition(m_app->getWindow());
        int gridX = mousePos.x / (int)CELL_PX;
        int gridY = mousePos.y / (int)CELL_PX;

        if (gridX >= 0 && gridX < m_cfg.width && gridY >= 0 && gridY < m_cfg.height) {
            if (sf::Mouse::isButtonPressed(sf::Mouse::Left)) {
                m_cfg.grid[gridY][gridX].isWall = true;
                m_cfg.grid[gridY][gridX].wallHeight = m_cfg.defaultWallHeight;
            } else {
                m_cfg.grid[gridY][gridX].isWall = false;
            }
            markSunMapDirty(); // Recalculer la heatmap si le balcon change
        }
    }
}

// ============================================================
//  RENDU SFML
// ============================================================

void StateBalconySim::draw(sf::RenderWindow& window) {
    renderGrid(window);
    if (m_showShadows) renderShadowLayer(window);
    if (m_showSunMap)  renderSunMapLayer(window);
}

void StateBalconySim::renderGrid(sf::RenderWindow& window) {
    sf::RectangleShape cell(sf::Vector2f(CELL_PX - 1.f, CELL_PX - 1.f));

    for (int r = 0; r < m_cfg.height; ++r) {
        for (int c = 0; c < m_cfg.width; ++c) {
            cell.setPosition(c * CELL_PX, r * CELL_PX);
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

void StateBalconySim::renderShadowLayer(sf::RenderWindow& window) {
    if (!m_sun.isAboveHorizon()) return;

    sf::RectangleShape shadow(sf::Vector2f(CELL_PX, CELL_PX));
    shadow.setFillColor(sf::Color(0, 0, 0, 150));

    for (int r = 0; r < m_cfg.height; ++r) {
        for (int c = 0; c < m_cfg.width; ++c) {
            if (!m_cfg.grid[r][c].isWall && SunCalculator::isInShadow(m_cfg, c, r, m_sun)) {
                shadow.setPosition(c * CELL_PX, r * CELL_PX);
                window.draw(shadow);
            }
        }
    }
}

void StateBalconySim::renderSunMapLayer(sf::RenderWindow& window) {
    sf::RectangleShape heat(sf::Vector2f(CELL_PX, CELL_PX));

    for (int r = 0; r < m_cfg.height; ++r) {
        for (int c = 0; c < m_cfg.width; ++c) {
            if (m_cfg.grid[r][c].isWall) continue;
            
            float hours = m_cfg.grid[r][c].sunHours;
            sf::Color col;
            if (hours < 2.f)       col = sf::Color(40, 60, 180, 160);  // Ombre
            else if (hours < 5.f)  col = sf::Color(60, 180, 80, 160);  // Mi-soleil
            else                   col = sf::Color(250, 220, 40, 160); // Plein soleil

            heat.setFillColor(col);
            heat.setPosition(c * CELL_PX, r * CELL_PX);
            window.draw(heat);
        }
    }
}

// ============================================================
//  INTERFACE IMGUI
// ============================================================

void StateBalconySim::drawImGui() {
    ImGui::SetNextWindowPos(ImVec2(620, 10), ImGuiCond_FirstUseEver);
    ImGui::Begin("☀️ Simulateur Solaire", nullptr, ImGuiWindowFlags_AlwaysAutoResize);

    if (ImGui::Button("< Menu Principal")) m_app->getStateMachine().removeState();
    
    ImGui::Separator();
    
    if (ImGui::CollapsingHeader("🧭 Orientation", ImGuiTreeNodeFlags_DefaultOpen)) {
        drawCompass();
    }

    if (ImGui::CollapsingHeader("📅 Temps & Lieu", ImGuiTreeNodeFlags_DefaultOpen)) {
        if (ImGui::DragFloat("Latitude", &m_cfg.latitude, 0.1f, -90.f, 90.f, "%.1f°")) recalcSun();
        if (ImGui::SliderFloat("Heure", &m_cfg.hour, 0.f, 23.9f, "%.1f h")) recalcSun();
        
        if (m_sun.isAboveHorizon())
            ImGui::TextColored(ImVec4(1, 1, 0, 1), "Soleil: Az %.1f°, El %.1f°", m_sun.azimuth, m_sun.elevation);
        else
            ImGui::TextDisabled("Nuit (Soleil sous l'horizon)");
    }

    if (ImGui::CollapsingHeader("🧱 Édition")) {
        ImGui::SliderFloat("Hauteur Mur (m)", &m_cfg.defaultWallHeight, 0.5f, 5.0f);
        ImGui::Checkbox("Afficher Ombres", &m_showShadows);
        if (ImGui::Checkbox("Afficher Heatmap", &m_showSunMap)) markSunMapDirty();
    }

    ImGui::End();
}

void StateBalconySim::drawCompass() {
    ImDrawList* drawList = ImGui::GetWindowDrawList();
    ImVec2 center = ImGui::GetCursorScreenPos();
    center.x += 60; center.y += 60;
    float radius = 50.0f;

    drawList->AddCircle(center, radius, IM_COL32(150, 150, 150, 255), 32, 2.0f);

    const char* labels[] = {"N","NE","E","SE","S","SW","W","NW"};
    for (int i = 0; i < 8; ++i) {
        float angle = (i * 45.0f - 90.0f) * (3.14159f / 180.0f);
        ImVec2 p(center.x + std::cos(angle) * radius, center.y + std::sin(angle) * radius);
        
        bool selected = (static_cast<int>(m_cfg.orientation) == i);
        ImU32 col = selected ? IM_COL32(255, 220, 0, 255) : IM_COL32(80, 80, 80, 255);
        
        drawList->AddCircleFilled(p, 10.0f, col);
        drawList->AddText(ImVec2(p.x - 5, p.y - 7), IM_COL32(255, 255, 255, 255), labels[i]);

        ImGui::SetCursorScreenPos(ImVec2(p.x - 10, p.y - 10));
        if (ImGui::InvisibleButton(labels[i], ImVec2(20, 20))) {
            m_cfg.orientation = static_cast<BalconyOrientation>(i);
            markSunMapDirty();
        }
    }
    ImGui::Dummy(ImVec2(120, 120));
}

void StateBalconySim::recalcSun() {
    m_sun = SunCalculator::compute(m_cfg); //
}

void StateBalconySim::markSunMapDirty() {
    m_sunMapDirty = true;
}