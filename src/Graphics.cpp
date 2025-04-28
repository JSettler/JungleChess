#include "Graphics.h"
#include "Common.h"
#include <SFML/Graphics.hpp>
#include <iostream>
#include <string>
#include <vector>

//vvv NEW vvv --- Define Night Mode Colors --- vvv
namespace NightColors {
    const sf::Color Background = sf::Color(40, 40, 50); // Dark grey-blue
    const sf::Color GridLine = sf::Color(100, 100, 110); // Medium grey
    const sf::Color Land = sf::Color(60, 60, 70); // Darker grey-blue
    const sf::Color Water = sf::Color(30, 50, 100); // Dark blue
    const sf::Color P1_Den = sf::Color(0, 90, 110); // Dark cyan/teal
    const sf::Color P2_Den = sf::Color(110, 40, 40); // Dark red/maroon
    const sf::Color P1_Trap = sf::Color(0, 60, 80); // Darker cyan/teal
    const sf::Color P2_Trap = sf::Color(90, 20, 20); // Darker red/maroon

    const sf::Color P1_Piece = sf::Color(0, 180, 220); // Brighter Cyan/Blue
    const sf::Color P2_Piece = sf::Color(230, 120, 0); // Brighter Orange/Red

    const sf::Color SelectedOutline = sf::Color(255, 255, 0, 200); // Bright Yellow outline
    const sf::Color LegalMoveFill = sf::Color(0, 255, 0, 100);     // Semi-transparent Green fill
    const sf::Color LastAiOutline = sf::Color(255, 0, 0, 200);    // Bright Red outline
}
//^^^ NEW ^^^------------------------------------^^^


// Constructor
Graphics::Graphics() {
    if (!font.loadFromFile("assets/arial.ttf")) {
         std::cerr << "Warning: Could not load default font 'assets/arial.ttf'." << std::endl;
    }
}

// Load graphical assets
void Graphics::loadAssets() {
    if (!font.loadFromFile("assets/arial.ttf")) {
         std::cerr << "Error loading font assets/arial.ttf in loadAssets!" << std::endl;
    }
    // TODO: Load piece textures here
}

// Convert mouse pixel coordinates
sf::Vector2i Graphics::getClickedSquare(const sf::Vector2i& mousePos) const {
    float relativeX = static_cast<float>(mousePos.x) - BOARD_OFFSET_X;
    float relativeY = static_cast<float>(mousePos.y) - BOARD_OFFSET_Y;
    int col = static_cast<int>(relativeX / SQUARE_SIZE);
    int row = static_cast<int>(relativeY / SQUARE_SIZE);
    return sf::Vector2i(col, row);
}

// Main drawing function
void Graphics::drawBoard(sf::RenderWindow& window,
                         const GameState& gameState,
                         const std::vector<Move>& legalMoveHighlights,
                         int selectedRow, int selectedCol,
                         const Move& lastAiMove) {
    //vvv MODIFIED vvv --- Use Night Mode Background --- vvv
    window.clear(NightColors::Background);
    //^^^ MODIFIED ^^^-----------------------------------^^^

    drawGrid(window, gameState);
    drawPieces(window, gameState);
    drawHighlights(window, legalMoveHighlights, selectedRow, selectedCol, lastAiMove);
}

// --- Private Helper Functions ---

// Draw the grid lines, coloring special squares
void Graphics::drawGrid(sf::RenderWindow& window, const GameState& gameState) {
    sf::RectangleShape square(sf::Vector2f(SQUARE_SIZE, SQUARE_SIZE));
    //vvv MODIFIED vvv --- Use Night Mode Grid Line --- vvv
    square.setOutlineColor(NightColors::GridLine);
    //^^^ MODIFIED ^^^----------------------------------^^^
    square.setOutlineThickness(1.0f);

    for (int r = 0; r < BOARD_ROWS; ++r) {
        for (int c = 0; c < BOARD_COLS; ++c) {
            square.setPosition(BOARD_OFFSET_X + c * SQUARE_SIZE, BOARD_OFFSET_Y + r * SQUARE_SIZE);
            //vvv MODIFIED vvv --- Use Night Mode Square Colors --- vvv
            if (gameState.isRiver(r, c)) square.setFillColor(NightColors::Water);
            else if (gameState.isOwnDen(r, c, Player::PLAYER1)) square.setFillColor(NightColors::P1_Den);
            else if (gameState.isOwnDen(r, c, Player::PLAYER2)) square.setFillColor(NightColors::P2_Den);
            else if (gameState.isOwnTrap(r, c, Player::PLAYER1)) square.setFillColor(NightColors::P1_Trap);
            else if (gameState.isOwnTrap(r, c, Player::PLAYER2)) square.setFillColor(NightColors::P2_Trap);
            else square.setFillColor(NightColors::Land);
            //^^^ MODIFIED ^^^---------------------------------------^^^
            window.draw(square);
        }
    }
}

// Draw the pieces onto the board
void Graphics::drawPieces(sf::RenderWindow& window, const GameState& gameState) {
    sf::Text pieceText;
    if (!font.loadFromFile("assets/arial.ttf")) { return; } // Basic error check
    pieceText.setFont(font);
    pieceText.setCharacterSize(static_cast<unsigned int>(SQUARE_SIZE * 0.6f));

    for (int r = 0; r < BOARD_ROWS; ++r) {
        for (int c = 0; c < BOARD_COLS; ++c) {
            Piece currentPiece = gameState.getPiece(r, c);
            if (currentPiece.type != PieceType::EMPTY) {
                std::string pieceChar = "?";
                switch (currentPiece.type) {
                    case PieceType::RAT: pieceChar = "R"; break; case PieceType::CAT: pieceChar = "C"; break;
                    case PieceType::DOG: pieceChar = "D"; break; case PieceType::WOLF: pieceChar = "W"; break;
                    case PieceType::LEOPARD: pieceChar = "P"; break; case PieceType::TIGER: pieceChar = "T"; break;
                    case PieceType::LION: pieceChar = "L"; break; case PieceType::ELEPHANT: pieceChar = "E"; break;
                    default: break;
                }
                pieceText.setString(pieceChar);
                //vvv MODIFIED vvv --- Use Night Mode Piece Colors --- vvv
                pieceText.setFillColor(currentPiece.owner == Player::PLAYER1 ? NightColors::P1_Piece : NightColors::P2_Piece);
                //^^^ MODIFIED ^^^------------------------------------^^^
                sf::FloatRect textBounds = pieceText.getLocalBounds();
                pieceText.setOrigin(textBounds.left + textBounds.width / 2.0f, textBounds.top + textBounds.height / 2.0f);
                pieceText.setPosition(BOARD_OFFSET_X + c * SQUARE_SIZE + SQUARE_SIZE / 2.0f,
                                      BOARD_OFFSET_Y + r * SQUARE_SIZE + SQUARE_SIZE / 2.0f);
                window.draw(pieceText);
            }
        }
    }
}

// Draw highlights
void Graphics::drawHighlights(sf::RenderWindow& window,
                              const std::vector<Move>& legalMoveHighlights,
                              int selectedRow, int selectedCol,
                              const Move& lastAiMove)
{
    sf::RectangleShape highlightShape(sf::Vector2f(SQUARE_SIZE, SQUARE_SIZE));

    // 1. Highlight Last AI Move
    if (selectedRow == -1 && lastAiMove.fromRow != -1) {
         highlightShape.setFillColor(sf::Color::Transparent);
         //vvv MODIFIED vvv --- Use Night Mode Highlight Color --- vvv
         highlightShape.setOutlineColor(NightColors::LastAiOutline);
         //^^^ MODIFIED ^^^---------------------------------------^^^
         highlightShape.setOutlineThickness(3.0f);
         highlightShape.setPosition(BOARD_OFFSET_X + lastAiMove.fromCol * SQUARE_SIZE, BOARD_OFFSET_Y + lastAiMove.fromRow * SQUARE_SIZE);
         window.draw(highlightShape);
         highlightShape.setPosition(BOARD_OFFSET_X + lastAiMove.toCol * SQUARE_SIZE, BOARD_OFFSET_Y + lastAiMove.toRow * SQUARE_SIZE);
         window.draw(highlightShape);
    }

    // 2. Highlight Selected Piece
    if (selectedRow != -1 && selectedCol != -1) {
        highlightShape.setPosition(BOARD_OFFSET_X + selectedCol * SQUARE_SIZE, BOARD_OFFSET_Y + selectedRow * SQUARE_SIZE);
        highlightShape.setFillColor(sf::Color::Transparent);
        //vvv MODIFIED vvv --- Use Night Mode Highlight Color --- vvv
        highlightShape.setOutlineColor(NightColors::SelectedOutline);
        //^^^ MODIFIED ^^^---------------------------------------^^^
        highlightShape.setOutlineThickness(3.0f);
        window.draw(highlightShape);
    }

    // 3. Highlight Legal Moves
    //vvv MODIFIED vvv --- Use Night Mode Highlight Color --- vvv
    highlightShape.setFillColor(NightColors::LegalMoveFill);
    //^^^ MODIFIED ^^^---------------------------------------^^^
    highlightShape.setOutlineThickness(0);
    for (const auto& move : legalMoveHighlights) {
        highlightShape.setPosition(BOARD_OFFSET_X + move.toCol * SQUARE_SIZE, BOARD_OFFSET_Y + move.toRow * SQUARE_SIZE);
        window.draw(highlightShape);
    }
}


