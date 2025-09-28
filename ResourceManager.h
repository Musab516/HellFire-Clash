#pragma once
#include <SFML/Graphics.hpp>
#include <iostream>
#include <map>
#include <iomanip>

// --- Resource Manager ---
class ResourceManager {
public:
    static sf::Font& getFont(const std::string& id) {
        if (fonts.find(id) == fonts.end()) {
            if (!fonts[id].loadFromFile(id)) {
                std::cerr << "Failed to load font '" << id << "'" << std::endl;
            }
        }
        return fonts[id];
    }

    // General texture loading method
    static sf::Texture& getTexture(const std::string& id) {
        if (textures.find(id) == textures.end()) {
            if (!textures[id].loadFromFile(id)) {
                std::cerr << "Failed to load texture '" << id << "'" << std::endl;
            }
        }
        return textures[id];
    }
    
    // Specific bool returning texture loader for resource checks
    static bool loadTexture(sf::Texture& tex, const std::string& filename) {
        if (!tex.loadFromFile(filename)) {
            std::cerr << "Failed to load texture: " << filename << std::endl;
            return false;
        }
        return true;
    }

 static void loadMenuBackgroundFrames(std::vector<sf::Texture>& frames, int count) {
    frames.resize(count);
    for (int i = 0; i < count; ++i) {
        std::stringstream ss;
        ss << "assets/" << std::setw(5) << std::setfill('0') << (i + 1) << ".png";
        if (!frames[i].loadFromFile(ss.str())) {
            std::cerr << "Failed to load menu frame: " << ss.str() << std::endl;
        }
    }
}


    static bool loadMapFrames(std::vector<sf::Texture>& frames, int frameCount,
                          const std::string& prefix, const std::string& suffix,
                          int startNum = 1, int zeroPadding = 0) {
    frames.clear();
    frames.resize(frameCount);
    for (int i = 0; i < frameCount; ++i) {
        std::stringstream ss;
        ss << "assets/" << prefix;  // Prepend assets folder
        if (zeroPadding > 0) ss << std::setw(zeroPadding) << std::setfill('0');
        ss << (i + startNum) << suffix;
        if (!frames[i].loadFromFile(ss.str())) {
            std::cerr << "Failed to load map frame: " << ss.str() << std::endl;
            return false;
        }
    }
    return true;
}
~ResourceManager() {
        // Clean up loaded resources if necessary
        fonts.clear();
        textures.clear();
    }
private:
    static std::map<std::string, sf::Font> fonts;
    static std::map<std::string, sf::Texture> textures;
};



// Static member definitions
std::map<std::string, sf::Font> ResourceManager::fonts;
std::map<std::string, sf::Texture> ResourceManager::textures;
