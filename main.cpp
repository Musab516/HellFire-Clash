#include <SFML/Graphics.hpp>
#include <SFML/Audio.hpp>
#include <iostream>
#include <map>
#include "Game.h"
#include "ResourceManager.h"

std::map<std::string, sf::Font> fonts;
std::map<std::string, sf::Texture> textures;


// --- main.cpp ---
int main() {
    sf::Music backgroundMusic;
    if (!backgroundMusic.openFromFile("assets/hellfire_music.ogg")) {
        return -1;
    }

    backgroundMusic.setLoop(true);
    backgroundMusic.setVolume(50);
    backgroundMusic.play();

    Game game; // Create the game instance
    game.run(); // Start the game loop
    return 0;
}