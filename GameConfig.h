#pragma once

// --- Configuration Constants ---
namespace GameConfig {
    // Virtual resolution which the game logic and UI elements are designed for (16:9)
    unsigned int WINDOW_WIDTH = 1280;
    unsigned int WINDOW_HEIGHT = 720;
    const unsigned int FRAMERATE_LIMIT = 60;
    const unsigned int MAX_NAME_LENGTH = 15;
    const float RUN_BOOST_MULTIPLIER = 1.30f;
    const float TRANSITION_DURATION = 0.35f;
    const float INPUT_BOX_PADDING = 15.0f;
    const float JUMP_STRENGTH = -14.0f;
    const float GRAVITY = 0.6f;
    const float MOVEMENT_SPEED = 5.0f;
    const float ATTACK_RANGE = 90.0f; // This is for AI decision, not directly hitbox size
    const float ATTACK_DAMAGE = 12.0f;
    const float MAX_HEALTH = 250.0f;
    const float HURT_DURATION = 0.4f;
    const float DAMAGE_FLASH_DURATION = 0.2f;
    const float DAMAGE_TEXT_LIFETIME = 0.7f;
    const float DAMAGE_TEXT_SPEED = -50.f;
    const float SCREEN_SHAKE_MAX_OFFSET = 5.0f;
    const float SCREEN_SHAKE_DURATION = 0.15f;

    const float GAME_ROUND_DURATION = 120.0f; // 2 minutes (120 seconds)
}
