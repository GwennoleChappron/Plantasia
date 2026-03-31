#include "SunGPU.hpp"
#include "SunCalculator.hpp"
#include <cmath>
#include <iostream>
#include <algorithm>
#include <vector>

// ─── GLAD isolé : inclus uniquement si compute shaders dispo ─
// On détecte à la compilation via CMake qui définit SUNGPU_HAS_COMPUTE
// si la plateforme est Windows ou Linux (pas macOS).
#ifdef SUNGPU_HAS_COMPUTE
  #include <glad/glad.h>
#endif

// ─────────────────────────────────────────────────────────────
//  SunColor
// ─────────────────────────────────────────────────────────────
SunColor SunColor::fromElevation(float elevDeg) {
    float t      = std::clamp(elevDeg / 90.f, 0.f, 1.f);
    float golden = std::max(0.f, 1.f - t * 4.f);
    return {
        1.f,
        std::clamp(0.55f + 0.45f*t - golden*0.25f, 0.f, 1.f),
        std::clamp(0.15f + 0.85f*t - golden*0.10f, 0.f, 1.f)
    };
}

// ─────────────────────────────────────────────────────────────
//  Shaders compute (compilés seulement si SUNGPU_HAS_COMPUTE)
// ─────────────────────────────────────────────────────────────
#ifdef SUNGPU_HAS_COMPUTE

static const char* RAYCAST_CS = R"(
#version 430 core
layout(local_size_x = 8, local_size_y = 8) in;

struct WallCell {
    float wallH; float wallOp; float plantH; float plantOp;
    float albedo; float refl;  float pad1;   float pad2;
};

layout(std430, binding = 0) readonly buffer WallBuf  { WallCell walls[];  };
layout(std430, binding = 1)          buffer AccumBuf { vec4     accum[];  };
layout(std430, binding = 2)          buffer BounceBuf{ vec4     bounce[]; };

uniform int   u_gridW, u_gridH;
uniform vec2  u_sunDir;
uniform float u_tanElev, u_hasRoof, u_roofHeight, u_cellM, u_weight;
uniform vec3  u_sunColor;
uniform int   u_bounceCount;

const float CELL_M = 0.05;

float traceRay(int tx, int ty, vec2 dir) {
    if (u_hasRoof > 0.5) {
        float dc = u_roofHeight / (u_tanElev * CELL_M);
        vec2 src = vec2(tx,ty) + dir*dc;
        if (src.x>=0&&src.x<u_gridW&&src.y>=0&&src.y<u_gridH) return 0.0;
    }
    float px=tx+.5, py=ty+.5, light=1.0;
    for (float d=0.5; d<120.0; d+=0.25) {
        int cx=int(px+dir.x*d), cy=int(py+dir.y*d);
        if (cx<0||cx>=u_gridW||cy<0||cy>=u_gridH) break;
        WallCell c=walls[cy*u_gridW+cx];
        float rayZ=u_tanElev*(d*CELL_M);
        if (c.wallH>rayZ) { if(c.wallOp>0.9) return 0.0; light*=(1.0-c.wallOp); }
        if (c.plantH>rayZ) light*=(1.0-c.plantOp);
        if (light<0.01) return 0.0;
    }
    return light;
}

void main() {
    int tx=int(gl_GlobalInvocationID.x), ty=int(gl_GlobalInvocationID.y);
    if (tx>=u_gridW||ty>=u_gridH) return;
    int idx=ty*u_gridW+tx;
    if (walls[idx].wallOp>0.9&&walls[idx].refl<0.01) return;

    float sp=0.5*3.14159265/180.0, cs=cos(sp), sn=sin(sp);
    vec2 d0=vec2(u_sunDir.x*cs-u_sunDir.y*sn, u_sunDir.x*sn+u_sunDir.y*cs);
    vec2 d2=vec2(u_sunDir.x*cs+u_sunDir.y*sn,-u_sunDir.x*sn+u_sunDir.y*cs);
    float lit=(traceRay(tx,ty,d0)+traceRay(tx,ty,u_sunDir)+traceRay(tx,ty,d2))/3.0;
    if (lit<0.01) return;

    vec3 energy=u_sunColor*lit*u_weight;
    accum[idx].rgb+=energy;
    accum[idx].a  +=lit*u_weight;

    int R=u_bounceCount*3+2;
    float albedo=walls[idx].albedo;
    for (int dy=-R;dy<=R;dy++) for (int dx=-R;dx<=R;dx++) {
        if (dx==0&&dy==0) continue;
        int nx=tx+dx, ny=ty+dy;
        if (nx<0||nx>=u_gridW||ny<0||ny>=u_gridH) continue;
        if (walls[ny*u_gridW+nx].wallOp>0.9) continue;
        float d2f=float(dx*dx+dy*dy);
        float fall=1.0/(1.0+d2f*0.5);
        bool blocked=false;
        float steps=max(abs(float(dx)),abs(float(dy)))*2.0+1.0;
        for (float t=1.0/steps;t<1.0;t+=1.0/steps) {
            int cx=int(float(tx)+dx*t), cy=int(float(ty)+dy*t);
            if (cx>=0&&cx<u_gridW&&cy>=0&&cy<u_gridH)
                if (walls[cy*u_gridW+cx].wallOp>0.9) { blocked=true; break; }
        }
        if (!blocked) bounce[ny*u_gridW+nx].rgb+=energy*albedo*fall;
    }
}
)";

static const char* MERGE_CS = R"(
#version 430 core
layout(local_size_x = 8, local_size_y = 8) in;

struct WallCell { float wallH,wallOp,plantH,plantOp,albedo,refl,pad1,pad2; };
layout(std430, binding=0) readonly buffer WallBuf  { WallCell walls[];  };
layout(std430, binding=1) readonly buffer AccumBuf { vec4     accum[];  };
layout(std430, binding=2) readonly buffer BounceBuf{ vec4     bounce[]; };
layout(rgba8,  binding=0) writeonly uniform image2D u_result;

uniform int u_gridW, u_gridH;
uniform float u_maxHours;

void main() {
    int tx=int(gl_GlobalInvocationID.x), ty=int(gl_GlobalInvocationID.y);
    if (tx>=u_gridW||ty>=u_gridH) return;
    int idx=ty*u_gridW+tx;
    if (walls[idx].wallOp>0.9) { imageStore(u_result,ivec2(tx,ty),vec4(0)); return; }

    float hours=accum[idx].a;
    vec3 direct=accum[idx].rgb, rebound=bounce[idx].rgb;
    vec3 total=direct+rebound*0.4;
    float norm=clamp(hours/u_maxHours,0.0,1.0);

    vec3 col;
    if      (norm<0.15) col=mix(vec3(.05,.08,.40),vec3(.10,.35,.80),norm/.15);
    else if (norm<0.40) col=mix(vec3(.10,.35,.80),vec3(.20,.72,.30),(norm-.15)/.25);
    else if (norm<0.70) col=mix(vec3(.20,.72,.30),vec3(.90,.70,.10),(norm-.40)/.30);
    else if (norm<0.90) col=mix(vec3(.90,.70,.10),vec3(1.00,.35,.05),(norm-.70)/.20);
    else                col=mix(vec3(1.00,.35,.05),vec3(1.00,.10,.00),(norm-.90)/.10);

    if (hours>0.01) col*=(direct/(hours+0.001))*1.2;
    col=clamp(col,0.0,1.0);
    imageStore(u_result,ivec2(tx,ty),vec4(col,0.78));
}
)";

#endif // SUNGPU_HAS_COMPUTE

// ─────────────────────────────────────────────────────────────
//  Init / Resize / Destructor
// ─────────────────────────────────────────────────────────────
bool SunGPU::init(int gridW, int gridH) {
    m_gridW = gridW; m_gridH = gridH;

#ifdef SUNGPU_HAS_COMPUTE
    if (tryInitGPU(gridW, gridH)) {
        std::cout << "[SunGPU] Backend: GPU (OpenGL 4.3 compute)\n";
        m_gpuAvailable = true;
        m_ready = true;
        return true;
    }
    std::cout << "[SunGPU] Compute shaders indisponibles, fallback CPU\n";
#else
    std::cout << "[SunGPU] Backend: CPU (plateforme sans compute shaders)\n";
#endif

    m_gpuAvailable = false;
    m_ready = initCPU(gridW, gridH);
    return m_ready;
}

void SunGPU::resize(int gridW, int gridH) {
    m_ready = false;
    if (m_gpuAvailable) shutdownGPU();
    init(gridW, gridH);
}

SunGPU::~SunGPU() {
    if (m_gpuAvailable) shutdownGPU();
}

// ─────────────────────────────────────────────────────────────
//  compute() — dispatch vers GPU ou CPU
// ─────────────────────────────────────────────────────────────
void SunGPU::compute(const BalconyConfig& cfg, int bounceCount) {
    if (!m_ready) return;
    if (m_gpuAvailable)
        computeGPU(cfg, bounceCount);
    else
        computeCPU(cfg, bounceCount);

    // Couleur courante du soleil (heure actuelle, pour les ombres live)
    SunPosition now = SunCalculator::compute(cfg);
    m_currentSunColor = now.isAboveHorizon()
        ? SunColor::fromElevation(now.elevation)
        : SunColor{0.05f, 0.05f, 0.15f};
}

// ─────────────────────────────────────────────────────────────
//  GPU PATH
// ─────────────────────────────────────────────────────────────
#ifdef SUNGPU_HAS_COMPUTE

unsigned int SunGPU::compileComputeShader(const char* src) {
    GLuint sh = glCreateShader(GL_COMPUTE_SHADER);
    glShaderSource(sh, 1, &src, nullptr);
    glCompileShader(sh);
    GLint ok = 0; glGetShaderiv(sh, GL_COMPILE_STATUS, &ok);
    if (!ok) {
        char log[2048]; glGetShaderInfoLog(sh, sizeof(log), nullptr, log);
        std::cerr << "[SunGPU] Shader error:\n" << log << "\n";
        glDeleteShader(sh); return 0;
    }
    GLuint prog = glCreateProgram();
    glAttachShader(prog, sh); glLinkProgram(prog); glDeleteShader(sh);
    glGetProgramiv(prog, GL_LINK_STATUS, &ok);
    if (!ok) {
        char log[2048]; glGetProgramInfoLog(prog, sizeof(log), nullptr, log);
        std::cerr << "[SunGPU] Link error:\n" << log << "\n";
        glDeleteProgram(prog); return 0;
    }
    return prog;
}

bool SunGPU::tryInitGPU(int gridW, int gridH) {
    if (!gladLoadGL()) return false;

    GLint major=0, minor=0;
    glGetIntegerv(GL_MAJOR_VERSION, &major);
    glGetIntegerv(GL_MINOR_VERSION, &minor);
    if (major < 4 || (major==4 && minor<3)) return false;

    // Vérifie que GL_COMPUTE_SHADER existe vraiment
    GLint numExt = 0;
    glGetIntegerv(GL_NUM_EXTENSIONS, &numExt);
    bool hasCompute = (major > 4 || (major==4 && minor>=3));
    if (!hasCompute) return false;

    m_csRaycast = compileComputeShader(RAYCAST_CS);
    m_csMerge   = compileComputeShader(MERGE_CS);
    if (!m_csRaycast || !m_csMerge) return false;

    int n = gridW * gridH;
    glGenBuffers(1, &m_wallSSBO);
    glBindBuffer(GL_SHADER_STORAGE_BUFFER, m_wallSSBO);
    glBufferData(GL_SHADER_STORAGE_BUFFER, n * 8*sizeof(float), nullptr, GL_DYNAMIC_DRAW);

    glGenBuffers(1, &m_accumSSBO);
    glBindBuffer(GL_SHADER_STORAGE_BUFFER, m_accumSSBO);
    glBufferData(GL_SHADER_STORAGE_BUFFER, n * 4*sizeof(float), nullptr, GL_DYNAMIC_COPY);

    glGenBuffers(1, &m_bounceSSBO);
    glBindBuffer(GL_SHADER_STORAGE_BUFFER, m_bounceSSBO);
    glBufferData(GL_SHADER_STORAGE_BUFFER, n * 4*sizeof(float), nullptr, GL_DYNAMIC_COPY);
    glBindBuffer(GL_SHADER_STORAGE_BUFFER, 0);

    m_resultTex.create(gridW, gridH);
    return true;
}

void SunGPU::shutdownGPU() {
    if (m_csRaycast)  { glDeleteProgram(m_csRaycast);   m_csRaycast  = 0; }
    if (m_csMerge)    { glDeleteProgram(m_csMerge);     m_csMerge    = 0; }
    if (m_wallSSBO)   { glDeleteBuffers(1,&m_wallSSBO);  m_wallSSBO   = 0; }
    if (m_accumSSBO)  { glDeleteBuffers(1,&m_accumSSBO); m_accumSSBO  = 0; }
    if (m_bounceSSBO) { glDeleteBuffers(1,&m_bounceSSBO);m_bounceSSBO = 0; }
}

void SunGPU::uploadWallsGPU(const BalconyConfig& cfg) {
    struct GPUCell { float wH,wOp,pH,pOp,albedo,refl,p1,p2; };
    std::vector<GPUCell> data(cfg.width * cfg.height);
    for (int r=0; r<cfg.height; ++r)
        for (int c=0; c<cfg.width; ++c) {
            const auto& cell = cfg.grid[r][c];
            float wOp=0,albedo=0.25f,refl=0;
            if      (cell.type==WallType::WALL)    { wOp=1.f; albedo=0.60f; }
            else if (cell.type==WallType::RAILING)  { wOp=0.4f; albedo=0.30f; }
            else if (cell.type==WallType::GLASS)    { wOp=0.15f; albedo=0.05f; refl=0.80f; }
            data[r*cfg.width+c] = {cell.wallHeight,wOp,cell.plantHeight,cell.plantOpacity,albedo,refl,0,0};
        }
    glBindBuffer(GL_SHADER_STORAGE_BUFFER, m_wallSSBO);
    glBufferSubData(GL_SHADER_STORAGE_BUFFER, 0, data.size()*8*sizeof(float), data.data());
    glBindBuffer(GL_SHADER_STORAGE_BUFFER, 0);
}

void SunGPU::computeGPU(const BalconyConfig& cfg, int bounceCount) {
    uploadWallsGPU(cfg);

    int n = m_gridW * m_gridH;
    std::vector<float> zeros(n*4, 0.f);
    glBindBuffer(GL_SHADER_STORAGE_BUFFER, m_accumSSBO);
    glBufferSubData(GL_SHADER_STORAGE_BUFFER, 0, n*16, zeros.data());
    glBindBuffer(GL_SHADER_STORAGE_BUFFER, m_bounceSSBO);
    glBufferSubData(GL_SHADER_STORAGE_BUFFER, 0, n*16, zeros.data());

    glUseProgram(m_csRaycast);
    glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 0, m_wallSSBO);
    glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 1, m_accumSSBO);
    glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 2, m_bounceSSBO);
    glUniform1i(glGetUniformLocation(m_csRaycast,"u_gridW"),  m_gridW);
    glUniform1i(glGetUniformLocation(m_csRaycast,"u_gridH"),  m_gridH);
    glUniform1f(glGetUniformLocation(m_csRaycast,"u_hasRoof"),cfg.hasRoof?1.f:0.f);
    glUniform1f(glGetUniformLocation(m_csRaycast,"u_roofHeight"),cfg.roofHeight);
    glUniform1f(glGetUniformLocation(m_csRaycast,"u_cellM"),  0.05f);
    glUniform1i(glGetUniformLocation(m_csRaycast,"u_bounceCount"),bounceCount);
    glUniform1f(glGetUniformLocation(m_csRaycast,"u_weight"), 0.5f);

    int gx=(m_gridW+7)/8, gy=(m_gridH+7)/8;
    for (float h=6.f; h<=20.f; h+=0.5f) {
        BalconyConfig tmp=cfg; tmp.hour=h;
        SunPosition sun=SunCalculator::compute(tmp);
        if (!sun.isAboveHorizon()) continue;
        float bAz=(float)cfg.orientation*45.f;
        float relAz=(sun.azimuth-bAz)*3.14159265f/180.f;
        SunColor sc=SunColor::fromElevation(sun.elevation);
        glUniform2f(glGetUniformLocation(m_csRaycast,"u_sunDir"),  std::sin(relAz), std::cos(relAz));
        glUniform1f(glGetUniformLocation(m_csRaycast,"u_tanElev"), std::tan(sun.elevation*3.14159265f/180.f));
        glUniform3f(glGetUniformLocation(m_csRaycast,"u_sunColor"),sc.r, sc.g, sc.b);
        glDispatchCompute(gx, gy, 1);
        glMemoryBarrier(GL_SHADER_STORAGE_BARRIER_BIT);
    }

    glUseProgram(m_csMerge);
    glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 0, m_wallSSBO);
    glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 1, m_accumSSBO);
    glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 2, m_bounceSSBO);
    glBindImageTexture(0, (GLuint)m_resultTex.getNativeHandle(), 0, GL_FALSE, 0, GL_WRITE_ONLY, GL_RGBA8);
    glUniform1i(glGetUniformLocation(m_csMerge,"u_gridW"), m_gridW);
    glUniform1i(glGetUniformLocation(m_csMerge,"u_gridH"), m_gridH);
    glUniform1f(glGetUniformLocation(m_csMerge,"u_maxHours"), 7.f);
    glDispatchCompute(gx, gy, 1);
    glMemoryBarrier(GL_SHADER_IMAGE_ACCESS_BARRIER_BIT);
}

#else
// Stubs vides si pas de compute shaders
unsigned int SunGPU::compileComputeShader(const char*) { return 0; }
bool SunGPU::tryInitGPU(int,int)                       { return false; }
void SunGPU::shutdownGPU()                             {}
void SunGPU::uploadWallsGPU(const BalconyConfig&)      {}
void SunGPU::computeGPU(const BalconyConfig&, int)     {}
#endif

// ─────────────────────────────────────────────────────────────
//  CPU FALLBACK — même qualité, multi-pass, palette identique
// ─────────────────────────────────────────────────────────────
bool SunGPU::initCPU(int gridW, int gridH) {
    int n = gridW * gridH;
    m_accumR .assign(n, 0.f); m_accumG .assign(n, 0.f);
    m_accumB .assign(n, 0.f); m_accumH .assign(n, 0.f);
    m_bounceR.assign(n, 0.f); m_bounceG.assign(n, 0.f);
    m_bounceB.assign(n, 0.f);
    m_resultTex.create(gridW, gridH);
    return true;
}

void SunGPU::computeCPU(const BalconyConfig& cfg, int bounceCount) {
    int n = m_gridW * m_gridH;
    std::fill(m_accumR.begin(), m_accumR.end(), 0.f);
    std::fill(m_accumG.begin(), m_accumG.end(), 0.f);
    std::fill(m_accumB.begin(), m_accumB.end(), 0.f);
    std::fill(m_accumH.begin(), m_accumH.end(), 0.f);
    std::fill(m_bounceR.begin(),m_bounceR.end(),0.f);
    std::fill(m_bounceG.begin(),m_bounceG.end(),0.f);
    std::fill(m_bounceB.begin(),m_bounceB.end(),0.f);

    const float STEP = 0.5f;
    const int   bounceR = bounceCount * 3 + 2;

    for (float h = 6.f; h <= 20.f; h += STEP) {
        BalconyConfig tmp = cfg; tmp.hour = h;
        SunPosition sun = SunCalculator::compute(tmp);
        if (!sun.isAboveHorizon()) continue;

        SunColor sc = SunColor::fromElevation(sun.elevation);

        for (int r = 0; r < m_gridH; ++r) {
            for (int c = 0; c < m_gridW; ++c) {
                const auto& cell = cfg.grid[r][c];
                // Mur opaque = pas de calcul
                if (cell.type == WallType::WALL && cell.plantHeight <= 0.f) continue;

                float cellLit = SunCalculator::softShadow(cfg, c, r, sun);
                if (cellLit < 0.01f) continue;

                int idx = r * m_gridW + c;
                float w = cellLit * STEP;
                m_accumR[idx] += sc.r * w;
                m_accumG[idx] += sc.g * w;
                m_accumB[idx] += sc.b * w;
                m_accumH[idx] += w;

                // Propagation rebonds (albédo simplifié)
                float albedo = (cell.type == WallType::WALL)    ? 0.60f :
                               (cell.type == WallType::RAILING) ? 0.30f : 0.25f;
                float bounceE_r = sc.r * w * albedo;
                float bounceE_g = sc.g * w * albedo;
                float bounceE_b = sc.b * w * albedo;

                for (int dy = -bounceR; dy <= bounceR; ++dy) {
                    for (int dx = -bounceR; dx <= bounceR; ++dx) {
                        if (dx==0 && dy==0) continue;
                        int nx = c+dx, ny = r+dy;
                        if (nx<0||nx>=m_gridW||ny<0||ny>=m_gridH) continue;
                        const auto& nc = cfg.grid[ny][nx];
                        if (nc.type==WallType::WALL) continue;
                        float d2 = (float)(dx*dx + dy*dy);
                        float fall = 1.f / (1.f + d2 * 0.5f);
                        int nidx = ny * m_gridW + nx;
                        m_bounceR[nidx] += bounceE_r * fall;
                        m_bounceG[nidx] += bounceE_g * fall;
                        m_bounceB[nidx] += bounceE_b * fall;
                    }
                }
            }
        }
    }

    buildResultTexture(cfg);
}

void SunGPU::buildResultTexture(const BalconyConfig& cfg) {
    std::vector<sf::Uint8> pixels(m_gridW * m_gridH * 4, 0);
    const float maxH = 7.f;

    for (int r = 0; r < m_gridH; ++r) {
        for (int c = 0; c < m_gridW; ++c) {
            int idx = r * m_gridW + c;
            const auto& cell = cfg.grid[r][c];

            // Mur opaque → transparent
            if (cell.type == WallType::WALL) {
                pixels[idx*4+3] = 0; continue;
            }

            float hours  = m_accumH[idx];
            float norm   = std::clamp(hours / maxH, 0.f, 1.f);

            // Palette identique au shader MERGE
            float pr, pg, pb;
            if      (norm < 0.15f) { float t=norm/.15f;      pr=.05f+(t*.05f); pg=.08f+(t*.27f); pb=.40f+(t*.40f); }
            else if (norm < 0.40f) { float t=(norm-.15f)/.25f;pr=.10f+(t*.10f); pg=.35f+(t*.37f); pb=.80f-(t*.50f); }
            else if (norm < 0.70f) { float t=(norm-.40f)/.30f;pr=.20f+(t*.70f); pg=.72f-(t*.02f); pb=.30f-(t*.20f); }
            else if (norm < 0.90f) { float t=(norm-.70f)/.20f;pr=.90f+(t*.10f); pg=.70f-(t*.35f); pb=.10f-(t*.05f); }
            else                   { float t=(norm-.90f)/.10f;pr=1.0f;          pg=.35f-(t*.25f); pb=.05f-(t*.05f); }

            // Teinte par couleur soleil accumulée
            if (hours > 0.01f) {
                float avgR = m_accumR[idx] / (hours + 0.001f);
                float avgG = m_accumG[idx] / (hours + 0.001f);
                float avgB = m_accumB[idx] / (hours + 0.001f);
                pr = std::clamp(pr * avgR * 1.2f, 0.f, 1.f);
                pg = std::clamp(pg * avgG * 1.2f, 0.f, 1.f);
                pb = std::clamp(pb * avgB * 1.2f, 0.f, 1.f);
            }

            // Ajout rebonds
            float br = std::clamp(m_bounceR[idx]*0.4f, 0.f, 0.4f);
            float bg = std::clamp(m_bounceG[idx]*0.4f, 0.f, 0.4f);
            float bb = std::clamp(m_bounceB[idx]*0.4f, 0.f, 0.4f);
            pr = std::clamp(pr + br, 0.f, 1.f);
            pg = std::clamp(pg + bg, 0.f, 1.f);
            pb = std::clamp(pb + bb, 0.f, 1.f);

            pixels[idx*4+0] = (sf::Uint8)(pr * 255.f);
            pixels[idx*4+1] = (sf::Uint8)(pg * 255.f);
            pixels[idx*4+2] = (sf::Uint8)(pb * 255.f);
            pixels[idx*4+3] = hours > 0.05f ? 200 : 0;
        }
    }
    m_resultTex.update(pixels.data());
}