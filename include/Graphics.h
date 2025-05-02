#pragma once

#include <SFML/Graphics.hpp>
#include "GameState.h" // Includes Common.h
#include <vector>
#include <string>
#include <map> // For button storage

class Graphics {
public:
    // --- Constants ---
    // Keep integral constants here if needed, or move all for consistency
    static const int SQUARE_SIZE = 60;
    static const int BOARD_OFFSET_X = 50;
    static const int BOARD_OFFSET_Y = 50;
    static const int UI_PANEL_X = BOARD_OFFSET_X + BOARD_COLS * SQUARE_SIZE + 20;
    static const int UI_PANEL_WIDTH = 250;
    static const int UI_BUTTON_WIDTH = 60;
    static const int UI_BUTTON_HEIGHT = 30;
    static const int UI_BUTTON_PADDING = 10;

    //vvv MODIFIED vvv --- Declare static const floats (no initialization) --- vvv
    static const float INDICATOR_RADIUS;
    static const float INDICATOR_X;
    static const float INDICATOR_Y;
    //^^^ MODIFIED ^^^-------------------------------------------------------^^^


    // --- Constructor & Asset Loading ---
    Graphics();
    void loadAssets();

    // --- Core Drawing Function ---
    void drawBoard(sf::RenderWindow& window,
                   const GameState& gameState,
                   AppMode currentMode,
                   Player setupPlayer,
                   PieceType selectedSetupPiece,
                   bool gameOver, // Needed to hide indicator when game ends
                   const std::vector<Move>& legalMoveHighlights,
                   int selectedRow, int selectedCol,
                   const Move& lastAiMove);


    // --- Utility Functions ---
    sf::Vector2i getClickedSquare(const sf::Vector2i& mousePos) const;
    PieceType getClickedSetupPieceButton(const sf::Vector2i& mousePos) const;
    bool isClickOnClearButton(const sf::Vector2i& mousePos) const;
    bool isClickOnSideButton(const sf::Vector2i& mousePos) const;
    bool isClickOnFinishButton(const sf::Vector2i& mousePos) const;


    // --- Method to toggle display mode ---
    void togglePieceDisplay();
    // --- Method to toggle board orientation ---
    void toggleBoardFlip();

private:
    // --- Private Members ---
    sf::Font font;
    int pieceDisplayMode = 0;
    bool boardFlipped = true; // Default: P1 (Blue) at bottom (visually)

    // UI Element Storage
    struct ButtonUI { sf::RectangleShape shape; sf::Text label; sf::FloatRect bounds; };
    std::map<PieceType, ButtonUI> pieceButtons;
    ButtonUI clearButton; ButtonUI sideButton; ButtonUI finishButton;
    sf::CircleShape turnIndicatorDot;

    // --- Private Helper Functions ---
    void setupUIElements();
    sf::Vector2f getScreenPos(int r, int c) const;
    void drawGrid(sf::RenderWindow& window, const GameState& gameState);
    void drawPieces(sf::RenderWindow& window, const GameState& gameState);
    void drawHighlights(sf::RenderWindow& window,
                        const std::vector<Move>& legalMoveHighlights,
                        int selectedRow, int selectedCol,
                        const Move& lastAiMove);
    void drawSetupUI(sf::RenderWindow& window, Player setupPlayer, PieceType selectedSetupPiece);
    void drawTurnIndicator(sf::RenderWindow& window, const GameState& gameState);
};


