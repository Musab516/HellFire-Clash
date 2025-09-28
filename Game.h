#include <SFML/Graphics.hpp>
#include "Character.h"
#include "DamageText.h"
#include "ResourceManager.h"

class Game; // Forward declaration of Game class
// --- Screen Base Class ---
class Screen {
public:
    virtual ~Screen() = default;
    // gamePtr is a pointer to the Game instance, allowing screens to interact with game state.
    virtual void handleEvent(sf::Event& event, sf::RenderWindow& window, GameStateID& nextState, bool& wantsTransition, GameStateID& gameResultState, Game* gamePtr) = 0;
    virtual void update(sf::Time dt, sf::Vector2f mousePos, Player& player, Enemy& enemy, GameStateID& gameResultState, Game* gamePtr = nullptr) = 0;
    virtual void draw(sf::RenderWindow& window, const Player& player, const Enemy& enemy) = 0;
    // onEnter method, with `data` for context specific information (e.g., game outcome in GameOverScreen)
    virtual void onEnter(const sf::RenderWindow& window, Player& player, Enemy& enemy, const std::string& data = "") {}
    virtual void onExit() {}
    // onResize method, now takes virtual width/height for layout calculations
    virtual void onResize(unsigned int width, unsigned int height, Player& player, Enemy& enemy) = 0;
};


// --- Game Class Definition (Moved up for "incomplete type" error) ---
class Game {
public:
    sf::RenderWindow window;
    std::map<GameStateID, std::unique_ptr<Screen>> screens;
    GameStateID currentStateID;
    GameStateID nextStateID;
    GameStateID gameResultState = GameStateID::GAME_PLAY; // Flag for game over condition check in Game::run
    GameMode currentMode = GameMode::PvAI;

    TransitionState currentTransition;
    sf::Clock transitionClock;
    sf::RectangleShape transitionRect;

    float gameTimeScale = 1.0f;
    sf::Clock gameClock;

    Player player;
    Enemy enemy;
    std::string playerNameFromInput;
    std::string player2NameFromInput;

    // Character selection storage
    CharacterTypeID selectedPlayer1Char = CharacterTypeID::KNIGHT;
    CharacterTypeID selectedEnemyChar = CharacterTypeID::ROGUE;

    std::vector<sf::Texture> map1Frames, map2Frames, map3Frames; // Added map3Frames
    bool map1Loaded = false, map2Loaded = false, map3Loaded = false; // Added map3Loaded
    int currentMapSelection = 0; // 0 for none, 1 for Map 1, 2 for Map 2, 3 for Map 3
    sf::Sprite gameBackgroundSprite;
    std::vector<sf::Texture>* currentGameBackgroundFrames = nullptr; // Pointer to currently active map frames
    int bgFrame = 0; // Current background animation frame
    float bgTimer = 0.f; // Timer for background animation
    const float MAP_FRAME_DELAY = 0.11f;

    bool isShaking = false;
    sf::Clock shakeClock;
    sf::Vector2f shakeOffset;

    Game();
    void run();
    void triggerScreenShake();


private:
    void processEvents();
    void update(sf::Time dt);
    void render();
    void changeScreen(GameStateID newStateID, const std::string& onEnterData = ""); // Added optional data for onEnter
    void handleScreenTransition(sf::Time dt);
    void handleResize(unsigned int width, unsigned int height);
    void updateScreenShake(sf::Time dt);
};


// --- ModeSelectionScreen ---
class ModeSelectionScreen : public Screen {
public:
    sf::Text promptText;
    sf::RectangleShape pvaButton, pvpButton;
    sf::Text pvaText, pvpText;
    sf::RenderWindow& m_windowRef;

    sf::Color defaultBtnColor = sf::Color::Black;
    sf::Color hoverBtnColor = sf::Color(128, 128, 128); // grey

    std::vector<sf::Texture> bgFrames;
    sf::Sprite background;
    int currentBgFrame = 0;
    float bgFrameTimer = 0.f;
    const float BG_FRAME_DELAY = 0.08f;


    ModeSelectionScreen(sf::RenderWindow& window) : m_windowRef(window) {
        promptText.setFont(ResourceManager::getFont("ariblk.ttf"));
        promptText.setString("SELECT GAME MODE:");
        promptText.setCharacterSize(55);
        promptText.setFillColor(sf::Color(255, 165, 0)); // Orange fill
        promptText.setOutlineColor(sf::Color::Black);   // Black outline
        promptText.setOutlineThickness(3);              // Prominent outline
        Utils::centerOrigin(promptText);

        sf::Vector2f buttonSize(400, 100); // Consistent button size
        pvaButton.setSize(buttonSize);
        pvaButton.setOutlineColor(sf::Color::White);
        pvaButton.setOutlineThickness(3);
        pvaText.setFont(ResourceManager::getFont("ariblk.ttf"));
        pvaText.setString("Player vs AI");
        pvaText.setCharacterSize(40);
        pvaText.setFillColor(sf::Color(255, 215, 0)); // Golden fill
        pvaText.setOutlineColor(sf::Color::Black);   // Black outline
        pvaText.setOutlineThickness(2);              // Prominent outline
        Utils::centerOrigin(pvaText);

        pvpButton.setSize(buttonSize);
        pvpButton.setOutlineColor(sf::Color::White);
        pvpButton.setOutlineThickness(3);
        pvpText.setFont(ResourceManager::getFont("ariblk.ttf"));
        pvpText.setString("Player vs Player");
        pvpText.setCharacterSize(40);
        pvpText.setFillColor(sf::Color(255, 215, 0)); // Golden fill
        pvpText.setOutlineColor(sf::Color::Black);   // Black outline
        pvpText.setOutlineThickness(2);              // Prominent outline
        Utils::centerOrigin(pvpText);

        ResourceManager::loadMenuBackgroundFrames(bgFrames, 12);
        if (!bgFrames.empty()) {
            background.setTexture(bgFrames[0]);
        }
    }

    void onEnter(const sf::RenderWindow& window, Player& playerRef, Enemy& enemyRef, const std::string& data) override {
        bgFrameTimer = 0.f; currentBgFrame = 0;
        if (!bgFrames.empty()) background.setTexture(bgFrames[currentBgFrame]);
        // Pass virtual resolution to onResize
        onResize(GameConfig::WINDOW_WIDTH, GameConfig::WINDOW_HEIGHT, playerRef, enemyRef);
    }

    void onResize(unsigned int width, unsigned int height, Player& playerRef, Enemy& enemyRef) override {
        promptText.setPosition(width / 2.0f, height * 0.2f);

        float buttonYStart = height * 0.4f;
        float buttonGap = 40;

        pvaButton.setPosition(width / 2.0f - pvaButton.getSize().x / 2.0f, buttonYStart);
        pvaText.setPosition(pvaButton.getPosition().x + pvaButton.getSize().x / 2.0f, pvaButton.getPosition().y + pvaButton.getSize().y / 2.0f);

        pvpButton.setPosition(width / 2.0f - pvpButton.getSize().x / 2.0f, buttonYStart + pvaButton.getSize().y + buttonGap);
        pvpText.setPosition(pvpButton.getPosition().x + pvpButton.getSize().x / 2.0f, pvpButton.getPosition().y + pvpButton.getSize().y / 2.0f);

        if (background.getTexture()) {
            background.setOrigin(0,0);
            background.setScale(
                static_cast<float>(width) / background.getTexture()->getSize().x,
                static_cast<float>(height) / background.getTexture()->getSize().y
            );
        }
    }

    void handleEvent(sf::Event& event, sf::RenderWindow& window, GameStateID& nextState, bool& wantsTransition, GameStateID& gameResultState, Game* gamePtr) override {
        if (event.type == sf::Event::MouseButtonPressed) {
            if (gamePtr) {
                sf::Vector2f mousePos = sf::Vector2f(sf::Mouse::getPosition(window)); // Get mouse in pixel coords
                // Transform mouse position to game coordinates for accurate hit detection
                sf::View gameView = window.getView();
                mousePos = window.mapPixelToCoords(sf::Vector2i(mousePos), gameView);

                if (pvaButton.getGlobalBounds().contains(mousePos)) {
                    gamePtr->currentMode = GameMode::PvAI;
                    nextState = GameStateID::CHARACTER_SELECTION; // Go to character selection
                    wantsTransition = true;
                } else if (pvpButton.getGlobalBounds().contains(mousePos)) {
                    gamePtr->currentMode = GameMode::PvP;
                    nextState = GameStateID::CHARACTER_SELECTION; // Go to character selection
                    wantsTransition = true;
                }
            }
        } else if (event.type == sf::Event::KeyPressed && event.key.code == sf::Keyboard::Escape) {
            nextState = GameStateID::NAME_INPUT; // Go back to Player 1 name input
            wantsTransition = true;
        }
    }

    void update(sf::Time dt, sf::Vector2f mousePos, Player& playerRef, Enemy& enemyRef, GameStateID& gameResultState, Game* gamePtr) override {
        // Transform mouse position to game coordinates for hover effect
        sf::View gameView = m_windowRef.getView();
        mousePos = m_windowRef.mapPixelToCoords(sf::Vector2i(mousePos), gameView); // Ensure mousePos is sf::Vector2i for mapPixelToCoords

        pvaButton.setFillColor(pvaButton.getGlobalBounds().contains(mousePos) ? hoverBtnColor : defaultBtnColor);
        pvpButton.setFillColor(pvpButton.getGlobalBounds().contains(mousePos) ? hoverBtnColor : defaultBtnColor);

        if (!bgFrames.empty() && bgFrames.size() > 1) {
            bgFrameTimer += dt.asSeconds();
            if (bgFrameTimer >= BG_FRAME_DELAY) {
                bgFrameTimer -= BG_FRAME_DELAY;
                currentBgFrame = (currentBgFrame + 1) % bgFrames.size();
                background.setTexture(bgFrames[currentBgFrame]);
            }
        }
    }

    void draw(sf::RenderWindow& window, const Player& playerRef, const Enemy& enemyRef) override {
        if(background.getTexture()) window.draw(background);
        else window.clear(sf::Color(20, 20, 40));

        window.draw(promptText);
        window.draw(pvaButton); window.draw(pvaText);
        window.draw(pvpButton); window.draw(pvpText);
    }
};


// --- MenuScreen ---
class MenuScreen : public Screen {
public:
    sf::Text title;
    sf::Text pressStart;
    std::vector<sf::Texture> bgFrames;
    sf::Sprite background;
    int currentFrame = 0;
    float frameTimer = 0.f;
    const float FRAME_DELAY = 0.08f;
    sf::Clock menuAnimClock;
    sf::RenderWindow& m_window;


    MenuScreen(sf::RenderWindow& window) : m_window(window) {
        title.setFont(ResourceManager::getFont("ariblk.ttf"));
        title.setString("HELLFIRE-CLASH");
        title.setCharacterSize(110);
        title.setFillColor(sf::Color(255, 69, 0));
        title.setOutlineColor(sf::Color::Black);
        title.setOutlineThickness(6);
        title.setStyle(sf::Text::Bold | sf::Text::Italic);
        Utils::centerOrigin(title);

        pressStart.setFont(ResourceManager::getFont("ariblk.ttf"));
        pressStart.setString("PRESS ENTER");
        pressStart.setCharacterSize(55);
        pressStart.setFillColor(sf::Color(255, 215, 0)); // Golden fill
        pressStart.setOutlineColor(sf::Color::Black);   // Black outline
        pressStart.setOutlineThickness(3);              // Prominent outline
        Utils::centerOrigin(pressStart);

        ResourceManager::loadMenuBackgroundFrames(bgFrames, 12);
        if (!bgFrames.empty()) {
            background.setTexture(bgFrames[0]);
        }
    }

    void onResize(unsigned int width, unsigned int height, Player& playerRef, Enemy& enemyRef) override {
        title.setPosition(width / 2.0f, height * 0.25f);
        pressStart.setPosition(width / 2.0f, height * 0.7f);
        if (background.getTexture()) {
            background.setOrigin(0,0);
            background.setScale(
                static_cast<float>(width) / background.getTexture()->getSize().x,
                static_cast<float>(height) / background.getTexture()->getSize().y
            );
        }
    }

    void onEnter(const sf::RenderWindow& window, Player& playerRef, Enemy& enemyRef, const std::string& data) override {
        menuAnimClock.restart();
        frameTimer = 0.f;
        currentFrame = 0;
        if (!bgFrames.empty()) background.setTexture(bgFrames[0]);
        // Pass virtual resolution to onResize
        onResize(GameConfig::WINDOW_WIDTH, GameConfig::WINDOW_HEIGHT, playerRef, enemyRef);
    }


    void handleEvent(sf::Event& event, sf::RenderWindow& window, GameStateID& nextState, bool& wantsTransition, GameStateID& gameResultState, Game* gamePtr) override {
        if (event.type == sf::Event::KeyPressed && event.key.code == sf::Keyboard::Enter) {
            nextState = GameStateID::NAME_INPUT;
            wantsTransition = true;
        }
    }

    void update(sf::Time dt, sf::Vector2f mousePos, Player& playerRef, Enemy& enemyRef, GameStateID& gameResultState, Game* gamePtr = nullptr) override {
        float time = menuAnimClock.getElapsedTime().asSeconds();
        // Use GameConfig::WINDOW_WIDTH/HEIGHT as the base for positioning
        title.setPosition(GameConfig::WINDOW_WIDTH / 2.0f, GameConfig::WINDOW_HEIGHT * 0.25f + 12.f * std::sin(time * 2.8f));
        float pulse = 0.95f + 0.05f * std::sin(time * 5.0f);
        pressStart.setScale(pulse, pulse);
        pressStart.setPosition(GameConfig::WINDOW_WIDTH / 2.0f, GameConfig::WINDOW_HEIGHT * 0.7f + 10.f * std::cos(time * 2.2f));

        if (!bgFrames.empty() && bgFrames.size() > 1) {
            frameTimer += dt.asSeconds();
            if (frameTimer >= FRAME_DELAY) {
                frameTimer -= FRAME_DELAY;
                currentFrame = (currentFrame + 1) % bgFrames.size();
                background.setTexture(bgFrames[currentFrame]);
            }
        }
    }

    void draw(sf::RenderWindow& window, const Player& playerRef, const Enemy& enemyRef) override {
        if(background.getTexture()) window.draw(background);
        window.draw(title);
        window.draw(pressStart);
    }
};

// --- NameInputScreen ---
class NameInputScreen : public Screen {
public:
    sf::Text promptText;
    sf::RectangleShape inputBox;
    sf::Text nameDisplay;
    sf::Text nameLengthHint;
    sf::Text cursorText;
    sf::Text continueText;

    std::string& playerNameRef;
    std::string promptPrefix;
    bool isActive = true;
    sf::Clock cursorBlinkClock;
    bool showCursor = true;

    std::vector<sf::Texture> bgFrames;
    sf::Sprite background;
    int currentBgFrame = 0;
    float bgFrameTimer = 0.f;
    const float BG_FRAME_DELAY = 0.08f;
    sf::RenderWindow& m_windowRef;


    NameInputScreen(sf::RenderWindow& window, std::string& playerNameOutput, const std::string& prefix)
        : m_windowRef(window), playerNameRef(playerNameOutput), promptPrefix(prefix) {
        promptText.setFont(ResourceManager::getFont("ariblk.ttf"));
        promptText.setCharacterSize(50);
        promptText.setFillColor(sf::Color(255, 165, 0)); // Orange fill
        promptText.setOutlineColor(sf::Color::Black);   // Black outline
        promptText.setOutlineThickness(3);              // Prominent outline

        inputBox.setSize(sf::Vector2f(600, 75));
        inputBox.setOutlineThickness(3);
        inputBox.setFillColor(sf::Color(30, 30, 30, 200));

        nameDisplay.setFont(ResourceManager::getFont("ariblk.ttf"));
        nameDisplay.setCharacterSize(45);
        nameDisplay.setFillColor(sf::Color(255, 215, 0)); // Golden fill
        nameDisplay.setOutlineColor(sf::Color::Black);   // Black outline
        nameDisplay.setOutlineThickness(2);              // Prominent outline

        nameLengthHint.setFont(ResourceManager::getFont("ariblk.ttf"));
        nameLengthHint.setCharacterSize(18);
        nameLengthHint.setFillColor(sf::Color(255, 140, 0)); // Dark orange fill
        nameLengthHint.setOutlineColor(sf::Color::Black);   // Black outline
        nameLengthHint.setOutlineThickness(1);              // Prominent outline

        cursorText.setFont(ResourceManager::getFont("ariblk.ttf"));
        cursorText.setString("|");
        cursorText.setCharacterSize(nameDisplay.getCharacterSize());
        cursorText.setFillColor(sf::Color::Yellow);

        continueText.setFont(ResourceManager::getFont("ariblk.ttf"));
        continueText.setString("PRESS ENTER TO CONTINUE");
        continueText.setCharacterSize(30);
        continueText.setFillColor(sf::Color(50, 205, 50)); // Lime green fill (kept as green for action)
        continueText.setOutlineColor(sf::Color::Black);   // Black outline
        continueText.setOutlineThickness(2);              // Prominent outline
        Utils::centerOrigin(continueText);

        ResourceManager::loadMenuBackgroundFrames(bgFrames, 12);
        if (!bgFrames.empty()) {
            background.setTexture(bgFrames[0]);
        }
    }

    void onEnter(const sf::RenderWindow& window, Player& playerRef, Enemy& enemyRef, const std::string& data) override {
        playerNameRef.clear(); // Clear previous name on entry
        nameDisplay.setString("");
        promptText.setString(promptPrefix + "NAME:");
        Utils::centerOrigin(promptText);

        isActive = true; // Make input box active by default on entry
        showCursor = true;
        cursorBlinkClock.restart();
        bgFrameTimer = 0.f; currentBgFrame = 0;
        if (!bgFrames.empty()) background.setTexture(bgFrames[currentBgFrame]);
        // Pass virtual resolution to onResize
        onResize(GameConfig::WINDOW_WIDTH, GameConfig::WINDOW_HEIGHT, playerRef, enemyRef);
    }


    void onResize(unsigned int width, unsigned int height, Player& playerRef, Enemy& enemyRef) override {
        promptText.setPosition(width / 2.0f, height * 0.3f);
        inputBox.setPosition(width / 2.0f - inputBox.getSize().x / 2.0f, height * 0.45f);

        nameDisplay.setPosition(
            inputBox.getPosition().x + GameConfig::INPUT_BOX_PADDING,
            inputBox.getPosition().y + (inputBox.getSize().y - nameDisplay.getGlobalBounds().height) / 2.f - nameDisplay.getLocalBounds().top
        );
        updateHint(); // Update hint position based on current layout

        continueText.setPosition(width / 2.0f, inputBox.getPosition().y + inputBox.getSize().y + 80);

         if (background.getTexture()) {
            background.setOrigin(0,0);
            background.setScale(
                static_cast<float>(width) / background.getTexture()->getSize().x,
                static_cast<float>(height) / background.getTexture()->getSize().y
            );
        }
    }

    void updateHint() {
        std::stringstream ss;
        ss << playerNameRef.length() << "/" << GameConfig::MAX_NAME_LENGTH;
        nameLengthHint.setString(ss.str());
        nameLengthHint.setPosition(
            inputBox.getPosition().x + inputBox.getSize().x - nameLengthHint.getLocalBounds().width - 10,
            inputBox.getPosition().y + inputBox.getSize().y + 5
        );
    }


    void handleEvent(sf::Event& event, sf::RenderWindow& window, GameStateID& nextState, bool& wantsTransition, GameStateID& gameResultState, Game* gamePtr) override {
        if (event.type == sf::Event::MouseButtonPressed) {
            sf::Vector2f mousePos = sf::Vector2f(sf::Mouse::getPosition(window)); // Get mouse in pixel coords
            // Transform mouse position to game coordinates for accurate hit detection
            sf::View gameView = window.getView();
            mousePos = window.mapPixelToCoords(sf::Vector2i(mousePos), gameView);

            isActive = inputBox.getGlobalBounds().contains(mousePos); // Check if input box was clicked
            if(isActive) { cursorBlinkClock.restart(); showCursor = true; } // Reset cursor blink
        }

        if (isActive && event.type == sf::Event::TextEntered) {
            bool changed = false;
            if (event.text.unicode == '\b') { // Backspace
                if (!playerNameRef.empty()) {
                    playerNameRef.pop_back();
                    changed = true;
                }
            } else if (event.text.unicode >= 32 && event.text.unicode < 128) { // Printable ASCII characters
                if (playerNameRef.length() < GameConfig::MAX_NAME_LENGTH) {
                    playerNameRef += static_cast<char>(event.text.unicode);
                    changed = true;
                }
            }
            if (changed) {
                nameDisplay.setString(playerNameRef);
                updateHint();
                // Recalculate Y pos of nameDisplay after string change
                nameDisplay.setPosition(
                    inputBox.getPosition().x + GameConfig::INPUT_BOX_PADDING,
                    inputBox.getPosition().y + (inputBox.getSize().y - nameDisplay.getGlobalBounds().height) / 2.f - nameDisplay.getLocalBounds().top
                );
                cursorBlinkClock.restart(); showCursor = true;
            }
        }

        if (event.type == sf::Event::KeyPressed) {
            if (event.key.code == sf::Keyboard::Enter) {
                // Only allow continuing if a name has been entered for P1, or it's P2 input
                if (gamePtr && ((gamePtr->currentStateID == GameStateID::NAME_INPUT && !playerNameRef.empty()) || gamePtr->currentStateID == GameStateID::NAME_INPUT_P2)) {
                    if (gamePtr->currentStateID == GameStateID::NAME_INPUT) {
                        nextState = GameStateID::MODE_SELECTION;
                    } else if (gamePtr->currentStateID == GameStateID::NAME_INPUT_P2) {
                        nextState = GameStateID::MAP_SELECTION; // P2 input, then proceed to map selection
                    }
                    wantsTransition = true;
                    isActive = false; // Deactivate input box
                }
            } else if (event.key.code == sf::Keyboard::Escape) {
                 if (gamePtr) {
                    if (gamePtr->currentStateID == GameStateID::NAME_INPUT) {
                        nextState = GameStateID::MENU; // Go back to main menu from P1 input
                    } else if (gamePtr->currentStateID == GameStateID::NAME_INPUT_P2) {
                        nextState = GameStateID::MODE_SELECTION; // Go back to mode selection from P2 input
                    }
                }
                wantsTransition = true;
                isActive = false;
            }
        }
    }

    void update(sf::Time dt, sf::Vector2f mousePos, Player& playerRef, Enemy& enemyRef, GameStateID& gameResultState, Game* gamePtr = nullptr) override {
        // Transform mouse position to game coordinates for input box focus
        sf::View gameView = m_windowRef.getView();
        mousePos = m_windowRef.mapPixelToCoords(sf::Vector2i(mousePos), gameView); // Ensure mousePos is sf::Vector2i

        inputBox.setOutlineColor(isActive ? sf::Color::Yellow : sf::Color(150, 150, 150)); // Highlight if active
        if (isActive && cursorBlinkClock.getElapsedTime().asSeconds() > 0.53f) {
            showCursor = !showCursor;
            cursorBlinkClock.restart();
        }

        if (!bgFrames.empty() && bgFrames.size() > 1) {
            bgFrameTimer += dt.asSeconds();
            if (bgFrameTimer >= BG_FRAME_DELAY) {
                bgFrameTimer -= BG_FRAME_DELAY;
                currentBgFrame = (currentBgFrame + 1) % bgFrames.size();
                background.setTexture(bgFrames[currentBgFrame]);
            }
        }
    }

    void draw(sf::RenderWindow& window, const Player& playerRef, const Enemy& enemyRef) override {
        if(background.getTexture()) window.draw(background);
        window.draw(promptText);
        window.draw(inputBox);
        window.draw(nameDisplay);
        window.draw(nameLengthHint);

        if (isActive && showCursor) {
            sf::Vector2f charPos = nameDisplay.findCharacterPos(playerNameRef.length());
            float cursorX = charPos.x;
            float cursorY = nameDisplay.getPosition().y + nameDisplay.getLocalBounds().top + (nameDisplay.getGlobalBounds().height - cursorText.getGlobalBounds().height) / 2.f - cursorText.getLocalBounds().top;
            float minCursorX = inputBox.getPosition().x + GameConfig::INPUT_BOX_PADDING;
            float maxCursorX = inputBox.getPosition().x + inputBox.getSize().x - GameConfig::INPUT_BOX_PADDING - cursorText.getLocalBounds().width;
            if (playerNameRef.empty()) { cursorX = minCursorX; }
            cursorX = Utils::clamp(cursorX, minCursorX, maxCursorX);
            cursorText.setPosition(cursorX, cursorY);
            window.draw(cursorText);
        }

        bool showContinue = false;
        if (gamePtr) {
             if ((gamePtr->currentStateID == GameStateID::NAME_INPUT && !playerNameRef.empty()) ||
                 (gamePtr->currentStateID == GameStateID::NAME_INPUT_P2)) {
                 showContinue = true;
            }
        } else if (!playerNameRef.empty()){
             showContinue = true;
        }

        if (showContinue) {
            window.draw(continueText);
        }
    }
    Game* gamePtr = nullptr; // For checking current state in draw
};


// --- CharacterSelectionScreen ---
class CharacterSelectionScreen : public Screen {
public:
    sf::Text promptText;
    sf::RectangleShape char1Frame, char2Frame, char3Frame;
    sf::Sprite char1TitleSprite, char2TitleSprite, char3TitleSprite;
    sf::Text char1NameText, char2NameText, char3NameText;

    sf::Color frameColor = sf::Color(50, 50, 50, 130);
    sf::Color frameHoverColor = sf::Color(80, 80, 80, 180);
    sf::Color frameSelectedColor = sf::Color(135, 206, 235, 180); // Sky blue for selection
    sf::Color frameOutlineColor = sf::Color::White;

    std::vector<sf::Texture> bgFrames;
    sf::Sprite background;
    int currentBgFrame = 0;
    float bgFrameTimer = 0.f;
    const float BG_FRAME_DELAY = 0.08f;

    sf::RenderWindow& m_windowRef;
    GameMode m_gameMode; // To know if PvAI or PvP
    int currentSelectionStep; // 0: Player 1 selection, 1: Player 2/AI selection

    // Temporary selections before applying to Game
    CharacterTypeID tempP1CharType;
    CharacterTypeID tempP2CharType;

private:
    sf::Texture char1TitleTexture, char2TitleTexture, char3TitleTexture;

public:
    Player* m_playerRef;
    Enemy* m_enemyRef;

    CharacterSelectionScreen(sf::RenderWindow& window) : m_windowRef(window), m_playerRef(nullptr), m_enemyRef(nullptr) {
        promptText.setFont(ResourceManager::getFont("ariblk.ttf"));
        promptText.setCharacterSize(55);
        promptText.setFillColor(sf::Color(255, 165, 0)); // Orange fill
        promptText.setOutlineColor(sf::Color::Black);   // Black outline
        promptText.setOutlineThickness(3);              // Prominent outline
        Utils::centerOrigin(promptText);

        // Load character title textures
        ResourceManager::loadTexture(char1TitleTexture, AllCharacterPresets.at(CharacterTypeID::KNIGHT).titlePath);
        char1TitleSprite.setTexture(char1TitleTexture);
        ResourceManager::loadTexture(char2TitleTexture, AllCharacterPresets.at(CharacterTypeID::ROGUE).titlePath);
        char2TitleSprite.setTexture(char2TitleTexture);
        ResourceManager::loadTexture(char3TitleTexture, AllCharacterPresets.at(CharacterTypeID::SAMURAI).titlePath);
        char3TitleSprite.setTexture(char3TitleTexture);

        sf::Vector2f frameSize(360, 220); // Frame size for title image + name

        char1Frame.setSize(frameSize);
        char1Frame.setOutlineColor(frameOutlineColor);
        char1Frame.setOutlineThickness(3);
        char1Frame.setFillColor(frameColor);
        char1NameText.setFont(ResourceManager::getFont("ariblk.ttf"));
        char1NameText.setString(AllCharacterPresets.at(CharacterTypeID::KNIGHT).name);
        char1NameText.setCharacterSize(30);
        char1NameText.setFillColor(sf::Color(255, 215, 0)); // Golden fill
        char1NameText.setOutlineColor(sf::Color::Black);   // Black outline
        char1NameText.setOutlineThickness(2);              // Prominent outline
        Utils::centerOrigin(char1NameText);

        char2Frame.setSize(frameSize);
        char2Frame.setOutlineColor(frameOutlineColor);
        char2Frame.setOutlineThickness(3);
        char2Frame.setFillColor(frameColor);
        char2NameText.setFont(ResourceManager::getFont("ariblk.ttf"));
        char2NameText.setString(AllCharacterPresets.at(CharacterTypeID::ROGUE).name);
        char2NameText.setCharacterSize(30);
        char2NameText.setFillColor(sf::Color(255, 215, 0)); // Golden fill
        char2NameText.setOutlineColor(sf::Color::Black);   // Black outline
        char2NameText.setOutlineThickness(2);              // Prominent outline
        Utils::centerOrigin(char2NameText);

        char3Frame.setSize(frameSize);
        char3Frame.setOutlineColor(frameOutlineColor);
        char3Frame.setOutlineThickness(3);
        char3Frame.setFillColor(frameColor);
        char3NameText.setFont(ResourceManager::getFont("ariblk.ttf"));
        char3NameText.setString(AllCharacterPresets.at(CharacterTypeID::SAMURAI).name);
        char3NameText.setCharacterSize(30);
        char3NameText.setFillColor(sf::Color(255, 215, 0)); // Golden fill
        char3NameText.setOutlineColor(sf::Color::Black);   // Black outline
        char3NameText.setOutlineThickness(2);              // Prominent outline
        Utils::centerOrigin(char3NameText);


        ResourceManager::loadMenuBackgroundFrames(bgFrames, 12);
        if (!bgFrames.empty()) {
            background.setTexture(bgFrames[0]);
        }
    }

    void onEnter(const sf::RenderWindow& window, Player& playerRef, Enemy& enemyRef, const std::string& data) override {
        bgFrameTimer = 0.f; currentBgFrame = 0;
        if (!bgFrames.empty()) background.setTexture(bgFrames[currentBgFrame]);

        // Reset selection and set prompt
        currentSelectionStep = 0; // Start with Player 1 selection
        tempP1CharType = CharacterTypeID::KNIGHT; // Default to Knight
        tempP2CharType = CharacterTypeID::ROGUE; // Default to Rogue (for AI or P2)

        // Pass virtual resolution to onResize
        onResize(GameConfig::WINDOW_WIDTH, GameConfig::WINDOW_HEIGHT, playerRef, enemyRef);
    }

    void onResize(unsigned int width, unsigned int height, Player& playerRef, Enemy& enemyRef) override {
        if (currentSelectionStep == 0) {
            promptText.setString("SELECT PLAYER 1 CHARACTER:");
        } else {
            promptText.setString((m_gameMode == GameMode::PvAI ? "SELECT AI CHARACTER:" : "SELECT PLAYER 2 CHARACTER:"));
        }
        Utils::centerOrigin(promptText);
        promptText.setPosition(width / 2.0f, height * 0.2f);

        float charY = height * 0.4f;
        float charSpacing = 30;
        float totalCharWidth = char1Frame.getSize().x * 3 + charSpacing * 2;
        float startX = (width - totalCharWidth) / 2.0f;

        char1Frame.setPosition(startX, charY);
        char2Frame.setPosition(startX + char1Frame.getSize().x + charSpacing, charY);
        char3Frame.setPosition(startX + (char1Frame.getSize().x + charSpacing) * 2, charY);

        float titleImageWidth = char1Frame.getSize().x * 0.8f; // Use 80% of frame width for image
        float titleImageHeight = titleImageWidth * (9.0f / 16.0f);
        float titleImageYOffset = 20.0f; // Offset from the top of the frame

        // Position and scale title sprites
        if (char1TitleSprite.getTexture()) {
            char1TitleSprite.setScale(titleImageWidth / char1TitleSprite.getTexture()->getSize().x,
                                     titleImageHeight / char1TitleSprite.getTexture()->getSize().y);
            Utils::centerOrigin(char1TitleSprite);
            char1TitleSprite.setPosition(char1Frame.getPosition().x + char1Frame.getSize().x / 2.0f,
                                         char1Frame.getPosition().y + titleImageYOffset + titleImageHeight / 2.0f);
        }
        if (char2TitleSprite.getTexture()) {
            char2TitleSprite.setScale(titleImageWidth / char2TitleSprite.getTexture()->getSize().x,
                                     titleImageHeight / char2TitleSprite.getTexture()->getSize().y);
            Utils::centerOrigin(char2TitleSprite);
            char2TitleSprite.setPosition(char2Frame.getPosition().x + char2Frame.getSize().x / 2.0f,
                                         char2Frame.getPosition().y + titleImageYOffset + titleImageHeight / 2.0f);
        }
        if (char3TitleSprite.getTexture()) {
            char3TitleSprite.setScale(titleImageWidth / char3TitleSprite.getTexture()->getSize().x,
                                     titleImageHeight / char3TitleSprite.getTexture()->getSize().y);
            Utils::centerOrigin(char3TitleSprite);
            char3TitleSprite.setPosition(char3Frame.getPosition().x + char3Frame.getSize().x / 2.0f,
                                         char3Frame.getPosition().y + titleImageYOffset + titleImageHeight / 2.0f);
        }

        // Position name texts below title images
        char1NameText.setPosition(char1Frame.getPosition().x + char1Frame.getSize().x / 2.0f,
                                  char1Frame.getPosition().y + titleImageYOffset + titleImageHeight + 25.0f);
        char2NameText.setPosition(char2Frame.getPosition().x + char2Frame.getSize().x / 2.0f,
                                  char2Frame.getPosition().y + titleImageYOffset + titleImageHeight + 25.0f);
        char3NameText.setPosition(char3Frame.getPosition().x + char3Frame.getSize().x / 2.0f,
                                  char3Frame.getPosition().y + titleImageYOffset + titleImageHeight + 25.0f);

        if (background.getTexture()) {
            background.setOrigin(0,0);
            background.setScale(
                static_cast<float>(width) / background.getTexture()->getSize().x,
                static_cast<float>(height) / background.getTexture()->getSize().y
            );
        }
    }

    void handleEvent(sf::Event& event, sf::RenderWindow& window, GameStateID& nextState, bool& wantsTransition, GameStateID& gameResultState, Game* gamePtr) override {
        if (event.type == sf::Event::MouseButtonPressed) {
            sf::Vector2f mousePos = sf::Vector2f(sf::Mouse::getPosition(window)); // Get mouse in pixel coords
            // Transform mouse position to game coordinates for accurate hit detection
            sf::View gameView = window.getView();
            mousePos = window.mapPixelToCoords(sf::Vector2i(mousePos), gameView);

            CharacterTypeID clickedType = CharacterTypeID::KNIGHT; // Default, will be overwritten
            bool clicked = false;

            if (char1Frame.getGlobalBounds().contains(mousePos)) { clickedType = CharacterTypeID::KNIGHT; clicked = true; }
            else if (char2Frame.getGlobalBounds().contains(mousePos)) { clickedType = CharacterTypeID::ROGUE; clicked = true; }
            else if (char3Frame.getGlobalBounds().contains(mousePos)) { clickedType = CharacterTypeID::SAMURAI; clicked = true; }

            if (clicked) {
                if (currentSelectionStep == 0) { // Player 1 selection
                    tempP1CharType = clickedType;
                    currentSelectionStep = 1; // Move to Player 2/AI selection
                    // PlayerRef/EnemyRef are not members of CharacterSelectionScreen.
                    // Pass default/placeholder for onResize within this context.
                    onResize(GameConfig::WINDOW_WIDTH, GameConfig::WINDOW_HEIGHT, *m_playerRef, *m_enemyRef); // Pass current player/enemy references
                } else { // Player 2/AI selection
                    tempP2CharType = clickedType;
                    if (gamePtr) {
                        gamePtr->selectedPlayer1Char = tempP1CharType;
                        gamePtr->selectedEnemyChar = tempP2CharType;
                    }
                    nextState = GameStateID::MAP_SELECTION; // Proceed to map selection
                    wantsTransition = true;
                }
            }
        } else if (event.type == sf::Event::KeyPressed && event.key.code == sf::Keyboard::Escape) {
            if (currentSelectionStep == 1) { // If selecting P2/AI, go back to P1 selection
                onResize(GameConfig::WINDOW_WIDTH, GameConfig::WINDOW_HEIGHT, *m_playerRef, *m_enemyRef); // Pass current player/enemy references
                currentSelectionStep = 0;
            } else { // If selecting P1, go back to mode selection
                nextState = GameStateID::MODE_SELECTION;
                wantsTransition = true;
            }
        }
    }

    void update(sf::Time dt, sf::Vector2f mousePos, Player& playerRef, Enemy& enemyRef, GameStateID& gameResultState, Game* gamePtr) override {
        // Store game mode for prompt text
        if (gamePtr) m_gameMode = gamePtr->currentMode;

        // Transform mouse position to game coordinates for hover effect
        sf::View gameView = m_windowRef.getView();
        mousePos = m_windowRef.mapPixelToCoords(sf::Vector2i(mousePos), gameView); // Ensure mousePos is sf::Vector2i

        // Apply hover and selection colors
        char1Frame.setFillColor(frameColor); // Reset colors first
        char2Frame.setFillColor(frameColor);
        char3Frame.setFillColor(frameColor);

        if (currentSelectionStep == 0) { // Player 1 selection
            if (char1Frame.getGlobalBounds().contains(mousePos)) char1Frame.setFillColor(frameHoverColor);
            else if (char2Frame.getGlobalBounds().contains(mousePos)) char2Frame.setFillColor(frameHoverColor);
            else if (char3Frame.getGlobalBounds().contains(mousePos)) char3Frame.setFillColor(frameHoverColor);

            // Highlight current selection
            if (tempP1CharType == CharacterTypeID::KNIGHT) char1Frame.setFillColor(frameSelectedColor);
            else if (tempP1CharType == CharacterTypeID::ROGUE) char2Frame.setFillColor(frameSelectedColor);
            else if (tempP1CharType == CharacterTypeID::SAMURAI) char3Frame.setFillColor(frameSelectedColor);

        } else { // Player 2/AI selection
            if (char1Frame.getGlobalBounds().contains(mousePos)) char1Frame.setFillColor(frameHoverColor);
            else if (char2Frame.getGlobalBounds().contains(mousePos)) char2Frame.setFillColor(frameHoverColor);
            else if (char3Frame.getGlobalBounds().contains(mousePos)) char3Frame.setFillColor(frameHoverColor);

            // Highlight current selection (P2/AI)
            if (tempP2CharType == CharacterTypeID::KNIGHT) char1Frame.setFillColor(frameSelectedColor);
            else if (tempP2CharType == CharacterTypeID::ROGUE) char2Frame.setFillColor(frameSelectedColor);
            else if (tempP2CharType == CharacterTypeID::SAMURAI) char3Frame.setFillColor(frameSelectedColor);
        }

        if (!bgFrames.empty() && bgFrames.size() > 1) {
            bgFrameTimer += dt.asSeconds();
            if (bgFrameTimer >= BG_FRAME_DELAY) {
                bgFrameTimer -= BG_FRAME_DELAY;
                currentBgFrame = (currentBgFrame + 1) % bgFrames.size();
                background.setTexture(bgFrames[currentBgFrame]);
            }
        }
    }

    void draw(sf::RenderWindow& window, const Player& playerRef, const Enemy& enemyRef) override {
        if(background.getTexture()) window.draw(background);
        window.draw(promptText);

        window.draw(char1Frame); window.draw(char1TitleSprite); window.draw(char1NameText);
        window.draw(char2Frame); window.draw(char2TitleSprite); window.draw(char2NameText);
        window.draw(char3Frame); window.draw(char3TitleSprite); window.draw(char3NameText);
    }
};


// --- MapSelectionScreen ---
class MapSelectionScreen : public Screen {
public:
    sf::Text promptText;
    sf::RectangleShape map1Frame, map2Frame, map3Frame;
    sf::Sprite map1PreviewSprite, map2PreviewSprite, map3PreviewSprite;
    sf::Text map1Text, map2Text, map3Text;

    sf::Color mapFrameColor = sf::Color(80, 80, 80, 130); // A neutral background color for frames
    sf::Color mapFrameHoverColor = sf::Color(120, 120, 120, 180);
    sf::Color mapFrameOutlineColor = sf::Color::White;

    std::vector<sf::Texture> bgFrames;
    sf::Sprite background;
    int currentBgFrame = 0;
    float bgFrameTimer = 0.f;
    const float BG_FRAME_DELAY = 0.08f;

    int& gameCurrentMapSelectionRef;
    sf::RenderWindow& m_windowRef;

private:
    sf::Texture map1PreviewTexture; // Member to hold the texture
    sf::Texture map2PreviewTexture; // Member to hold the texture
    sf::Texture map3PreviewTexture; // Member to hold the texture for Map 3

public:
    MapSelectionScreen(sf::RenderWindow& window, int& currentMapSelection)
        : m_windowRef(window), gameCurrentMapSelectionRef(currentMapSelection) {
        promptText.setFont(ResourceManager::getFont("ariblk.ttf"));
        promptText.setString("SELECT MAP:");
        promptText.setCharacterSize(55);
        promptText.setFillColor(sf::Color(255, 165, 0)); // Orange fill
        promptText.setOutlineColor(sf::Color::Black);   // Black outline
        promptText.setOutlineThickness(3);              // Prominent outline
        Utils::centerOrigin(promptText);

        // Load map preview textures
        ResourceManager::loadTexture(map1PreviewTexture, "assets/Map1_title.png");
        map1PreviewSprite.setTexture(map1PreviewTexture);

        ResourceManager::loadTexture(map2PreviewTexture, "assets/Map2_title.png");
        map2PreviewSprite.setTexture(map2PreviewTexture);

        ResourceManager::loadTexture(map3PreviewTexture, "assets/bg1.1.png"); // Map 3 preview is bg1.1.png
        map3PreviewSprite.setTexture(map3PreviewTexture);

        sf::Vector2f frameSize(340, 240); // Frame size for preview image + name

        map1Frame.setSize(frameSize);
        map1Frame.setOutlineColor(mapFrameOutlineColor);
        map1Frame.setOutlineThickness(3);
        map1Frame.setFillColor(mapFrameColor);

        map1Text.setFont(ResourceManager::getFont("ariblk.ttf"));
        map1Text.setString("Templum Draconis");
        map1Text.setCharacterSize(30); // Smaller size for text below image
        map1Text.setFillColor(sf::Color(255, 215, 0)); // Golden fill
        map1Text.setOutlineColor(sf::Color::Black);   // Black outline
        map1Text.setOutlineThickness(2);              // Prominent outline
        Utils::centerOrigin(map1Text);

        map2Frame.setSize(frameSize);
        map2Frame.setOutlineColor(mapFrameOutlineColor);
        map2Frame.setOutlineThickness(3);
        map2Frame.setFillColor(mapFrameColor);

        map2Text.setFont(ResourceManager::getFont("ariblk.ttf"));
        map2Text.setString("Terra Ruinae");
        map2Text.setCharacterSize(30);
        map2Text.setFillColor(sf::Color(255, 215, 0)); // Golden fill
        map2Text.setOutlineColor(sf::Color::Black);   // Black outline
        map2Text.setOutlineThickness(2);              // Prominent outline
        Utils::centerOrigin(map2Text);

        // Map 3 Frame and Text setup
        map3Frame.setSize(frameSize);
        map3Frame.setOutlineColor(mapFrameOutlineColor);
        map3Frame.setOutlineThickness(3);
        map3Frame.setFillColor(mapFrameColor);

        map3Text.setFont(ResourceManager::getFont("ariblk.ttf"));
        map3Text.setString("Shogun's Curse");
        map3Text.setCharacterSize(30);
        map3Text.setFillColor(sf::Color(255, 215, 0)); // Golden fill
        map3Text.setOutlineColor(sf::Color::Black);   // Black outline
        map3Text.setOutlineThickness(2);              // Prominent outline
        Utils::centerOrigin(map3Text);


        ResourceManager::loadMenuBackgroundFrames(bgFrames, 12);
        if (!bgFrames.empty()) {
            background.setTexture(bgFrames[0]);
        }
    }

    void onEnter(const sf::RenderWindow& window, Player& playerRef, Enemy& enemyRef, const std::string& data) override {
        bgFrameTimer = 0.f; currentBgFrame = 0;
        if (!bgFrames.empty()) background.setTexture(bgFrames[currentBgFrame]);
        // Pass virtual resolution to onResize
        onResize(GameConfig::WINDOW_WIDTH, GameConfig::WINDOW_HEIGHT, playerRef, enemyRef);
    }

    void onResize(unsigned int width, unsigned int height, Player& playerRef, Enemy& enemyRef) override {
        promptText.setPosition(width / 2.0f, height * 0.2f);
        float mapButtonY = height * 0.4f;
        float mapButtonSpacing = 30; // Adjusted spacing for 3 maps
        float totalMapWidth = map1Frame.getSize().x * 3 + mapButtonSpacing * 2; // For 3 maps
        float startX = (width - totalMapWidth) / 2.0f;

        // Position map frames
        map1Frame.setPosition(startX, mapButtonY);
        map2Frame.setPosition(startX + map1Frame.getSize().x + mapButtonSpacing, mapButtonY);
        map3Frame.setPosition(startX + (map1Frame.getSize().x + mapButtonSpacing) * 2, mapButtonY);

        float previewPadding = 10.0f; // Padding around the image within the frame
        float availableWidth = map1Frame.getSize().x - (previewPadding * 2);
        float previewHeight = availableWidth * (9.0f / 16.0f); // Maintain 16:9 aspect ratio for preview images
        float previewYOffset = previewPadding; // Offset from the top of the frame

        // Position preview images within frames (centered and maintaining 16:9)
        if (map1PreviewSprite.getTexture()) {
            map1PreviewSprite.setScale(availableWidth / map1PreviewSprite.getTexture()->getSize().x,
                                     previewHeight / map1PreviewSprite.getTexture()->getSize().y);
            map1PreviewSprite.setPosition(map1Frame.getPosition().x + previewPadding,
                                          map1Frame.getPosition().y + previewYOffset);
        }
        if (map2PreviewSprite.getTexture()) {
            map2PreviewSprite.setScale(availableWidth / map2PreviewSprite.getTexture()->getSize().x,
                                     previewHeight / map2PreviewSprite.getTexture()->getSize().y);
            map2PreviewSprite.setPosition(map2Frame.getPosition().x + previewPadding,
                                          map2Frame.getPosition().y + previewYOffset);
        }
        if (map3PreviewSprite.getTexture()) {
            map3PreviewSprite.setScale(availableWidth / map3PreviewSprite.getTexture()->getSize().x,
                                     previewHeight / map3PreviewSprite.getTexture()->getSize().y);
            map3PreviewSprite.setPosition(map3Frame.getPosition().x + previewPadding,
                                          map3Frame.getPosition().y + previewYOffset);
        }

        // Position text below images
        map1Text.setPosition(map1Frame.getPosition().x + map1Frame.getSize().x/2.0f,
                             map1Frame.getPosition().y + previewYOffset + previewHeight + 20.0f);
        map2Text.setPosition(map2Frame.getPosition().x + map2Frame.getSize().x/2.0f,
                             map2Frame.getPosition().y + previewYOffset + previewHeight + 20.0f);
        map3Text.setPosition(map3Frame.getPosition().x + map3Frame.getSize().x/2.0f,
                             map3Frame.getPosition().y + previewYOffset + previewHeight + 20.0f);


        if (background.getTexture()) {
            background.setOrigin(0,0);
            background.setScale(
                static_cast<float>(width) / background.getTexture()->getSize().x,
                static_cast<float>(height) / background.getTexture()->getSize().y
            );
        }
    }

    void handleEvent(sf::Event& event, sf::RenderWindow& window, GameStateID& nextState, bool& wantsTransition, GameStateID& gameResultState, Game* gamePtr) override {
        if (event.type == sf::Event::MouseButtonPressed) {
            sf::Vector2f mousePos = sf::Vector2f(sf::Mouse::getPosition(window)); // Get mouse in pixel coords
            // Transform mouse position to game coordinates for accurate hit detection
            sf::View gameView = window.getView();
            mousePos = window.mapPixelToCoords(sf::Vector2i(mousePos), gameView);

            int selectedMap = 0;
            if (map1Frame.getGlobalBounds().contains(mousePos)) selectedMap = 1;
            else if (map2Frame.getGlobalBounds().contains(mousePos)) selectedMap = 2;
            else if (map3Frame.getGlobalBounds().contains(mousePos)) selectedMap = 3; // Check for map 3

            if (selectedMap > 0) {
                gameCurrentMapSelectionRef = selectedMap;
                nextState = GameStateID::GAME_PLAY;
                wantsTransition = true;
            }
        } else if (event.type == sf::Event::KeyPressed && event.key.code == sf::Keyboard::Escape) {
            // Go back to Character Selection
            nextState = GameStateID::CHARACTER_SELECTION;
            wantsTransition = true;
        }
    }

    void update(sf::Time dt, sf::Vector2f mousePos, Player& playerRef, Enemy& enemyRef, GameStateID& gameResultState, Game* gamePtr = nullptr) override {
        // Transform mouse position to game coordinates for hover effect
        sf::View gameView = m_windowRef.getView();
        mousePos = m_windowRef.mapPixelToCoords(sf::Vector2i(mousePos), gameView); // Ensure mousePos is sf::Vector2i

        map1Frame.setFillColor(map1Frame.getGlobalBounds().contains(mousePos) ? mapFrameHoverColor : mapFrameColor);
        map2Frame.setFillColor(map2Frame.getGlobalBounds().contains(mousePos) ? mapFrameHoverColor : mapFrameColor);
        map3Frame.setFillColor(map3Frame.getGlobalBounds().contains(mousePos) ? mapFrameHoverColor : mapFrameColor); // Update Map 3 hover

        if (!bgFrames.empty() && bgFrames.size() > 1) {
            bgFrameTimer += dt.asSeconds();
            if (bgFrameTimer >= BG_FRAME_DELAY) {
                bgFrameTimer -= BG_FRAME_DELAY;
                currentBgFrame = (currentBgFrame + 1) % bgFrames.size();
                background.setTexture(bgFrames[currentBgFrame]);
            }
        }
    }

    void draw(sf::RenderWindow& window, const Player& playerRef, const Enemy& enemyRef) override {
        if(background.getTexture()) window.draw(background);
        window.draw(promptText);
        window.draw(map1Frame); window.draw(map1PreviewSprite); window.draw(map1Text);
        window.draw(map2Frame); window.draw(map2PreviewSprite); window.draw(map2Text);
        window.draw(map3Frame); window.draw(map3PreviewSprite); window.draw(map3Text); // Draw Map 3 elements
    }
};

// --- GamePlayScreen ---
class GamePlayScreen : public Screen {
public:
    Game* m_gamePtr;

    sf::Text playerNameText_UI;
    sf::RectangleShape playerUiPanel;
    sf::RectangleShape playerHealthBarBg, playerHealthBar;

    sf::Text enemyNameText_UI;
    sf::RectangleShape enemyUiPanel;
    sf::RectangleShape enemyHealthBarBg, enemyHealthBar;

    sf::Sprite& gameBgSpriteRef;

    sf::RectangleShape playerAttackHitboxShape;
    sf::RectangleShape enemyAttackHitboxShape;
    sf::RectangleShape playerHurtboxShapeDebug; // For debugging hurtbox
    sf::RectangleShape enemyHurtboxShapeDebug; // For debugging hurtbox
    bool showDebugHitboxes = false;

    std::vector<DamageText> damageTexts;

    sf::Text timerText; // For game countdown
    sf::Clock gameTimerClock;
    bool timerEnded = false; // Flag to prevent repeated win/draw checks

    GamePlayScreen(sf::RenderWindow& window, Game* gamePtr, sf::Sprite& gameBgSprite)
        : m_gamePtr(gamePtr), gameBgSpriteRef(gameBgSprite) {

        playerUiPanel.setSize(sf::Vector2f(340, 110));
        playerUiPanel.setFillColor(sf::Color(20, 20, 30, 160));
        playerUiPanel.setOutlineColor(sf::Color(100, 100, 120, 180));
        playerUiPanel.setOutlineThickness(2);

        playerHealthBarBg.setSize(sf::Vector2f(300, 30));
        playerHealthBarBg.setFillColor(sf::Color(80, 0, 0, 200));
        playerHealthBarBg.setOutlineColor(sf::Color::Black);
        playerHealthBarBg.setOutlineThickness(1);

        playerHealthBar.setSize(sf::Vector2f(300, 30));
        playerHealthBar.setFillColor(sf::Color(0, 200, 0));

        playerNameText_UI.setFont(ResourceManager::getFont("ariblk.ttf"));
        playerNameText_UI.setCharacterSize(26);
        playerNameText_UI.setFillColor(sf::Color(255, 215, 0)); // Golden fill
        playerNameText_UI.setOutlineColor(sf::Color::Black);   // Black outline
        playerNameText_UI.setOutlineThickness(2);              // Prominent outline


        enemyUiPanel.setSize(sf::Vector2f(340, 110));
        enemyUiPanel.setFillColor(sf::Color(30, 20, 20, 160));
        enemyUiPanel.setOutlineColor(sf::Color(120, 100, 100, 180));
        enemyUiPanel.setOutlineThickness(2);

        enemyNameText_UI.setFont(ResourceManager::getFont("ariblk.ttf"));
        enemyNameText_UI.setCharacterSize(26);
        enemyNameText_UI.setFillColor(sf::Color(255, 215, 0)); // Golden fill
        enemyNameText_UI.setOutlineColor(sf::Color::Black);   // Black outline
        enemyNameText_UI.setOutlineThickness(2);              // Prominent outline

        enemyHealthBarBg.setSize(sf::Vector2f(300, 30));
        enemyHealthBarBg.setFillColor(sf::Color(80,0,0,200));
        enemyHealthBarBg.setOutlineColor(sf::Color::Black);
        enemyHealthBarBg.setOutlineThickness(1);

        enemyHealthBar.setSize(sf::Vector2f(300,30));
        enemyHealthBar.setFillColor(sf::Color(200,0,0));

        playerAttackHitboxShape.setFillColor(sf::Color(255,0,0,100));
        enemyAttackHitboxShape.setFillColor(sf::Color(0,0,255,100));

        playerHurtboxShapeDebug.setFillColor(sf::Color(0, 255, 0, 100)); // Green for hurtbox
        enemyHurtboxShapeDebug.setFillColor(sf::Color(0, 255, 0, 100)); // Green for hurtbox

        // Timer Text Setup
        timerText.setFont(ResourceManager::getFont("ariblk.ttf"));
        timerText.setCharacterSize(40);
        timerText.setFillColor(sf::Color(255, 215, 0)); // Gold color
        timerText.setOutlineColor(sf::Color::Black);
        timerText.setOutlineThickness(2);
        Utils::centerOrigin(timerText);
    }

    void onEnter(const sf::RenderWindow& window, Player& playerRef, Enemy& enemyRef, const std::string& data) override {
        if (m_gamePtr) {
            playerNameText_UI.setString(m_gamePtr->playerNameFromInput.empty() ? "Player 1" : m_gamePtr->playerNameFromInput);
            if (m_gamePtr->currentMode == GameMode::PvP) {
                enemyNameText_UI.setString(m_gamePtr->player2NameFromInput.empty() ? "Player 2" : m_gamePtr->player2NameFromInput);
            } else {
                enemyNameText_UI.setString("Rival");
            }
        }
        damageTexts.clear();
        gameTimerClock.restart(); // Start game timer
        timerEnded = false; // Reset timer ended flag for a new game
        // Pass virtual resolution to onResize
        onResize(GameConfig::WINDOW_WIDTH, GameConfig::WINDOW_HEIGHT, playerRef, enemyRef);
    }

    void onResize(unsigned int width, unsigned int height, Player& playerRef, Enemy& enemyRef) override {
    playerUiPanel.setPosition(15, 15);
    playerHealthBarBg.setPosition(playerUiPanel.getPosition().x + 15, playerUiPanel.getPosition().y + 15);
    playerHealthBar.setPosition(playerHealthBarBg.getPosition());
    playerNameText_UI.setPosition(playerUiPanel.getPosition().x + 20, playerUiPanel.getPosition().y + playerHealthBarBg.getSize().y + 25);

    enemyUiPanel.setPosition(width - enemyUiPanel.getSize().x - 15, 15);
    enemyHealthBarBg.setPosition(enemyUiPanel.getPosition().x + 15, enemyUiPanel.getPosition().y + 15);
    enemyHealthBar.setPosition(enemyHealthBarBg.getPosition());
    enemyNameText_UI.setPosition(enemyUiPanel.getPosition().x + 20, enemyUiPanel.getPosition().y + enemyHealthBarBg.getSize().y + 25);

    timerText.setPosition(width / 2.0f, 50.0f);

    float commonGroundY = height - (playerRef.frameHeight * playerRef.spriteScale) - 20;
    playerRef.setGroundY(commonGroundY);
    enemyRef.setGroundY(commonGroundY);

    // Updated background scaling code starts here
    if (gameBgSpriteRef.getTexture()) {
        // Get the original texture size
        sf::Vector2u texSize = gameBgSpriteRef.getTexture()->getSize();

        // Calculate scale factors to fill the width while maintaining aspect ratio
        float scaleX = static_cast<float>(width) / texSize.x;
        float scaleY = static_cast<float>(height) / texSize.y;

        // Use the larger scale factor to ensure full coverage
        float uniformScale = std::max(scaleX, scaleY);

        gameBgSpriteRef.setScale(uniformScale, uniformScale);

        // Center the background
        gameBgSpriteRef.setOrigin(0, 0);
        gameBgSpriteRef.setPosition(0, (height - (texSize.y * uniformScale)) / 2);
    }

}

    void handleEvent(sf::Event& event, sf::RenderWindow& window, GameStateID& nextState, bool& wantsTransition, GameStateID& gameResultState, Game* gamePtr) override {
        if (event.type == sf::Event::KeyPressed) {
            if (event.key.code == sf::Keyboard::Escape) {
                nextState = GameStateID::PAUSE;
                wantsTransition = false;
            }
            if (event.key.code == sf::Keyboard::F1) {
                showDebugHitboxes = !showDebugHitboxes;
            }
        }
    }

    void update(sf::Time dt, sf::Vector2f mousePos, Player& playerRef, Enemy& enemyRef, GameStateID& gameResultState, Game* gamePtr) override {
        if (timerEnded) return; // Stop game logic if timer ended and winner is determined by time

        playerRef.handleInput();
        if (enemyRef.isPlayerControlled) {
            enemyRef.handlePlayer2Input();
        }

        playerRef.update(dt.asSeconds(), GameConfig::WINDOW_WIDTH, &enemyRef);
        enemyRef.update(dt.asSeconds(), GameConfig::WINDOW_WIDTH, &playerRef);

        playerHealthBar.setSize(sf::Vector2f(playerHealthBarBg.getSize().x * Utils::clamp(playerRef.currentHealth / playerRef.maxHealth, 0.f, 1.f), playerHealthBarBg.getSize().y));
        enemyHealthBar.setSize(sf::Vector2f(enemyHealthBarBg.getSize().x * Utils::clamp(enemyRef.currentHealth / enemyRef.maxHealth, 0.f, 1.f), enemyHealthBarBg.getSize().y));

        // Player attack collision check
        if (playerRef.isAttacking && !playerRef.dealtDamageThisAttack) {
            sf::FloatRect playerHitbox = playerRef.getAttackHitbox();
            // Use enemyRef.getHurtbox() for precise collision detection
            if (enemyRef.isAlive && playerHitbox.intersects(enemyRef.getHurtbox())) {
                enemyRef.takeDamage(GameConfig::ATTACK_DAMAGE);
                playerRef.dealtDamageThisAttack = true;
                sf::FloatRect targetBounds = enemyRef.sprite.getGlobalBounds(); // Use sprite bounds for text position
                sf::Vector2f textPos(targetBounds.left + targetBounds.width / 2.f, targetBounds.top - 20.f);
                damageTexts.emplace_back("-" + std::to_string(static_cast<int>(GameConfig::ATTACK_DAMAGE)), ResourceManager::getFont("ariblk.ttf"), 24, sf::Color::Yellow, textPos);
                if(gamePtr) gamePtr->triggerScreenShake();
            }
        }

        // Enemy attack collision check
        if (enemyRef.isAttacking && !enemyRef.dealtDamageThisAttack) {
            sf::FloatRect enemyHitbox = enemyRef.getAttackHitbox();
             // Use playerRef.getHurtbox() for precise collision detection
            if (playerRef.isAlive && enemyHitbox.intersects(playerRef.getHurtbox())) {
                playerRef.takeDamage(GameConfig::ATTACK_DAMAGE);
                enemyRef.dealtDamageThisAttack = true;
                sf::FloatRect targetBounds = playerRef.sprite.getGlobalBounds(); // Use sprite bounds for text position
                sf::Vector2f textPos(targetBounds.left + targetBounds.width / 2.f, targetBounds.top - 20.f);
                damageTexts.emplace_back("-" + std::to_string(static_cast<int>(GameConfig::ATTACK_DAMAGE)), ResourceManager::getFont("ariblk.ttf"), 24, sf::Color::Red, textPos);
                if(gamePtr) gamePtr->triggerScreenShake();
            }
        }

        for (auto it = damageTexts.begin(); it != damageTexts.end(); ) {
            it->update(dt.asSeconds());
            if (it->isExpired()) {
                it = damageTexts.erase(it);
            } else {
                ++it;
            }
        }

        if (showDebugHitboxes) {
            playerAttackHitboxShape.setPosition(playerRef.getAttackHitbox().left, playerRef.getAttackHitbox().top);
            playerAttackHitboxShape.setSize(sf::Vector2f(playerRef.getAttackHitbox().width, playerRef.getAttackHitbox().height));

            enemyAttackHitboxShape.setPosition(enemyRef.getAttackHitbox().left, enemyRef.getAttackHitbox().top);
            enemyAttackHitboxShape.setSize(sf::Vector2f(enemyRef.getAttackHitbox().width, enemyRef.getAttackHitbox().height));

            playerHurtboxShapeDebug.setPosition(playerRef.getHurtbox().left, playerRef.getHurtbox().top);
            playerHurtboxShapeDebug.setSize(sf::Vector2f(playerRef.getHurtbox().width, playerRef.getHurtbox().height));

            enemyHurtboxShapeDebug.setPosition(enemyRef.getHurtbox().left, enemyRef.getHurtbox().top);
            enemyHurtboxShapeDebug.setSize(sf::Vector2f(enemyRef.getHurtbox().width, enemyRef.getHurtbox().height));
        }
        else { // Hide debug shapes when not active
            playerAttackHitboxShape.setSize(sf::Vector2f(0,0));
            enemyAttackHitboxShape.setSize(sf::Vector2f(0,0));
            playerHurtboxShapeDebug.setSize(sf::Vector2f(0,0));
            enemyHurtboxShapeDebug.setSize(sf::Vector2f(0,0));
        }

        // Check win conditions: KO first
        if (!playerRef.isAlive || !enemyRef.isAlive) {
            if (!timerEnded) { // Only trigger game over once from health depletion
                gameResultState = GameStateID::GAME_OVER;
                timerEnded = true; // Set flag to prevent timer from interfering
            }
        }

        // Update timer
        float remainingTime = GameConfig::GAME_ROUND_DURATION - gameTimerClock.getElapsedTime().asSeconds();
        if (remainingTime <= 0.f) {
            remainingTime = 0.f; // Ensure time doesn't go negative for display
            if (!timerEnded) { // Timer ran out, determine winner by health
                if (playerRef.currentHealth > enemyRef.currentHealth) {
                    gameResultState = GameStateID::GAME_OVER;
                } else if (enemyRef.currentHealth > playerRef.currentHealth) {
                    gameResultState = GameStateID::GAME_OVER;
                } else {
                    gameResultState = GameStateID::GAME_OVER; // Indicate draw
                }
                timerEnded = true; // Set flag to prevent re-evaluation
            }
        }
        timerText.setString(Utils::formatTime(remainingTime));
    }

    void draw(sf::RenderWindow& window, const Player& playerRef, const Enemy& enemyRef) override {
        if(gameBgSpriteRef.getTexture()) window.draw(gameBgSpriteRef);
        else window.clear(sf::Color::Cyan); // Debug color if map doesn't draw

        playerRef.draw(window);
        enemyRef.draw(window);

        window.draw(playerUiPanel);
        window.draw(playerHealthBarBg);
        window.draw(playerHealthBar);
        window.draw(playerNameText_UI);

        window.draw(enemyUiPanel);
        window.draw(enemyHealthBarBg);
        window.draw(enemyHealthBar);
        window.draw(enemyNameText_UI);

        window.draw(timerText); // Draw the timer

        for (const auto& dt : damageTexts) {
            window.draw(dt.text);
        }

        if (showDebugHitboxes) {
            window.draw(playerAttackHitboxShape);
            window.draw(enemyAttackHitboxShape);
            window.draw(playerHurtboxShapeDebug);
            window.draw(enemyHurtboxShapeDebug);
        }
    }
};

// --- PauseScreen ---
class PauseScreen : public Screen {
public:
    sf::RectangleShape overlay;
    sf::Text pauseText;
    sf::RectangleShape resumeButton, restartButton, menuButtonShape;
    sf::Text resumeText, restartText, menuText;

    sf::Color defaultBtnColor = sf::Color(100, 100, 100, 180);
    sf::Color hoverBtnColor = sf::Color(150, 150, 150, 230);
    sf::Color resumeColor = sf::Color(100, 255, 100, 180);
    sf::Color resumeHoverColor = sf::Color(150, 255, 150, 230);
    sf::Color restartColor = sf::Color(255, 255, 100, 180);
    sf::Color restartHoverColor = sf::Color(255, 255, 150, 230);
    sf::Color menuBtnColor = sf::Color(255, 100, 100, 180);
    sf::Color menuBtnHoverColor = sf::Color(255, 150, 150, 230);
    sf::RenderWindow& m_windowRef;

    PauseScreen(sf::RenderWindow& window) : m_windowRef(window) {
        overlay.setFillColor(sf::Color(0, 0, 0, 150));

        pauseText.setFont(ResourceManager::getFont("ariblk.ttf"));
        pauseText.setString("PAUSED");
        pauseText.setCharacterSize(90);
        pauseText.setFillColor(sf::Color(255, 165, 0)); // Orange fill
        pauseText.setOutlineColor(sf::Color::Black);   // Black outline
        pauseText.setOutlineThickness(4);              // Prominent outline
        Utils::centerOrigin(pauseText);

        sf::Vector2f buttonSize(420, 85);
        sf::Color buttonTextColor = sf::Color::Black;

        resumeButton.setSize(buttonSize);
        resumeButton.setOutlineColor(sf::Color::White);
        resumeButton.setOutlineThickness(3);
        resumeText.setFont(ResourceManager::getFont("ariblk.ttf"));
        resumeText.setString("RESUME (ESC)");
        resumeText.setCharacterSize(50);
        resumeText.setFillColor(buttonTextColor);
        Utils::centerOrigin(resumeText);

        restartButton.setSize(buttonSize);
        restartButton.setOutlineColor(sf::Color::White);
        restartButton.setOutlineThickness(3);
        restartText.setFont(ResourceManager::getFont("ariblk.ttf"));
        restartText.setString("RESTART");
        restartText.setCharacterSize(50);
        restartText.setFillColor(buttonTextColor);
        Utils::centerOrigin(restartText);

        menuButtonShape.setSize(buttonSize);
        menuButtonShape.setOutlineColor(sf::Color::White);
        menuButtonShape.setOutlineThickness(3);
        menuText.setFont(ResourceManager::getFont("ariblk.ttf"));
        menuText.setString("MAIN MENU");
        menuText.setCharacterSize(50);
        menuText.setFillColor(buttonTextColor);
        Utils::centerOrigin(menuText);
    }

    void onEnter(const sf::RenderWindow& window, Player& playerRef, Enemy& enemyRef, const std::string& data) override {
        // Call onResize to ensure positions are correct based on current window size
        // Pass virtual resolution to onResize
        onResize(GameConfig::WINDOW_WIDTH, GameConfig::WINDOW_HEIGHT, playerRef, enemyRef);
    }

    void onResize(unsigned int width, unsigned int height, Player& playerRef, Enemy& enemyRef) override {
        // `overlay.setSize` should span the entire actual window. It's handled in Game::render directly for transition rect.
        // The screen elements here are positioned relative to the `width` and `height` parameters, which are the virtual resolution.
        pauseText.setPosition(width / 2.0f, height * 0.25f);

        float buttonX = width / 2.0f - resumeButton.getSize().x / 2.0f;
        float buttonYStart = height * 0.4f;
        float buttonGap = 25;

        resumeButton.setPosition(buttonX, buttonYStart);
        resumeText.setPosition(resumeButton.getPosition().x + resumeButton.getSize().x/2.f, resumeButton.getPosition().y + resumeButton.getSize().y/2.f);

        restartButton.setPosition(buttonX, buttonYStart + resumeButton.getSize().y + buttonGap);
        restartText.setPosition(restartButton.getPosition().x + restartButton.getSize().x/2.f, restartButton.getPosition().y + restartButton.getSize().y/2.f);

        menuButtonShape.setPosition(buttonX, restartButton.getPosition().y + restartButton.getSize().y + buttonGap);
        menuText.setPosition(menuButtonShape.getPosition().x + menuButtonShape.getSize().x/2.f, menuButtonShape.getPosition().y + menuButtonShape.getSize().y/2.f);
    }

    void handleEvent(sf::Event& event, sf::RenderWindow& window, GameStateID& nextState, bool& wantsTransition, GameStateID& gameResultState, Game* gamePtr) override {
        if (event.type == sf::Event::MouseButtonPressed) {
            sf::Vector2f mousePos = sf::Vector2f(sf::Mouse::getPosition(window)); // Get mouse in pixel coords
            // Mouse position needs to be transformed to game coordinates if view is letterboxed/pillarboxed
            sf::View gameView = window.getView();
            mousePos = window.mapPixelToCoords(sf::Vector2i(mousePos), gameView);

            if (resumeButton.getGlobalBounds().contains(mousePos)) {
                nextState = GameStateID::GAME_PLAY;
                wantsTransition = false; // No transition effect for resume
            } else if (restartButton.getGlobalBounds().contains(mousePos)) {
                nextState = GameStateID::GAME_PLAY;
                wantsTransition = true; // Restart needs a transition
            } else if (menuButtonShape.getGlobalBounds().contains(mousePos)) {
                nextState = GameStateID::MENU;
                wantsTransition = true; // Menu needs a transition
            }
        } else if (event.type == sf::Event::KeyPressed && event.key.code == sf::Keyboard::Escape) {
            nextState = GameStateID::GAME_PLAY; // Resume game on Escape
            wantsTransition = false;
        }
    }

    void update(sf::Time dt, sf::Vector2f mousePos, Player& playerRef, Enemy& enemyRef, GameStateID& gameResultState, Game* gamePtr = nullptr) override {
        // Mouse position needs to be transformed to game coordinates for hover effect
        sf::View gameView = m_windowRef.getView();
        mousePos = m_windowRef.mapPixelToCoords(sf::Vector2i(mousePos), gameView); // Ensure mousePos is sf::Vector2i

        resumeButton.setFillColor(resumeButton.getGlobalBounds().contains(mousePos) ? resumeHoverColor : resumeColor);
        restartButton.setFillColor(restartButton.getGlobalBounds().contains(mousePos) ? restartHoverColor : restartColor);
        menuButtonShape.setFillColor(menuButtonShape.getGlobalBounds().contains(mousePos) ? menuBtnHoverColor : menuBtnColor);
    }

    void draw(sf::RenderWindow& window, const Player& playerRef, const Enemy& enemyRef) override {
        // Drawing is relative to the current view which is already set by Game::render
        window.draw(overlay);
        window.draw(pauseText);
        window.draw(resumeButton); window.draw(resumeText);
        window.draw(restartButton); window.draw(restartText);
        window.draw(menuButtonShape); window.draw(menuText);
    }
};

// --- GameOverScreen ---
class GameOverScreen : public Screen {
public:
    sf::Text gameOverText;
    sf::Text resultText;
    sf::RectangleShape restartButton, menuButtonShape;
    sf::Text restartText, menuText;

    sf::Color restartColor = sf::Color(255, 255, 100, 180);
    sf::Color restartHoverColor = sf::Color(255, 255, 150, 230);
    sf::Color menuBtnColor = sf::Color(255, 100, 100, 180);
    sf::Color menuBtnHoverColor = sf::Color(255, 150, 150, 230);

    Game* m_gamePtr;
    sf::RenderWindow& m_windowRef;

    GameOverScreen(sf::RenderWindow& window, Game* gamePtr)
        : m_windowRef(window), m_gamePtr(gamePtr) {
        gameOverText.setFont(ResourceManager::getFont("ariblk.ttf"));
        gameOverText.setCharacterSize(100);
        gameOverText.setOutlineColor(sf::Color::Black);
        gameOverText.setOutlineThickness(5);

        resultText.setFont(ResourceManager::getFont("ariblk.ttf"));
        resultText.setCharacterSize(60);
        resultText.setOutlineColor(sf::Color::Black); // Add black outline
        resultText.setOutlineThickness(3);           // Prominent outline


        sf::Vector2f buttonSize(380, 75);
        sf::Color buttonTextColor = sf::Color::Black;

        restartButton.setSize(buttonSize);
        restartButton.setOutlineColor(sf::Color::White);
        restartButton.setOutlineThickness(3);
        restartText.setFont(ResourceManager::getFont("ariblk.ttf"));
        restartText.setString("RESTART");
        restartText.setCharacterSize(45);
        restartText.setFillColor(buttonTextColor);
        Utils::centerOrigin(restartText);

        menuButtonShape.setSize(buttonSize);
        menuButtonShape.setOutlineColor(sf::Color::White);
        menuButtonShape.setOutlineThickness(3);
        menuText.setFont(ResourceManager::getFont("ariblk.ttf"));
        menuText.setString("MAIN MENU");
        menuText.setCharacterSize(45);
        menuText.setFillColor(buttonTextColor);
        Utils::centerOrigin(menuText);
    }

    void onEnter(const sf::RenderWindow& window, Player& playerRef, Enemy& enemyRef, const std::string& gameOutcome) override {
        if (m_gamePtr) {
            std::string p1Name = m_gamePtr->playerNameFromInput.empty() ? "Player 1" : m_gamePtr->playerNameFromInput;
            std::string p2Name = (m_gamePtr->currentMode == GameMode::PvP) ?
                                 (m_gamePtr->player2NameFromInput.empty() ? "Player 2" : m_gamePtr->player2NameFromInput) :
                                 "Rival";

            // Determine outcome based on the `gameOutcome` string passed from Game::run
            if (gameOutcome == "P1_WON_BY_TIME") {
                gameOverText.setString("VICTORY!");
                gameOverText.setFillColor(sf::Color(255, 215, 0)); // Golden fill
                resultText.setString(p1Name + " WINS by Time!");
                resultText.setFillColor(sf::Color(50, 205, 50)); // Lime green fill
            } else if (gameOutcome == "P2_WON_BY_TIME") {
                gameOverText.setString("DEFEATED!");
                gameOverText.setFillColor(sf::Color(255, 69, 0)); // Red-orange fill
                resultText.setString(p2Name + " WINS by Time!");
                resultText.setFillColor(sf::Color(50, 205, 50)); // Lime green fill
            } else if (gameOutcome == "DRAW_BY_TIME") {
                gameOverText.setString("TIME OVER!");
                gameOverText.setFillColor(sf::Color(255, 165, 0)); // Orange fill
                resultText.setString("It's a DRAW!");
                resultText.setFillColor(sf::Color(255, 140, 0)); // Dark orange fill
            }
            else if (!playerRef.isAlive) { // P1 lost by KO
                gameOverText.setString("DEFEATED!");
                gameOverText.setFillColor(sf::Color(255, 69, 0)); // Red-orange fill
                resultText.setString(p2Name + " WINS!");
                resultText.setFillColor(sf::Color(50, 205, 50)); // Lime green fill
            } else if (!enemyRef.isAlive) { // P1 won by KO
                gameOverText.setString("VICTORY!");
                gameOverText.setFillColor(sf::Color(255, 215, 0)); // Golden fill
                resultText.setString(p1Name + " WINS!");
                resultText.setFillColor(sf::Color(50, 205, 50)); // Lime green fill
            } else { // Fallback, should not happen if all cases are covered
                gameOverText.setString("GAME OVER");
                gameOverText.setFillColor(sf::Color(255, 69, 0)); // Red-orange fill
                resultText.setString("DRAW!");
                resultText.setFillColor(sf::Color(255, 140, 0)); // Dark orange fill
            }
        }
        Utils::centerOrigin(gameOverText);
        Utils::centerOrigin(resultText);
        // Pass virtual resolution to onResize
        onResize(GameConfig::WINDOW_WIDTH, GameConfig::WINDOW_HEIGHT, playerRef, enemyRef);
    }

    void onResize(unsigned int width, unsigned int height, Player& playerRef, Enemy& enemyRef) override {
        gameOverText.setPosition(width / 2.0f, height * 0.25f);
        resultText.setPosition(width / 2.0f, height * 0.45f);

        float buttonX = width / 2.0f - restartButton.getSize().x / 2.0f;
        float buttonYStart = height * 0.6f;
        float buttonGap = 20;

        restartButton.setPosition(buttonX, buttonYStart);
        restartText.setPosition(restartButton.getPosition().x + restartButton.getSize().x/2.f, restartButton.getPosition().y + restartButton.getSize().y/2.f);

        menuButtonShape.setPosition(buttonX, restartButton.getPosition().y + restartButton.getSize().y + buttonGap);
        menuText.setPosition(menuButtonShape.getPosition().x + menuButtonShape.getSize().x/2.f, menuButtonShape.getPosition().y + menuButtonShape.getSize().y/2.f);
    }

    void handleEvent(sf::Event& event, sf::RenderWindow& window, GameStateID& nextState, bool& wantsTransition, GameStateID& gameResultState, Game* gamePtr) override {
         if (event.type == sf::Event::MouseButtonPressed) {
             sf::Vector2f mousePos = sf::Vector2f(sf::Mouse::getPosition(window)); // Get mouse in pixel coords
             // Transform mouse position to game coordinates for accurate hit detection
             sf::View gameView = window.getView();
             mousePos = window.mapPixelToCoords(sf::Vector2i(mousePos), gameView);

             if (restartButton.getGlobalBounds().contains(mousePos)) {
                 nextState = GameStateID::GAME_PLAY;
                 wantsTransition = true;
             } else if (menuButtonShape.getGlobalBounds().contains(mousePos)) {
                 nextState = GameStateID::MENU;
                 wantsTransition = true;
             }
         } else if (event.type == sf::Event::KeyPressed && event.key.code == sf::Keyboard::Enter) {
              nextState = GameStateID::GAME_PLAY;
              wantsTransition = true;
         }
    }

    void update(sf::Time dt, sf::Vector2f mousePos, Player& playerRef, Enemy& enemyRef, GameStateID& gameResultState, Game* gamePtr = nullptr) override {
        // Transform mouse position to game coordinates for hover effect
        sf::View gameView = m_windowRef.getView();
        mousePos = m_windowRef.mapPixelToCoords(sf::Vector2i(mousePos), gameView); // Ensure mousePos is sf::Vector2i

        restartButton.setFillColor(restartButton.getGlobalBounds().contains(mousePos) ? restartHoverColor : restartColor);
        menuButtonShape.setFillColor(menuButtonShape.getGlobalBounds().contains(mousePos) ? menuBtnHoverColor : menuBtnColor);
    }

    void draw(sf::RenderWindow& window, const Player& playerRef, const Enemy& enemyRef) override {
        window.clear(sf::Color(30,10,10, 200)); // Clear with a semi-transparent dark red
        window.draw(gameOverText);
        window.draw(resultText);
        window.draw(restartButton); window.draw(restartText);
        window.draw(menuButtonShape); window.draw(menuText);
    }
};


// --- Game Class Implementation ---
Game::Game() : window(sf::VideoMode(GameConfig::WINDOW_WIDTH, GameConfig::WINDOW_HEIGHT), "HellFire-Clash | OOP AI", sf::Style::Default | sf::Style::Resize), // Fixed size and resizable
               currentStateID(GameStateID::MENU), nextStateID(GameStateID::MENU), currentTransition(TransitionState::FADING_IN) {

    // GameConfig::WINDOW_WIDTH and GameConfig::WINDOW_HEIGHT are already set to 1280x720.
    // These are the virtual resolution that the game logic and UI are designed for.
    // The window is created with these dimensions initially and set to be resizable.
    window.setFramerateLimit(GameConfig::FRAMERATE_LIMIT);
    window.setVerticalSyncEnabled(true);

    ResourceManager::getFont("ariblk.ttf"); // Pre-load a default font

    screens[GameStateID::MENU] = std::make_unique<MenuScreen>(window);
    screens[GameStateID::NAME_INPUT] = std::make_unique<NameInputScreen>(window, playerNameFromInput, "PLAYER 1 ");
    dynamic_cast<NameInputScreen*>(screens[GameStateID::NAME_INPUT].get())->gamePtr = this;
    screens[GameStateID::NAME_INPUT_P2] = std::make_unique<NameInputScreen>(window, player2NameFromInput, "PLAYER 2 ");
    dynamic_cast<NameInputScreen*>(screens[GameStateID::NAME_INPUT_P2].get())->gamePtr = this;
    screens[GameStateID::MODE_SELECTION] = std::make_unique<ModeSelectionScreen>(window);
    screens[GameStateID::CHARACTER_SELECTION] = std::make_unique<CharacterSelectionScreen>(window); // New screen for character selection
    screens[GameStateID::MAP_SELECTION] = std::make_unique<MapSelectionScreen>(window, currentMapSelection);
    screens[GameStateID::GAME_PLAY] = std::make_unique<GamePlayScreen>(window, this, gameBackgroundSprite);
    screens[GameStateID::PAUSE] = std::make_unique<PauseScreen>(window);
    screens[GameStateID::GAME_OVER] = std::make_unique<GameOverScreen>(window, this);


    // Initialize transitionRect to cover the initial window size
    transitionRect.setSize(sf::Vector2f((float)window.getSize().x, (float)window.getSize().y));
    transitionRect.setFillColor(sf::Color::Black);

    screens[currentStateID]->onEnter(window, player, enemy); // Call onEnter for the initial screen
    handleResize(window.getSize().x, window.getSize().y); // Initial call to set up view and element positions
    transitionClock.restart(); // Start transition clock for initial fade-in
}

void Game::triggerScreenShake() {
    isShaking = true;
    shakeClock.restart();
}

void Game::updateScreenShake(sf::Time dt) {
    if (isShaking) {
        float elapsed = shakeClock.getElapsedTime().asSeconds();
        if (elapsed >= GameConfig::SCREEN_SHAKE_DURATION) {
            isShaking = false;
            shakeOffset = sf::Vector2f(0, 0); // Reset offset when shake ends
        } else {
            float intensity = GameConfig::SCREEN_SHAKE_MAX_OFFSET * (1.f - (elapsed / GameConfig::SCREEN_SHAKE_DURATION));
            shakeOffset.x = Utils::randomFloat(-intensity, intensity);
            shakeOffset.y = Utils::randomFloat(-intensity, intensity);
        }
    }
}


void Game::run() {
    while (window.isOpen()) {
        sf::Time dt = gameClock.restart();
        if (dt.asSeconds() > (1.0f / 20.0f)) { // Cap delta time to prevent physics glitches on lag spikes
             dt = sf::seconds(1.0f / 60.0f);
        }

        processEvents(); // Handle user input and window events
        if (currentTransition == TransitionState::NONE) { // Only update game logic if not transitioning
             update(dt * gameTimeScale);
        }
        handleScreenTransition(dt); // Manage screen fade in/out
        render(); // Draw everything to the window

        // Check for game over condition and trigger screen change
        if (currentStateID == GameStateID::GAME_PLAY && gameResultState != GameStateID::GAME_PLAY) {
            std::string outcomeMessage = "";
            // Determine outcome message if GamePlayScreen set gameResultState to GAME_OVER due to timer
            GamePlayScreen* gs = dynamic_cast<GamePlayScreen*>(screens[GameStateID::GAME_PLAY].get());
            if (gs && gs->timerEnded) { // Check GamePlayScreen's timerEnded flag
                if (player.currentHealth > enemy.currentHealth) {
                    outcomeMessage = "P1_WON_BY_TIME";
                } else if (enemy.currentHealth > player.currentHealth) {
                    outcomeMessage = "P2_WON_BY_TIME";
                } else {
                    outcomeMessage = "DRAW_BY_TIME";
                }
            }
            changeScreen(gameResultState, outcomeMessage); // Pass outcome message to changeScreen
        }
    }
}

void Game::processEvents() {
    sf::Event event;
    while (window.pollEvent(event)) {
        if (event.type == sf::Event::Closed) {
            window.close(); // Close window when close button is clicked
        }
        if (event.type == sf::Event::Resized) {
            // GameConfig::WINDOW_WIDTH and GameConfig::WINDOW_HEIGHT are the VIRTUAL resolution.
            // They are NOT updated by the actual window resize event.
            // The handleResize function should adapt the view to the new ACTUAL window size.
            handleResize(event.size.width, event.size.height);
        }

        bool wantsTransition = false; // Flag to indicate if a screen change is requested
        GameStateID potentialNextState = currentStateID; // Store potential next state

        if (currentStateID == GameStateID::PAUSE) { // Special handling for pause state
             screens[GameStateID::PAUSE]->handleEvent(event, window, potentialNextState, wantsTransition, gameResultState, this);
             if (wantsTransition) { // If pause screen requests a state change (e.g., restart, main menu)
                 changeScreen(potentialNextState);
             } else if (potentialNextState == GameStateID::GAME_PLAY && currentStateID == GameStateID::PAUSE) {
                 currentStateID = GameStateID::GAME_PLAY; // Resume game
                 gameTimeScale = 1.0f; // Restore game time
             }
        } else if (currentTransition != TransitionState::FADING_OUT) { // Only handle events if not fading out
            screens[currentStateID]->handleEvent(event, window, potentialNextState, wantsTransition, gameResultState, this);
            if (potentialNextState == GameStateID::PAUSE && currentStateID == GameStateID::GAME_PLAY && !wantsTransition) {
                currentStateID = GameStateID::PAUSE; // Pause game
                gameTimeScale = 0.0f; // Stop game time
                screens[GameStateID::PAUSE]->onEnter(window, player, enemy); // Call onEnter for pause screen
            } else if (wantsTransition && currentTransition == TransitionState::NONE) {
                changeScreen(potentialNextState); // Trigger screen change
            }
        }
    }
}

void Game::update(sf::Time dt) {
    // Mouse position needs to be transformed to game coordinates
    // This assumes the current view is the game's logical view (1280x720 scaled by Game::handleResize)
    sf::Vector2f mousePos = window.mapPixelToCoords(sf::Mouse::getPosition(window), window.getView());

    updateScreenShake(dt); // Update screen shake effect

    // Update the current active screen
    if (currentStateID == GameStateID::PAUSE) {
        screens[GameStateID::PAUSE]->update(dt, mousePos, player, enemy, gameResultState, this);
    } else {
        screens[currentStateID]->update(dt, mousePos, player, enemy, gameResultState, this);
    }

    // Update background animation for GamePlayScreen if active and not paused
    if (currentStateID == GameStateID::GAME_PLAY && gameTimeScale > 0.f) {
        if (currentGameBackgroundFrames && !currentGameBackgroundFrames->empty()) {
            bgTimer += dt.asSeconds();
            if (bgTimer >= MAP_FRAME_DELAY) {
                bgTimer -= MAP_FRAME_DELAY;
                bgFrame = (bgFrame + 1) % currentGameBackgroundFrames->size();
                gameBackgroundSprite.setTexture((*currentGameBackgroundFrames)[bgFrame]);
            }
        }
    }
}

void Game::render() {
    window.clear(sf::Color::Black); // Clear to black for letterboxing/pillarboxing

    // Get the base view setup by handleResize (for aspect ratio)
    sf::View currentWindowView = window.getView();
    sf::View gameContentOnlyView = currentWindowView; // Copy this view for game content

    // Apply screen shake to the game content view if active during gameplay
    if (isShaking && currentStateID == GameStateID::GAME_PLAY) {
        gameContentOnlyView.move(shakeOffset);
    }
    window.setView(gameContentOnlyView); // Set view for drawing game elements

    // Draw the current active screen (or GamePlayScreen if paused)
    if (currentStateID == GameStateID::PAUSE) {
        if(screens.count(GameStateID::GAME_PLAY)) {
            screens[GameStateID::GAME_PLAY]->draw(window, player, enemy); // Draw underlying game state
        }
        screens[GameStateID::PAUSE]->draw(window, player, enemy); // Draw pause menu on top
    } else {
        screens[currentStateID]->draw(window, player, enemy);
    }

    window.setView(currentWindowView); // Revert to the non-shaking view for transitionRect to cover the whole window

    // Draw transition rectangle if a transition is active
    if (currentTransition != TransitionState::NONE) {
        window.draw(transitionRect);
    }
    window.display(); // Display rendered frame
}

// Triggers a screen transition
void Game::changeScreen(GameStateID newStateID, const std::string& onEnterData /* = "" */) {
    if (currentTransition == TransitionState::NONE) { // Only allow transition if no transition is active
        nextStateID = newStateID; // Set the target state
        currentTransition = TransitionState::FADING_OUT; // Start fade-out
        transitionClock.restart(); // Reset transition timer

        if (currentStateID != GameStateID::PAUSE) { // Stop game time if not already paused
             gameTimeScale = 0.0f;
        }
        if (screens.count(currentStateID)) screens[currentStateID]->onExit(); // Call onExit for current screen
    }
}

// Handles the fade in/out effect during screen transitions
void Game::handleScreenTransition(sf::Time dt) {
    if (currentTransition == TransitionState::NONE) {
        // Ensure gameTimeScale is correct when no transition is active
        gameTimeScale = (currentStateID == GameStateID::PAUSE || currentStateID == GameStateID::GAME_OVER) ? 0.0f : 1.0f;
        return;
    }

    float elapsed = transitionClock.getElapsedTime().asSeconds();
    float t = Utils::clamp(elapsed / GameConfig::TRANSITION_DURATION, 0.0f, 1.0f);
    sf::Uint8 alpha;

    if (currentTransition == TransitionState::FADING_OUT) {
        alpha = static_cast<sf::Uint8>(Utils::lerp(0.f, 255.f, t)); // Fade from transparent to opaque black
        transitionRect.setFillColor(sf::Color(0, 0, 0, alpha));
        if (t >= 1.0f) { // Fade-out complete
            currentStateID = nextStateID; // Change to the new state
            std::string onEnterDataForNextScreen = ""; // This will be populated if needed by a special case

            // Special handling for GAME_PLAY state entry
            if (currentStateID == GameStateID::GAME_PLAY) {
                // Load selected character assets for player and enemy
                player.loadCharacterAssets(selectedPlayer1Char);
                enemy.loadCharacterAssets(selectedEnemyChar);

                player.name = playerNameFromInput.empty() ? "Player 1" : playerNameFromInput;
                player.reset(); // Reset player state
                player.resetPosition(GameConfig::WINDOW_WIDTH * 0.25f); // Set player start position

                enemy.isPlayerControlled = (currentMode == GameMode::PvP); // Set if enemy is controlled by player 2
                enemy.name = (enemy.isPlayerControlled) ?
                                 (player2NameFromInput.empty() ? "Player 2" : player2NameFromInput) :
                                 "Rival"; // Set enemy name
                enemy.reset(); // Reset enemy state (no xPos param)
                enemy.resetPosition(GameConfig::WINDOW_WIDTH * 0.75f); // Set enemy start position

                // Set common ground Y for both characters
                float commonGroundY = GameConfig::WINDOW_HEIGHT - (player.frameHeight * player.spriteScale) - 20;
                player.setGroundY(commonGroundY);
                enemy.setGroundY(commonGroundY);
                gameResultState = GameStateID::GAME_PLAY; // Reset game result state for new game

                bool mapLoadSuccess = false;
                currentGameBackgroundFrames = nullptr;
                if (currentMapSelection == 0) currentMapSelection = 1; // Default to Map 1 if none chosen

                // Load map frames based on selection
                if (currentMapSelection == 1) {
                    if (!map1Loaded) map1Loaded = ResourceManager::loadMapFrames(map1Frames, 7, "frame_", "_delay-0.11s.png");
                    if (map1Loaded) { currentGameBackgroundFrames = &map1Frames; mapLoadSuccess = true; }
                } else if (currentMapSelection == 2) {
                    // Changed to 20 frames, no prefix, .png suffix, starting from 1, with 6 zero padding
                    if (!map2Loaded) map2Loaded = ResourceManager::loadMapFrames(map2Frames, 20, "", ".png", 1, 6);
                    if (map2Loaded) { currentGameBackgroundFrames = &map2Frames; mapLoadSuccess = true; }
                } else if (currentMapSelection == 3) { // Load Map 3
                    if (!map3Loaded) map3Loaded = ResourceManager::loadMapFrames(map3Frames, 8, "bg1.", ".png", 1);
                    if (map3Loaded) { currentGameBackgroundFrames = &map3Frames; mapLoadSuccess = true; }
                }

                // Set game background sprite
                if (mapLoadSuccess && currentGameBackgroundFrames && !currentGameBackgroundFrames->empty()) {
                    gameBackgroundSprite.setTexture((*currentGameBackgroundFrames)[0]);
                    bgFrame = 0; bgTimer = 0.f;
                } else {
                    std::cerr << "Error: Failed to load map " << currentMapSelection << " assets. Attempting to load default Map 1." << std::endl;
                    // Fallback to Map 1 if selected map fails to load
                    if(!map1Loaded) map1Loaded = ResourceManager::loadMapFrames(map1Frames, 8, "frame_", "_delay-0.11s.png");
                    if(map1Loaded) { currentGameBackgroundFrames = &map1Frames; gameBackgroundSprite.setTexture((*currentGameBackgroundFrames)[0]); }
                    else { currentStateID = GameStateID::MENU; nextStateID = GameStateID::MENU; std::cerr << "Critical: No maps could be loaded. Returning to menu." << std::endl; }
                }
            }
            // Call onEnter for the new screen state.
            // The `onEnterData` parameter to `changeScreen` is passed here.
            screens[currentStateID]->onEnter(window, player, enemy, onEnterDataForNextScreen); // This still needs to be correctly populated if Game::run passes data.
                                                                                              // For now, rely on Game::run logic directly setting the outcome string.
            handleResize(window.getSize().x, window.getSize().y);

            currentTransition = TransitionState::FADING_IN; // Start fade-in
            transitionClock.restart();
        }
    } else if (currentTransition == TransitionState::FADING_IN) {
        alpha = static_cast<sf::Uint8>(Utils::lerp(255.f, 0.f, t)); // Fade from opaque black to transparent
        transitionRect.setFillColor(sf::Color(0, 0, 0, alpha));
        if (t >= 1.0f) { // Fade-in complete
            currentTransition = TransitionState::NONE; // No active transition
            // Restore game time scale for the current state
            gameTimeScale = (currentStateID == GameStateID::PAUSE || currentStateID == GameStateID::GAME_OVER) ? 0.0f : 1.0f;
             if(currentStateID == GameStateID::MENU || currentStateID == GameStateID::NAME_INPUT ||
                 currentStateID == GameStateID::NAME_INPUT_P2 || currentStateID == GameStateID::MODE_SELECTION ||
                 currentStateID == GameStateID::CHARACTER_SELECTION || currentStateID == GameStateID::MAP_SELECTION) {
                 // Restart specific clocks for animations/cursors in these screens
                 auto* menuScreenPtr = dynamic_cast<MenuScreen*>(screens[currentStateID].get());
                 if(menuScreenPtr) menuScreenPtr->menuAnimClock.restart();
                 auto* nameInputScreenPtr = dynamic_cast<NameInputScreen*>(screens[currentStateID].get());
                 if(nameInputScreenPtr) nameInputScreenPtr->cursorBlinkClock.restart();
             }
        }
    }
}

// Handles window resizing event to maintain aspect ratio
void Game::handleResize(unsigned int width, unsigned int height) {
    // GameConfig::WINDOW_WIDTH and GameConfig::WINDOW_HEIGHT are the VIRTUAL resolution (1280x720).
    // They are NOT updated by the actual window resize event.
    // The view is adjusted to map the virtual resolution to the actual window size,
    // maintaining aspect ratio with letterboxing/pillarboxing.

    float virtualWidth = static_cast<float>(GameConfig::WINDOW_WIDTH);
    float virtualHeight = static_cast<float>(GameConfig::WINDOW_HEIGHT);
    float virtualAspectRatio = virtualWidth / virtualHeight;

    float actualWidth = static_cast<float>(width);
    float actualHeight = static_cast<float>(height);
    float windowAspectRatio = actualWidth / actualHeight;

    // Create a new view with the logical game world size (1280x720)
    sf::View newView(sf::FloatRect(0, 0, virtualWidth, virtualHeight));

    sf::FloatRect viewport(0, 0, 1, 1); // Normalized viewport (0 to 1)

    if (windowAspectRatio > virtualAspectRatio) { // Window is wider, add pillarboxing (black bars on sides)
        float scale = actualHeight / virtualHeight;
        float viewportWidth = virtualWidth * scale;
        float letterboxSize = (actualWidth - viewportWidth) / 2.0f;
        viewport = sf::FloatRect(letterboxSize / actualWidth, 0.0f, viewportWidth / actualWidth, 1.0f);
    } else { // Window is taller or same aspect ratio, add letterboxing (black bars on top/bottom)
        float scale = actualWidth / virtualWidth;
        float viewportHeight = virtualHeight * scale;
        float pillarboxSize = (actualHeight - viewportHeight) / 2.0f;
        viewport = sf::FloatRect(0.0f, pillarboxSize / actualHeight, 1.0f, viewportHeight / actualHeight);
    }

    newView.setViewport(viewport); // Apply the calculated viewport
    window.setView(newView); // Set the window's view

    // Transition rectangle needs to cover the entire physical window, not just the scaled viewport
    transitionRect.setSize(sf::Vector2f(actualWidth, actualHeight));
    transitionRect.setPosition(0,0);

    // Now call onResize for the current screen(s), passing the fixed virtual width/height
    if (screens.count(currentStateID) && screens[currentStateID]) {
        screens[currentStateID]->onResize(virtualWidth, virtualHeight, player, enemy);
    }
    // If paused, the underlying GamePlayScreen also needs its elements resized (e.g., UI elements)
    if (currentStateID == GameStateID::PAUSE && screens.count(GameStateID::GAME_PLAY) && screens[GameStateID::GAME_PLAY]) {
        screens[GameStateID::GAME_PLAY]->onResize(virtualWidth, virtualHeight, player, enemy);
    }
}