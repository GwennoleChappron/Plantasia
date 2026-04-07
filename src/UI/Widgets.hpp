#pragma once

#include "Data/EnumInfo.hpp"
#include <imgui.h>
#include <imgui_internal.h>
#include <cmath>
#include <unordered_map>
#include <string>
#include <functional>
#include <algorithm>

// ═════════════════════════════════════════════════════════════════════════════
//  UI::Theme — Couleurs et Constantes partagées
// ═════════════════════════════════════════════════════════════════════════════

namespace UI::Theme {
    constexpr float  ColLabel = 150.f;
    constexpr float  TagPadX  = 10.f;
    constexpr float  TagPadY  = 4.f;
    
    constexpr ImVec4 Muted    = { 0.40f, 0.50f, 0.40f, 1.f };
    constexpr ImVec4 Titre    = { 0.88f, 0.92f, 0.88f, 1.f };
    constexpr ImVec4 Accent   = { 0.20f, 0.75f, 0.40f, 1.f };
    constexpr ImVec4 CardBg   = { 0.04f, 0.07f, 0.04f, 0.60f };
    constexpr ImVec4 CardBord = { 0.18f, 0.28f, 0.18f, 1.00f };
}

// ═════════════════════════════════════════════════════════════════════════════
//  UI::Anim — Moteur d'animations Modernes (Spring, Overshoot, Stagger)
// ═════════════════════════════════════════════════════════════════════════════

namespace UI::Anim {

    inline float DeltaTime() { return ImGui::GetIO().DeltaTime; }
    inline float Time()      { return (float)ImGui::GetTime(); }

    inline float SmoothStep(float t) {
        t = std::clamp(t, 0.f, 1.f);
        return t * t * (3.0f - 2.0f * t);
    }

    inline float Overshoot(float t) {
        t = std::clamp(t, 0.f, 1.f) - 1.0f;
        return t * t * (2.5f * t + 1.5f) + 1.0f;
    }

    inline float EaseOut(float t) {
        t = std::clamp(t, 0.f, 1.f);
        return 1.0f - std::pow(1.0f - t, 3.0f);
    }

    inline float Pulse(float speed = 1.0f) {
        return 0.5f + 0.5f * std::sin(Time() * speed);
    }

    inline float StepSmooth(float current, float target, float speed = 10.0f) {
        return current + (target - current) * (1.0f - std::exp(-speed * DeltaTime()));
    }

    inline ImVec4 LerpColor(const ImVec4& a, const ImVec4& b, float t) {
        return {
            a.x + (b.x - a.x) * t,
            a.y + (b.y - a.y) * t,
            a.z + (b.z - a.z) * t,
            a.w + (b.w - a.w) * t
        };
    }

    inline ImVec4 WithAlpha(const ImVec4& c, float a) {
        return { c.x, c.y, c.z, a };
    }

    // ── ANIMATIONS AVEC ÉTAT (Stateful) ──

    inline float HoverProgress(ImGuiID id, bool isHovered, float speedIn = 8.f, float speedOut = 5.f) {
        static std::unordered_map<ImGuiID, float> states;
        float& t = states[id];
        t += (isHovered ? speedIn : -speedOut) * DeltaTime();
        t = std::clamp(t, 0.f, 1.f);
        return SmoothStep(t); 
    }

    struct SpringState { float value = 0.f; float velocity = 0.f; };
    
    inline float Spring(ImGuiID id, float target, float stiffness = 200.f, float damping = 20.f) {
        static std::unordered_map<ImGuiID, SpringState> states;
        SpringState& s = states[id];
        
        if (s.value == 0.f && target != 0.f && states.count(id) == 1) s.value = target;

        float dt = DeltaTime();
        float force = stiffness * (target - s.value);
        s.velocity += force * dt;
        s.velocity *= std::exp(-damping * dt);
        s.value += s.velocity * dt;
        return s.value;
    }

    // ── TRANSITIONS DE LAYOUT ──

    inline float Stagger(int index, float globalTime, float delayPerItem = 0.05f) {
        return std::max(0.f, globalTime - (index * delayPerItem));
    }

    inline void ApplySlideFade(float t, float slideDistance = 20.f) {
        float eased = Overshoot(t); 
        float alpha = SmoothStep(t * 1.5f); 
        ImGui::SetCursorPosY(ImGui::GetCursorPosY() + (1.f - eased) * slideDistance);
        ImGui::PushStyleVar(ImGuiStyleVar_Alpha, ImGui::GetStyle().Alpha * alpha);
    }

    inline float Shimmer(float cycleDuration = 3.0f) {
        // Retourne la position du reflet (0 à 1) avec une longue pause
        float t = std::fmod(Time(), cycleDuration) / (cycleDuration * 0.3f); 
        return std::clamp(t, 0.f, 1.f);
    }

    // Respiration organique (gonfle et dégonfle très doucement)
    inline float Breath(float speed = 1.5f, float amplitude = 0.02f) {
        return 1.0f + (std::sin(Time() * speed) * 0.5f + 0.5f) * amplitude;
    }

    // Tremblement violent (Wiggle) pour les alertes
    inline float Wiggle(ImGuiID id, bool trigger, float strength = 150.0f) {
        static std::unordered_map<ImGuiID, SpringState> states;
        SpringState& s = states[id];
        
        if (trigger) s.velocity += strength; // Coup de pied dans le ressort !
        
        float dt = DeltaTime();
        float force = 400.f * (0.f - s.value); // Retour fort vers 0
        s.velocity += force * dt;
        s.velocity *= std::exp(-15.f * dt); // Amortissement brutal
        s.value += s.velocity * dt;
        
        return s.value;
    }
    
}

// ═════════════════════════════════════════════════════════════════════════════
//  UI::Widgets — Composants Agnostiques
// ═════════════════════════════════════════════════════════════════════════════

namespace UI {

    // ── 1. TAGS & BADGES ──
    
    inline void TagBadge(const char* text, const ImVec4& color, const char* tooltip = nullptr) {
        const ImVec2 textSize = ImGui::CalcTextSize(text);
        const ImVec2 size = { textSize.x + Theme::TagPadX * 2.f, textSize.y + Theme::TagPadY * 2.f };
        const ImVec4 bg = { color.x * 0.22f, color.y * 0.22f, color.z * 0.22f, 0.85f };

        const ImVec2 pos = ImGui::GetCursorScreenPos();
        ImDrawList* dl = ImGui::GetWindowDrawList();
        dl->AddRectFilled(pos, { pos.x + size.x, pos.y + size.y }, ImGui::ColorConvertFloat4ToU32(bg), 4.f);
        dl->AddRect(pos, { pos.x + size.x, pos.y + size.y }, ImGui::ColorConvertFloat4ToU32({ color.x, color.y, color.z, 0.45f }), 4.f, 0, 0.5f);

        ImGui::InvisibleButton(text, size);
        dl->AddText({ pos.x + Theme::TagPadX, pos.y + Theme::TagPadY }, ImGui::ColorConvertFloat4ToU32(color), text);

        if (tooltip && ImGui::IsItemHovered()) ImGui::SetTooltip("%s", tooltip);
    }
    
    inline void TagBadge(const std::string& text, const ImVec4& color, const char* tooltip = nullptr) { 
        TagBadge(text.c_str(), color, tooltip); 
    }

    // ── 2. INFO CARDS & ROWS ──
    
    inline void InfoRow(const char* label, const char* value, const ImVec4& valueColor = Theme::Titre, float colOffset = Theme::ColLabel) {
        ImGui::TextColored(Theme::Muted, "%s", label);
        ImGui::SameLine(colOffset);
        ImGui::TextColored(valueColor, "%s", value);
    }
    
    inline void InfoRow(const char* label, const std::string& value, const ImVec4& valueColor = Theme::Titre, float colOffset = Theme::ColLabel) {
        InfoRow(label, value.c_str(), valueColor, colOffset);
    }
    
    inline void InfoRow(const char* label, bool value, float colOffset = Theme::ColLabel) {
        InfoRow(label, value ? "Oui" : "Non", value ? ImVec4{0.2f,0.8f,0.3f,1.f} : Theme::Muted, colOffset);
    }

    template <typename T>
    inline void InfoRow(const char* label, const T& meta, float colOffset = Theme::ColLabel) {
        InfoRow(label, meta.label.c_str(), meta.color, colOffset);
    }

    inline void InfoCard(const char* title, const char* id, float height, std::function<void()> content, const ImVec4& bgColor = Theme::CardBg, const ImVec4& rimColor = Theme::CardBord) {
        if (title && title[0] != '\0') ImGui::SeparatorText(title);
        ImGui::PushStyleColor(ImGuiCol_ChildBg, bgColor);
        ImGui::PushStyleColor(ImGuiCol_Border, rimColor);
        ImGui::BeginChild(id, { 0.f, height }, true);
        ImGui::Spacing();
        content();
        ImGui::Spacing();
        ImGui::EndChild();
        ImGui::PopStyleColor(2);
    }
    
    inline void InfoCardRaw(const char* id, float height, std::function<void()> content, const ImVec4& bgColor = Theme::CardBg, const ImVec4& rimColor = Theme::CardBord) {
        InfoCard(nullptr, id, height, std::move(content), bgColor, rimColor);
    }

    // ── 3. JAUGES ──
    
    inline void Gauge(float value, float maxValue, const ImVec4& color, const char* labelBefore = nullptr, const char* labelAfter = nullptr, ImVec2 size = { 80.f, 8.f }) {
        if (labelBefore && labelBefore[0] != '\0') {
            ImGui::TextColored(Theme::Muted, "%s", labelBefore);
            ImGui::SameLine();
        }
        const float ratio = (maxValue > 0.f) ? (value / maxValue) : 0.f;
        ImGui::PushStyleColor(ImGuiCol_PlotHistogram, color);
        ImGui::PushStyleColor(ImGuiCol_FrameBg, { color.x * 0.15f, color.y * 0.15f, color.z * 0.15f, 1.f });
        ImGui::ProgressBar(ratio, size, "");
        ImGui::PopStyleColor(2);
        if (labelAfter && labelAfter[0] != '\0') {
            ImGui::SameLine();
            ImGui::TextColored(color, "%s", labelAfter);
        }
    }
    
    inline void Gauge(int value, int maxValue, const ImVec4& color, const char* labelBefore = nullptr, const char* labelAfter = nullptr) {
        Gauge((float)value, (float)maxValue, color, labelBefore, labelAfter);
    }

    inline void GaugeSegmented(int value, int maxValue, const ImVec4& colorOn, const ImVec4& colorOff = { 0.15f, 0.20f, 0.15f, 1.f }, float segWidth = 14.f, float segHeight = 8.f, float gap = 2.f) {
        ImDrawList* dl = ImGui::GetWindowDrawList();
        ImVec2 pos = ImGui::GetCursorScreenPos();
        const float totalW = maxValue * segWidth + (maxValue - 1) * gap;
        for (int i = 0; i < maxValue; ++i) {
            const float x0 = pos.x + i * (segWidth + gap);
            dl->AddRectFilled({ x0, pos.y }, { x0 + segWidth, pos.y + segHeight }, ImGui::ColorConvertFloat4ToU32((i < value) ? colorOn : colorOff), 2.f);
        }
        ImGui::Dummy({ totalW, segHeight });
    }

    inline void GaugeWaterDrops(int value, int maxValue, const ImVec4& colorOn, const ImVec4& colorOff = { 0.15f, 0.20f, 0.15f, 1.f }, float dropWidth = 10.f, float dropHeight= 14.f, float gap = 4.f) {
        ImDrawList* dl = ImGui::GetWindowDrawList();
        ImVec2 pos = ImGui::GetCursorScreenPos();
        const float totalW = maxValue * dropWidth + (maxValue - 1) * gap;
        for (int i = 0; i < maxValue; ++i) {
            float cx = pos.x + i * (dropWidth + gap) + dropWidth * 0.5f;
            float r = dropWidth * 0.5f;
            float cy = pos.y + dropHeight - r;
            ImU32 col = ImGui::ColorConvertFloat4ToU32((i < value) ? colorOn : colorOff);

            dl->PathClear();
            dl->PathLineTo({cx, pos.y});
            dl->PathBezierCubicCurveTo({cx + r * 0.8f, pos.y + dropHeight * 0.3f}, {cx + r, cy - r * 0.5f}, {cx + r, cy});
            dl->PathArcTo({cx, cy}, r, 0.0f, 3.14159265f, 12);
            dl->PathBezierCubicCurveTo({cx - r, cy - r * 0.5f}, {cx - r * 0.8f, pos.y + dropHeight * 0.3f}, {cx, pos.y});
            dl->PathFillConvex(col);
        }
        ImGui::Dummy({ totalW, dropHeight });
    }

    // ── 4. BOUTONS NAV & UTILITAIRES ──
    
    inline void NavButton(const char* label, std::function<void()> onClic, const char* tooltip = nullptr, bool enabled = true) {
        if (!enabled) ImGui::BeginDisabled();
        ImGui::PushID(label);
        if (ImGui::SmallButton(label) && onClic) onClic();
        ImGui::PopID();
        if (!enabled) ImGui::EndDisabled();
        if (tooltip && ImGui::IsItemHovered(ImGuiHoveredFlags_AllowWhenDisabled)) ImGui::SetTooltip("%s", tooltip);
    }
    
    inline void NavButton(const std::string& label, std::function<void()> onClic, const char* tooltip = nullptr, bool enabled = true) {
        NavButton(label.c_str(), std::move(onClic), tooltip, enabled);
    }

    inline void NavButtonPrimary(const char* label, std::function<void()> onClic, const char* tooltip = nullptr, bool enabled = true) {
        if (!enabled) ImGui::BeginDisabled();
        ImGui::PushID(label);
        ImGui::PushStyleColor(ImGuiCol_Button, { 0.10f, 0.35f, 0.18f, 1.f });
        ImGui::PushStyleColor(ImGuiCol_ButtonHovered, { 0.15f, 0.55f, 0.30f, 1.f });
        ImGui::PushStyleColor(ImGuiCol_ButtonActive, { 0.20f, 0.75f, 0.40f, 1.f });
        if (ImGui::Button(label) && onClic) onClic();
        ImGui::PopStyleColor(3);
        ImGui::PopID();
        if (!enabled) ImGui::EndDisabled();
        if (tooltip && ImGui::IsItemHovered(ImGuiHoveredFlags_AllowWhenDisabled)) ImGui::SetTooltip("%s", tooltip);
    }

    inline void NavButtonIcon(const char* icon, const char* label, std::function<void()> onClic, const char* tooltip = nullptr, bool enabled = true) {
        std::string full = std::string(icon) + " " + label + "##" + label;
        NavButton(full.c_str(), std::move(onClic), tooltip, enabled);
    }

    inline void Section(const char* title) { 
        ImGui::Spacing(); 
        ImGui::SeparatorText(title); 
    }
    
    inline void LabelAccent(const char* label, const char* value, const ImVec4& valueColor = Theme::Accent) {
        ImGui::TextColored(Theme::Muted, "%s", label); 
        ImGui::SameLine(0.f, 4.f); 
        ImGui::TextColored(valueColor, "%s", value);
    }
    
    inline void BodyText(const char* text, const ImVec4& color = Theme::Titre) {
        ImGui::PushTextWrapPos(0.f); 
        ImGui::TextColored(color, "%s", text); 
        ImGui::PopTextWrapPos();
    }
    
    inline void Gap(float pixels = 8.f) { 
        ImGui::Dummy({ 0.f, pixels }); 
    }

    // ─────────────────────────────────────────────────────────────────────────────
    //  SectionAnimated — Liane AAA (Vent, Respiration, Souris & Spores)
    // ─────────────────────────────────────────────────────────────────────────────

   // ── CONFIGURATION DE LA LIANE ──
    struct VineStyle {
        int   nombreLianes    = 2;      // Nombre de lianes à entrelacer
        float frequenceX      = 40.f;   // Longueur d'onde de base
        float amplitudeY      = 6.f;    // Hauteur de la vague
        float epaisseurTige   = 1.5f;   // Épaisseur de la ligne
        float tailleFeuille   = 6.5f;   // Longueur de la feuille
        float ecartFeuille    = 8.f;    // Longueur du pétiole
        float organicVariance = 0.5f;   // Facteur de chaos
        
        ImVec4 couleurTige    = Theme::Muted;
        ImVec4 couleurFeuille = Theme::Accent;
    };

    inline void SectionAnimated(const char* title, float progress, bool isBlooming = false, const VineStyle& style = VineStyle()) {
        ImGui::Spacing();

        float t = (float)ImGui::GetTime();

        // 🌿 Respiration du titre
        float breathe = 0.5f + 0.5f * sinf(t * 1.5f);
        ImVec4 titleCol = Anim::LerpColor(Theme::Accent, ImVec4(0.4f, 1.0f, 0.5f, 1.f), breathe * 0.3f);

        const char* displayEnd = ImGui::FindRenderedTextEnd(title);
        ImGui::TextColored(titleCol, "%.*s", (int)(displayEnd - title), title);

        ImVec2 p = ImGui::GetCursorScreenPos();
        float fullWidth = ImGui::GetContentRegionAvail().x;
        float eased = Anim::EaseOut(progress);
        float currentWidth = fullWidth * eased;

        if (currentWidth <= 1.0f) return;

        ImDrawList* dl = ImGui::GetWindowDrawList();
        float midY = p.y + style.amplitudeY * 0.5f;

        dl->PushClipRect(
            {p.x, p.y - style.amplitudeY - 25.f},
            {p.x + currentWidth, p.y + style.amplitudeY * 2.f + 15.f},
            true
        );

        // ── FONCTION MATHÉMATIQUE DE LA LIANE (Sans souris, avec gestion des brins) ──
        auto evaluateVineY = [&](float x, int vineIndex) {
            float localX = x - p.x;
            
            // Décalage temporel et spatial pour que chaque liane soit unique et s'entrecroise
            float phaseOffset = vineIndex * 2.4f; 
            float speedOffset = 1.0f + vineIndex * 0.15f; 
            
            float wave1 = sinf(localX / (style.frequenceX * 0.5f) - t * 0.8f * speedOffset + phaseOffset) * style.amplitudeY;
            float wave2 = cosf(localX / (style.frequenceX * 0.25f) + t * 1.5f + phaseOffset * 1.3f) * (style.amplitudeY * style.organicVariance);
            
            return midY + wave1 + wave2;
        };

        // ── HELPERS POUR LE DESSIN ORGANIQUE ──
        auto drawLeaf = [&](float x, float y, float size, float angle, ImU32 color) {
            float cosA = cosf(angle);
            float sinA = sinf(angle);
            auto transform = [&](float lx, float ly) {
                return ImVec2(x + lx * cosA - ly * sinA, y + lx * sinA + ly * cosA);
            };
            ImVec2 poly[6] = {
                transform(0.f, 0.f), transform(size * 0.3f, size * 0.35f), transform(size * 0.7f, size * 0.2f),
                transform(size, 0.f), transform(size * 0.7f, -size * 0.2f), transform(size * 0.3f, -size * 0.35f)
            };
            dl->AddConvexPolyFilled(poly, 6, color);
        };

        auto drawFlower = [&](float cx, float cy, float size, ImU32 color, ImU32 centerColor) {
            for(int j = 0; j < 5; j++) {
                float a = j * (6.28318f / 5.0f) + t * 0.5f; 
                float px = cx + cosf(a) * (size * 0.4f);
                float py = cy + sinf(a) * (size * 0.4f);
                dl->AddCircleFilled({px, py}, size * 0.45f, color);
            }
            dl->AddCircleFilled({cx, cy}, size * 0.3f, centerColor); 
        };

        // ── DESSIN DES LIANES (De l'arrière-plan vers le premier plan) ──
        uint32_t seed = ImGui::GetID(title);

        for (int v = style.nombreLianes - 1; v >= 0; --v) {
            
            // Calcul de la profondeur (assombrit légèrement les lianes de fond)
            float depthFactor = 1.0f - (v * 0.2f); // 1.0, 0.8, 0.6...
            ImVec4 stemCol = { style.couleurTige.x * depthFactor, style.couleurTige.y * depthFactor, style.couleurTige.z * depthFactor, style.couleurTige.w };
            ImVec4 leafCol = { style.couleurFeuille.x * depthFactor, style.couleurFeuille.y * depthFactor, style.couleurFeuille.z * depthFactor, style.couleurFeuille.w };
            
            ImU32 uStemCol = ImGui::ColorConvertFloat4ToU32(stemCol);
            ImU32 uLeafCol = ImGui::ColorConvertFloat4ToU32(leafCol);

            // 1. TRACÉ DE LA TIGE
            dl->PathClear();
            const float stepX = 4.0f; 
            for (float x = p.x; x <= p.x + currentWidth; x += stepX) {
                dl->PathLineTo({x, evaluateVineY(x, v)});
            }
            dl->PathLineTo({p.x + currentWidth, evaluateVineY(p.x + currentWidth, v)});
            dl->PathStroke(uStemCol, 0, style.epaisseurTige * depthFactor); // Les lianes du fond sont très légèrement plus fines

            // 2. VÉGÉTATION & PÉTIOLES
            uint32_t rng_flora = seed + 456 + v * 1000; // Un RNG unique par liane
            auto getRand = [](uint32_t& state) {
                state = state * 1664525 + 1013904223;
                return (float)state / (float)0xFFFFFFFF;
            };

            float leafSpacing = style.frequenceX * 0.6f;
            int numLeaves = (int)(currentWidth / leafSpacing);

            for (int i = 0; i < numLeaves; ++i) {
                float leafBaseX = p.x + i * leafSpacing;
                float stemY = evaluateVineY(leafBaseX, v);

                float offsetX = (getRand(rng_flora) - 0.5f) * (leafSpacing * 0.5f);
                float offsetY = (i % 2 == 0) ? style.ecartFeuille : -style.ecartFeuille;
                offsetY += (getRand(rng_flora) - 0.5f) * 4.f; 

                float targetLeafX = leafBaseX + offsetX + style.ecartFeuille; 
                float targetLeafY = stemY + offsetY;

                // Mouvement organique continu (vent)
                float leafX = targetLeafX + sinf(t * 2.0f + i) * 2.0f;
                float leafY = targetLeafY + cosf(t * 1.5f + i) * 1.0f;

                // 🌿 DESSIN DU PÉTIOLE
                float cpX = leafBaseX + (leafX - leafBaseX) * 0.5f;
                float cpY = stemY + (leafY - stemY) * 0.5f + 3.0f; 

                dl->PathClear();
                dl->PathLineTo({leafBaseX, stemY});
                dl->PathBezierQuadraticCurveTo({cpX, cpY}, {leafX, leafY});
                dl->PathStroke(uStemCol, 0, style.epaisseurTige * 0.6f * depthFactor);

                float leafAngle = atan2f(leafY - cpY, leafX - cpX);

                ImVec4 finalColor = leafCol;
                float life = 0.8f + sinf(t * 3.0f + i) * 0.2f;
                float finalSize = style.tailleFeuille * life * (0.8f + getRand(rng_flora) * 0.4f) * depthFactor;

                // 🌸 FLORAISON (Les fleurs apparaissent un peu moins sur les lianes du fond pour ne pas surcharger)
                bool isFlower = false;
                if (isBlooming && getRand(rng_flora) < (0.35f * depthFactor)) {
                    isFlower = true;
                    float colorRng = getRand(rng_flora);
                    if (colorRng < 0.25f)      finalColor = { 0.95f, 0.45f, 0.65f, 1.f };
                    else if (colorRng < 0.50f) finalColor = { 0.95f, 0.85f, 0.25f, 1.f };
                    else if (colorRng < 0.75f) finalColor = { 0.40f, 0.75f, 0.95f, 1.f };
                    else                       finalColor = { 0.75f, 0.45f, 0.85f, 1.f };

                    finalSize *= 1.2f;
                    
                    ImU32 colFleur = ImGui::ColorConvertFloat4ToU32(finalColor);
                    ImU32 colCoeur = ImGui::ColorConvertFloat4ToU32({1.f * depthFactor, 0.9f * depthFactor, 0.4f * depthFactor, 1.f}); 
                    drawFlower(leafX, leafY, finalSize, colFleur, colCoeur);
                    
                    // ✨ SPORES
                    float sporeTime = t * (0.4f + getRand(rng_flora) * 0.4f) + i; 
                    float cycle = fmodf(sporeTime, 1.0f); 
                    float sporeYOffset = cycle * 25.f; 
                    float sporeX = leafX + sinf(sporeTime * 15.f) * 4.f; 
                    float baseAlpha = 4.0f * cycle * (1.0f - cycle);
                    float sporeAlpha = baseAlpha * (sinf(sporeTime * 30.f) * 0.3f + 0.7f); 
                    
                    if (sporeAlpha > 0.05f) {
                        ImVec4 sporeColor = { finalColor.x, finalColor.y, finalColor.z, sporeAlpha };
                        dl->AddCircleFilled({sporeX, leafY - sporeYOffset}, 1.2f * depthFactor, ImGui::ColorConvertFloat4ToU32(sporeColor));
                    }
                } else {
                    // 🌿 Dessin de la vraie Feuille
                    drawLeaf(leafX, leafY, finalSize, leafAngle, ImGui::ColorConvertFloat4ToU32(finalColor));
                    
                    // 🌱 Bourgeon secondaire
                    if (getRand(rng_flora) < 0.40f) {
                        float subX = leafX + cosf(leafAngle - 1.5f) * 2.f;
                        float subY = leafY + sinf(leafAngle - 1.5f) * 2.f;
                        drawLeaf(subX, subY, finalSize * 0.6f, leafAngle + 0.8f, ImGui::ColorConvertFloat4ToU32(leafCol));
                    }
                }
            }
        }

        dl->PopClipRect();
        ImGui::Dummy({0.f, style.amplitudeY + style.tailleFeuille * 1.5f + 6.f});
    }

} // namespace UI