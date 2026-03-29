#include "BoidsSwarm.hpp"
#include <cmath>
#include <random>
#include <unordered_map>
#include <vector>

static std::mt19937 rng_boids(12345);

static float blen(sf::Vector2f v) { return std::sqrt(v.x*v.x + v.y*v.y); }
static sf::Vector2f bnorm(sf::Vector2f v) {
    float l = blen(v); return l > 0 ? v/l : sf::Vector2f{};
}

BoidsSwarm::BoidsSwarm(int count, float width, float height) {
    m_shape.setPointCount(3);
    m_shape.setPoint(0, {6.f,  0.f});
    m_shape.setPoint(1, {-4.f, 3.f});
    m_shape.setPoint(2, {-4.f,-3.f});
    m_shape.setFillColor(sf::Color(100,200,120,150));

    std::uniform_real_distribution<float> dx(0.f, width);
    std::uniform_real_distribution<float> dy(0.f, height);
    std::uniform_real_distribution<float> da(0.f, 6.28318f);
    for (int i = 0; i < count; ++i) {
        Boid b;
        b.position = {dx(rng_boids), dy(rng_boids)};
        float a = da(rng_boids);
        b.velocity = {std::cos(a)*m_maxSpeed, std::sin(a)*m_maxSpeed};
        m_boids.push_back(b);
    }
}

void BoidsSwarm::update(float dt, sf::Vector2f mousePos, float width, float height) {
    const float CELL = m_perceptionRadius;
    const int   GW   = (int)(width / CELL) + 1;

    std::unordered_map<int, std::vector<int>> grid;
    grid.reserve(m_boids.size());
    for (int i = 0; i < (int)m_boids.size(); ++i) {
        int cx = (int)(m_boids[i].position.x / CELL);
        int cy = (int)(m_boids[i].position.y / CELL);
        grid[cy * GW + cx].push_back(i);
    }

    for (int bi = 0; bi < (int)m_boids.size(); ++bi) {
        auto& b = m_boids[bi];
        int cx = (int)(b.position.x / CELL);
        int cy = (int)(b.position.y / CELL);

        sf::Vector2f sep{}, ali{}, coh{};
        int total = 0;

        for (int ny = cy-1; ny <= cy+1; ++ny) {
            for (int nx = cx-1; nx <= cx+1; ++nx) {
                auto it = grid.find(ny * GW + nx);
                if (it == grid.end()) continue;
                for (int oi : it->second) {
                    if (oi == bi) continue;
                    auto& other = m_boids[oi];
                    sf::Vector2f diff = b.position - other.position;
                    float d = blen(diff);
                    if (d < m_perceptionRadius && d > 0) {
                        sep += diff / (d * d);
                        ali += other.velocity;
                        coh += other.position;
                        ++total;
                    }
                }
            }
        }

        if (total > 0) {
            ali /= (float)total;
            coh  = (coh / (float)total) - b.position;
        }

        sf::Vector2f mf{};
        sf::Vector2f md = b.position - mousePos;
        float mdist = blen(md);
        if (mdist < m_mouseRepulsionRadius && mdist > 0)
            mf = bnorm(md) * (300.f / mdist);

        b.velocity += (sep*15.f + ali*0.1f + coh*0.2f + mf*50.f) * dt;

        float spd = blen(b.velocity);
        if      (spd > m_maxSpeed)       b.velocity = b.velocity/spd * m_maxSpeed;
        else if (spd < m_maxSpeed*0.5f)  b.velocity = b.velocity/spd * (m_maxSpeed*0.5f);

        b.position += b.velocity * dt;
        if (b.position.x < 0)     b.position.x += width;
        if (b.position.x > width) b.position.x -= width;
        if (b.position.y < 0)     b.position.y += height;
        if (b.position.y > height)b.position.y -= height;
    }
}

void BoidsSwarm::draw(sf::RenderTarget& target) {
    for (const auto& b : m_boids) {
        m_shape.setPosition(b.position);
        m_shape.setRotation(std::atan2(b.velocity.y, b.velocity.x) * 180.f / 3.14159f);
        target.draw(m_shape);
    }
}
