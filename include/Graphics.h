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
    //vvv NEW vvv --- UI Constants --- vvv
    static const int UI_PANEL_X = BOARD_OFFSET_X + BOARD_COLS * SQUARE_SIZE + 20;
    static const int UI_PANEL_WIDTH = 250; // Adjust as needed
    static const int UI_BUTTON_WIDTH = 60;
    static const int UI_BUTTON_HEIGHT = 30;
    static const int UI_BUTTON_PADDING = 10;
    //^^^ NEW ^^^----------------------^^^


    // --- Constructor & Asset Loading ---
    Graphics();
    void loadAssets();

    // --- Core Drawing Function ---
    //vvv MODIFIED vvv --- Added AppMode parameter --- vvv
    void drawBoard(sf::RenderWindow& window,
                   const GameState& gameState,
                   AppMode currentMode, // To know whether to draw setup UI
                   Player setupPlayer, // To color setup buttons
                   PieceType selectedSetupPiece, // To highlight selected piece button
                   const std::vector<Move>& legalMoveHighlights,
                   int selectedRow, int selectedCol,
                   const Move& lastAiMove);
    //^^^ MODIFIED ^^^---------------------------------^^^


    // --- Utility Functions ---
    sf::Vector2i getClickedSquare(const sf::Vector2i& mousePos) const;
    //vvv NEW vvv --- Check clicks on UI elements --- vvv
    // Returns piece type if a piece button clicked, EMPTY otherwise (use special value for other buttons?)
    // Let's return EMPTY for non-piece button clicks for now and check bounds in main.
    PieceType getClickedSetupPieceButton(const sf::Vector2i& mousePos) const;
    bool isClickOnClearButton(const sf::Vector2i& mousePos) const;
    bool isClickOnSideButton(const sf::Vector2i& mousePos) const;
    bool isClickOnFinishButton(const sf::Vector2i& mousePos) const;
    //^^^ NEW ^^^-------------------------------------^^^


    // --- Method to toggle display mode ---
    void togglePieceDisplay();

private:
    // --- Private Members ---
    sf::Font font;
    int pieceDisplayMode = 0; // 0=L+N, 1=N+L, 2=L only

    //vvv NEW vvv --- UI Element Storage --- vvv
    struct ButtonUI {
        sf::RectangleShape shape;
        sf::Text label;
        sf::FloatRect bounds; // Store bounds for click detection
    };
    std::map<PieceType, ButtonUI> pieceButtons;
    ButtonUI clearButton;
    ButtonUI sideButton;
    ButtonUI finishButton;
    //^^^ NEW ^^^----------------------------^^^

    // --- Private Helper Functions ---
    void setupUIElements(); // Helper to create buttons
    void drawGrid(sf::RenderWindow& window, const GameState& gameState);
    void drawPieces(sf::RenderWindow& window, const GameState& gameState);
    void drawHighlights(sf::RenderWindow& window,
                        const std::vector<Move>& legalMoveHighlights,
                        int selectedRow, int selectedCol,
                        const Move& lastAiMove);
    //vvv NEW vvv --- Draw Setup UI --- vvv
    void drawSetupUI(sf::RenderWindow& window, Player setupPlayer, PieceType selectedSetupPiece);
    //^^^ NEW ^^^-----------------------^^^
};


