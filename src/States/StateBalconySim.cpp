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
    
    m_sunGPU.init(m_cfg.width, m_cfg.height);
    
    for (const auto& p : m_app->getUserBalcony().getMesPlantes()) {
        if (m_plantTextures.find(p.nom_espece) == m_plantTextures.end()) {
            sf::Texture tex;
            tex.loadFromFile("assets/" + p.nom_espece + "_icon.png"); 
            m_plantTextures[p.nom_espece] = tex;
        }
    }

    recalcSun();
    updateGridVertices();
    markSunMapDirty();
    frameCamera();
}

void StateBalconySim::onExit() {}

void StateBalconySim::update(float dt) {
    bakePlantsIntoGrid(); 

    if (m_sunMapDirty) {
        if (m_cfg.width != m_newW || m_cfg.height != m_newH) {
            m_sunGPU.resize(m_cfg.width, m_cfg.height);
        }
        m_sunGPU.compute(m_cfg,20); 
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
    
    m_cameraPos = sf::Vector2f(gridW / 2.0f, gridH / 2.0f); 

    auto& win = m_app->getWindow();
    float winW = (float)win.getSize().x;
    float winH = (float)win.getSize().y;

    float zoomX = winW / (gridW + 100.0f);
    float zoomY = winH / (gridH + 100.0f);
    m_cameraZoom = std::min({1.5f, zoomX, zoomY});
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
                col = sf::Color(80+(int)(t*60), 60, 30);      
            } else if (cell.isRailing()) {
                col = sf::Color(60, 100, 120);                
            } else if (cell.isGlass()) {
                col = sf::Color(150, 220, 255); // NOUVEAU : Vitre = Bleu clair lumineux             
            } else {
                col = sf::Color(45, 50, 45);                  
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

    sf::View view = win.getDefaultView();
    view.setCenter(m_cameraPos);
    view.setSize(view.getSize().x / m_cameraZoom, view.getSize().y / m_cameraZoom);

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

    sf::Vector2f worldPos = win.mapPixelToCoords(mPos, view);
    auto& plantes = m_app->getUserBalcony().getMesPlantesRef();

    m_hoveredPlantIdx = -1;
    for (int i = (int)plantes.size() - 1; i >= 0; i--) {
        auto& p = plantes[i];
        if (p.position_balcon.x >= 0.0f) {
            float radius = std::max(CELL_PX * 0.4f, std::sqrt((float)p.volume_pot_actuel_L) * 2.0f);
            float dx = worldPos.x - p.position_balcon.x;
            float dy = worldPos.y - p.position_balcon.y;
            
            if ((dx * dx + dy * dy) <= (radius * radius)) {
                m_hoveredPlantIdx = i; 
                break;
            }
        }
    }

    if (m_editMode == EditMode::PLANTE) {
        if (lmb) {
            if (m_draggedPlantIdx == -1) {
                if (m_hoveredPlantIdx != -1) {
                    m_draggedPlantIdx = m_hoveredPlantIdx;
                    m_selectedPlantIdx = m_hoveredPlantIdx;
                    m_dragOffset = plantes[m_hoveredPlantIdx].position_balcon - worldPos; 
                }
                else if (m_selectedPlantIdx >= 0 && m_selectedPlantIdx < (int)plantes.size()) {
                    plantes[m_selectedPlantIdx].position_balcon = worldPos;
                    m_draggedPlantIdx = m_selectedPlantIdx;
                    m_dragOffset = sf::Vector2f(0, 0);
                }
            } else {
                plantes[m_draggedPlantIdx].position_balcon = worldPos + m_dragOffset;
            }
        } else {
            if (m_draggedPlantIdx != -1) {
                m_draggedPlantIdx = -1;
                m_app->getUserBalcony().sauvegarderProfil("mon_balcon.json");
            }
        }

        if (rmb && m_hoveredPlantIdx != -1) {
            plantes[m_hoveredPlantIdx].position_balcon = sf::Vector2f(-100.f, -100.f);
            m_app->getUserBalcony().sauvegarderProfil("mon_balcon.json");
        }
    }
    else {
        m_draggedPlantIdx = -1;
        if (!lmb && !rmb) return; 

        int gx = static_cast<int>(worldPos.x / CELL_PX);
        int gy = static_cast<int>(worldPos.y / CELL_PX);
        if (gx < 0 || gx >= m_cfg.width || gy < 0 || gy >= m_cfg.height) return;

        bool isPlantHere = false;
        sf::Vector2f cellCenter((gx + 0.5f) * CELL_PX, (gy + 0.5f) * CELL_PX);
        
        for (const auto& p : plantes) {
            if (p.position_balcon.x >= 0.0f) { 
                float radius = std::max(CELL_PX * 0.4f, std::sqrt((float)p.volume_pot_actuel_L) * 2.0f);
                float dx = cellCenter.x - p.position_balcon.x;
                float dy = cellCenter.y - p.position_balcon.y;
                
                if ((dx * dx + dy * dy) <= (radius * radius)) {
                    isPlantHere = true;
                    break;
                }
            }
        }

        bool modified = false;
        auto& cell = m_cfg.grid[gy][gx];

        if (m_editMode == EditMode::MUR) {
            if (lmb && !isPlantHere) { cell.type = WallType::WALL; cell.wallHeight = m_cfg.defaultWallHeight; modified = true; }
            if (rmb) { cell.type = WallType::NONE; modified = true; }
        }
        else if (m_editMode == EditMode::RAMBARDE) {
            if (lmb && !isPlantHere) { cell.type = WallType::RAILING; cell.wallHeight = m_cfg.defaultRailingHeight; modified = true; }
            if (rmb) { cell.type = WallType::NONE; modified = true; }
        }
        // NOUVEAU: Ajout des vitres
        else if (m_editMode == EditMode::VITRE) {
            if (lmb && !isPlantHere) { cell.type = WallType::GLASS; cell.wallHeight = m_cfg.defaultGlassHeight; modified = true; }
            if (rmb) { cell.type = WallType::NONE; modified = true; }
        }

        if (modified) {
            updateGridVertices();
            markSunMapDirty();
            m_app->getUserBalcony().getBalconyConfigRef() = m_cfg;
            m_app->getUserBalcony().sauvegarderProfil("mon_balcon.json");
        }
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

    // 2. Le fond
    renderGrid(window);

    // --- CORRECTION : LE NOUVEL ORDRE ---

    // 3. On dessine d'abord l'ombre portée du balcon (le noir)
    if (m_showShadows) renderShadowLayer(window);

    // 4. On dessine ensuite la Heatmap (qui contient les reflets de vitre)
    // Elle va venir "éclairer" les zones d'ombre !
    if (m_showSunMap) renderSunMapLayer(window);

    // 5. Les plantes par-dessus
    renderPlants(window);

    // 3. On remet la caméra par défaut pour l'UI ImGui
    window.setView(window.getDefaultView());

    static sf::Texture shadowTex;
    if (shadowTex.getSize().x != m_cfg.width || shadowTex.getSize().y != m_cfg.height) {
        shadowTex.create(m_cfg.width, m_cfg.height);
        shadowTex.setSmooth(true); // Flou doux sur les bords
    }

    sf::Sprite shadowSpr(shadowTex);
    shadowSpr.setScale(CELL_PX, CELL_PX);
    
    // On dessine l'ombre en mode Alpha classique
    window.draw(shadowSpr, sf::BlendAlpha);
}

void StateBalconySim::renderGrid(sf::RenderWindow& window) {
    window.draw(m_gridVA);
}

void StateBalconySim::renderShadowLayer(sf::RenderWindow& window) {
    if (!m_sun.isAboveHorizon()) return;

    // Le fameux rectangle brut, calé sur ta grille
    sf::RectangleShape rect(sf::Vector2f(CELL_PX, CELL_PX));

    for (int r = 0; r < m_cfg.height; ++r) {
        for (int c = 0; c < m_cfg.width; ++c) {
            const auto& cell = m_cfg.grid[r][c];
            
            // On ne dessine pas d'ombre par-dessus les murs ou les vitres
            if (cell.isObstacle()) continue;

            // softShadow te renvoie une valeur fine (ex: vitre semi-transparente)
            float soft = SunCalculator::softShadow(m_cfg, c, r, m_sun);
            
            // On convertit ça en opacité (0 = invisible, 180 = ombre bien noire)
            sf::Uint8 alpha = (sf::Uint8)((1.0f - soft) * 180.f);
            if (alpha < 5) continue; // Optimisation : on ne dessine pas ce qu'on ne voit pas

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
    
    window.draw(spr, sf::BlendAlpha);
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
        // NOUVEAU: "Vitre" ajouté au tableau
        const char* modes[] = {"Mur", "Observer", "Plante", "Rambarde", "Vitre"};
        int m = (int)m_editMode;
        if (ImGui::Combo("Mode", &m, modes, 5)) m_editMode = (EditMode)m;

        if (m_editMode == EditMode::MUR) {
            if (ImGui::SliderFloat("H. mur", &m_cfg.defaultWallHeight, 0.5f, 5.0f, "%.1f m"))
                markSunMapDirty();
        }
        if (m_editMode == EditMode::RAMBARDE) {
            if (ImGui::SliderFloat("H. rambarde", &m_cfg.defaultRailingHeight, 0.3f, 1.5f, "%.1f m"))
                markSunMapDirty();
        }
        // NOUVEAU : Slider pour la Vitre
        if (m_editMode == EditMode::VITRE) {
            if (ImGui::SliderFloat("H. vitre", &m_cfg.defaultGlassHeight, 0.5f, 5.0f, "%.1f m"))
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
        
        if (ImGui::Button("Appliquer dimensions")) {
            m_cfg.resize(m_newW, m_newH);
            m_sunGPU.resize(m_newW, m_newH); 
            updateGridVertices();
            markSunMapDirty();
            m_app->getUserBalcony().getBalconyConfigRef() = m_cfg;
            m_app->getUserBalcony().sauvegarderProfil("mon_balcon.json");
            frameCamera(); 
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

    // ========================================================
    //  BULLE DE SURVOL DE PLANTE
    // ========================================================
    if (m_hoveredPlantIdx >= 0 && m_hoveredPlantIdx < (int)m_app->getUserBalcony().getMesPlantes().size()) {
        const auto& p = m_app->getUserBalcony().getMesPlantes()[m_hoveredPlantIdx];

        ImVec2 windowPos = ImVec2(ImGui::GetIO().DisplaySize.x - 50.0f, 50.0f);
        ImGui::SetNextWindowPos(windowPos, ImGuiCond_Always, ImVec2(1.0f, 0.0f)); 
        ImGui::SetNextWindowBgAlpha(0.85f);

        ImGuiWindowFlags flags = ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoResize | 
                                 ImGuiWindowFlags_AlwaysAutoResize | ImGuiWindowFlags_NoMove | 
                                 ImGuiWindowFlags_NoSavedSettings | ImGuiWindowFlags_NoFocusOnAppearing;

        ImGui::Begin("HoverBubble", nullptr, flags);
        
        if (m_plantTextures.count(p.nom_espece)) {
            sf::Texture& tex = m_plantTextures[p.nom_espece];
            ImVec2 imgSize(tex.getSize().x, tex.getSize().y);
            ImGui::SetCursorPosX((ImGui::GetWindowSize().x - imgSize.x) * 0.5f);
            ImGui::Image((ImTextureID)(intptr_t)tex.getNativeHandle(), imgSize);
        }

        ImGui::Spacing();
        ImGui::TextColored(ImVec4(0.4f, 0.9f, 0.5f, 1.0f), "%s", p.surnom.c_str());
        ImGui::TextDisabled("Pot de %d Litres", p.volume_pot_actuel_L);

        ImGui::End();
    }
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
    for (int r = 0; r < m_cfg.height; ++r) {
        for (int c = 0; c < m_cfg.width; ++c) {
            m_cfg.grid[r][c].plantHeight = 0.0f;
            m_cfg.grid[r][c].plantOpacity = 0.0f;
        }
    }

    const auto& plantes = m_app->getUserBalcony().getMesPlantes();
    for (const auto& p : plantes) {
        if (p.position_balcon.x < 0.0f) continue;

        float px_center = p.position_balcon.x / CELL_PX;
        float py_center = p.position_balcon.y / CELL_PX;

        float radiusCells = std::max(1.0f, std::sqrt((float)p.volume_pot_actuel_L) * 2.0f / CELL_PX);
        
        float pHeight = 0.2f + (p.volume_pot_actuel_L * 0.08f); 
        float pOpacity = 0.5f; 

        int rC = std::ceil(radiusCells);
        
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
void StateBalconySim::renderPlants(sf::RenderWindow& window) {
    sf::CircleShape plantShape;
    const auto& plantes = m_app->getUserBalcony().getMesPlantes();
    
    for (int i = 0; i < (int)plantes.size(); ++i) {
        const auto& p = plantes[i];
        
        // On ne dessine que si la plante a une position valide sur le balcon
        if (p.position_balcon.x >= 0.0f) {
            // Le rayon dépend du volume du pot
            float radius = std::max(CELL_PX * 0.4f, std::sqrt((float)p.volume_pot_actuel_L) * 2.0f);
            plantShape.setRadius(radius);
            plantShape.setOrigin(radius, radius); 
            
            // La position est directement en coordonnées "Monde" grâce à la sf::View
            plantShape.setPosition(p.position_balcon);
            
            // Couleurs de survol et de sélection
            if (m_editMode == EditMode::PLANTE && m_selectedPlantIdx == i) {
                // Plante sélectionnée (glissée/déposée)
                plantShape.setFillColor(sf::Color(100, 255, 120, 220));
                plantShape.setOutlineThickness(3.f);
                plantShape.setOutlineColor(sf::Color::White);
            } else if (m_editMode == EditMode::PLANTE && m_hoveredPlantIdx == i) {
                // Plante survolée par la souris
                plantShape.setFillColor(sf::Color(80, 200, 100, 220));
                plantShape.setOutlineThickness(2.f);
                plantShape.setOutlineColor(sf::Color(200, 255, 200));
            } else {
                // Plante normale
                plantShape.setFillColor(sf::Color(40, 130, 60, 220));
                plantShape.setOutlineThickness(1.f);
                plantShape.setOutlineColor(sf::Color(20, 80, 30));
            }
            
            window.draw(plantShape);
        }
    }
}
void StateBalconySim::recalcSun()       { m_sun = SunCalculator::compute(m_cfg); }
void StateBalconySim::markSunMapDirty() { m_sunMapDirty = true; }