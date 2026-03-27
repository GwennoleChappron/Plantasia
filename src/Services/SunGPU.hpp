#pragma once
#include <glad/glad.h>          // DOIT être avant tout OpenGL/SFML
#include <string>
#include <vector>
#include "Data/BalconyConfig.hpp"
#include "Services/SunCalculator.hpp"
#include <SFML/Graphics.hpp>

struct SunColor {
    float r, g, b;
    static SunColor fromElevation(float elevDeg);
};

class SunGPU {
public:
    SunGPU()  = default;
    ~SunGPU();

    bool init(int gridW, int gridH);
    void resize(int gridW, int gridH);
    void compute(const BalconyConfig& cfg, int bounceCount = 2);

    const sf::Texture& getResultTexture() const { return m_resultTex; }
    SunColor getCurrentSunColor()         const { return m_currentSunColor; }
    bool isReady()                        const { return m_ready; }

private:
    bool   m_ready = false;
    int    m_gridW = 0, m_gridH = 0;

    GLuint m_csRaycast  = 0;
    GLuint m_csMerge    = 0;
    GLuint m_wallSSBO   = 0;
    GLuint m_accumSSBO  = 0;
    GLuint m_bounceSSBO = 0;
    GLuint m_resultTex2D = 0; 

    sf::Texture  m_resultTex; 
    SunColor     m_currentSunColor = {1.f, 1.f, 1.f};

    GLuint compileComputeShader(const char* src);
    void   uploadWalls(const BalconyConfig& cfg);
    void   downloadResult(); 
};