#pragma once
#include <SFML/Graphics.hpp>
#include <vector>
#include <functional>
#include <memory>

#include "IBackgroundLayer.hpp"
#include "BoidsSwarm.hpp"
#include "VineSystem.hpp"
#include "GrassSystem.hpp"
#include "CloudLayer.hpp"
#include "LeafSystem.hpp"
#include "RainSystem.hpp"
#include "ConstellationLayer.hpp"
#include "PostFX.hpp"

// ─────────────────────────────────────────────
//  SYSTÈME JOUR / NUIT
// ─────────────────────────────────────────────
struct DayNightSystem {
    float dayTime    = 0.55f;
    bool  autoAdvance= false;
    float autoSpeed  = 0.01f;

    void update(float dt) {
        if (!autoAdvance) return;
        dayTime = std::fmod(dayTime + autoSpeed * dt, 1.f);
    }

    void getSkyColors(sf::Color& top, sf::Color& bottom) const;
    sf::Vector2f getSunPos(float width, float height) const;
    bool  isNight()        const { return dayTime > 0.65f || dayTime < 0.1f; }
    float getNightFactor() const;
};

// ─────────────────────────────────────────────
//  ÉVÉNEMENTS
// ─────────────────────────────────────────────
struct BackgroundEvent {
    float duration;
    float timer = 0.f;
    std::function<void(float t, float dt)> tick;
};

// ─────────────────────────────────────────────
//  LUCIOLES
// ─────────────────────────────────────────────
struct Luciole {
    sf::Vector2f basePos;
    float offset;
    float speed;
    float radius;
    float flickerPhase;
};

// ─────────────────────────────────────────────
//  PRESETS
// ─────────────────────────────────────────────
enum class BackgroundPreset {
    NORMAL, ZEN, NUIT, VENT, SUNSET, AUTOMNE, PLUIE
};

// ─────────────────────────────────────────────
//  BACKGROUND ENGINE
// ─────────────────────────────────────────────
class BackgroundEngine {
private:
    float m_width, m_height;

    // ── Systèmes visuels ─────────────────────
    CloudLayer         m_clouds;
    VineSystem         m_vines;
    BoidsSwarm         m_swarm;
    GrassSystem        m_grass;
    LeafSystem         m_leaves;
    RainSystem         m_rain;
    ConstellationLayer m_constellation;
    PostFX             m_postFX;
    std::vector<Luciole> m_lucioles;

    // ── Temps ────────────────────────────────
    float m_temps     = 0.0f;
    float m_timeScale = 1.0f;

    // ── Toggles ──────────────────────────────
    bool  m_enableLucioles      = true;
    bool  m_enableBoids         = true;
    bool  m_enableVines         = true;
    bool  m_enableGrass         = true;
    bool  m_enableClouds        = true;
    bool  m_enableDayNight      = false;
    bool  m_enableLeaves        = false;
    bool  m_enableRain          = false;
    bool  m_enableConstellation = true;
    bool  m_enablePostFX        = true;

    // ── Lucioles ─────────────────────────────
    float m_lucioleIntensity   = 0.8f;
    float m_glowSizeMultiplier = 3.5f;
    float m_coreBrightness     = 1.2f;
    float m_constellMaxDist    = 120.f;

    // ── Bloom ────────────────────────────────
    sf::RenderTexture m_glowTexture;
    sf::RenderTexture m_glowPing;
    sf::Sprite        m_glowSprite;
    sf::Shader        m_blurShaderH;
    sf::Shader        m_blurShaderV;
    bool              m_shaderActif     = false;
    float             m_bloomRadius     = 10.0f;
    float             m_bloomMultiplier = 2.2f;
    float             m_bloomThreshold  = 0.15f;
    float             m_bloomSoftness   = 0.5f;

    // ── Jour / Nuit ───────────────────────────
    DayNightSystem m_dayNight;

    // ── Events ───────────────────────────────
    std::vector<BackgroundEvent> m_events;

    sf::Vector2i m_mousePos;

    // ── Helpers privés ────────────────────────
    std::vector<sf::Vector3f> computeLuciolePositions();
    void drawSky(sf::RenderTarget& target);
    void drawSunMoon(sf::RenderTarget& target);
    void drawLucioles(sf::RenderTarget& target);
    void applyBloom(sf::RenderTarget& target);
    void updateEvents(float dt);
    void syncPostFXGrade();

public:
    BackgroundEngine(float width, float height);

    void update(float dt, sf::Vector2i mousePos, float width, float height);
    void draw(sf::RenderTarget& target);
    void drawImGui();

    // ── API événements ────────────────────────
    void triggerEvent(BackgroundEvent e);
    void triggerWindBurst();
    void triggerLucioleBurst();
    void triggerLeafShower();
    void triggerRainStart(float intensity = 0.8f);
    void triggerRainStop();

    // ── Saisons ──────────────────────────────
    void setSeason(Season s);

    // ── Presets ──────────────────────────────
    void applyPreset(BackgroundPreset p);

    // ── Accès depuis les States ───────────────
    DayNightSystem& getDayNight() { return m_dayNight; }
    LeafSystem&     getLeaves()   { return m_leaves; }
    RainSystem&     getRain()     { return m_rain; }
    PostFX&         getPostFX()   { return m_postFX; }
};
