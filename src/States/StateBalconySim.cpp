#include "StateBalconySim.hpp"
#include "Core/Application.hpp"
#include "Services/SunCalculator.hpp"
#include <imgui.h>
#include <cmath>
#include <algorithm>

// ============================================================
//  INITIALISATION ET CYCLE DE VIE
// ============================================================

StateBalconySim::StateBalconySim(Application* app) : State(app) {}

void StateBalconySim::onEnter() {
    m_cfg = m_app->getUserBalcony().getBalconyConfigRef();
    m_cfg.syncRealDate();
    m_newW = m_cfg.width;
    m_newH = m_cfg.height;
    
    // Initialisation du moteur OpenGL
    m_sunGPU.init(m_cfg.width, m_cfg.height);

    recalcSun();
    updateGridVertices();
    markSunMapDirty();
    
    // On centre la caméra pour tout voir d'un coup
    frameCamera();
}

void StateBalconySim::onExit() {}

void StateBalconySim::update(float dt) {
    // Toujours mettre à jour les plantes avant de calculer la lumière !
    bakePlantsIntoGrid(); 

    if (m_sunMapDirty) {
        if (m_cfg.width != m_newW || m_cfg.height != m_newH) {
            m_sunGPU.resize(m_cfg.width, m_cfg.height);
        }
        m_sunGPU.compute(m_cfg); 
        m_sunMapDirty = false;
    }
    handleInput();
}

// ============================================================
//  CAMÉRA ET GÉOMÉTRIE
// ============================================================

void StateBalconySim::frameCamera() {
    float gridW = m_cfg.width * CELL_PX;
    float gridH = m_cfg.height * CELL_PX;
    
    // On pointe la caméra vers le centre du balcon
    m_cameraPos = sf::Vector2f(gridW / 2.0f, gridH / 2.0f); 

    auto& win = m_app->getWindow();
    float winW = (float)win.getSize().x;
    float winH = (float)win.getSize().y;

    // Calcul du zoom pour que ça rentre avec une petite marge
    float zoomX = winW / (gridW + 100.0f);
    float zoomY = winH / (gridH + 100.0f);
    m_cameraZoom = std::min({1.5f, zoomX, zoomY}); // Ne zoome jamais trop près
}

void StateBalconySim::updateGridVertices() {
    m_gridVA.setPrimitiveType(sf::Quads);
    m_gridVA.resize(m_cfg.width * m_cfg.height * 4);

    for (int r = 0; r < m_cfg.height; ++r) {
        for (int c = 0; c < m_cfg.width; ++c) {
            int idx = (r * m_cfg.width + c) * 4;
            float x = c * CELL_PX, y = r * CELL_PX, sz = CELL_PX - 1.f;

            m_gridVA[idx+0].position = {x,    y   };
            m_gridVA[idx+1].position = {x+sz, y   };
            m_gridVA[idx+2].position = {x+sz, y+sz};
            m_gridVA[idx+3].position = {x,    y+sz};

            const auto& cell = m_cfg.grid[r][c];
            sf::Color col;
            if (cell.isWall()) {
                float t = std::clamp((cell.wallHeight - 0.5f) / 3.5f, 0.f, 1.f);
                col = sf::Color(80+(int)(t*60), 60, 30);      // brun → brun clair selon hauteur
            } else if (cell.isRailing()) {
                col = sf::Color(60, 100, 120);                 // bleu-gris = rambarde
            } else {
                col = sf::Color(45, 50, 45);                   // sol béton
            }
            for (int i=0;i<4;i++) m_gridVA[idx+i].color = col;
        }
    }
}

// ============================================================
//  INPUT (Gestion Clavier / Souris)
// ============================================================

void StateBalconySim::handleInput() {
    auto& win = m_app->getWindow();
    sf::Vector2i mPos = sf::Mouse::getPosition(win);

    // On reconstitue la Caméra pour lire les bonnes coordonnées de la souris
    sf::View view = win.getDefaultView();
    view.setCenter(m_cameraPos);
    view.setSize(view.getSize().x / m_cameraZoom, view.getSize().y / m_cameraZoom);

    // --- DÉPLACEMENT DE LA CAMÉRA (Clic Molette) ---
    if (sf::Mouse::isButtonPressed(sf::Mouse::Middle)) {
        if (!m_isPanning) {
            m_isPanning = true;
            m_lastMousePos = mPos;
        } else {
            sf::Vector2f delta = win.mapPixelToCoords(m_lastMousePos, view) - win.mapPixelToCoords(mPos, view);
            m_cameraPos += delta;
            m_lastMousePos = mPos;
            view.setCenter(m_cameraPos); 
        }
    } else {
        m_isPanning = false;
    }

    if (ImGui::GetIO().WantCaptureMouse) return;

    bool lmb = sf::Mouse::isButtonPressed(sf::Mouse::Left);
    bool rmb = sf::Mouse::isButtonPressed(sf::Mouse::Right);
    if (!lmb && !rmb) return;

    // --- LECTURE DU CLIC SUR LA GRILLE ---
    sf::Vector2f worldPos = win.mapPixelToCoords(mPos, view);
    int gx = static_cast<int>(worldPos.x / CELL_PX);
    int gy = static_cast<int>(worldPos.y / CELL_PX);

    if (gx < 0 || gx >= m_cfg.width || gy < 0 || gy >= m_cfg.height) return;

    bool modified = false;
    auto& cell = m_cfg.grid[gy][gx];

    if (m_editMode == EditMode::MUR) {
        if (lmb) { cell.type = WallType::WALL;   cell.wallHeight = m_cfg.defaultWallHeight;   modified = true; }
        if (rmb) { cell.type = WallType::NONE;    modified = true; }
    }
    else if (m_editMode == EditMode::RAMBARDE) {
        if (lmb) { cell.type = WallType::RAILING; cell.wallHeight = m_cfg.defaultRailingHeight; modified = true; }
        if (rmb) { cell.type = WallType::NONE;    modified = true; }
    }
    else if (m_editMode == EditMode::PLANTE && m_selectedPlantIdx >= 0) {
        auto& plantes = m_app->getUserBalcony().getMesPlantesRef();
        if (lmb && !cell.isObstacle())
            plantes[m_selectedPlantIdx].position_balcon = sf::Vector2f(gx*CELL_PX, gy*CELL_PX);
        if (rmb)
            plantes[m_selectedPlantIdx].position_balcon = sf::Vector2f(-100.f,-100.f);
        m_app->getUserBalcony().sauvegarderProfil("mon_balcon.json");
    }

    if (modified) {
        updateGridVertices();
        markSunMapDirty();
        m_app->getUserBalcony().getBalconyConfigRef() = m_cfg;
        m_app->getUserBalcony().sauvegarderProfil("mon_balcon.json");
    }
}

// ============================================================
//  RENDU
// ============================================================

void StateBalconySim::draw(sf::RenderWindow& window) {
    // 1. On applique notre Caméra spatiale
    sf::View view = window.getDefaultView();
    view.setCenter(m_cameraPos);
    view.setSize(view.getSize().x / m_cameraZoom, view.getSize().y / m_cameraZoom);
    window.setView(view);

    // 2. On dessine la scène (sans offset)
    renderGrid(window);
    if (m_showSunMap)    renderSunMapLayer(window);
    if (m_showShadows)   renderShadowLayer(window);
    renderPlants(window);

    // 3. On remet la caméra par défaut pour l'UI ImGui (très important)
    window.setView(window.getDefaultView());
}

void StateBalconySim::renderGrid(sf::RenderWindow& window) {
    window.draw(m_gridVA);
}

void StateBalconySim::renderShadowLayer(sf::RenderWindow& window) {
    if (!m_sun.isAboveHorizon()) return;

    sf::RectangleShape rect(sf::Vector2f(CELL_PX, CELL_PX));

    for (int r = 0; r < m_cfg.height; ++r) {
        for (int c = 0; c < m_cfg.width; ++c) {
            const auto& cell = m_cfg.grid[r][c];
            if (cell.isObstacle()) continue;

            float soft = SunCalculator::softShadow(m_cfg, c, r, m_sun);
            sf::Uint8 alpha = (sf::Uint8)((1.0f - soft) * 180.f);
            if (alpha < 5) continue;

            rect.setFillColor(sf::Color(0, 0, 0, alpha));
            rect.setPosition(c * CELL_PX, r * CELL_PX);
            window.draw(rect);
        }
    }
}

void StateBalconySim::renderSunMapLayer(sf::RenderWindow& window) {
    if (!m_sunGPU.isReady()) return;
    
    sf::Sprite spr(m_sunGPU.getResultTexture());
    spr.setPosition(0.f, 0.f);
    spr.setScale(CELL_PX, CELL_PX);
    
    // Le GPU a déjà calculé les bonnes couleurs et transparences
    window.draw(spr, sf::BlendAlpha);
}

void StateBalconySim::renderPlants(sf::RenderWindow& window) {
    sf::CircleShape sh;
    const auto& plantes = m_app->getUserBalcony().getMesPlantes();

    for (int i = 0; i < (int)plantes.size(); ++i) {
        const auto& p = plantes[i];
        if (p.position_balcon.x < 0) continue;

        float radius = std::max(CELL_PX*0.4f, std::sqrt((float)p.volume_pot_actuel_L)*2.f);
        sh.setRadius(radius);
        sh.setOrigin(radius, radius);
        sh.setPosition(p.position_balcon.x + CELL_PX*0.5f, p.position_balcon.y + CELL_PX*0.5f);

        bool sel = (m_editMode==EditMode::PLANTE && m_selectedPlantIdx==i);
        sh.setFillColor(sel ? sf::Color(100,255,120) : sf::Color(60,150,80));
        sh.setOutlineThickness(sel ? 2.f : 1.f);
        sh.setOutlineColor(sel ? sf::Color::White : sf::Color(30,90,40));
        window.draw(sh);
    }
}

// ============================================================
//  IMGUI INTERFACE
// ============================================================

void StateBalconySim::drawImGui() {
    ImGui::SetNextWindowPos({10,10}, ImGuiCond_FirstUseEver);
    ImGui::SetNextWindowSize({280, 0}, ImGuiCond_FirstUseEver);
    ImGui::Begin("Atelier du Balcon", nullptr, ImGuiWindowFlags_AlwaysAutoResize);

    if (ImGui::Button("< Menu")) m_app->getStateMachine().removeState();
    ImGui::Spacing();

    // ── CAMERA
    if (ImGui::CollapsingHeader("🎥 Caméra", ImGuiTreeNodeFlags_DefaultOpen)) {
        ImGui::SliderFloat("Zoom", &m_cameraZoom, 0.1f, 3.0f, "x %.2f");
        if (ImGui::Button("Recadrer automatiquement")) frameCamera();
        ImGui::TextDisabled("(Maintenez le Clic Molette pour glisser)");
    }

    // ── BOUSSOLE
    if (ImGui::CollapsingHeader("Orientation", ImGuiTreeNodeFlags_DefaultOpen))
        drawCompass();

    // ── OUTILS
    if (ImGui::CollapsingHeader("Outils", ImGuiTreeNodeFlags_DefaultOpen)) {
        const char* modes[] = {"Mur","Observer","Plante","Rambarde"};
        int m = (int)m_editMode;
        if (ImGui::Combo("Mode", &m, modes, 4)) m_editMode = (EditMode)m;

        if (m_editMode == EditMode::MUR) {
            if (ImGui::SliderFloat("H. mur", &m_cfg.defaultWallHeight, 0.5f, 5.0f, "%.1f m"))
                markSunMapDirty();
        }
        if (m_editMode == EditMode::RAMBARDE) {
            if (ImGui::SliderFloat("H. rambarde", &m_cfg.defaultRailingHeight, 0.3f, 1.5f, "%.1f m"))
                markSunMapDirty();
        }
        if (m_editMode == EditMode::PLANTE) {
            auto& pl = m_app->getUserBalcony().getMesPlantesRef();
            ImGui::BeginChild("##plants", {0, 100}, true);
            for (int i=0;i<(int)pl.size();++i)
                if (ImGui::Selectable((pl[i].surnom+"##"+std::to_string(i)).c_str(), m_selectedPlantIdx==i))
                    m_selectedPlantIdx = i;
            ImGui::EndChild();
        }
    }

    // ── STRUCTURE
    if (ImGui::CollapsingHeader("Structure")) {
        bool roofChanged = ImGui::Checkbox("Toit (balcon du dessus)", &m_cfg.hasRoof);
        if (m_cfg.hasRoof) {
            roofChanged |= ImGui::SliderFloat("H. toit", &m_cfg.roofHeight, 1.0f, 5.0f, "%.1f m");
        }
        if (roofChanged) markSunMapDirty();

        ImGui::Spacing();
        ImGui::TextColored({0.6f,0.8f,0.6f,1.f}, "Dimensions du balcon");
        ImGui::SetNextItemWidth(80); ImGui::InputInt("L (cases)", &m_newW);
        ImGui::SameLine();
        ImGui::SetNextItemWidth(80); ImGui::InputInt("H (cases)", &m_newH);
        m_newW = std::clamp(m_newW, 5, 100);
        m_newH = std::clamp(m_newH, 5, 100);
        ImGui::TextColored({0.5f,0.5f,0.5f,1.f}, "(1 case = 5x5 cm | %dx%d cm)",
            m_newW*5, m_newH*5);
        
        // BOUTON MAGIQUE POUR LE REDIMENSIONNEMENT
        if (ImGui::Button("Appliquer dimensions")) {
            m_cfg.resize(m_newW, m_newH);
            m_sunGPU.resize(m_newW, m_newH); // Préviens la carte graphique !
            updateGridVertices();
            markSunMapDirty();
            m_app->getUserBalcony().getBalconyConfigRef() = m_cfg;
            m_app->getUserBalcony().sauvegarderProfil("mon_balcon.json");
            frameCamera(); // Recadre l'écran
        }
    }

    // ── SIMULATION
    if (ImGui::CollapsingHeader("Simulation soleil")) {
        if (ImGui::SliderFloat("Heure", &m_cfg.hour, 0.f, 23.9f, "%.1f h")) recalcSun();
        if (ImGui::SliderInt("Jour", &m_cfg.day, 1, 31))   markSunMapDirty();
        if (ImGui::SliderInt("Mois", &m_cfg.month, 1, 12)) markSunMapDirty();

        ImGui::Spacing();
        ImGui::Checkbox("Ombres live", &m_showShadows);
        if (ImGui::Checkbox("Heatmap journaliere", &m_showSunMap))
            markSunMapDirty();

        if (m_sun.isAboveHorizon())
            ImGui::TextColored({1.f,0.9f,0.2f,1.f}, "Soleil : az=%.0f elev=%.1f",
                m_sun.azimuth, m_sun.elevation);
        else
            ImGui::TextColored({0.4f,0.4f,0.6f,1.f}, "Soleil sous l'horizon");

        if (m_showSunMap) {
            ImGui::Spacing();
            ImGui::TextColored({0.12f,0.31f,0.78f,1.f}, "■"); ImGui::SameLine(); ImGui::Text("< 30 min");
            ImGui::TextColored({0.12f,0.31f,0.78f,1.f}, "■"); ImGui::SameLine(); ImGui::Text("< 2h");
            ImGui::TextColored({0.24f,0.71f,0.31f,1.f}, "■"); ImGui::SameLine(); ImGui::Text("< 4h");
            ImGui::TextColored({0.86f,0.67f,0.12f,1.f}, "■"); ImGui::SameLine(); ImGui::Text("< 6h");
            ImGui::TextColored({1.0f, 0.31f,0.08f,1.f}, "■"); ImGui::SameLine(); ImGui::Text("> 6h (plein soleil)");
        }
    }

    ImGui::End();
}

void StateBalconySim::drawCompass() {
    ImDrawList* dl = ImGui::GetWindowDrawList();
    ImVec2 cursor  = ImGui::GetCursorScreenPos();
    ImVec2 center  = {cursor.x + 70, cursor.y + 70};
    float  radius  = 55.f;

    dl->AddCircle(center, radius, IM_COL32(180,180,180,120), 32, 1.5f);

    float facadeAngle = ((float)m_cfg.orientation*45.f - 90.f) * (3.14159f/180.f);
    ImVec2 facadePt = {center.x + std::cos(facadeAngle)*radius*0.7f,
                       center.y + std::sin(facadeAngle)*radius*0.7f};
    dl->AddLine(center, facadePt, IM_COL32(80,220,100,255), 2.5f);

    const char* labels[] = {"N","NE","E","SE","S","SW","W","NW"};
    for (int i = 0; i < 8; ++i) {
        float angle = (i*45.f - 90.f) * (3.14159f/180.f);
        ImVec2 p = {center.x + std::cos(angle)*radius, center.y + std::sin(angle)*radius};
        bool active = ((int)m_cfg.orientation == i);

        ImGui::SetCursorScreenPos({p.x-12, p.y-12});
        if (active) ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.2f,0.6f,0.3f,1.f));
        if (ImGui::Button((std::string(labels[i])+"##c").c_str(), {24,24})) {
            m_cfg.orientation = (BalconyOrientation)i;
            markSunMapDirty();
            m_app->getUserBalcony().getBalconyConfigRef() = m_cfg;
            m_app->getUserBalcony().sauvegarderProfil("mon_balcon.json");
        }
        if (active) ImGui::PopStyleColor();
    }

    ImGui::SetCursorScreenPos({center.x - ImGui::CalcTextSize("facade").x*0.5f, center.y - 8});
    ImGui::TextColored({0.5f,0.8f,0.5f,1.f}, "facade");
    ImGui::Dummy({140, 140});
    ImGui::TextColored({0.5f,0.5f,0.5f,1.f}, "Orientation: %s", labels[(int)m_cfg.orientation]);
}
void StateBalconySim::bakePlantsIntoGrid() {
    // 1. On efface les données de plantes du tour précédent
    for (int r = 0; r < m_cfg.height; ++r) {
        for (int c = 0; c < m_cfg.width; ++c) {
            m_cfg.grid[r][c].plantHeight = 0.0f;
            m_cfg.grid[r][c].plantOpacity = 0.0f;
        }
    }

    // 2. On dessine les plantes "virtuellement" sur la grille
    const auto& plantes = m_app->getUserBalcony().getMesPlantes();
    for (const auto& p : plantes) {
        if (p.position_balcon.x < 0.0f) continue;

        // Position de la plante en cases
        float px_center = (p.position_balcon.x / CELL_PX) + 0.5f;
        float py_center = (p.position_balcon.y / CELL_PX) + 0.5f;

        // On estime la taille du feuillage selon le pot (ex: pot 5L = ~20cm de rayon)
        float radiusCells = std::max(1.0f, std::sqrt((float)p.volume_pot_actuel_L) * 2.0f / CELL_PX);
        
        // On estime la hauteur de la plante (ex: 5L = 60cm de haut)
        float pHeight = 0.2f + (p.volume_pot_actuel_L * 0.08f); 
        float pOpacity = 0.5f; // Le feuillage laisse passer 50% de la lumière !

        int rC = std::ceil(radiusCells);
        
        // On dessine un cercle sur la grille
        for(int dy = -rC; dy <= rC; dy++) {
            for(int dx = -rC; dx <= rC; dx++) {
                if (dx*dx + dy*dy <= rC*rC) { 
                    int cx = static_cast<int>(px_center) + dx;
                    int cy = static_cast<int>(py_center) + dy;
                    if (cx >= 0 && cx < m_cfg.width && cy >= 0 && cy < m_cfg.height) {
                        m_cfg.grid[cy][cx].plantHeight = std::max(m_cfg.grid[cy][cx].plantHeight, pHeight);
                        m_cfg.grid[cy][cx].plantOpacity = pOpacity;
                    }
                }
            }
        }
    }
}
void StateBalconySim::recalcSun()       { m_sun = SunCalculator::compute(m_cfg); }
void StateBalconySim::markSunMapDirty() { m_sunMapDirty = true; }