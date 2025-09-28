#pragma once
#include <string>
#include <SFML/Graphics.hpp>
#include "Utils.h"
#include "GameConfig.h"

// --- Damage Text Struct ---
struct DamageText {
    sf::Text text;
    sf::Vector2f velocity;
    float lifetime;
    sf::Clock clock;

    DamageText(const std::string& str, sf::Font& font, unsigned int charSize, sf::Color color, sf::Vector2f startPos) {
        text.setFont(font);
        text.setString(str);
        text.setCharacterSize(charSize);
        text.setFillColor(color);
        Utils::centerOrigin(text);
        text.setPosition(startPos);

        velocity = sf::Vector2f(Utils::randomFloat(-10.f, 10.f), GameConfig::DAMAGE_TEXT_SPEED + Utils::randomFloat(-10.f, 10.f));
        lifetime = GameConfig::DAMAGE_TEXT_LIFETIME;
    }

    void update(float dt) {
        text.move(velocity * dt);
        float t = clock.getElapsedTime().asSeconds() / lifetime;
        sf::Color color = text.getFillColor();
        color.a = static_cast<sf::Uint8>(Utils::lerp(255.f, 0.f, t));
        text.setFillColor(color);
    }

    bool isExpired() const {
        return clock.getElapsedTime().asSeconds() >= lifetime;
    }
};
