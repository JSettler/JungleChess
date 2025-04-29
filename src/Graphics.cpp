#include "Graphics.h"
#include "Common.h"
#include <SFML/Graphics.hpp>
#include <iostream>
#include <string>
#include <vector>

// Define Night Mode Colors (Unchanged)
namespace NightColors {
    const sf::Color Background = sf::Color(40, 40, 50);
    const sf::Color GridLine = sf::Color(100, 100, 110);
    const sf::Color Land = sf::Color(0, 80, 20); // Dark Green Land
    const sf::Color Water = sf::Color(30, 50, 100);
    const sf::Color P1_Den = sf::Color(0, 90, 110);
    const sf::Color P2_Den = sf::Color(110, 40, 40);
    const sf::Color P1_Trap = sf::Color(0, 60, 80);
    const sf::Color P2_Trap = sf::Color(90, 20, 20);
    const sf::Color P1_Piece = sf::Color(0, 180, 220);
    const sf::Color P2_Piece = sf::Color(230, 120, 0);
    const sf::Color SelectedOutline = sf::Color(255, 255, 0, 200);
    const sf::Color LegalMoveFill = sf::Color(0, 255, 0, 100);
    const sf::Color LastAiOutline = sf::Color(255, 0, 0, 200);
}


// Constructor (Initializes mode to 0)
Graphics::Graphics() : pieceDisplayMode(0) {
    if (!font.loadFromFile("assets/arial.ttf")) {
         std::cerr << "Warning: Could not load default font 'assets/arial.ttf'." << std::endl;
    }
}

// Load graphical assets (Unchanged)
void Graphics::loadAssets() {
    if (!font.loadFromFile("assets/arial.ttf")) {
         std::cerr << "Error loading font assets/arial.ttf in loadAssets!" << std::endl;
    }
}

// Convert mouse pixel coordinates (Unchanged)
sf::Vector2i Graphics::getClickedSquare(const sf::Vector2i& mousePos) const {
    float relativeX = static_cast<float>(mousePos.x) - BOARD_OFFSET_X;
    float relativeY = static_cast<float>(mousePos.y) - BOARD_OFFSET_Y;
    int col = static_cast<int>(relativeX / SQUARE_SIZE);
    int row = static_cast<int>(relativeY / SQUARE_SIZE);
    return sf::Vector2i(col, row);
}

// Toggle display implementation (Cycles through 3 modes)
//vvv MODIFIED vvv --- Cycle through 3 modes --- vvv
void Graphics::togglePieceDisplay() {
    pieceDisplayMode = (pieceDisplayMode + 1) % 3; // Cycle 0 -> 1 -> 2 -> 0
    std::string modeStr;
    switch (pieceDisplayMode) {
        case 0: modeStr = "Letters + Rank"; break;
        case 1: modeStr = "Rank + Letters"; break;
        case 2: modeStr = "Letters Only"; break;
        default: modeStr = "Unknown"; break; // Should not happen
    }
    std::cout << "Piece display toggled to: " << modeStr << std::endl;
}
//^^^ MODIFIED ^^^-----------------------------^^^


// Main drawing function (Unchanged)
void Graphics::drawBoard(sf::RenderWindow& window,
                         const GameState& gameState,
                         const std::vector<Move>& legalMoveHighlights,
                         int selectedRow, int selectedCol,
                         const Move& lastAiMove) {
    window.clear(NightColors::Background);
    drawGrid(window, gameState);
    drawPieces(window, gameState); // Updated drawPieces handles the mode
    drawHighlights(window, legalMoveHighlights, selectedRow, selectedCol, lastAiMove);
}

// --- Private Helper Functions ---

// Draw the grid lines, coloring special squares (Unchanged)
void Graphics::drawGrid(sf::RenderWindow& window, const GameState& gameState) {
    sf::RectangleShape square(sf::Vector2f(SQUARE_SIZE, SQUARE_SIZE));
    square.setOutlineColor(NightColors::GridLine);
    square.setOutlineThickness(1.0f);
    for (int r = 0; r < BOARD_ROWS; ++r) {
        for (int c = 0; c < BOARD_COLS; ++c) {
            square.setPosition(BOARD_OFFSET_X + c * SQUARE_SIZE, BOARD_OFFSET_Y + r * SQUARE_SIZE);
            if (gameState.isRiver(r, c)) square.setFillColor(NightColors::Water);
            else if (gameState.isOwnDen(r, c, Player::PLAYER1)) square.setFillColor(NightColors::P1_Den);
            else if (gameState.isOwnDen(r, c, Player::PLAYER2)) square.setFillColor(NightColors::P2_Den);
            else if (gameState.isOwnTrap(r, c, Player::PLAYER1)) square.setFillColor(NightColors::P1_Trap);
            else if (gameState.isOwnTrap(r, c, Player::PLAYER2)) square.setFillColor(NightColors::P2_Trap);
            else square.setFillColor(NightColors::Land);
            window.draw(square);
        }
    }
}

// Draw the pieces onto the board (Handles 3 display modes)
//vvv MODIFIED vvv --- Updated drawPieces logic for 3 modes --- vvv
void Graphics::drawPieces(sf::RenderWindow& window, const GameState& gameState) {
    sf::Text mainText;
    sf::Text subText; // Secondary text (small number or letter)

    if (!font.loadFromFile("assets/arial.ttf")) { return; }
    mainText.setFont(font);
    subText.setFont(font);

    // Define sizes
    const unsigned int mainSize = static_cast<unsigned int>(SQUARE_SIZE * 0.55f);
    const unsigned int subSize = static_cast<unsigned int>(SQUARE_SIZE * 0.25f);
    mainText.setCharacterSize(mainSize);
    subText.setCharacterSize(subSize);

    // Define sub-text padding from bottom-right corner
    const float subPadding = 3.0f;

    for (int r = 0; r < BOARD_ROWS; ++r) {
        for (int c = 0; c < BOARD_COLS; ++c) {
            Piece currentPiece = gameState.getPiece(r, c);
            if (currentPiece.type != PieceType::EMPTY) {
                std::string letterChar = "?";
                std::string numberChar = "?";

                // Get Letter Representation
                switch (currentPiece.type) {
                    case PieceType::RAT: letterChar = "R"; break; case PieceType::CAT: letterChar = "C"; break;
                    case PieceType::DOG: letterChar = "D"; break; case PieceType::WOLF: letterChar = "W"; break;
                    case PieceType::LEOPARD: letterChar = "P"; break; case PieceType::TIGER: letterChar = "T"; break;
                    case PieceType::LION: letterChar = "L"; break; case PieceType::ELEPHANT: letterChar = "E"; break;
                    default: break;
                }
                // Get Number Representation (Rank)
                numberChar = std::to_string(currentPiece.rank);

                // Assign strings based on display mode
                switch (pieceDisplayMode) {
                    case 0: // Letters main, Numbers sub
                        mainText.setString(letterChar);
                        subText.setString(numberChar);
                        break;
                    case 1: // Numbers main, Letters sub
                        mainText.setString(numberChar);
                        subText.setString(letterChar);
                        break;
                    case 2: // Letters only
                        mainText.setString(letterChar);
                        subText.setString(""); // No sub text
                        break;
                }

                // Set Color for both
                sf::Color pieceColor = (currentPiece.owner == Player::PLAYER1) ? NightColors::P1_Piece : NightColors::P2_Piece;
                mainText.setFillColor(pieceColor);
                subText.setFillColor(pieceColor); // Set color even if empty, doesn't hurt

                // Position Main Text (Centered)
                sf::FloatRect mainBounds = mainText.getLocalBounds();
                mainText.setOrigin(mainBounds.left + mainBounds.width / 2.0f, mainBounds.top + mainBounds.height / 2.0f);
                mainText.setPosition(BOARD_OFFSET_X + c * SQUARE_SIZE + SQUARE_SIZE / 2.0f,
                                     BOARD_OFFSET_Y + r * SQUARE_SIZE + SQUARE_SIZE / 2.0f);

                // Draw Main Text
                window.draw(mainText);

                // Position and Draw Sub Text (Only if not mode 2)
                if (pieceDisplayMode != 2) {
                    sf::FloatRect subBounds = subText.getLocalBounds();
                    subText.setOrigin(subBounds.left + subBounds.width, subBounds.top + subBounds.height);
                    subText.setPosition(BOARD_OFFSET_X + (c + 1) * SQUARE_SIZE - subPadding,
                                        BOARD_OFFSET_Y + (r + 1) * SQUARE_SIZE - subPadding);
                    window.draw(subText);
                }
            }
        }
    }
}
//^^^ MODIFIED ^^^---------------------------------------------^^^


// Draw highlights (Unchanged)
void Graphics::drawHighlights(sf::RenderWindow& window,
                              const std::vector<Move>& legalMoveHighlights,
                              int selectedRow, int selectedCol,
                              const Move& lastAiMove)
{
    sf::RectangleShape highlightShape(sf::Vector2f(SQUARE_SIZE, SQUARE_SIZE));
    // 1. Highlight Last AI Move
    if (selectedRow == -1 && lastAiMove.fromRow != -1) {
         highlightShape.setFillColor(sf::Color::Transparent);
         highlightShape.setOutlineColor(NightColors::LastAiOutline);
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
        highlightShape.setOutlineColor(NightColors::SelectedOutline);
        highlightShape.setOutlineThickness(3.0f);
        window.draw(highlightShape);
    }
    // 3. Highlight Legal Moves
    highlightShape.setFillColor(NightColors::LegalMoveFill);
    highlightShape.setOutlineThickness(0);
    for (const auto& move : legalMoveHighlights) {
        highlightShape.setPosition(BOARD_OFFSET_X + move.toCol * SQUARE_SIZE, BOARD_OFFSET_Y + move.toRow * SQUARE_SIZE);
        window.draw(highlightShape);
    }
}


