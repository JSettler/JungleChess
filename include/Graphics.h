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
    static const int UI_BUTTON_WIDTH = 60;
    static const int UI_BUTTON_HEIGHT = 30;
    static const int UI_BUTTON_PADDING = 10;


    // --- Constructor & Asset Loading ---
    Graphics();
    void loadAssets();

    // --- Core Drawing Function ---
    void drawBoard(sf::RenderWindow& window,
                   const GameState& gameState,
                   AppMode currentMode,
                   Player setupPlayer,
                   PieceType selectedSetupPiece,
                   const std::vector<Move>& legalMoveHighlights,
                   int selectedRow, int selectedCol,
                   const Move& lastAiMove);

    // --- Utility Functions ---
    sf::Vector2i getClickedSquare(const sf::Vector2i& mousePos) const; // Needs update for flip
    PieceType getClickedSetupPieceButton(const sf::Vector2i& mousePos) const;
    bool isClickOnClearButton(const sf::Vector2i& mousePos) const;
    bool isClickOnSideButton(const sf::Vector2i& mousePos) const;
    bool isClickOnFinishButton(const sf::Vector2i& mousePos) const;


    // --- Method to toggle display mode ---
    void togglePieceDisplay();
    //vvv NEW vvv --- Method to toggle board orientation --- vvv
    void toggleBoardFlip();
    //^^^ NEW ^^^------------------------------------------^^^

private:
    // --- Private Members ---
    sf::Font font;
    int pieceDisplayMode = 0;
    //vvv NEW vvv --- Flag for board orientation --- vvv
    bool boardFlipped = false; // Default: P1 (Blue) at bottom
    //^^^ NEW ^^^----------------------------------^^^

    // UI Element Storage
    struct ButtonUI { sf::RectangleShape shape; sf::Text label; sf::FloatRect bounds; };
    std::map<PieceType, ButtonUI> pieceButtons;
    ButtonUI clearButton; ButtonUI sideButton; ButtonUI finishButton;

    // --- Private Helper Functions ---
    void setupUIElements();
    //vvv NEW vvv --- Helper to get screen coords from board coords --- vvv
    sf::Vector2f getScreenPos(int r, int c) const;
    //^^^ NEW ^^^-----------------------------------------------------^^^
    void drawGrid(sf::RenderWindow& window, const GameState& gameState);
    void drawPieces(sf::RenderWindow& window, const GameState& gameState);
    void drawHighlights(sf::RenderWindow& window,
                        const std::vector<Move>& legalMoveHighlights,
                        int selectedRow, int selectedCol,
                        const Move& lastAiMove);
    void drawSetupUI(sf::RenderWindow& window, Player setupPlayer, PieceType selectedSetupPiece);
};


