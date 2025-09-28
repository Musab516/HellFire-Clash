#pragma once

// --- Game Enums ---
enum class GameStateID { MENU, NAME_INPUT, NAME_INPUT_P2, MODE_SELECTION, CHARACTER_SELECTION, MAP_SELECTION, GAME_PLAY, PAUSE, GAME_OVER };
enum class TransitionState { NONE, FADING_OUT, FADING_IN };
enum class GameMode { PvAI, PvP };

// --- Character Type Enum ---
enum class CharacterTypeID { KNIGHT, ROGUE, SAMURAI };
