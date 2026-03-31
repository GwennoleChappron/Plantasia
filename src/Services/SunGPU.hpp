#pragma once
#include <string>
#include <vector>
#include "Data/BalconyConfig.hpp"
#include "Services/SunCalculator.hpp"
#include <SFML/Graphics.hpp>

// Pas d'include glad ici — on l'isole dans le .cpp avec garde

struct SunColor {
    float r, g, b;
    static SunColor fromElevation(float elevDeg);
};

// ─────────────────────────────────────────────────────────────
//  SunGPU : facade avec détection automatique GPU/CPU
//  - GPU path : compute shaders OpenGL 4.3 (Windows/Linux)
//  - CPU path : fallback multi-thread léger (Mac + tout le reste)
//  Interface identique dans les deux cas.
// ─────────────────────────────────────────────────────────────
class SunGPU {
public:
    SunGPU()  = default;
    ~SunGPU();

    // Initialise le backend disponible.
    // Retourne toujours true (CPU fallback garanti).
    bool init(int gridW, int gridH);
    void resize(int gridW, int gridH);

    // Lance le calcul complet (toutes heures, rebonds).
    void compute(const BalconyConfig& cfg, int bounceCount = 2);

    const sf::Texture& getResultTexture() const { return m_resultTex; }
    SunColor           getCurrentSunColor() const { return m_currentSunColor; }
    bool               isReady()           const { return m_ready; }
    bool               isUsingGPU()        const { return m_gpuAvailable; }

private:
    bool m_ready        = false;
    bool m_gpuAvailable = false;
    int  m_gridW = 0, m_gridH = 0;

    sf::Texture m_resultTex;
    SunColor    m_currentSunColor = {1.f, 1.f, 1.f};

    // ── GPU members (non utilisés si CPU path) ────────────────
    unsigned int m_csRaycast  = 0;
    unsigned int m_csMerge    = 0;
    unsigned int m_wallSSBO   = 0;
    unsigned int m_accumSSBO  = 0;
    unsigned int m_bounceSSBO = 0;

    bool        tryInitGPU(int gridW, int gridH);
    void        shutdownGPU();
    void        computeGPU(const BalconyConfig& cfg, int bounceCount);
    void        uploadWallsGPU(const BalconyConfig& cfg);
    unsigned int compileComputeShader(const char* src);

    // ── CPU fallback ──────────────────────────────────────────
    bool        initCPU(int gridW, int gridH);
    void        computeCPU(const BalconyConfig& cfg, int bounceCount);
    void        buildResultTexture(const BalconyConfig& cfg);

    // Accumulateurs CPU (RGB énergie + heures soleil)
    std::vector<float> m_accumR, m_accumG, m_accumB, m_accumH;
    std::vector<float> m_bounceR, m_bounceG, m_bounceB;
};