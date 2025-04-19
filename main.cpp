#include <SFML/Graphics.hpp>
#include <iostream>
#include <map>

struct Animation {
    sf::Texture texture;
    int frameCount;
    float frameDuration;
    bool loop;
};

class Enemy {
private:
    std::map<std::string, Animation> animations;
    sf::Sprite sprite;

    std::string currentAnimation;
    int currentFrame = 0;
    float frameTimer = 0.f;
    bool animationFinished = false;

    float speed;
    int health;
    int maxHealth;
    int normalAttackDamage;
    int specialAttackDamage;

    sf::Vector2f position;

public:
    Enemy(float moveSpeed = 2.0f, int hp = 100, int normalDmg = 10, int specialDmg = 25)
        : speed(moveSpeed), health(hp), maxHealth(hp),
          normalAttackDamage(normalDmg), specialAttackDamage(specialDmg) {}

    void addAnimation(const std::string& name, const std::string& file, int frameCount, float frameDuration, bool loop = true) {
        Animation anim;
        anim.frameCount = frameCount;
        anim.frameDuration = frameDuration;
        anim.loop = loop;
        if (!anim.texture.loadFromFile(file)) {
            throw std::runtime_error("Failed to load: " + file);
        }
        animations[name] = anim;
    }

    void play(const std::string& name, bool forceRestart = false) {
        if (name != currentAnimation || forceRestart) {
            currentAnimation = name;
            currentFrame = 0;
            frameTimer = 0.f;
            animationFinished = false;
            sprite.setTexture(animations[name].texture);
        }
    }

    void update(float dt) {
        if (animations.find(currentAnimation) == animations.end()) return;

        Animation& anim = animations[currentAnimation];
        int frameWidth = 128; // Fixed width based on your sprite sheet
        int frameHeight = 128;

        frameTimer += dt;
        if (frameTimer >= anim.frameDuration) {
            frameTimer = 0.f;
            currentFrame++;
            if (currentFrame >= anim.frameCount) {
                if (anim.loop) {
                    currentFrame = 0;
                } else {
                    currentFrame = anim.frameCount - 1;
                    animationFinished = true;
                }
            }
        }

        sprite.setTextureRect(sf::IntRect(frameWidth * currentFrame, 0, frameWidth, frameHeight));
        sprite.setPosition(position);
    }

    void move(float dx) {
        position.x += dx * speed;
    }

    void setPosition(float x, float y) {
        position = {x, y};
        sprite.setPosition(position);
    }

    void draw(sf::RenderWindow& window) {
        window.draw(sprite);
    }

    void takeDamage(int dmg) {
        health -= dmg;
        if (health < 0) health = 0;
    }

    int getHealth() const { return health; }
    bool isDead() const { return health <= 0; }

    bool isAnimationFinished() const { return animationFinished; }
    std::string getCurrentAnimation() const { return currentAnimation; }
};

int main() {
    sf::RenderWindow window(sf::VideoMode(800, 600), "Ninja Animation");
    window.setFramerateLimit(60);

    Enemy enemy;
    try {
        enemy.addAnimation("attack2", "Attack_2.png", 5, 0.07f, false);
        enemy.addAnimation("attack3", "Attack_3.png", 5, 0.07f, false);
        enemy.addAnimation("dead",    "Dead.png",     6, 0.12f, false);
    } catch (const std::exception& e) {
        std::cerr << e.what() << "\n";
        return -1;
    }

    enemy.setPosition(300, 400);
    enemy.play("attack2"); // start with attack2

    sf::Font font;
    sf::Text hpText;
    if (!font.loadFromFile("arial.ttf")) {
        std::cerr << "Font load failed\n";
        return -1;
    }
    hpText.setFont(font);
    hpText.setCharacterSize(18);
    hpText.setFillColor(sf::Color::White);
    hpText.setPosition(10, 10);

    sf::Clock clock;

    while (window.isOpen()) {
        float dt = clock.restart().asSeconds();
        sf::Event event;
        while (window.pollEvent(event)) {
            if (event.type == sf::Event::Closed)
                window.close();
        }

        if (sf::Keyboard::isKeyPressed(sf::Keyboard::A)) {
            enemy.play("attack2", true);
        }
        else if (sf::Keyboard::isKeyPressed(sf::Keyboard::S)) {
            enemy.play("attack3", true);
        }
        else if (sf::Keyboard::isKeyPressed(sf::Keyboard::J)) {
            enemy.play("dead", true);
        }
        else if (enemy.isDead() && enemy.getCurrentAnimation() != "dead") {
            enemy.play("dead", true);
        }

        if (sf::Keyboard::isKeyPressed(sf::Keyboard::Space)) {
            enemy.takeDamage(5);
        }

        enemy.update(dt);

        hpText.setString("HP: " + std::to_string(enemy.getHealth()));

        window.clear();
        enemy.draw(window);
        window.draw(hpText);
        window.display();
    }

    return 0;
}
