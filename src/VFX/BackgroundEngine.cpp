#include "BackgroundEngine.hpp"
#include <cmath>
#include <imgui.h>

// 🔥 SHADER PLUS DOUX ET PLUS LARGE
const std::string blurShaderCode = R"(
uniform sampler2D u_texture;
uniform vec2 resolution;
uniform float blurRadius;
uniform float threshold;
uniform float softness;

void main() {
    vec2 uv = gl_TexCoord[0].xy;
    vec2 off = blurRadius / resolution;

    vec4 sum = vec4(0.0);
    float total = 0.0;

    for(int x = -4; x <= 4; x++) {
        for(int y = -4; y <= 4; y++) {

            vec2 offset = vec2(float(x), float(y)) * off;
            vec4 sample = texture2D(u_texture, uv + offset);

            float brightness = dot(sample.rgb, vec3(0.299, 0.587, 0.114));
            float glow = smoothstep(threshold, threshold + softness, brightness);

            float weight = exp(-(x*x + y*y) / 12.0);

            sum += sample * glow * weight;
            total += weight;
        }
    }

    vec4 blurred = sum / total;
    gl_FragColor = blurred * gl_Color;
}
)";

BackgroundEngine::BackgroundEngine(float width, float height) 
    : m_swarm(60, width, height), m_vines(width, height), m_grass(width, height) 
{
    for (int i = 0; i < 80; i++) {
        Luciole l;
        l.basePos = sf::Vector2f(std::fmod(i * 137.5f, width), std::fmod(i * 93.1f, height));
        l.offset = (float)i;
        l.speed = 5.f + (i % 8);
        l.radius = 2.0f + (i % 4) * 0.8f;
        m_lucioles.push_back(l);
    }

    if (sf::Shader::isAvailable()) {
        if (m_blurShader.loadFromMemory(blurShaderCode, sf::Shader::Fragment)) {
            m_shaderActif = true;
            m_glowTexture.create((unsigned int)width, (unsigned int)height);
            m_glowSprite.setTexture(m_glowTexture.getTexture());
        }
    }
}

void BackgroundEngine::update(float dt, sf::Vector2i mousePos, float width, float height) {
    float scaledDt = dt * m_timeScale; 
    m_temps += scaledDt;
    
    if (m_enableBoids) m_swarm.update(scaledDt, sf::Vector2f((float)mousePos.x, (float)mousePos.y), width, height);
    if (m_enableVines) m_vines.update(scaledDt, mousePos, width, height);
    if (m_enableGrass) m_grass.update(scaledDt, mousePos);
}

void BackgroundEngine::draw(sf::RenderTarget& target) {
    if (m_enableBoids) m_swarm.draw(target);
    if (m_enableVines) m_vines.draw(target);
    if (m_enableGrass) m_grass.draw(target);

    if (!m_enableLucioles) return;

    sf::CircleShape core;
    sf::CircleShape glow;

    if (m_shaderActif)
        m_glowTexture.clear(sf::Color::Transparent);

    for (const auto& l : m_lucioles) {

        float x = l.basePos.x + std::sin(m_temps * 0.4f + l.offset) * 50.f;
        float y = l.basePos.y - std::fmod(m_temps * l.speed, target.getSize().y);
        if (y < 0) y += target.getSize().y;

        float pulse = (std::sin(m_temps * 2.0f + l.offset) * 0.5f + 0.5f);
        float alpha = pulse * 255.f * m_lucioleIntensity;

        // 🔥 CORE (NET)
        core.setRadius(l.radius);
        core.setOrigin(l.radius, l.radius);
        core.setPosition(x, y);
        core.setFillColor(sf::Color(
            200,
            255,
            140,
            (sf::Uint8)std::clamp(alpha * m_coreBrightness, 0.f, 255.f)
        ));

        target.draw(core);

        // 🔥 GLOW (PLUS LARGE ET PLUS DOUX)
        glow.setRadius(l.radius * m_glowSizeMultiplier);
        glow.setOrigin(glow.getRadius(), glow.getRadius());
        glow.setPosition(x, y);

        glow.setFillColor(sf::Color(
            180,
            255,
            120,
            (sf::Uint8)std::clamp(alpha * 0.6f, 0.f, 255.f)
        ));

        if (m_shaderActif)
            m_glowTexture.draw(glow);
    }

    if (m_shaderActif) {
        m_glowTexture.display();

        m_blurShader.setUniform("u_texture", sf::Shader::CurrentTexture);
        m_blurShader.setUniform("resolution", sf::Vector2f(target.getSize()));
        m_blurShader.setUniform("blurRadius", m_bloomRadius);
        m_blurShader.setUniform("threshold", m_bloomThreshold);
        m_blurShader.setUniform("softness", m_bloomSoftness);

        sf::Uint8 intensity = (sf::Uint8)std::clamp(255.f * m_bloomMultiplier, 0.f, 255.f);
        m_glowSprite.setColor(sf::Color(intensity, intensity, intensity, 255));

        sf::RenderStates states;
        states.shader = &m_blurShader;
        states.blendMode = sf::BlendAdd;

        target.draw(m_glowSprite, states);
    }
}

void BackgroundEngine::drawImGui()
{
    ImGui::SetNextWindowPos(ImVec2(10, 10), ImGuiCond_FirstUseEver);
    ImGui::SetNextWindowSize(ImVec2(320, 420), ImGuiCond_FirstUseEver);

    ImGui::Begin("✨ Ambiance & VFX");

    ImGui::TextColored(ImVec4(0.4f, 0.8f, 0.5f, 1.0f), "Météo / Ambiance");
    if (ImGui::Button("🌿 Calme", ImVec2(90, 30))) {
        m_timeScale = 0.4f; m_lucioleIntensity = 0.5f; m_enableBoids = true;
    }
    ImGui::SameLine();
    if (ImGui::Button("🌪️ Vent", ImVec2(90, 30))) {
        m_timeScale = 3.5f; m_lucioleIntensity = 0.0f; m_enableBoids = false;
    }
    ImGui::SameLine();
    if (ImGui::Button("🌌 Nuit", ImVec2(90, 30))) {
        m_timeScale = 0.8f; m_lucioleIntensity = 1.0f; m_enableBoids = false;
    }

    ImGui::Separator();

    ImGui::Text("Bloom");
    ImGui::SliderFloat("Radius", &m_bloomRadius, 0.f, 30.f);
    ImGui::SliderFloat("Intensity", &m_bloomMultiplier, 0.f, 5.f);
    ImGui::SliderFloat("Threshold", &m_bloomThreshold, 0.f, 1.f);
    ImGui::SliderFloat("Softness", &m_bloomSoftness, 0.f, 1.f);

    ImGui::Separator();

    ImGui::Text("Glow Shape");
    ImGui::SliderFloat("Glow Size", &m_glowSizeMultiplier, 1.f, 6.f);
    ImGui::SliderFloat("Core Brightness", &m_coreBrightness, 0.5f, 3.f);

    ImGui::Separator();

    ImGui::Text("Global");
    ImGui::SliderFloat("Time Scale", &m_timeScale, 0.f, 5.f);
    ImGui::SliderFloat("Luciole Intensity", &m_lucioleIntensity, 0.f, 1.5f);

    ImGui::Separator();

    ImGui::Checkbox("Boids", &m_enableBoids);
    ImGui::Checkbox("Vines", &m_enableVines);
    ImGui::Checkbox("Grass", &m_enableGrass);
    ImGui::Checkbox("Lucioles", &m_enableLucioles);

    ImGui::End();
}