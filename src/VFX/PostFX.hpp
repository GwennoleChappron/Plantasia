#pragma once
#include <SFML/Graphics.hpp>
#include <string>

// PostFX : vignette + color grading appliqués sur une RenderTexture intermédiaire.
//
// Usage dans BackgroundEngine :
//   1. Rendre toute la scène dans m_sceneTexture (au lieu de direct sur target)
//   2. Appeler postFX.apply(m_sceneTexture, target)
//
// Le shader combine en une seule passe :
//   - Vignette (assombrit les bords)
//   - Color grading (teinte chaude/froide selon le moment de la journée)
//   - Légère distorsion organique (optionnelle)

class PostFX {
private:
    sf::Shader        m_shader;
    sf::RenderTexture m_sceneTexture; // scène rendue avant post-process
    bool              m_available  = false;

    // Paramètres exposés
    float m_vignetteStrength = 0.45f;
    float m_vignetteRadius   = 0.75f;

    // Color grading : teinte RGB multipliée sur la scène
    sf::Glsl::Vec3 m_colorGrade = {1.f, 1.f, 1.f}; // neutre par défaut

    float m_time = 0.f; // pour la distorsion

    static const char* s_fragSrc;

public:
    PostFX() = default;

    bool init(unsigned int width, unsigned int height) {
        if (!sf::Shader::isAvailable()) return false;
        if (!m_shader.loadFromMemory(s_fragSrc, sf::Shader::Fragment)) return false;
        if (!m_sceneTexture.create(width, height)) return false;
        m_available = true;
        return true;
    }

    bool isAvailable() const { return m_available; }

    // Retourne la RenderTexture où dessiner la scène
    sf::RenderTexture& getSceneTarget() { return m_sceneTexture; }

    // Applique les effets et blit sur target final
    void apply(sf::RenderTarget& finalTarget) {
        if (!m_available) return;

        m_sceneTexture.display();

        m_shader.setUniform("u_texture",         sf::Shader::CurrentTexture);
        m_shader.setUniform("u_resolution",      sf::Glsl::Vec2(
            (float)m_sceneTexture.getSize().x,
            (float)m_sceneTexture.getSize().y));
        m_shader.setUniform("u_vignetteStrength",m_vignetteStrength);
        m_shader.setUniform("u_vignetteRadius",  m_vignetteRadius);
        m_shader.setUniform("u_colorGrade",      m_colorGrade);
        m_shader.setUniform("u_time",            m_time);

        sf::Sprite scene(m_sceneTexture.getTexture());
        sf::RenderStates st;
        st.shader = &m_shader;
        finalTarget.draw(scene, st);
    }

    void update(float dt) { m_time += dt; }

    // Setters
    void setVignette(float strength, float radius = 0.75f) {
        m_vignetteStrength = strength;
        m_vignetteRadius   = radius;
    }

    // Color grading prédéfini
    void setGradeNeutral()  { m_colorGrade = {1.00f, 1.00f, 1.00f}; }
    void setGradeWarm()     { m_colorGrade = {1.12f, 0.95f, 0.80f}; }
    void setGradeCool()     { m_colorGrade = {0.85f, 0.95f, 1.15f}; }
    void setGradeNight()    { m_colorGrade = {0.60f, 0.70f, 0.90f}; }
    void setGradeSunset()   { m_colorGrade = {1.20f, 0.80f, 0.70f}; }
    void setGradeDream()    { m_colorGrade = {0.90f, 0.85f, 1.20f}; }
    void setGradeCustom(float r, float g, float b) { m_colorGrade = {r, g, b}; }

    float getVignetteStrength() const { return m_vignetteStrength; }
    float getVignetteRadius()   const { return m_vignetteRadius; }
    sf::Glsl::Vec3 getColorGrade() const { return m_colorGrade; }
};

// ── Shader GLSL ─────────────────────────────────────────────────────────────
inline const char* PostFX::s_fragSrc = R"(
uniform sampler2D  u_texture;
uniform vec2       u_resolution;
uniform float      u_vignetteStrength;  // 0 = aucune, 1 = très forte
uniform float      u_vignetteRadius;    // 0.5 = compact, 1.0 = léger
uniform vec3       u_colorGrade;        // multiplicateur RGB
uniform float      u_time;

void main() {
    vec2 uv = gl_TexCoord[0].xy;

    // Légère distorsion organique (imperceptible, ~1px)
    float distort = sin(uv.y * 80.0 + u_time * 0.3) * 0.0008
                  + sin(uv.x * 60.0 + u_time * 0.2) * 0.0006;
    vec2 uvD = uv + vec2(distort, distort * 0.5);

    vec4 color = texture2D(u_texture, uvD);

    // Color grading
    color.rgb *= u_colorGrade;

    // Vignette (ellipse centrée)
    vec2 center = uv - vec2(0.5);
    center.x   *= u_resolution.x / u_resolution.y; // correction aspect ratio
    float dist  = length(center);
    float vign  = smoothstep(u_vignetteRadius, u_vignetteRadius - 0.35, dist);
    float dark  = mix(1.0 - u_vignetteStrength, 1.0, vign);
    color.rgb  *= dark;

    // Clamp propre
    color.rgb = clamp(color.rgb, 0.0, 1.0);

    gl_FragColor = color * gl_Color;
}
)";
