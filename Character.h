#pragma once
#include "Enums.h"
#include <string>
#include <map>
#include "GameConfig.h"
#include "ResourceManager.h"
#include "Utils.h"
#include <SFML/Graphics.hpp>

// Forward declaration for Character::Action (enum inside Character class)
class Character;

struct CharacterPreset {
    CharacterTypeID type;
    std::string name;
    std::string titlePath; // Path to title image
    
    // Paths to individual action textures
    std::string idlePath, runPath, jumpPath, attack1Path, attack2Path, attack3Path, shieldPath, hurtPath, deadPath;
    
    // Frame counts for each action
    int idleFrames, runFrames, jumpFrames, attack1Frames, attack2Frames, attack3Frames, shieldFrames, hurtFrames, deadFrames;
    
    // Animation speeds for each action
    float idleSpeed, runSpeed, jumpSpeed, attackSpeed, hurtSpeed, deadSpeed;
    float spriteScale;
};

// Define all character presets
// Note: These assume the textures are in the root directory of the executable.
const std::map<CharacterTypeID, CharacterPreset> AllCharacterPresets = {
    {CharacterTypeID::KNIGHT, {
        CharacterTypeID::KNIGHT, "Knight", "assets/char1_title.png",
        "assets/Idle.png", "assets/Run.png", "assets/Jump.png", "assets/Attack_1.png", "assets/Attack_2.png", "assets/Attack_3.png", "assets/Shield.png", "assets/Hurt.png", "assets/Dead.png",
        6, 8, 10, 4, 3, 4, 2, 3, 3,
        0.15f, 0.08f, 0.1f, 0.1f, GameConfig::HURT_DURATION / 3.f, 0.15f, 2.6f
    }},
    {CharacterTypeID::ROGUE, {
        CharacterTypeID::ROGUE, "Rogue", "assets/Enemy_title.png",
        "assets/Enemy_Idle.png", "assets/Enemy_Run.png", "assets/Enemy_Jump.png", "assets/Enemy_Attack_1.png", "assets/Enemy_Attack_2.png", "assets/Enemy_Attack_3.png", "assets/Enemy_Shield.png", "assets/Enemy_Hurt.png", "assets/Enemy_Dead.png",
        6, 8, 12, 6, 4, 3, 2, 2, 3,
        0.15f, 0.08f, 0.1f, 0.1f, GameConfig::HURT_DURATION / 2.f, 0.15f, 2.5f
    }},
    {CharacterTypeID::SAMURAI, {
        CharacterTypeID::SAMURAI, "Samurai", "assets/S_title.png",
        "assets/S_Idle.png", "assets/S_Run.png", "assets/S_Jump.png", "assets/S_Attack_1.png", "assets/S_Attack_2.png", "assets/S_Attack_3.png", "assets/S_Shield.png", "assets/S_Hurt.png", "assets/S_Dead.png",
        6, 8, 9, 4, 5, 4, 2, 3, 6,
        0.15f, 0.08f, 0.1f, 0.1f, GameConfig::HURT_DURATION / 3.f, 0.15f, 2.7f
    }}
};

// --- Character Base Class (Common properties for Player and Enemy) ---
class Character {
public:
    enum class Action { IDLE, RUN, JUMP, ATTACK1, ATTACK2, ATTACK3, SHIELD, HURT, DEAD };

    // `charType` declared at the top of the class
    CharacterTypeID charType; 

    sf::Sprite sprite;
    Action currentAction = Action::IDLE;
    Action previousAction = Action::IDLE;
    bool facingRight = true;
    bool isJumping = false;
    bool isAttacking = false;
    bool isShielding = false;
    bool isHurt = false;
    bool isAlive = true;
    bool dealtDamageThisAttack = false;
    bool isDamageFlashing = false;
    sf::Clock damageFlashTimer;

    float verticalVelocity = 0.0f;
    int currentFrame = 0;
    float animTime = 0.0f;
    sf::Clock attackCooldownClock;
    bool canAttack = true;
    sf::Clock hurtClock;

    float maxHealth = GameConfig::MAX_HEALTH;
    float currentHealth = GameConfig::MAX_HEALTH;
    std::string name;

    // Character-specific textures (loaded dynamically via preset)
    sf::Texture texIdle, texRun, texJump;
    sf::Texture texAttack1, texAttack2, texAttack3, texShield;
    sf::Texture texHurt, texDead;

    // Animation frame counts and speeds (set dynamically via preset)
    int idleFrames, runFrames, jumpFrames;
    int attack1Frames, attack2Frames, attack3Frames, shieldFrames;
    int hurtFrames, deadFrames;

    float idleSpeed, runSpeed, jumpSpeed, attackSpeed;
    float hurtSpeed, deadSpeed;

    // Frame widths (calculated dynamically via preset) and common frame height
    int idleWidth, runWidth, jumpWidth, attack1Width, attack2Width, attack3Width, shieldWidth, hurtWidth, deadWidth, frameHeight;
    float spriteScale; // Sprite scaling factor (set dynamically via preset)
    float groundY; // Y-coordinate of the ground level

    Character() {}

    // This method loads all character-specific assets based on the provided type
    void loadCharacterAssets(CharacterTypeID type) {
        charType = type;
        const CharacterPreset& preset = AllCharacterPresets.at(type);
        
        name = preset.name;
        spriteScale = preset.spriteScale;

        bool loaded = true;
        loaded &= ResourceManager::loadTexture(texIdle, preset.idlePath);
        loaded &= ResourceManager::loadTexture(texRun, preset.runPath);
        loaded &= ResourceManager::loadTexture(texJump, preset.jumpPath);
        loaded &= ResourceManager::loadTexture(texAttack1, preset.attack1Path);
        loaded &= ResourceManager::loadTexture(texAttack2, preset.attack2Path);
        loaded &= ResourceManager::loadTexture(texAttack3, preset.attack3Path);
        loaded &= ResourceManager::loadTexture(texShield, preset.shieldPath);
        loaded &= ResourceManager::loadTexture(texHurt, preset.hurtPath);
        loaded &= ResourceManager::loadTexture(texDead, preset.deadPath);

        if (!loaded) { 
            std::cerr << "CRITICAL: Failed to load one or more " << preset.name << " character textures." << std::endl; 
        }

        // Set frame counts and speeds from preset
        idleFrames = preset.idleFrames;
        runFrames = preset.runFrames;
        jumpFrames = preset.jumpFrames;
        attack1Frames = preset.attack1Frames;
        attack2Frames = preset.attack2Frames;
        attack3Frames = preset.attack3Frames;
        shieldFrames = preset.shieldFrames;
        hurtFrames = preset.hurtFrames;
        deadFrames = preset.deadFrames;

        idleSpeed = preset.idleSpeed;
        runSpeed = preset.runSpeed;
        jumpSpeed = preset.jumpSpeed;
        attackSpeed = preset.attackSpeed;
        hurtSpeed = preset.hurtSpeed;
        deadSpeed = preset.deadSpeed;

        // Calculate frame widths based on loaded textures and frame counts
        if (texIdle.getSize().y > 0) frameHeight = texIdle.getSize().y; else frameHeight = 100; 

        idleWidth = texIdle.getSize().x > 0 ? texIdle.getSize().x / idleFrames : 100;
        runWidth = texRun.getSize().x > 0 ? texRun.getSize().x / runFrames : 100;
        jumpWidth = texJump.getSize().x > 0 ? texJump.getSize().x / jumpFrames : 100;
        attack1Width = texAttack1.getSize().x > 0 ? texAttack1.getSize().x / attack1Frames : 100;
        attack2Width = texAttack2.getSize().x > 0 ? texAttack2.getSize().x / attack2Frames : 100;
        attack3Width = texAttack3.getSize().x > 0 ? texAttack3.getSize().x / attack3Frames : 100;
        shieldWidth = texShield.getSize().x > 0 ? texShield.getSize().x / shieldFrames : 100;
        hurtWidth = texHurt.getSize().x > 0 ? texHurt.getSize().x / hurtFrames : 100;
        deadWidth = texDead.getSize().x > 0 ? texDead.getSize().x / deadFrames : 100;

        setupSprite(); 
    }

    void setupSprite() {
        sprite.setTexture(texIdle);
        sprite.setTextureRect(sf::IntRect(0, 0, idleWidth, frameHeight));
        sprite.setScale(spriteScale, spriteScale);
    }

    void resetPosition(float xPos) {
        sprite.setPosition(xPos, groundY);
    }


    void setGroundY(float newGroundY) {
        groundY = newGroundY;
        if (!isJumping && isAlive) {
            sprite.setPosition(sprite.getPosition().x, groundY);
        }
    }

    virtual void update(float dt, float windowWidth, Character* opponent = nullptr) {
        previousAction = currentAction;

        if (!isAlive) {
            currentAction = Action::DEAD;
        } else if (isHurt) {
            currentAction = Action::HURT;
            if (hurtClock.getElapsedTime().asSeconds() >= hurtSpeed * hurtFrames) {
                isHurt = false;
                currentAction = Action::IDLE;
            }
        }

        if (isDamageFlashing) {
            if (damageFlashTimer.getElapsedTime().asSeconds() >= GameConfig::DAMAGE_FLASH_DURATION) {
                isDamageFlashing = false;
                sprite.setColor(sf::Color::White); 
            }
        }

        if (isAlive && !isHurt) {
            if (!canAttack && attackCooldownClock.getElapsedTime().asSeconds() > 0.8f) { 
                canAttack = true;
            }

            if (isJumping) {
                verticalVelocity += GameConfig::GRAVITY * dt * 60.f; 
                sprite.move(0, verticalVelocity * dt * 60.f); 

                if (sprite.getPosition().y >= groundY) { 
                    sprite.setPosition(sprite.getPosition().x, groundY);
                    isJumping = false;
                    verticalVelocity = 0;
                    if (!isAttacking && !isShielding && currentAction == Action::JUMP) {
                        currentAction = Action::IDLE;
                    }
                } else {
                     if (!isAttacking && currentAction != Action::HURT) currentAction = Action::JUMP; 
                }
            }
        }

        if (previousAction != currentAction) { 
            currentFrame = 0;
            animTime = 0.0f;
        }

        sf::FloatRect bounds = sprite.getGlobalBounds();
        if (bounds.left < 0) sprite.setPosition(0, sprite.getPosition().y);
        if (bounds.left + bounds.width > windowWidth) {
            sprite.setPosition(windowWidth - bounds.width, sprite.getPosition().y);
        }

        updateAnimationFrame(dt); 

        sprite.setScale(facingRight ? spriteScale : -spriteScale, spriteScale);
        if (sprite.getTexture()) { 
             sf::FloatRect localBounds = sprite.getLocalBounds();
             sprite.setOrigin(facingRight ? 0 : localBounds.width, 0);
        }
    }

    virtual void handleInput() { /* Player specific input handling */ }

    virtual void updateAnimationFrame(float dt) {
        float speed = 0.f;
        int maxFrames = 0;
        int currentTextureWidth = 0;
        sf::Texture* currentTexture = nullptr;

        Action actionToAnimate = currentAction;

        // Use character-specific frame counts and speeds based on current loaded preset
        switch (actionToAnimate) {
            case Action::IDLE:
                speed = idleSpeed; maxFrames = idleFrames;
                currentTexture = &texIdle; currentTextureWidth = idleWidth;
                break;
            case Action::RUN:
                speed = runSpeed; maxFrames = runFrames;
                currentTexture = &texRun; currentTextureWidth = runWidth;
                break;
            case Action::JUMP:
                speed = jumpSpeed; maxFrames = jumpFrames;
                currentTexture = &texJump; currentTextureWidth = jumpWidth;
                break;
            case Action::ATTACK1:
                speed = attackSpeed; maxFrames = attack1Frames;
                currentTexture = &texAttack1; currentTextureWidth = attack1Width;
                break;
            case Action::ATTACK2:
                speed = attackSpeed; maxFrames = attack2Frames;
                currentTexture = &texAttack2; currentTextureWidth = attack2Width;
                break;
            case Action::ATTACK3:
                speed = attackSpeed; maxFrames = attack3Frames;
                currentTexture = &texAttack3; currentTextureWidth = attack3Width;
                break;
            case Action::SHIELD:
                speed = idleSpeed; maxFrames = shieldFrames; 
                currentTexture = &texShield; currentTextureWidth = shieldWidth;
                break;
            case Action::HURT:
                speed = hurtSpeed; maxFrames = hurtFrames;
                currentTexture = &texHurt; currentTextureWidth = hurtWidth;
                break;
            case Action::DEAD:
                speed = deadSpeed; maxFrames = deadFrames;
                currentTexture = &texDead; currentTextureWidth = deadWidth;
                break;
        }

        animTime += dt;
        if (animTime >= speed && maxFrames > 0) { 
            animTime = 0;
            currentFrame++;

            if (actionToAnimate == Action::DEAD) {
                if (currentFrame >= maxFrames) {
                     currentFrame = maxFrames -1;
                }
            } else if (actionToAnimate == Action::HURT) {
                if (currentFrame >= maxFrames) {
                    currentFrame = maxFrames - 1; 
                }
            } else if (isAttacking && (currentAction == Action::ATTACK1 || currentAction == Action::ATTACK2 || currentAction == Action::ATTACK3)) {
                if (currentFrame >= maxFrames) {
                    isAttacking = false; 
                    if (!isHurt && isAlive) {
                        currentAction = Action::IDLE; 
                        currentFrame = 0;
                    }
                }
            } else if (currentFrame >= maxFrames) {
                currentFrame = 0; 
                if (actionToAnimate == Action::JUMP && isJumping) { 
                    currentFrame = std::min(currentFrame, maxFrames -1); 
                }
            }
        }

        if (currentTexture && currentTexture->getSize().x > 0 && currentTextureWidth > 0) {
            sprite.setTexture(*currentTexture);
            sprite.setTextureRect(sf::IntRect(currentFrame * currentTextureWidth, 0,
                                            currentTextureWidth, frameHeight));
        }
    }

    virtual void takeDamage(float damage) {
        if (!isAlive || isShielding) return; 

        currentHealth -= damage;
        isHurt = true;
        hurtClock.restart(); 

        isDamageFlashing = true;
        damageFlashTimer.restart();
        sprite.setColor(sf::Color(255, 100, 100, 220)); 

        isAttacking = false; 

        if (currentHealth <= 0) {
            currentHealth = 0;
            isAlive = false;
            isHurt = false; 
        }
    }

    // Calculates the bounding box for the character's attack
    sf::FloatRect getAttackHitbox() const {
        if (!isAttacking) return sf::FloatRect(); 

        sf::FloatRect spriteBounds = sprite.getGlobalBounds();
        float hitboxWidth = 70.f;
        float hitboxHeight = spriteBounds.height * 0.7f; 
        float hitboxY = spriteBounds.top + spriteBounds.height * 0.15f; 

        float reachOffset = 10.f; 
        float forwardProjection = spriteBounds.width * 0.3f; 

        float hitboxX;
        if (facingRight) {
            hitboxX = spriteBounds.left + spriteBounds.width - forwardProjection + reachOffset;
        } else {
            hitboxX = spriteBounds.left + forwardProjection - hitboxWidth - reachOffset;
        }
        return sf::FloatRect(hitboxX, hitboxY, hitboxWidth, hitboxHeight);
    }

    // Calculates the character's "hurtbox" (collidable body area)
    sf::FloatRect getHurtbox() const {
        float widthRatio = 0.35f; 
        float heightRatio = 0.8f; 

        float xOffsetRatio = (1.0f - widthRatio) / 2.0f;
        float yOffsetRatio = 0.1f; 

        sf::FloatRect globalBounds = sprite.getGlobalBounds();

        float actualWidth = globalBounds.width * widthRatio;
        float actualHeight = globalBounds.height * heightRatio;

        float xPos, yPos;

        yPos = globalBounds.top + globalBounds.height * yOffsetRatio;

        if (facingRight) {
            xPos = globalBounds.left + globalBounds.width * xOffsetRatio;
        } else {
            xPos = globalBounds.left + globalBounds.width - (globalBounds.width * xOffsetRatio) - actualWidth;
        }

        return sf::FloatRect(xPos, yPos, actualWidth, actualHeight);
    }


    virtual void reset() {
        currentAction = Action::IDLE;
        previousAction = Action::IDLE;
        isJumping = false;
        isAttacking = false;
        isShielding = false;
        isHurt = false;
        isAlive = true;
        dealtDamageThisAttack = false;
        isDamageFlashing = false;
        sprite.setColor(sf::Color::White);
        verticalVelocity = 0;
        currentFrame = 0;
        animTime = 0;
        canAttack = true;
        attackCooldownClock.restart();
        currentHealth = maxHealth;
        
        // Reset sprite texture and rect based on current loaded type
        setupSprite(); 
    }

    virtual void draw(sf::RenderWindow& window) const {
        window.draw(sprite);
    }
    virtual ~Character() {
        // Destructor
        // No dynamic memory allocation, so nothing to clean up here.
    }
};

// --- Player Class ---
class Player : public Character {
public:
    Player() : Character() {
        // Default to Knight initially. This is called only once at game start.
        // Actual character loading happens in Game::handleScreenTransition.
        loadCharacterAssets(CharacterTypeID::KNIGHT);
        name = "Player 1"; // Initial name, will be set by input screen
    }

    void update(float dt, float windowWidth, Character* opponent = nullptr) override {
        if (!isAlive || isHurt) { 
             Character::update(dt, windowWidth, opponent);
             return;
        }

        bool isMoving = false;

        if (!isAttacking && !isShielding) { 
            float moveSpeed = GameConfig::MOVEMENT_SPEED * (sf::Keyboard::isKeyPressed(sf::Keyboard::LShift) ?
                             GameConfig::RUN_BOOST_MULTIPLIER : 1.f) * dt * 60.f;

            if (sf::Keyboard::isKeyPressed(sf::Keyboard::A)) {
                sprite.move(-moveSpeed, 0);
                isMoving = true;
                facingRight = false;
            }
            if (sf::Keyboard::isKeyPressed(sf::Keyboard::D)) {
                sprite.move(moveSpeed, 0);
                isMoving = true;
                facingRight = true;
            }
            if (sf::Keyboard::isKeyPressed(sf::Keyboard::W) && !isJumping) {
                isJumping = true;
                verticalVelocity = GameConfig::JUMP_STRENGTH; 
            }
        }

        if (!isAttacking && !isShielding) {
            if (isJumping) currentAction = Action::JUMP;
            else if (isMoving) currentAction = Action::RUN;
            else currentAction = Action::IDLE;
        } else if (isShielding) {
            currentAction = Action::SHIELD;
        }

        Character::update(dt, windowWidth, opponent); 
    }


    void handleInput() override {
        if (!isAlive || isHurt) return; 

        if (sf::Keyboard::isKeyPressed(sf::Keyboard::T)) {
            if (!isAttacking) {
                isShielding = true;
                currentAction = Action::SHIELD;
            }
        } else if (isShielding) {
            isShielding = false;
        }

        if (!isShielding && canAttack && !isAttacking) {
            Action attackAttempt = Action::IDLE;
            if (sf::Keyboard::isKeyPressed(sf::Keyboard::F)) attackAttempt = Action::ATTACK1;
            else if (sf::Keyboard::isKeyPressed(sf::Keyboard::G)) attackAttempt = Action::ATTACK2;
            else if (sf::Keyboard::isKeyPressed(sf::Keyboard::H)) attackAttempt = Action::ATTACK3;

            if (attackAttempt != Action::IDLE) { 
                isAttacking = true;
                dealtDamageThisAttack = false; 
                canAttack = false; 
                currentAction = attackAttempt;
                currentFrame = 0; 
                animTime = 0;
                attackCooldownClock.restart(); 
            }
        }
    }

    void reset() override {
        Character::reset();
    }
    ~Player() {
        // Destructor
        // No dynamic memory allocation, so nothing to clean up here.
    }
};


class Enemy : public Character {
public:
    bool isPlayerControlled = false;

    float detectionRange = 450.0f;
    float optimalAttackRangeMin = GameConfig::ATTACK_RANGE * 0.3f; 
    float optimalAttackRangeMax = GameConfig::ATTACK_RANGE * 0.7f; 
    sf::Clock aiDecisionClock;
    float aiDecisionInterval = 0.15f; 
    bool isActivelyChasing = false; 

    Enemy() : Character() {
        // Default to Rogue initially. Actual character loading happens in Game::handleScreenTransition.
        loadCharacterAssets(CharacterTypeID::ROGUE);
        name = "Rival"; 
        facingRight = false; 
        sprite.setScale(spriteScale, spriteScale); 
    }

    void handlePlayer2Input() {
        if (!isAlive || isHurt) return; 

        if (sf::Keyboard::isKeyPressed(sf::Keyboard::Numpad0)) { 
            if (!isAttacking) {
                isShielding = true;
                currentAction = Action::SHIELD;
            }
        } else if (isShielding) {
            isShielding = false;
        }

        if (!isShielding && canAttack && !isAttacking) { 
            Action attackAttempt = Action::IDLE;
            if (sf::Keyboard::isKeyPressed(sf::Keyboard::Numpad1)) attackAttempt = Action::ATTACK1; 
            else if (sf::Keyboard::isKeyPressed(sf::Keyboard::Numpad2)) attackAttempt = Action::ATTACK2; 
            else if (sf::Keyboard::isKeyPressed(sf::Keyboard::Numpad3)) attackAttempt = Action::ATTACK3; 

            if (attackAttempt != Action::IDLE) {
                isAttacking = true;
                dealtDamageThisAttack = false;
                canAttack = false;
                currentAction = attackAttempt;
                currentFrame = 0;
                animTime = 0;
                attackCooldownClock.restart();
            }
        }
    }


    void update(float dt, float windowWidth, Character* playerPtr) override {
        if (!isAlive || isHurt) { 
             Character::update(dt, windowWidth, playerPtr);
             return;
        }

        if (isPlayerControlled) {
            bool isMoving = false;
            if (!isAttacking && !isShielding) { 
                float moveSpeed = GameConfig::MOVEMENT_SPEED * (sf::Keyboard::isKeyPressed(sf::Keyboard::RShift) ?
                                 GameConfig::RUN_BOOST_MULTIPLIER : 1.f) * dt * 60.f;

                if (sf::Keyboard::isKeyPressed(sf::Keyboard::Left)) {
                    sprite.move(-moveSpeed, 0); isMoving = true; facingRight = false;
                }
                if (sf::Keyboard::isKeyPressed(sf::Keyboard::Right)) {
                    sprite.move(moveSpeed, 0); isMoving = true; facingRight = true;
                }
                if (sf::Keyboard::isKeyPressed(sf::Keyboard::Up) && !isJumping) {
                    isJumping = true; verticalVelocity = GameConfig::JUMP_STRENGTH;
                }
            }

            if (!isAttacking && !isShielding) {
                if (isJumping) currentAction = Action::JUMP;
                else if (isMoving) currentAction = Action::RUN;
                else currentAction = Action::IDLE;
            } else if (isShielding) {
                currentAction = Action::SHIELD;
            }

        } else { // AI Logic
            if (playerPtr && playerPtr->isAlive) { 
                if (aiDecisionClock.getElapsedTime().asSeconds() > aiDecisionInterval) {
                    aiDecisionClock.restart();
                    float distToPlayer = Utils::distance(sprite.getPosition(), playerPtr->sprite.getPosition());

                    if (!isAttacking && !isShielding) { 
                        if (distToPlayer <= optimalAttackRangeMax && distToPlayer >= optimalAttackRangeMin && canAttack) {
                            isAttacking = true;
                            dealtDamageThisAttack = false;
                            canAttack = false;
                            currentAction = Action::ATTACK1; 
                            isActivelyChasing = false; 
                        } else if (distToPlayer < detectionRange) {
                            isActivelyChasing = true;
                        } else {
                            isActivelyChasing = false;
                        }
                    }
                }

                if (isAttacking) {
                } else if (isActivelyChasing && !isShielding) { 
                    currentAction = Action::RUN;
                    float moveSpeed = GameConfig::MOVEMENT_SPEED * 0.7f * dt * 60.f; 

                    if (playerPtr->sprite.getPosition().x < sprite.getPosition().x - optimalAttackRangeMin * 0.5f) {
                        sprite.move(-moveSpeed, 0);
                        facingRight = false;
                    } else if (playerPtr->sprite.getPosition().x > sprite.getPosition().x + optimalAttackRangeMin * 0.5f) {
                        sprite.move(moveSpeed, 0);
                        facingRight = true;
                    } else {
                    }
                    if (playerPtr->sprite.getPosition().x < sprite.getPosition().x && facingRight) facingRight = false;
                    else if (playerPtr->sprite.getPosition().x > sprite.getPosition().x && !facingRight) facingRight = true;

                } else if (!isShielding) { 
                    currentAction = Action::IDLE;
                }
            } else { 
                isActivelyChasing = false;
                if (!isAttacking && !isShielding) currentAction = Action::IDLE;
            }
        }
        Character::update(dt, windowWidth, playerPtr); 
    }

    // Enemy::reset(float xPos) override -> Corrected to match Character::reset() signature
    void reset() override {
        Character::reset(); // Call base class reset
        // The xPos will be set by Game::handleScreenTransition directly on the sprite
        // after calling this reset, so no xPos param here.
        isActivelyChasing = false;
        // Ensure sprite texture is set back to Idle after reset
        setupSprite(); // Call setupSprite to apply texIdle and rect
    }
    ~Enemy() override {
        // Destructor
        // No dynamic memory allocation, so nothing to clean up here.
    }
};
