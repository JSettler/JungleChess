#pragma once

#include <SFML/Graphics.hpp>
#include "GameState.h" // Includes Common.h
#include <vector>
#include <string>
#include <map> // For button storage

class Graphics {
public:
    // --- Constants ---
    static const int SQUARE_SIZE = 60;
    static const int BOARD_OFFSET_X = 50;
    static const int BOARD_OFFSET_Y = 50;
    static const int UI_PANEL_X = BOARD_OFFSET_X + BOARD_COLS * SQUARE_SIZE + 20;
    static const int UI_PANEL_WIDTH = 250;
    static const int UI_BUTTON_WIDTH = 140; // Keep wider buttons for consistency? Or adjust? Let's keep 140 for now.
    static const int UI_BUTTON_HEIGHT = 30;
    static const int UI_BUTTON_PADDING = 10;

    static const float INDICATOR_RADIUS;
    static const float INDICATOR_X;
    static const float INDICATOR_Y;


    // --- Constructor & Asset Loading ---
    Graphics();
    void loadAssets();

    // --- Core Drawing Function ---
    void drawBoard(sf::RenderWindow& window,
                   const GameState& gameState,
                   AppMode currentMode,
                   Player setupPlayer,
                   PieceType selectedSetupPiece,
                   bool gameOver,
                   // Highlighting data:
                   const std::vector<Move>& legalMoveHighlights,
                   int selectedRow, int selectedCol,
                   const Move& lastAiMove,
                   // Book Editor Highlights
                   const std::vector<sf::Vector2i>& bookStartingSquares,
                   const std::vector<sf::Vector2i>& bookTargetSquares,
                   // <<< NEW: State for Game UI >>>
                   bool isBookEnabled,
                   int currentSearchDepth
                  );


    // --- Utility Functions ---
    sf::Vector2i getClickedSquare(const sf::Vector2i& mousePos) const;
    // Setup UI Click Detectors
    PieceType getClickedSetupPieceButton(const sf::Vector2i& mousePos) const;
    bool isClickOnClearButton(const sf::Vector2i& mousePos) const;
    bool isClickOnSideButton(const sf::Vector2i& mousePos) const;
    bool isClickOnFinishButton(const sf::Vector2i& mousePos) const;
    // Book Editor UI Click Detectors
    bool isClickOnSaveLineButton(const sf::Vector2i& mousePos) const;
    bool isClickOnResetBoardButton(const sf::Vector2i& mousePos) const;
    bool isClickOnExitEditorButton(const sf::Vector2i& mousePos) const;
    bool isClickOnUndoEditorButton(const sf::Vector2i& mousePos) const;
    // <<< NEW: Game UI Click Detectors >>>
    bool isClickOnBookToggleButton(const sf::Vector2i& mousePos) const;
    bool isClickOnDepthAdjustButton(const sf::Vector2i& mousePos) const;


    // --- Method to toggle display mode ---
    void togglePieceDisplay();
    // --- Method to toggle board orientation ---
    void toggleBoardFlip();

private:
    // --- Private Members ---
    sf::Font font;
    int pieceDisplayMode = 0;
    bool boardFlipped = true;

    // UI Element Storage
    struct ButtonUI { sf::RectangleShape shape; sf::Text label; sf::FloatRect bounds; };
    // Setup UI
    std::map<PieceType, ButtonUI> pieceButtons;
    ButtonUI clearButton; ButtonUI sideButton; ButtonUI finishButton;
    // Game UI
    sf::CircleShape turnIndicatorDot;
    // <<< NEW: Game Mode Buttons >>>
    ButtonUI bookToggleButton;
    ButtonUI depthAdjustButton;
    // Book Editor UI
    ButtonUI saveLineButton;
    ButtonUI resetBoardButton;
    ButtonUI exitEditorButton;
    ButtonUI undoEditorButton;


    // --- Private Helper Functions ---
    void setupButton(ButtonUI& button, const std::string& text, float x, float y, float width, float height);
    void setupUIElements();
    sf::Vector2f getScreenPos(int r, int c) const;
    void drawGrid(sf::RenderWindow& window, const GameState& gameState);
    void drawPieces(sf::RenderWindow& window, const GameState& gameState);
    void drawHighlights(sf::RenderWindow& window,
                        AppMode currentMode, // Need mode to decide which highlights to draw
                        const std::vector<Move>& legalMoveHighlights,
                        int selectedRow, int selectedCol,
                        const Move& lastAiMove,
                        const std::vector<sf::Vector2i>& bookStartingSquares,
                        const std::vector<sf::Vector2i>& bookTargetSquares
                       );
    void drawSetupUI(sf::RenderWindow& window, Player setupPlayer, PieceType selectedSetupPiece);
    void drawTurnIndicator(sf::RenderWindow& window, const GameState& gameState);
    void drawBookEditorUI(sf::RenderWindow& window);
    // <<< NEW: Draw Game UI >>>
    void drawGameUI(sf::RenderWindow& window, bool isBookEnabled, int currentSearchDepth);
};


