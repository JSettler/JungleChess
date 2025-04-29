#pragma once

#include <SFML/Graphics.hpp>
#include "GameState.h" // Includes Common.h
#include <vector>
#include <string>

class Graphics {
public:
    // --- Constants ---
    static const int SQUARE_SIZE = 60;
    static const int BOARD_OFFSET_X = 50;
    static const int BOARD_OFFSET_Y = 50;

    // --- Constructor & Asset Loading ---
    Graphics();
    void loadAssets();

    // --- Core Drawing Function ---
    void drawBoard(sf::RenderWindow& window,
                   const GameState& gameState,
                   const std::vector<Move>& legalMoveHighlights,
                   int selectedRow, int selectedCol,
                   const Move& lastAiMove);

    // --- Utility Functions ---
    sf::Vector2i getClickedSquare(const sf::Vector2i& mousePos) const;

    // --- Method to toggle display mode ---
    void togglePieceDisplay();

private:
    // --- Private Members ---
    sf::Font font;

    //vvv MODIFIED vvv --- Changed bool to int for 3 modes --- vvv
    // 0: Letters main, Numbers sub
    // 1: Numbers main, Letters sub
    // 2: Letters only
    int pieceDisplayMode = 0;
    //^^^ MODIFIED ^^^---------------------------------------^^^

    // --- Private Helper Functions ---
    void drawGrid(sf::RenderWindow& window, const GameState& gameState);
    void drawPieces(sf::RenderWindow& window, const GameState& gameState);
    void drawHighlights(sf::RenderWindow& window,
                        const std::vector<Move>& legalMoveHighlights,
                        int selectedRow, int selectedCol,
                        const Move& lastAiMove);
};


