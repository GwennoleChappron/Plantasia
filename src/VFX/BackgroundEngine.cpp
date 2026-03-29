#include "BackgroundEngine.hpp"
#include <cmath>
#include <algorithm>
#include <imgui.h>

// ─────────────────────────────────────────────
//  SHADERS BLOOM
// ─────────────────────────────────────────────
static const char* blurH = R"(
uniform sampler2D u_texture;
uniform vec2      resolution;
uniform float     blurRadius;
uniform float     threshold;
uniform float     softness;
void main() {
    vec2 uv  = gl_TexCoord[0].xy;
    vec2 off = vec2(blurRadius / resolution.x, 0.0);
    vec4 sum = vec4(0.0); float total = 0.0;
    for (int x = -4; x <= 4; x++) {
        vec4 s = texture2D(u_texture, uv + off * float(x));
        float b = dot(s.rgb, vec3(0.299,0.587,0.114));
        float g = smoothstep(threshold, threshold + softness, b);
        float w = exp(-float(x*x) / 8.0);
        sum += s * g * w; total += w;
    }
    gl_FragColor = (sum / total) * gl_Color;
})";

static const char* blurV = R"(
uniform sampler2D u_texture;
uniform vec2      resolution;
uniform float     blurRadius;
uniform float     threshold;
uniform float     softness;
void main() {
    vec2 uv  = gl_TexCoord[0].xy;
    vec2 off = vec2(0.0, blurRadius / resolution.y);
    vec4 sum = vec4(0.0); float total = 0.0;
    for (int y = -4; y <= 4; y++) {
        vec4 s = texture2D(u_texture, uv + off * float(y));
        float b = dot(s.rgb, vec3(0.299,0.587,0.114));
        float g = smoothstep(threshold, threshold + softness, b);
        float w = exp(-float(y*y) / 8.0);
        sum += s * g * w; total += w;
    }
    gl_FragColor = (sum / total) * gl_Color;
})";

// ─────────────────────────────────────────────
//  DAY NIGHT SYSTEM
// ─────────────────────────────────────────────
static sf::Color lerpColor(sf::Color a, sf::Color b, float t) {
    t = std::max(0.f, std::min(1.f, t));
    return sf::Color(
        (sf::Uint8)(a.r + (b.r - a.r) * t),
        (sf::Uint8)(a.g + (b.g - a.g) * t),
        (sf::Uint8)(a.b + (b.b - a.b) * t),
        255
    );
}

void DayNightSystem::getSkyColors(sf::Color& top, sf::Color& bottom) const {
    const sf::Color topKeys[4] = {
        sf::Color(255, 120,  80),   // aube
        sf::Color( 80, 160, 240),   // jour
        sf::Color(200,  60, 100),   // sunset
        sf::Color(  8,  15,  30)    // nuit
    };
    const sf::Color bottomKeys[4] = {
        sf::Color(255, 200, 120),
        sf::Color(160, 210, 255),
        sf::Color(255, 140,  60),
        sf::Color( 12,  25,  45)
    };
    int   seg  = (int)(dayTime * 4.f) % 4;
    float t    = std::fmod(dayTime * 4.f, 1.f);
    int   next = (seg + 1) % 4;
    top    = lerpColor(topKeys[seg],    topKeys[next],    t);
    bottom = lerpColor(bottomKeys[seg], bottomKeys[next], t);
}

sf::Vector2f DayNightSystem::getSunPos(float width, float height) const {
    float angle = dayTime * 3.14159f * 2.f;
    float x = width  * 0.5f + std::cos(angle - 3.14159f * 0.5f) * width  * 0.42f;
    float y = height * 0.38f - std::sin(angle - 3.14159f * 0.5f) * height * 0.32f;
    return {x, y};
}

float DayNightSystem::getNightFactor() const {
    float d = dayTime;
    if (d > 0.5f) return std::sin((d - 0.5f) / 0.5f * 3.14159f);
    return 0.f;
}

// ─────────────────────────────────────────────
//  CONSTRUCTEUR
// ─────────────────────────────────────────────
BackgroundEngine::BackgroundEngine(float width, float height)
    : m_width(width), m_height(height),
      m_clouds(width, height),
      m_vines(width, height),
      m_swarm(60, width, height),
      m_grass(width, height),
      m_leaves(width, height),
      m_rain(width, height)
{
    // Lucioles
    for (int i = 0; i < 80; i++) {
        Luciole l;
        l.basePos     = {std::fmod(i*137.5f, width), std::fmod(i*93.1f, height)};
        l.offset      = (float)i;
        l.speed       = 5.f + (i % 8);
        l.radius      = 2.f + (i % 4) * 0.8f;
        l.flickerPhase= (float)(i * 17 % 100) * 0.1f;
        m_lucioles.push_back(l);
    }

    // Callback pluie → agite l'herbe localement
    m_rain.setOnImpact([this](float x, float strength) {
        m_grass.addWindBoost(strength);
    });

    // Bloom
    if (sf::Shader::isAvailable()) {
        bool ok = m_blurShaderH.loadFromMemory(blurH, sf::Shader::Fragment) &&
                  m_blurShaderV.loadFromMemory(blurV, sf::Shader::Fragment);
        if (ok) {
            m_shaderActif = true;
            m_glowTexture.create((unsigned int)(width/2), (unsigned int)(height/2));
            m_glowPing.create   ((unsigned int)(width/2), (unsigned int)(height/2));
            m_glowSprite.setTexture(m_glowTexture.getTexture());
            m_glowSprite.setScale(2.f, 2.f);
        }
    }

    // PostFX
    m_postFX.init((unsigned int)width, (unsigned int)height);
}

// ─────────────────────────────────────────────
//  UPDATE
// ─────────────────────────────────────────────
void BackgroundEngine::update(float dt, sf::Vector2i mousePos, float width, float height) {
    float s    = dt * m_timeScale;
    m_temps   += s;
    m_mousePos = mousePos;

    m_dayNight.update(s);
    updateEvents(s);
    if (m_enablePostFX) { m_postFX.update(s); syncPostFXGrade(); }

    if (m_enableClouds) m_clouds.update(s);
    if (m_enableBoids)  m_swarm.update(s, {(float)mousePos.x, (float)mousePos.y}, width, height);
    if (m_enableVines)  m_vines.update(s, mousePos, width, height);
    if (m_enableGrass)  m_grass.update(s, mousePos);
    if (m_enableLeaves) m_leaves.update(s);
    if (m_enableRain)   m_rain.update(s);
}

void BackgroundEngine::updateEvents(float dt) {
    for (int i = 0; i < (int)m_events.size(); ) {
        auto& e = m_events[i];
        e.timer += dt;
        float t = std::min(e.timer / e.duration, 1.f);
        if (e.tick) e.tick(t, dt);
        if (e.timer >= e.duration) {
            m_events[i] = m_events.back();
            m_events.pop_back();
        } else { ++i; }
    }
}

// Color grade synchronisé avec le moment de la journée
void BackgroundEngine::syncPostFXGrade() {
    if (!m_enableDayNight) { m_postFX.setGradeNeutral(); return; }

    float dt = m_dayNight.dayTime;
    // aube [0, 0.2] : warm
    if (dt < 0.2f)       m_postFX.setGradeWarm();
    // jour [0.2, 0.45] : neutre
    else if (dt < 0.45f) m_postFX.setGradeNeutral();
    // sunset [0.45, 0.65] : sunset
    else if (dt < 0.65f) m_postFX.setGradeSunset();
    // nuit [0.65, 1.0] : night
    else                  m_postFX.setGradeNight();
}

// ─────────────────────────────────────────────
//  CALCUL POSITIONS LUCIOLES (pour constellation)
// ─────────────────────────────────────────────
std::vector<sf::Vector3f> BackgroundEngine::computeLuciolePositions() {
    std::vector<sf::Vector3f> out;
    if (!m_enableLucioles) return out;

    float nightBoost = 0.4f + 0.6f * m_dayNight.getNightFactor();
    float effectiveI = m_lucioleIntensity * nightBoost;

    out.reserve(m_lucioles.size());
    for (const auto& l : m_lucioles) {
        float x = l.basePos.x + std::sin(m_temps*0.4f + l.offset) * 50.f;
        float y = l.basePos.y - std::fmod(m_temps*l.speed, m_height);
        if (y < 0) y += m_height;
        float pulse = std::sin(m_temps*2.f + l.offset) * 0.5f + 0.5f;
        float alpha = pulse * effectiveI; // 0→1
        out.push_back({x, y, alpha});
    }
    return out;
}

// ─────────────────────────────────────────────
//  DRAW SKY
// ─────────────────────────────────────────────
void BackgroundEngine::drawSky(sf::RenderTarget& target) {
    sf::Color skyTop, skyBottom;
    if (m_enableDayNight) m_dayNight.getSkyColors(skyTop, skyBottom);
    else { skyTop = sf::Color(8,14,8); skyBottom = sf::Color(12,20,12); }

    sf::VertexArray bg(sf::Quads, 4);
    float w = (float)target.getSize().x;
    float h = (float)target.getSize().y;
    bg[0].position = {0,0}; bg[0].color = skyTop;
    bg[1].position = {w,0}; bg[1].color = skyTop;
    bg[2].position = {w,h}; bg[2].color = skyBottom;
    bg[3].position = {0,h}; bg[3].color = skyBottom;
    target.draw(bg);
}

// ─────────────────────────────────────────────
//  DRAW SOLEIL / LUNE
// ─────────────────────────────────────────────
void BackgroundEngine::drawSunMoon(sf::RenderTarget& target) {
    if (!m_enableDayNight) return;

    float w = (float)target.getSize().x;
    float h = (float)target.getSize().y;
    sf::Vector2f pos  = m_dayNight.getSunPos(w, h);
    float nightFactor = m_dayNight.getNightFactor();
    float dt          = m_dayNight.dayTime;

    sf::CircleShape body;

    // Soleil
    if (dt > 0.05f && dt < 0.60f) {
        float alpha = std::min(1.f, std::min(dt / 0.1f, (0.60f - dt) / 0.1f));
        body.setRadius(28.f); body.setOrigin(28.f,28.f); body.setPosition(pos);
        body.setFillColor(sf::Color(255,230,120,(sf::Uint8)(220*alpha)));
        target.draw(body);
        if (m_shaderActif) {
            sf::CircleShape g; g.setRadius(50.f); g.setOrigin(50.f,50.f);
            g.setPosition(pos.x*0.5f, pos.y*0.5f);
            g.setFillColor(sf::Color(255,200,80,(sf::Uint8)(180*alpha)));
            m_glowTexture.draw(g);
        }
    }

    // Lune
    if (dt > 0.60f || dt < 0.10f) {
        sf::Vector2f moonPos = {w - pos.x, h * 0.15f + (1.f - nightFactor) * h * 0.1f};
        moonPos.x = std::max(30.f, std::min(w-30.f, moonPos.x));
        body.setRadius(18.f); body.setOrigin(18.f,18.f); body.setPosition(moonPos);
        body.setFillColor(sf::Color(220,230,255,(sf::Uint8)(200*nightFactor)));
        target.draw(body);
        if (m_shaderActif) {
            sf::CircleShape g; g.setRadius(32.f); g.setOrigin(32.f,32.f);
            g.setPosition(moonPos.x*0.5f, moonPos.y*0.5f);
            g.setFillColor(sf::Color(180,200,255,(sf::Uint8)(120*nightFactor)));
            m_glowTexture.draw(g);
        }
    }
}

// ─────────────────────────────────────────────
//  DRAW LUCIOLES
// ─────────────────────────────────────────────
void BackgroundEngine::drawLucioles(sf::RenderTarget& target) {
    if (!m_enableLucioles) return;

    float nightBoost     = 0.4f + 0.6f * m_dayNight.getNightFactor();
    float effectiveI     = m_lucioleIntensity * nightBoost;
    sf::CircleShape core, glowShape;

    // Positions pour constellations
    std::vector<sf::Vector3f> positions;
    positions.reserve(m_lucioles.size());

    for (const auto& l : m_lucioles) {
        float x = l.basePos.x + std::sin(m_temps*0.4f + l.offset) * 50.f;
        float y = l.basePos.y - std::fmod(m_temps*l.speed, m_height);
        if (y < 0) y += m_height;

        float pulse   = std::sin(m_temps*2.f + l.offset) * 0.5f + 0.5f;
        float fk      = std::sin(m_temps * 13.7f + l.flickerPhase);
        float flicker = (fk > 0.92f) ? 0.25f : 1.f;
        float alpha   = pulse * 255.f * effectiveI * flicker;

        core.setRadius(l.radius);
        core.setOrigin(l.radius, l.radius);
        core.setPosition(x, y);
        core.setFillColor({200,255,140,(sf::Uint8)std::clamp(alpha*m_coreBrightness,0.f,255.f)});
        target.draw(core);

        if (m_shaderActif) {
            glowShape.setRadius(l.radius * m_glowSizeMultiplier);
            glowShape.setOrigin(glowShape.getRadius(), glowShape.getRadius());
            glowShape.setPosition(x*0.5f, y*0.5f);
            glowShape.setFillColor({180,255,120,(sf::Uint8)std::clamp(alpha*0.6f,0.f,255.f)});
            m_glowTexture.draw(glowShape);
        }

        positions.push_back({x, y, alpha / 255.f});
    }

    // Mise à jour + rendu des constellations
    if (m_enableConstellation && m_dayNight.getNightFactor() > 0.2f) {
        m_constellation.setMaxDist(m_constellMaxDist);
        m_constellation.rebuild(positions);
        m_constellation.draw(target);
        if (m_shaderActif) m_constellation.drawGlow(m_glowTexture);
    }
}

// ─────────────────────────────────────────────
//  BLOOM
// ─────────────────────────────────────────────
void BackgroundEngine::applyBloom(sf::RenderTarget& target) {
    if (!m_shaderActif) return;

    m_glowTexture.display();
    sf::Vector2f halfRes((float)m_glowTexture.getSize().x, (float)m_glowTexture.getSize().y);

    m_glowPing.clear(sf::Color::Transparent);
    {
        m_blurShaderH.setUniform("u_texture",  sf::Shader::CurrentTexture);
        m_blurShaderH.setUniform("resolution", halfRes);
        m_blurShaderH.setUniform("blurRadius", m_bloomRadius);
        m_blurShaderH.setUniform("threshold",  m_bloomThreshold);
        m_blurShaderH.setUniform("softness",   m_bloomSoftness);
        sf::Sprite src(m_glowTexture.getTexture());
        sf::RenderStates st; st.shader = &m_blurShaderH;
        m_glowPing.draw(src, st);
        m_glowPing.display();
    }
    {
        m_blurShaderV.setUniform("u_texture",  sf::Shader::CurrentTexture);
        m_blurShaderV.setUniform("resolution", halfRes);
        m_blurShaderV.setUniform("blurRadius", m_bloomRadius);
        m_blurShaderV.setUniform("threshold",  m_bloomThreshold);
        m_blurShaderV.setUniform("softness",   m_bloomSoftness);

        sf::Uint8 intensity = (sf::Uint8)std::clamp(255.f*m_bloomMultiplier, 0.f, 255.f);
        sf::Sprite pingSprite(m_glowPing.getTexture());
        pingSprite.setScale(2.f, 2.f);
        pingSprite.setColor({intensity, intensity, intensity, 255});

        sf::RenderStates st;
        st.shader    = &m_blurShaderV;
        st.blendMode = sf::BlendAdd;
        target.draw(pingSprite, st);
    }
}

// ─────────────────────────────────────────────
//  DRAW PRINCIPAL
// ─────────────────────────────────────────────
void BackgroundEngine::draw(sf::RenderTarget& finalTarget) {
    // Choisir la cible : scène intermédiaire si PostFX actif, sinon direct
    sf::RenderTarget* target = &finalTarget;
    if (m_enablePostFX && m_postFX.isAvailable()) {
        target = &m_postFX.getSceneTarget();
        m_postFX.getSceneTarget().clear(sf::Color::Black);
    }

    // ── Gradient de ciel ──────────────────────
    drawSky(*target);

    // ── Glow texture reset ────────────────────
    if (m_shaderActif) m_glowTexture.clear(sf::Color::Transparent);

    // ── Soleil / Lune ─────────────────────────
    drawSunMoon(*target);

    // ── Pluie (avant les autres pour être "derrière") ─
    if (m_enableRain) m_rain.draw(*target);

    // ── Nuages ────────────────────────────────
    if (m_enableClouds) m_clouds.draw(*target);

    // ── Lianes ────────────────────────────────
    if (m_enableVines) m_vines.draw(*target);

    // ── Boids ─────────────────────────────────
    if (m_enableBoids) m_swarm.draw(*target);

    // ── Herbe ─────────────────────────────────
    if (m_enableGrass) m_grass.draw(*target);

    // ── Feuilles ──────────────────────────────
    if (m_enableLeaves) m_leaves.draw(*target);

    // ── Lucioles + Constellations ─────────────
    drawLucioles(*target);

    // ── Bloom pass ────────────────────────────
    applyBloom(*target);

    // ── PostFX : vignette + color grading ─────
    if (m_enablePostFX && m_postFX.isAvailable()) {
        m_postFX.apply(finalTarget);
    }
}

// ─────────────────────────────────────────────
//  API ÉVÉNEMENTS
// ─────────────────────────────────────────────
void BackgroundEngine::triggerEvent(BackgroundEvent e) {
    m_events.push_back(std::move(e));
}

void BackgroundEngine::triggerWindBurst() {
    triggerEvent({2.5f, 0.f, [this](float t, float dt) {
        float i = std::sin(t * 3.14159f);
        m_grass.addWindBoost(i * 8.f * dt);
        m_vines.addAgitation(i * 5.f * dt);
        m_clouds.addWind(i * 3.f * dt);
        if (m_enableLeaves) m_leaves.addWind(i * 0.4f * dt);
    }});
}

void BackgroundEngine::triggerLucioleBurst() {
    float saved = m_lucioleIntensity;
    triggerEvent({3.0f, 0.f, [this, saved](float t, float dt) {
        m_lucioleIntensity = saved * (1.f + std::sin(t * 3.14159f) * 0.8f);
    }});
}

void BackgroundEngine::triggerLeafShower() {
    m_enableLeaves = true;
    m_leaves.burst(40);
}

void BackgroundEngine::triggerRainStart(float intensity) {
    m_enableRain = true;
    m_rain.setIntensity(intensity);
    // La pluie réduit le bloom (effet humide) et refroidit le color grade
    m_bloomMultiplier = std::max(0.8f, m_bloomMultiplier * 0.6f);
    m_postFX.setGradeCool();
}

void BackgroundEngine::triggerRainStop() {
    // Fade out progressif
    triggerEvent({3.0f, 0.f, [this](float t, float dt) {
        m_rain.setIntensity(std::max(0.f, m_rain.getIntensity() - dt * 0.3f));
        if (m_rain.getIntensity() <= 0.f) m_enableRain = false;
    }});
}

void BackgroundEngine::setSeason(Season s) {
    m_leaves.setSeason(s);
    switch (s) {
        case Season::SPRING:
            m_enableLeaves = true;
            break;
        case Season::SUMMER:
            m_enableLeaves = false;
            break;
        case Season::AUTUMN:
            m_enableLeaves = true;
            break;
        case Season::WINTER:
            m_enableLeaves = true;
            m_enableLucioles = false;
            break;
    }
}

// ─────────────────────────────────────────────
//  PRESETS
// ─────────────────────────────────────────────
void BackgroundEngine::applyPreset(BackgroundPreset p) {
    // Reset
    m_enableBoids         = true;
    m_enableGrass         = true;
    m_enableVines         = true;
    m_enableLucioles      = true;
    m_enableClouds        = true;
    m_enableDayNight      = false;
    m_enableLeaves        = false;
    m_enableRain          = false;
    m_enableConstellation = true;
    m_dayNight.autoAdvance= false;
    m_rain.setIntensity(0.f);
    m_bloomMultiplier     = 2.2f;
    m_postFX.setGradeNeutral();

    switch (p) {
        case BackgroundPreset::NORMAL:
            m_timeScale        = 1.0f;
            m_lucioleIntensity = 0.8f;
            break;

        case BackgroundPreset::ZEN:
            m_timeScale        = 0.35f;
            m_lucioleIntensity = 0.4f;
            m_bloomMultiplier  = 1.4f;
            m_enableBoids      = false;
            m_postFX.setVignette(0.55f);
            break;

        case BackgroundPreset::NUIT:
            m_timeScale           = 0.7f;
            m_lucioleIntensity    = 1.3f;
            m_bloomMultiplier     = 3.0f;
            m_enableBoids         = false;
            m_enableDayNight      = true;
            m_dayNight.dayTime    = 0.78f;
            m_enableConstellation = true;
            m_postFX.setGradeNight();
            m_postFX.setVignette(0.6f);
            break;

        case BackgroundPreset::VENT:
            m_timeScale        = 2.8f;
            m_lucioleIntensity = 0.f;
            m_bloomMultiplier  = 1.0f;
            m_enableBoids      = false;
            m_enableLucioles   = false;
            m_enableLeaves     = true;
            triggerWindBurst();
            break;

        case BackgroundPreset::SUNSET:
            m_timeScale           = 0.8f;
            m_lucioleIntensity    = 0.5f;
            m_bloomMultiplier     = 2.8f;
            m_enableDayNight      = true;
            m_dayNight.dayTime    = 0.52f;
            m_postFX.setGradeSunset();
            m_postFX.setVignette(0.5f);
            break;

        case BackgroundPreset::AUTOMNE:
            m_timeScale           = 0.9f;
            m_lucioleIntensity    = 0.3f;
            m_bloomMultiplier     = 1.6f;
            m_enableDayNight      = true;
            m_dayNight.dayTime    = 0.30f;
            m_enableLeaves        = true;
            m_enableConstellation = false;
            m_leaves.setSeason(Season::AUTUMN);
            m_postFX.setGradeCustom(1.10f, 0.90f, 0.75f); // teinte automnale
            m_postFX.setVignette(0.5f);
            break;

        case BackgroundPreset::PLUIE:
            m_timeScale           = 0.7f;
            m_lucioleIntensity    = 0.2f;
            m_bloomMultiplier     = 1.0f;
            m_enableDayNight      = true;
            m_dayNight.dayTime    = 0.35f; // jour gris
            m_enableConstellation = false;
            triggerRainStart(0.85f);
            m_postFX.setGradeCool();
            m_postFX.setVignette(0.6f);
            break;
    }
}

// ─────────────────────────────────────────────
//  IMGUI COMPOSER
// ─────────────────────────────────────────────
void BackgroundEngine::drawImGui() {
    ImGui::SetNextWindowPos({10, 10}, ImGuiCond_FirstUseEver);
    ImGui::SetNextWindowSize({320, 0}, ImGuiCond_FirstUseEver);
    ImGui::Begin("Background Composer", nullptr, ImGuiWindowFlags_AlwaysAutoResize);

    // ── PRESETS ───────────────────────────────
    ImGui::TextColored({0.55f,0.75f,0.55f,1.f}, "Presets");

    auto PBtn = [&](const char* label, BackgroundPreset p) {
        ImGui::PushStyleColor(ImGuiCol_Button,        {0.08f,0.18f,0.10f,1.f});
        ImGui::PushStyleColor(ImGuiCol_ButtonHovered, {0.15f,0.35f,0.18f,1.f});
        if (ImGui::Button(label, {92.f, 28.f})) applyPreset(p);
        ImGui::PopStyleColor(2);
    };
    PBtn("Normal",  BackgroundPreset::NORMAL);  ImGui::SameLine(0,4);
    PBtn("Zen",     BackgroundPreset::ZEN);     ImGui::SameLine(0,4);
    PBtn("Sunset",  BackgroundPreset::SUNSET);
    PBtn("Nuit",    BackgroundPreset::NUIT);    ImGui::SameLine(0,4);
    PBtn("Automne", BackgroundPreset::AUTOMNE); ImGui::SameLine(0,4);
    PBtn("Pluie",   BackgroundPreset::PLUIE);
    PBtn("Vent",    BackgroundPreset::VENT);

    ImGui::Dummy({0,5}); ImGui::Separator();

    // ── ÉVÉNEMENTS ────────────────────────────
    ImGui::TextColored({0.55f,0.75f,0.55f,1.f}, "Evenements");
    ImGui::PushStyleColor(ImGuiCol_Button,        {0.08f,0.18f,0.10f,1.f});
    ImGui::PushStyleColor(ImGuiCol_ButtonHovered, {0.15f,0.35f,0.18f,1.f});
    if (ImGui::Button("Rafale Vent",   {140.f,24.f})) triggerWindBurst();   ImGui::SameLine(0,4);
    if (ImGui::Button("Lucioles Burst",{140.f,24.f})) triggerLucioleBurst();
    if (ImGui::Button("Averse Feuilles",{140.f,24.f})) triggerLeafShower(); ImGui::SameLine(0,4);
    if (ImGui::Button("Pluie ON",      {140.f,24.f})) triggerRainStart();
    if (ImGui::Button("Pluie OFF",     {140.f,24.f})) triggerRainStop();
    ImGui::PopStyleColor(2);

    ImGui::Dummy({0,5}); ImGui::Separator();

    // ── JOUR / NUIT ───────────────────────────
    ImGui::TextColored({0.55f,0.75f,0.55f,1.f}, "Cycle Jour / Nuit");
    ImGui::Checkbox("Activer##dn", &m_enableDayNight);
    if (m_enableDayNight) {
        ImGui::SameLine(); ImGui::Checkbox("Auto", &m_dayNight.autoAdvance);
        ImGui::PushItemWidth(-1.f);
        ImGui::SliderFloat("##dt",  &m_dayNight.dayTime,    0.f, 1.f,   "Heure   %.2f");
        if (m_dayNight.autoAdvance)
            ImGui::SliderFloat("##spd",&m_dayNight.autoSpeed, 0.001f, 0.05f,"Vitesse %.3f");
        ImGui::PopItemWidth();
        sf::Color st, sb; m_dayNight.getSkyColors(st, sb);
        ImGui::ColorButton("##skyT", {st.r/255.f,st.g/255.f,st.b/255.f,1.f},
                           ImGuiColorEditFlags_NoTooltip, {22,14});
        ImGui::SameLine(); ImGui::TextDisabled("haut");
        ImGui::SameLine(90.f);
        ImGui::ColorButton("##skyB", {sb.r/255.f,sb.g/255.f,sb.b/255.f,1.f},
                           ImGuiColorEditFlags_NoTooltip, {22,14});
        ImGui::SameLine(); ImGui::TextDisabled("bas");
    }

    ImGui::Dummy({0,5}); ImGui::Separator();

    // ── PLUIE ─────────────────────────────────
    ImGui::TextColored({0.55f,0.75f,0.55f,1.f}, "Pluie");
    ImGui::Checkbox("Activer##rain", &m_enableRain);
    if (m_enableRain) {
        float intensity = m_rain.getIntensity();
        ImGui::PushItemWidth(-1.f);
        if (ImGui::SliderFloat("##ri", &intensity, 0.f, 1.f, "Intensite %.2f"))
            m_rain.setIntensity(intensity);
        float windAngle = -8.f;
        if (ImGui::SliderFloat("##rw", &windAngle, -30.f, 30.f, "Vent      %.1f deg"))
            m_rain.setWindAngle(windAngle);
        ImGui::PopItemWidth();
    }

    ImGui::Dummy({0,5}); ImGui::Separator();

    // ── FEUILLES ──────────────────────────────
    ImGui::TextColored({0.55f,0.75f,0.55f,1.f}, "Feuilles");
    ImGui::Checkbox("Activer##leaves", &m_enableLeaves);
    if (m_enableLeaves) {
        Season cur = m_leaves.getSeason();
        const char* seasons[] = {"Printemps","Ete","Automne","Hiver"};
        int s = (int)cur;
        ImGui::PushItemWidth(-1.f);
        if (ImGui::Combo("##season", &s, seasons, 4))
            m_leaves.setSeason((Season)s);
        ImGui::PopItemWidth();
    }

    ImGui::Dummy({0,5}); ImGui::Separator();

    // ── CONSTELLATIONS ────────────────────────
    ImGui::TextColored({0.55f,0.75f,0.55f,1.f}, "Constellations");
    ImGui::Checkbox("Activer##cons", &m_enableConstellation);
    if (m_enableConstellation) {
        ImGui::PushItemWidth(-1.f);
        if (ImGui::SliderFloat("##cdist", &m_constellMaxDist, 40.f, 250.f, "Dist max %.0f px"))
            m_constellation.setMaxDist(m_constellMaxDist);
        ImGui::PopItemWidth();
    }

    ImGui::Dummy({0,5}); ImGui::Separator();

    // ── POST FX ───────────────────────────────
    ImGui::TextColored({0.55f,0.75f,0.55f,1.f}, "Post FX");
    ImGui::Checkbox("Activer##pfx", &m_enablePostFX);
    if (m_enablePostFX && m_postFX.isAvailable()) {
        float vs = m_postFX.getVignetteStrength();
        float vr = m_postFX.getVignetteRadius();
        ImGui::PushItemWidth(-1.f);
        if (ImGui::SliderFloat("##vs", &vs, 0.f, 1.f, "Vignette force %.2f"))
            m_postFX.setVignette(vs, vr);
        if (ImGui::SliderFloat("##vr", &vr, 0.3f, 1.f, "Vignette rayon %.2f"))
            m_postFX.setVignette(vs, vr);
        ImGui::PopItemWidth();
        ImGui::TextDisabled("Color grade:");
        ImGui::SameLine();
        if (ImGui::SmallButton("Neutre"))  m_postFX.setGradeNeutral();
        ImGui::SameLine();
        if (ImGui::SmallButton("Chaud"))   m_postFX.setGradeWarm();
        ImGui::SameLine();
        if (ImGui::SmallButton("Nuit"))    m_postFX.setGradeNight();
        if (ImGui::SmallButton("Sunset"))  m_postFX.setGradeSunset();
        ImGui::SameLine();
        if (ImGui::SmallButton("Froid"))   m_postFX.setGradeCool();
        ImGui::SameLine();
        if (ImGui::SmallButton("Dream"))   m_postFX.setGradeDream();
    } else if (!m_postFX.isAvailable()) {
        ImGui::TextDisabled("(shaders non disponibles)");
    }

    ImGui::Dummy({0,5}); ImGui::Separator();

    // ── BLOOM ─────────────────────────────────
    ImGui::TextColored({0.55f,0.75f,0.55f,1.f}, "Bloom");
    ImGui::PushItemWidth(-1.f);
    ImGui::SliderFloat("##brad", &m_bloomRadius,     0.f, 30.f, "Radius    %.1f");
    ImGui::SliderFloat("##bint", &m_bloomMultiplier, 0.f,  5.f, "Intensite %.2f");
    ImGui::SliderFloat("##bthr", &m_bloomThreshold,  0.f,  1.f, "Seuil     %.2f");
    ImGui::SliderFloat("##bsft", &m_bloomSoftness,   0.f,  1.f, "Douceur   %.2f");
    ImGui::PopItemWidth();

    ImGui::Dummy({0,5}); ImGui::Separator();

    // ── LUCIOLES ──────────────────────────────
    ImGui::TextColored({0.55f,0.75f,0.55f,1.f}, "Lucioles");
    ImGui::PushItemWidth(-1.f);
    ImGui::SliderFloat("##lglow", &m_glowSizeMultiplier, 1.f, 6.f,  "Taille glow %.1f");
    ImGui::SliderFloat("##lcore", &m_coreBrightness,     0.5f,3.f,  "Luminosite  %.1f");
    ImGui::SliderFloat("##lint",  &m_lucioleIntensity,   0.f, 1.5f, "Intensite   %.2f");
    ImGui::PopItemWidth();

    ImGui::Dummy({0,5}); ImGui::Separator();

    // ── TEMPS ─────────────────────────────────
    ImGui::TextColored({0.55f,0.75f,0.55f,1.f}, "Temps");
    ImGui::PushItemWidth(-1.f);
    ImGui::SliderFloat("##ts", &m_timeScale, 0.f, 5.f, "Vitesse %.2fx");
    ImGui::PopItemWidth();

    ImGui::Dummy({0,5}); ImGui::Separator();

    // ── TOGGLES ───────────────────────────────
    ImGui::TextColored({0.55f,0.75f,0.55f,1.f}, "Systemes");
    auto Toggle = [&](const char* label, bool* v) {
        if (*v) ImGui::PushStyleColor(ImGuiCol_Button, {0.15f,0.45f,0.25f,1.f});
        else    ImGui::PushStyleColor(ImGuiCol_Button, {0.08f,0.15f,0.10f,1.f});
        if (ImGui::Button(label, {96.f, 24.f})) *v = !*v;
        ImGui::PopStyleColor();
    };
    Toggle("Boids",     &m_enableBoids);         ImGui::SameLine(0,4);
    Toggle("Lucioles",  &m_enableLucioles);
    Toggle("Lianes",    &m_enableVines);          ImGui::SameLine(0,4);
    Toggle("Herbe",     &m_enableGrass);
    Toggle("Nuages",    &m_enableClouds);         ImGui::SameLine(0,4);
    Toggle("Feuilles",  &m_enableLeaves);
    Toggle("Pluie",     &m_enableRain);           ImGui::SameLine(0,4);
    Toggle("Constell.", &m_enableConstellation);
    Toggle("JourNuit",  &m_enableDayNight);       ImGui::SameLine(0,4);
    Toggle("PostFX",    &m_enablePostFX);

    if (!m_events.empty()) {
        ImGui::Dummy({0,4});
        ImGui::TextColored({0.8f,0.6f,0.2f,1.f}, "%d event(s) actif(s)", (int)m_events.size());
    }

    ImGui::End();
}
