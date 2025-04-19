#include <SFML/Graphics.hpp>
#include <iostream>
#include <vector>
#include <string>

int main() {
    sf::RenderWindow window(sf::VideoMode(800, 600), "Fighter Game");
    window.setFramerateLimit(60);

    const int GIF_FRAME_COUNT = 8;
    const float FRAME_DELAY = 0.11f;
    std::vector<sf::Texture> bgFrames(GIF_FRAME_COUNT);

    for (int i = 0; i < GIF_FRAME_COUNT; ++i) {
        std::string filename = "frame_" + std::to_string(i) + "_delay-0.11s.png";
        if (!bgFrames[i].loadFromFile(filename)) {
            std::cerr << "Failed to load background frame: " << filename << std::endl;
            return -1;
        }
    }

    sf::Sprite background(bgFrames[0]);
    background.setScale(
        window.getSize().x / background.getLocalBounds().width,
        window.getSize().y / background.getLocalBounds().height
    );

    // Load textures
    sf::Texture idleTexture, runTexture, jumpTexture;
    sf::Texture attack1Texture, attack2Texture, attack3Texture, shieldTexture;

    if (!idleTexture.loadFromFile("Idle.png") ||
        !runTexture.loadFromFile("Run.png") ||
        !jumpTexture.loadFromFile("Jump.png") ||
        !attack1Texture.loadFromFile("Attack_1.png") ||
        !attack2Texture.loadFromFile("Attack_2.png") ||
        !attack3Texture.loadFromFile("Attack_3.png") ||
        !shieldTexture.loadFromFile("Shield.png")) {
        std::cerr << "Error loading textures!" << std::endl;
        return -1;
    }

    const int IDLE_FRAMES = 6, RUN_FRAMES = 8, JUMP_FRAMES = 10;
    const int ATTACK1_FRAMES = 4, ATTACK2_FRAMES = 3, ATTACK3_FRAMES = 4, SHIELD_FRAMES = 2;

    const float IDLE_SPEED = 0.15f, RUN_SPEED = 0.08f, JUMP_SPEED = 0.1f, ATTACK_SPEED = 0.1f;

    int frameHeight = idleTexture.getSize().y;
    int idleWidth = idleTexture.getSize().x / IDLE_FRAMES;
    int runWidth = runTexture.getSize().x / RUN_FRAMES;
    int jumpWidth = jumpTexture.getSize().x / JUMP_FRAMES;
    int attack1Width = attack1Texture.getSize().x / ATTACK1_FRAMES;
    int attack2Width = attack2Texture.getSize().x / ATTACK2_FRAMES;
    int attack3Width = attack3Texture.getSize().x / ATTACK3_FRAMES;
    int shieldWidth = shieldTexture.getSize().x / SHIELD_FRAMES;

    sf::Sprite character;
    float scale = 1.5f;
    character.setScale(scale, scale);
    character.setTexture(idleTexture);
    character.setTextureRect(sf::IntRect(0, 0, idleWidth, frameHeight));

    float charWidth = idleWidth * scale;
    float charHeight = frameHeight * scale;
    float groundY = window.getSize().y - charHeight - 20;
    character.setPosition(window.getSize().x / 2 - charWidth / 2, groundY);

    float baseSpeed = 5.0f;
    float jumpSpeed = -12.0f;
    float gravity = 0.6f;
    float verticalVelocity = 0.0f;
    bool facingRight = true, isJumping = false, isAttacking = false;

    enum Action { IDLE, RUN, JUMP, ATTACK1, ATTACK2, ATTACK3, SHIELD };
    Action currentAction = IDLE;

    int currentFrame = 0;
    float animTime = 0;
    sf::Clock animClock;
    sf::Clock attackCooldownClock;
    bool canAttack = true;

    int bgFrame = 0;
    float bgTimer = 0;

    while (window.isOpen()) {
        sf::Event event;
        while (window.pollEvent(event)) {
            if (event.type == sf::Event::Closed)
                window.close();
        }

        if (!canAttack && attackCooldownClock.getElapsedTime().asSeconds() > 0.5f)
            canAttack = true;

        float speed = baseSpeed;
        if (sf::Keyboard::isKeyPressed(sf::Keyboard::LShift))
            speed *= 1.3f;

        bool isMoving = false;

        if (!isAttacking && !sf::Keyboard::isKeyPressed(sf::Keyboard::C)) {
            if (sf::Keyboard::isKeyPressed(sf::Keyboard::Left)) {
                character.move(-speed, 0);
                isMoving = true;
                facingRight = false;
            }
            if (sf::Keyboard::isKeyPressed(sf::Keyboard::Right)) {
                character.move(speed, 0);
                isMoving = true;
                facingRight = true;
            }
        }

        if (sf::Keyboard::isKeyPressed(sf::Keyboard::Space) && !isJumping) {
            isJumping = true;
            verticalVelocity = jumpSpeed;
        }

        if (sf::Keyboard::isKeyPressed(sf::Keyboard::X) && canAttack && !isAttacking) {
            isAttacking = true;
            canAttack = false;
            currentAction = ATTACK1;
            currentFrame = 0;
            attackCooldownClock.restart();
        }

        if (sf::Keyboard::isKeyPressed(sf::Keyboard::V) && canAttack && !isAttacking) {
            isAttacking = true;
            canAttack = false;
            currentAction = ATTACK2;
            currentFrame = 0;
            attackCooldownClock.restart();
        }

        if (sf::Keyboard::isKeyPressed(sf::Keyboard::Z) && canAttack && !isAttacking) {
            isAttacking = true;
            canAttack = false;
            currentAction = ATTACK3;
            currentFrame = 0;
            attackCooldownClock.restart();
        }

        if (isJumping) {
            character.move(0, verticalVelocity);
            verticalVelocity += gravity;
            if (character.getPosition().y >= groundY) {
                character.setPosition(character.getPosition().x, groundY);
                isJumping = false;
                verticalVelocity = 0;
            }
        }

        sf::FloatRect bounds = character.getGlobalBounds();
        if (bounds.left < 0) character.setPosition(0, character.getPosition().y);
        if (bounds.left + bounds.width > window.getSize().x)
            character.setPosition(window.getSize().x - bounds.width, character.getPosition().y);

        float deltaTime = animClock.restart().asSeconds();
        bgTimer += deltaTime;
        if (bgTimer >= FRAME_DELAY) {
            bgTimer = 0;
            bgFrame = (bgFrame + 1) % GIF_FRAME_COUNT;
            background.setTexture(bgFrames[bgFrame]);
        }

        animTime += deltaTime;

        if (sf::Keyboard::isKeyPressed(sf::Keyboard::C)) {
            isAttacking = false;
            currentAction = SHIELD;
            character.setTexture(shieldTexture);
            character.setTextureRect(sf::IntRect(0, 0, shieldWidth, frameHeight));
        }
        else if (isAttacking) {
            if (animTime >= ATTACK_SPEED) {
                animTime = 0;
                currentFrame++;
                if ((currentAction == ATTACK1 && currentFrame >= ATTACK1_FRAMES) ||
                    (currentAction == ATTACK2 && currentFrame >= ATTACK2_FRAMES) ||
                    (currentAction == ATTACK3 && currentFrame >= ATTACK3_FRAMES)) {
                    isAttacking = false;
                    currentAction = IDLE;
                    currentFrame = 0;
                }
            }

            if (currentAction == ATTACK1) {
                character.setTexture(attack1Texture);
                character.setTextureRect(sf::IntRect(currentFrame * attack1Width, 0, attack1Width, frameHeight));
            }
            else if (currentAction == ATTACK2) {
                character.setTexture(attack2Texture);
                character.setTextureRect(sf::IntRect(currentFrame * attack2Width, 0, attack2Width, frameHeight));
            }
            else if (currentAction == ATTACK3) {
                character.setTexture(attack3Texture);
                character.setTextureRect(sf::IntRect(currentFrame * attack3Width, 0, attack3Width, frameHeight));
            }
        }
        else if (isJumping) {
            if (animTime >= JUMP_SPEED) {
                animTime = 0;
                currentFrame = (currentFrame + 1) % JUMP_FRAMES;
            }
            character.setTexture(jumpTexture);
            character.setTextureRect(sf::IntRect(currentFrame * jumpWidth, 0, jumpWidth, frameHeight));
        }
        else if (isMoving) {
            if (animTime >= RUN_SPEED) {
                animTime = 0;
                currentFrame = (currentFrame + 1) % RUN_FRAMES;
            }
            character.setTexture(runTexture);
            character.setTextureRect(sf::IntRect(currentFrame * runWidth, 0, runWidth, frameHeight));
        }
        else {
            if (animTime >= IDLE_SPEED) {
                animTime = 0;
                currentFrame = (currentFrame + 1) % IDLE_FRAMES;
            }
            character.setTexture(idleTexture);
            character.setTextureRect(sf::IntRect(currentFrame * idleWidth, 0, idleWidth, frameHeight));
        }

        character.setScale(facingRight ? scale : -scale, scale);
        character.setOrigin(facingRight ? 0 : character.getLocalBounds().width, 0);

        window.clear();
        window.draw(background);
        window.draw(character);
        window.display();
    }

    return 0;
}
