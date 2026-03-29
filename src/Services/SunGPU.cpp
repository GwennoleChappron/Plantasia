#include <glad/glad.h>
#include "SunGPU.hpp"
#include "SunCalculator.hpp"
#include <SFML/OpenGL.hpp>
#include <cmath>
#include <iostream>
#include <algorithm>

SunColor SunColor::fromElevation(float elevDeg) {
    float t = std::clamp(elevDeg / 90.0f, 0.0f, 1.0f);
    float golden = std::max(0.0f, 1.0f - t * 4.0f); 
    SunColor c;
    c.r = 1.0f;                                     
    c.g = 0.55f + 0.45f * t - golden * 0.25f;     
    c.b = 0.15f + 0.85f * t - golden * 0.10f;     
    c.r = std::clamp(c.r, 0.f, 1.f);
    c.g = std::clamp(c.g, 0.f, 1.f);
    c.b = std::clamp(c.b, 0.f, 1.f);
    return c;
}

static const char* RAYCAST_CS = R"(
#version 430 core
layout(local_size_x = 8, local_size_y = 8) in;

// Nouveau format étendu (8 floats)
struct WallCell { 
    float wallH; float wallOp; float plantH; float plantOp; 
    float albedo; float refl; float pad1; float pad2; 
};

layout(std430, binding = 0) readonly  buffer WallBuf  { WallCell walls[]; };
layout(std430, binding = 1)           buffer AccumBuf { vec4 accum[]; };   
layout(std430, binding = 2)           buffer BounceBuf{ vec4 bounce[]; };  

uniform int   u_gridW;
uniform int   u_gridH;
uniform vec2  u_sunDir;      
uniform float u_tanElev;     
uniform vec3  u_sunColor;    
uniform float u_hasRoof;     
uniform float u_roofHeight;  
uniform float u_cellM;       
uniform int   u_bounceCount; 
uniform float u_weight;      

const float CELL_M = 0.05;

float traceRay(int tx, int ty, vec2 dir, float tanE) {
    if (u_hasRoof > 0.5) {
        float distCells = u_roofHeight / (tanE * CELL_M);
        vec2 src = vec2(tx, ty) + dir * distCells;
        if (src.x >= 0.0 && src.x < float(u_gridW) && src.y >= 0.0 && src.y < float(u_gridH))
            return 0.0;
    }
    float px = float(tx) + 0.5;
    float py = float(ty) + 0.5;
    float light = 1.0; 

    for (float d = 0.5; d < 120.0; d += 0.25) {
        int cx = int(px + dir.x * d);
        int cy = int(py + dir.y * d);
        if (cx < 0 || cx >= u_gridW || cy < 0 || cy >= u_gridH) break;
        
        int idx = cy * u_gridW + cx;
        float rayZ = tanE * (d * CELL_M);
        WallCell c = walls[idx];

        if (c.wallH > rayZ) light *= (1.0 - c.wallOp);
        if (c.plantH > rayZ) light *= (1.0 - c.plantOp);

        if (light < 0.01) return 0.0;
    }
    return light;
}

void main() {
    int tx = int(gl_GlobalInvocationID.x);
    int ty = int(gl_GlobalInvocationID.y);
    if (tx >= u_gridW || ty >= u_gridH) return;

    int idx = ty * u_gridW + tx;
    
    // CORRECTION : On ne stoppe pas si c'est du verre (refl > 0.01)
    // même si l'opacité est forte, pour permettre au reflet de "partir" de la vitre
    if (walls[idx].wallOp > 0.9 && walls[idx].refl < 0.01) return; 

    // ... (garder le code du spread et des dirs) ...
    float spread = 0.5 * 3.14159265 / 180.0;
    float c = cos(spread), s = sin(spread);
    vec2 dirs[3];
    dirs[0] = vec2(u_sunDir.x*c - u_sunDir.y*s, u_sunDir.x*s + u_sunDir.y*c); 
    dirs[1] = u_sunDir;
    dirs[2] = vec2(u_sunDir.x*c + u_sunDir.y*s, -u_sunDir.x*s + u_sunDir.y*c); 

    float lit = 0.0;
    for (int i = 0; i < 3; i++) lit += traceRay(tx, ty, dirs[i], u_tanElev);
    lit /= 3.0;

    if (lit < 0.01) return;
    
    vec3 energy = u_sunColor * lit * u_weight;
    
    // On n'accumule pas de lumière directe SUR le mur opaque, 
    // mais on calcule ses rebonds
    if (walls[idx].wallOp < 0.9) {
        accum[idx].rgb += energy;
        accum[idx].a   += lit * u_weight;
    }

    if (u_bounceCount <= 0) return;

    float albedo = walls[idx].albedo;
    float refl   = walls[idx].refl;

    // --- REBOND DIFFUS --- (Inchangé)
    if (albedo > 0.01 && walls[idx].wallOp < 0.9) {
        // ... ton code actuel de rebond diffus ...
    }

    // --- REBOND SPÉCULAIRE (Le reflet de la fenêtre) ---
    if (refl > 0.01) {
        // On force la normale vers l'intérieur du balcon (vers le bas sur l'écran si c'est le mur du fond)
        // Si y est positif, le soleil vient du bas, donc la vitre renvoie vers le haut.
        vec2 normal = vec2(0.0, 1.0); 
        if (abs(u_sunDir.x) > abs(u_sunDir.y)) {
             normal = vec2(-sign(u_sunDir.x), 0.0);
        } else {
             normal = vec2(0.0, -sign(u_sunDir.y));
        }

        vec2 refDir = reflect(u_sunDir, normal);
        
        // Fresnel boosté pour la visibilité
        float cosTheta = abs(dot(u_sunDir, normal));
        float fresnel = 0.1 + 0.9 * pow(1.0 - cosTheta, 5.0);
        
        // On multiplie par 15.0 pour que le reflet "perce" l'ombre portée
        vec3 specularEnergy = energy * refl * fresnel * 15.0; 

        float px = float(tx) + 0.5;
        float py = float(ty) + 0.5;

        for (float d = 1.0; d < float(u_bounceCount) * 8.0; d += 0.5) {
            int nx = int(px + refDir.x * d);
            int ny = int(py + refDir.y * d);
            if (nx < 0 || nx >= u_gridW || ny < 0 || ny >= u_gridH) break;
            
            int nidx = ny * u_gridW + nx;
            if (walls[nidx].wallOp > 0.9) break; 
            
            // On ajoute l'énergie du reflet dans le buffer de bounce
            bounce[nidx].rgb += specularEnergy * (1.0 / (1.0 + d * 0.1));
        }
    }
}
)";

static const char* MERGE_CS = R"(
#version 430 core
layout(local_size_x = 8, local_size_y = 8) in;

layout(std430, binding = 1) readonly buffer AccumBuf  { vec4 accum[];  };
layout(std430, binding = 2) readonly buffer BounceBuf { vec4 bounce[]; };
layout(rgba8,  binding = 0) writeonly uniform image2D u_result;

struct WallCell { 
    float wallH; float wallOp; float plantH; float plantOp; 
    float albedo; float refl; float pad1; float pad2; 
};
layout(std430, binding = 0) readonly buffer WallBuf { WallCell walls[]; };

uniform int   u_gridW;
uniform int   u_gridH;
uniform float u_maxHours; 

void main() {
    int tx = int(gl_GlobalInvocationID.x);
    int ty = int(gl_GlobalInvocationID.y);
    if (tx >= u_gridW || ty >= u_gridH) return;
    int idx = ty * u_gridW + tx;

    if (walls[idx].wallOp > 0.9) {
        imageStore(u_result, ivec2(tx, ty), vec4(0.0));
        return;
    }

    vec3 direct = accum[idx].rgb;   
    float hours  = accum[idx].a;    
    vec3 rebound = bounce[idx].rgb; 

    vec3 total = direct + rebound * 0.5;
    float norm = clamp(hours / u_maxHours, 0.0, 1.0);

    vec3 color;
    if (norm < 0.15)      color = mix(vec3(0.05,0.08,0.40), vec3(0.10,0.35,0.80), norm/0.15);
    else if (norm < 0.40) color = mix(vec3(0.10,0.35,0.80), vec3(0.20,0.72,0.30), (norm-0.15)/0.25);
    else if (norm < 0.70) color = mix(vec3(0.20,0.72,0.30), vec3(0.90,0.70,0.10), (norm-0.40)/0.30);
    else if (norm < 0.90) color = mix(vec3(0.90,0.70,0.10), vec3(1.00,0.35,0.05), (norm-0.70)/0.20);
    else                  color = mix(vec3(1.00,0.35,0.05), vec3(1.00,0.10,0.00), (norm-0.90)/0.10);

    if (hours > 0.01 || length(rebound) > 0.01) {
        color *= (total / (hours + length(rebound) + 0.001)) * 1.2; 
    }
    
    color = clamp(color, 0.0, 1.0);
    imageStore(u_result, ivec2(tx, ty), vec4(color, 0.75));
}
)";

GLuint SunGPU::compileComputeShader(const char* src) {
    GLuint shader = glCreateShader(GL_COMPUTE_SHADER);
    glShaderSource(shader, 1, &src, nullptr);
    glCompileShader(shader);

    GLint ok = 0;
    glGetShaderiv(shader, GL_COMPILE_STATUS, &ok);
    if (!ok) {
        char log[2048];
        glGetShaderInfoLog(shader, sizeof(log), nullptr, log);
        std::cerr << "[SunGPU] Compute shader error:\n" << log << "\n";
        glDeleteShader(shader);
        return 0;
    }

    GLuint prog = glCreateProgram();
    glAttachShader(prog, shader);
    glLinkProgram(prog);
    glDeleteShader(shader);
    return prog;
}

bool SunGPU::init(int gridW, int gridH) {
    if (!gladLoadGL()) {
        std::cerr << "[SunGPU] gladLoadGL() failed\n";
        return false;
    }
    m_csRaycast = compileComputeShader(RAYCAST_CS);
    m_csMerge   = compileComputeShader(MERGE_CS);
    if (!m_csRaycast || !m_csMerge) return false;

    m_gridW = gridW;
    m_gridH = gridH;
    int n = gridW * gridH;

    glGenBuffers(1, &m_wallSSBO);
    glBindBuffer(GL_SHADER_STORAGE_BUFFER, m_wallSSBO);
    glBufferData(GL_SHADER_STORAGE_BUFFER, n * sizeof(float) * 8, nullptr, GL_DYNAMIC_DRAW);

    glGenBuffers(1, &m_accumSSBO);
    glBindBuffer(GL_SHADER_STORAGE_BUFFER, m_accumSSBO);
    glBufferData(GL_SHADER_STORAGE_BUFFER, n * sizeof(float) * 4, nullptr, GL_DYNAMIC_COPY);

    glGenBuffers(1, &m_bounceSSBO);
    glBindBuffer(GL_SHADER_STORAGE_BUFFER, m_bounceSSBO);
    glBufferData(GL_SHADER_STORAGE_BUFFER, n * sizeof(float) * 4, nullptr, GL_DYNAMIC_COPY);
    glBindBuffer(GL_SHADER_STORAGE_BUFFER, 0);

    m_resultTex.create(gridW, gridH);
    m_ready = true;
    return true;
}

void SunGPU::resize(int gridW, int gridH) {
    m_ready = false;
    if (m_wallSSBO)   { glDeleteBuffers(1, &m_wallSSBO);   m_wallSSBO   = 0; }
    if (m_accumSSBO)  { glDeleteBuffers(1, &m_accumSSBO);  m_accumSSBO  = 0; }
    if (m_bounceSSBO) { glDeleteBuffers(1, &m_bounceSSBO); m_bounceSSBO = 0; }
    init(gridW, gridH);
}

SunGPU::~SunGPU() {
    if (m_csRaycast)  glDeleteProgram(m_csRaycast);
    if (m_csMerge)    glDeleteProgram(m_csMerge);
    if (m_wallSSBO)   glDeleteBuffers(1, &m_wallSSBO);
    if (m_accumSSBO)  glDeleteBuffers(1, &m_accumSSBO);
    if (m_bounceSSBO) glDeleteBuffers(1, &m_bounceSSBO);
}

void SunGPU::uploadWalls(const BalconyConfig& cfg) {
    struct GPUCell { 
        float wH; float wOp; float pH; float pOp; 
        float albedo; float refl; float pad1; float pad2; 
    };
    std::vector<GPUCell> data(cfg.width * cfg.height);
    
    for (int r = 0; r < cfg.height; ++r) {
        for (int c = 0; c < cfg.width; ++c) {
            const auto& cell = cfg.grid[r][c];
            
            float wOp = 0.0f;
            float albedo = 0.25f; // Sol du balcon par défaut
            float refl = 0.0f;    

            if (cell.type == WallType::WALL) { 
                wOp = 1.0f; 
                albedo = 0.60f; 
            } 
            else if (cell.type == WallType::RAILING) { 
                wOp = 0.4f; 
                albedo = 0.30f; 
            }
            // ---> C'EST ICI QUE LA MAGIE DU VERRE S'ACTIVE ! <---
            else if (cell.type == WallType::GLASS) {
                wOp = 0.15f;    // Laisse passer 85% de la lumière
                albedo = 0.05f; // Très peu de diffusion, c'est du verre
                refl = 0.80f;   // Forte réflexion sous forme de faisceau !
            }
            
            data[r * cfg.width + c] = { 
                cell.wallHeight, wOp, cell.plantHeight, cell.plantOpacity, 
                albedo, refl, 0.f, 0.f 
            };
        }
    }
    glBindBuffer(GL_SHADER_STORAGE_BUFFER, m_wallSSBO);
    glBufferSubData(GL_SHADER_STORAGE_BUFFER, 0, data.size() * sizeof(GPUCell), data.data());
    glBindBuffer(GL_SHADER_STORAGE_BUFFER, 0);
}

void SunGPU::compute(const BalconyConfig& cfg, int bounceCount) {
    if (!m_ready) return;
    uploadWalls(cfg);

    int n = m_gridW * m_gridH;
    std::vector<float> zeros(n * 4, 0.0f);
    glBindBuffer(GL_SHADER_STORAGE_BUFFER, m_accumSSBO);
    glBufferSubData(GL_SHADER_STORAGE_BUFFER, 0, zeros.size()*sizeof(float), zeros.data());
    glBindBuffer(GL_SHADER_STORAGE_BUFFER, m_bounceSSBO);
    glBufferSubData(GL_SHADER_STORAGE_BUFFER, 0, zeros.size()*sizeof(float), zeros.data());

    glUseProgram(m_csRaycast);
    glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 0, m_wallSSBO);
    glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 1, m_accumSSBO);
    glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 2, m_bounceSSBO);

    glUniform1i(glGetUniformLocation(m_csRaycast, "u_gridW"), m_gridW);
    glUniform1i(glGetUniformLocation(m_csRaycast, "u_gridH"), m_gridH);
    glUniform1f(glGetUniformLocation(m_csRaycast, "u_hasRoof"), cfg.hasRoof ? 1.0f : 0.0f);
    glUniform1f(glGetUniformLocation(m_csRaycast, "u_roofHeight"), cfg.roofHeight);
    glUniform1f(glGetUniformLocation(m_csRaycast, "u_cellM"), 0.05f);
    glUniform1i(glGetUniformLocation(m_csRaycast, "u_bounceCount"), bounceCount);
    glUniform1f(glGetUniformLocation(m_csRaycast, "u_weight"), 0.5f);

    int workGroupX = (m_gridW + 7) / 8;
    int workGroupY = (m_gridH + 7) / 8;

    for (float h = 6.0f; h <= 20.0f; h += 0.5f) {
        BalconyConfig passCfg = cfg;
        passCfg.hour = h;
        SunPosition sun = SunCalculator::compute(passCfg);
        if (!sun.isAboveHorizon()) continue;

        float bAz = static_cast<float>(cfg.orientation) * 45.0f;
        float relAz = (sun.azimuth - bAz) * 3.14159265f / 180.0f;
        sf::Vector2f sunDir(std::sin(relAz), std::cos(relAz));
        float tanElev = std::tan(sun.elevation * 3.14159265f / 180.0f);
        SunColor sColor = SunColor::fromElevation(sun.elevation);

        glUniform2f(glGetUniformLocation(m_csRaycast, "u_sunDir"), sunDir.x, sunDir.y);
        glUniform1f(glGetUniformLocation(m_csRaycast, "u_tanElev"), tanElev);
        glUniform3f(glGetUniformLocation(m_csRaycast, "u_sunColor"), sColor.r, sColor.g, sColor.b);

        glDispatchCompute(workGroupX, workGroupY, 1);
        glMemoryBarrier(GL_SHADER_STORAGE_BARRIER_BIT);
    }

    glUseProgram(m_csMerge);
    glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 0, m_wallSSBO);
    glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 1, m_accumSSBO);
    glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 2, m_bounceSSBO);

    glBindImageTexture(0, static_cast<GLuint>(m_resultTex.getNativeHandle()), 0, GL_FALSE, 0, GL_WRITE_ONLY, GL_RGBA8);

    glUniform1i(glGetUniformLocation(m_csMerge, "u_gridW"), m_gridW);
    glUniform1i(glGetUniformLocation(m_csMerge, "u_gridH"), m_gridH);
    glUniform1f(glGetUniformLocation(m_csMerge, "u_maxHours"), 7.0f);

    glDispatchCompute(workGroupX, workGroupY, 1);
    glMemoryBarrier(GL_SHADER_IMAGE_ACCESS_BARRIER_BIT);
}

void SunGPU::downloadResult() {}