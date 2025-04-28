#pragma once
#include <SFML/Graphics.hpp>
#include "GameState.h" // Include GameState here
#include "Common.h"    // Include Common for Move struct
#include <map>
#include <utility>
#include <vector>      // Include vector for legal moves list

class Graphics {
public:
    Graphics();

    //vvv MODIFIED vvv --- Added highlight parameters --- vvv
    void drawBoard(sf::RenderWindow& window,
                   const GameState& gameState,
                   const std::vector<Move>& legalMoveHighlights, // Moves for selected piece
                   int selectedRow, int selectedCol,             // Coords of selected piece (-1 if none)
                   const Move& lastAiMove);                      // Last move AI made
    //^^^ MODIFIED ^^^------------------------------------^^^

    void loadAssets();
    sf::Vector2i getClickedSquare(const sf::Vector2i& mousePos) const;

private:
    // Board drawing constants
    const float SQUARE_SIZE = 64.0f;
    const float BOARD_OFFSET_X = 50.0f;
    const float BOARD_OFFSET_Y = 50.0f;

    // SFML resources
    std::map<std::pair<PieceType, Player>, sf::Texture> pieceTextures;
    sf::Font font;

    // Highlight colors (Defined here for easy access in drawHighlights)
    const sf::Color selectedPieceColor = sf::Color(255, 255, 0, 100); // Semi-transparent Yellow outline
    const sf::Color legalMoveColor = sf::Color(0, 255, 0, 100);     // Semi-transparent Green fill
    const sf::Color lastAiMoveColor = sf::Color(255, 0, 0, 100);    // Semi-transparent Red outline

    // --- Private Helper Function Signatures ---
    void drawPieces(sf::RenderWindow& window, const GameState& gameState);
    void drawGrid(sf::RenderWindow& window, const GameState& gameState);

    //vvv NEW vvv --- Helper for drawing all highlights --- vvv
    void drawHighlights(sf::RenderWindow& window,
                        const std::vector<Move>& legalMoveHighlights,
                        int selectedRow, int selectedCol,
                        const Move& lastAiMove);
    //^^^ NEW ^^^--------------------------------------------^^^
};


