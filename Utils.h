#pragma once
#include <random>
#include <string>
#include <cmath>
#include <iomanip>
#include <sstream>
#include <algorithm>
#include <map>
#include <SFML/Graphics.hpp> // Include SFML Graphics for sf::Text and sf::Sprite

namespace Utils {
    void centerOrigin(sf::Text &text) {
        sf::FloatRect bounds = text.getLocalBounds();
        text.setOrigin(bounds.left + bounds.width / 2.0f, bounds.top + bounds.height / 2.0f);
    }

    void centerOrigin(sf::Sprite &sprite) {
        sf::FloatRect bounds = sprite.getLocalBounds();
        sprite.setOrigin(bounds.left + bounds.width / 2.0f, bounds.top + bounds.height / 2.0f);
    }

    template <typename T>
    T clamp(T value, T minVal, T maxVal) {
        return std::max(minVal, std::min(value, maxVal));
    }

    float lerp(float a, float b, float t) {
        t = clamp(t, 0.0f, 1.0f);
        return a + t * (b - a);
    }

    float distance(const sf::Vector2f& p1, const sf::Vector2f& p2) {
        return std::sqrt(std::pow(p2.x - p1.x, 2) + std::pow(p2.y - p1.y, 2));
    }
    std::mt19937 rng(std::random_device{}());
    float randomFloat(float min, float max) {
        std::uniform_real_distribution<float> dist(min, max);
        return dist(rng);
    }
    std::string formatTime(float seconds) {
        int min = static_cast<int>(seconds) / 60;
        int sec = static_cast<int>(seconds) % 60;
        std::stringstream ss;
        ss << std::setfill('0') << std::setw(2) << min << ":" << std::setfill('0') << std::setw(2) << sec;
        return ss.str();
    }
}
