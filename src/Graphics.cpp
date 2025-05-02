#include "Graphics.h"
#include "Common.h"
#include <SFML/Graphics.hpp>
#include <iostream>
#include <string>
#include <vector>
#include <map>
#include <cmath> // For std::floor

//vvv NEW vvv --- Define and initialize static const floats --- vvv
const float Graphics::INDICATOR_RADIUS = 10.0f;
const float Graphics::INDICATOR_X = Graphics::BOARD_OFFSET_X; // Use scope resolution
const float Graphics::INDICATOR_Y = Graphics::BOARD_OFFSET_Y - Graphics::INDICATOR_RADIUS * 2.5f;
//^^^ NEW ^^^-------------------------------------------------^^^

// Define Night Mode Colors
namespace NightColors {
    const sf::Color Background = sf::Color(40, 40, 50);
    const sf::Color GridLine = sf::Color(100, 100, 110);
    const sf::Color Land = sf::Color(0, 80, 20); // Dark Green Land
    const sf::Color Water = sf::Color(30, 50, 100);
    const sf::Color P1_Den = sf::Color(0, 90, 110);
    const sf::Color P2_Den = sf::Color(110, 40, 40);
    const sf::Color P1_Trap = sf::Color(0, 60, 80);
    const sf::Color P2_Trap = sf::Color(90, 20, 20);
    const sf::Color P1_Piece = sf::Color(0, 180, 220); // Blue for Player 1
    const sf::Color P2_Piece = sf::Color(230, 120, 0); // Orange/Red for Player 2
    const sf::Color SelectedOutline = sf::Color(255, 255, 0, 200);
    const sf::Color LegalMoveFill = sf::Color(0, 255, 0, 100);
    const sf::Color LastAiOutline = sf::Color(255, 0, 0, 200);
}


// Constructor
Graphics::Graphics() : pieceDisplayMode(0), boardFlipped(true) { // Initialize flags
    if (!font.loadFromFile("assets/arial.ttf")) {
         std::cerr << "Warning: Could not load default font 'assets/arial.ttf'." << std::endl;
    }
    setupUIElements(); // Setup UI elements
    // Initialize Turn Indicator Dot using the static const members
    turnIndicatorDot.setRadius(INDICATOR_RADIUS);
    turnIndicatorDot.setOrigin(INDICATOR_RADIUS, INDICATOR_RADIUS); // Center origin
    turnIndicatorDot.setPosition(INDICATOR_X + INDICATOR_RADIUS, INDICATOR_Y); // Position center
    turnIndicatorDot.setOutlineColor(sf::Color(200, 200, 200));
    turnIndicatorDot.setOutlineThickness(1.0f);
}

// Load graphical assets
void Graphics::loadAssets() {
    if (!font.loadFromFile("assets/arial.ttf")) {
         std::cerr << "Error loading font assets/arial.ttf in loadAssets!" << std::endl;
    }
    // Re-setup UI in case font failed before but loaded now
    setupUIElements();
}

// Helper to create UI buttons
void Graphics::setupUIElements() {
    if (!font.loadFromFile("assets/arial.ttf")) return; // Need font

    // --- Clear and Side Buttons ---
    float currentY = BOARD_OFFSET_Y;
    clearButton.shape.setSize(sf::Vector2f(UI_BUTTON_WIDTH, UI_BUTTON_HEIGHT));
    clearButton.shape.setPosition(UI_PANEL_X, currentY);
    clearButton.shape.setFillColor(sf::Color(100, 100, 100));
    clearButton.label.setFont(font);
    clearButton.label.setString("Clear");
    clearButton.label.setCharacterSize(16);
    clearButton.label.setFillColor(sf::Color::White);
    sf::FloatRect cbounds = clearButton.label.getLocalBounds();
    clearButton.label.setOrigin(cbounds.left + cbounds.width / 2.0f, cbounds.top + cbounds.height / 2.0f);
    clearButton.label.setPosition(clearButton.shape.getPosition().x + UI_BUTTON_WIDTH / 2.0f,
                                   clearButton.shape.getPosition().y + UI_BUTTON_HEIGHT / 2.0f);
    clearButton.bounds = clearButton.shape.getGlobalBounds();

    sideButton.shape.setSize(sf::Vector2f(UI_BUTTON_WIDTH, UI_BUTTON_HEIGHT));
    sideButton.shape.setPosition(UI_PANEL_X + UI_BUTTON_WIDTH + UI_BUTTON_PADDING, currentY);
    // Color set dynamically
    sideButton.label.setFont(font);
    sideButton.label.setString("Side"); // Text updated dynamically too
    sideButton.label.setCharacterSize(16);
    sideButton.label.setFillColor(sf::Color::White);
    // Origin/Position set dynamically
    sideButton.bounds = sideButton.shape.getGlobalBounds();

    currentY += UI_BUTTON_HEIGHT + UI_BUTTON_PADDING * 2;

    // --- Piece Buttons ---
    PieceType pieceOrder[] = { PieceType::RAT, PieceType::CAT, PieceType::DOG, PieceType::WOLF,
                              PieceType::LEOPARD, PieceType::TIGER, PieceType::LION, PieceType::ELEPHANT };
    int buttonsPerRow = 2;
    int currentX = UI_PANEL_X;

    for (int i = 0; i < 8; ++i) {
        PieceType type = pieceOrder[i];
        ButtonUI& btn = pieceButtons[type];
        btn.shape.setSize(sf::Vector2f(UI_BUTTON_WIDTH, UI_BUTTON_HEIGHT));
        btn.shape.setPosition(currentX, currentY);
        btn.label.setFont(font);
        btn.label.setString(std::to_string(GameState().getRank(type))); // Use rank for button label
        btn.label.setCharacterSize(18);
        btn.label.setFillColor(sf::Color::Black);
        sf::FloatRect pbounds = btn.label.getLocalBounds();
        btn.label.setOrigin(pbounds.left + pbounds.width / 2.0f, pbounds.top + pbounds.height / 2.0f);
        btn.label.setPosition(btn.shape.getPosition().x + UI_BUTTON_WIDTH / 2.0f,
                              btn.shape.getPosition().y + UI_BUTTON_HEIGHT / 2.0f);
        btn.bounds = btn.shape.getGlobalBounds();
        if ((i + 1) % buttonsPerRow == 0) { currentX = UI_PANEL_X; currentY += UI_BUTTON_HEIGHT + UI_BUTTON_PADDING; }
        else { currentX += UI_BUTTON_WIDTH + UI_BUTTON_PADDING; }
    }
    if (8 % buttonsPerRow != 0) { currentY += UI_BUTTON_HEIGHT + UI_BUTTON_PADDING; }
    currentY += UI_BUTTON_PADDING;

    // --- Finish Button ---
    finishButton.shape.setSize(sf::Vector2f(UI_BUTTON_WIDTH * buttonsPerRow + UI_BUTTON_PADDING * (buttonsPerRow - 1), UI_BUTTON_HEIGHT));
    finishButton.shape.setPosition(UI_PANEL_X, currentY);
    finishButton.shape.setFillColor(sf::Color(50, 150, 50));
    finishButton.label.setFont(font);
    finishButton.label.setString("Finish (F)");
    finishButton.label.setCharacterSize(16);
    finishButton.label.setFillColor(sf::Color::White);
    sf::FloatRect fbounds = finishButton.label.getLocalBounds();
    finishButton.label.setOrigin(fbounds.left + fbounds.width / 2.0f, fbounds.top + fbounds.height / 2.0f);
    finishButton.label.setPosition(finishButton.shape.getPosition().x + finishButton.shape.getSize().x / 2.0f,
                                   finishButton.shape.getPosition().y + UI_BUTTON_HEIGHT / 2.0f);
    finishButton.bounds = finishButton.shape.getGlobalBounds();
}


// Convert mouse pixel coordinates to board coordinates (Handles flip)
sf::Vector2i Graphics::getClickedSquare(const sf::Vector2i& mousePos) const {
    float relativeX = static_cast<float>(mousePos.x) - BOARD_OFFSET_X;
    float relativeY = static_cast<float>(mousePos.y) - BOARD_OFFSET_Y;

    if (relativeX < 0 || relativeY < 0 ||
        relativeX >= BOARD_COLS * SQUARE_SIZE || relativeY >= BOARD_ROWS * SQUARE_SIZE) {
        return sf::Vector2i(-1, -1); // Click outside board bounds
    }

    // Calculate apparent row/col based on screen position
    int apparentCol = static_cast<int>(std::floor(relativeX / SQUARE_SIZE));
    int apparentRow = static_cast<int>(std::floor(relativeY / SQUARE_SIZE));

    if (boardFlipped) {
        // If board is flipped, translate apparent coords back to internal coords
        int internalRow = BOARD_ROWS - 1 - apparentRow;
        int internalCol = BOARD_COLS - 1 - apparentCol;
        return sf::Vector2i(internalCol, internalRow);
    } else {
        // If not flipped, apparent coords are internal coords
        return sf::Vector2i(apparentCol, apparentRow);
    }
}


// Click detection for UI buttons
PieceType Graphics::getClickedSetupPieceButton(const sf::Vector2i& mousePos) const {
    for (const auto& pair : pieceButtons) {
        if (pair.second.bounds.contains(static_cast<float>(mousePos.x), static_cast<float>(mousePos.y))) {
            return pair.first;
        }
    }
    return PieceType::EMPTY;
}

bool Graphics::isClickOnClearButton(const sf::Vector2i& mousePos) const {
    return clearButton.bounds.contains(static_cast<float>(mousePos.x), static_cast<float>(mousePos.y));
}

bool Graphics::isClickOnSideButton(const sf::Vector2i& mousePos) const {
    return sideButton.bounds.contains(static_cast<float>(mousePos.x), static_cast<float>(mousePos.y));
}

bool Graphics::isClickOnFinishButton(const sf::Vector2i& mousePos) const {
    return finishButton.bounds.contains(static_cast<float>(mousePos.x), static_cast<float>(mousePos.y));
}

// Toggle display implementation
void Graphics::togglePieceDisplay() {
    pieceDisplayMode = (pieceDisplayMode + 1) % 3; // Cycle 0 -> 1 -> 2 -> 0
    std::string modeStr;
    switch (pieceDisplayMode) {
        case 0: modeStr = "Letters + Rank"; break;
        case 1: modeStr = "Rank + Letters"; break;
        case 2: modeStr = "Letters Only"; break;
        default: modeStr = "Unknown"; break;
    }
    std::cout << "Piece display toggled to: " << modeStr << std::endl;
}

// Toggle board flip implementation
void Graphics::toggleBoardFlip() {
    boardFlipped = !boardFlipped;
    // Message now reflects the state AFTER the toggle
    std::cout << "Board orientation toggled. Player 1 is now at the "
              << (boardFlipped ? "BOTTOM" : "TOP") << "." << std::endl; // Corrected message logic
}

// Helper to get screen position based on flip
sf::Vector2f Graphics::getScreenPos(int r, int c) const {
    int displayRow = r;
    int displayCol = c;
    if (boardFlipped) {
        displayRow = BOARD_ROWS - 1 - r;
        displayCol = BOARD_COLS - 1 - c;
    }
    return sf::Vector2f(BOARD_OFFSET_X + displayCol * SQUARE_SIZE,
                        BOARD_OFFSET_Y + displayRow * SQUARE_SIZE);
}


// Main drawing function
void Graphics::drawBoard(sf::RenderWindow& window,
                         const GameState& gameState,
                         AppMode currentMode,
                         Player setupPlayer,
                         PieceType selectedSetupPiece,
                         bool gameOver, // Added gameOver flag
                         const std::vector<Move>& legalMoveHighlights,
                         int selectedRow, int selectedCol,
                         const Move& lastAiMove) {
    window.clear(NightColors::Background);

    // Draw indicator first (only in game mode, not game over)
    if (currentMode == AppMode::GAME && !gameOver) {
         drawTurnIndicator(window, gameState);
    }

    drawGrid(window, gameState);
    drawPieces(window, gameState);

    if (currentMode == AppMode::SETUP) {
        drawSetupUI(window, setupPlayer, selectedSetupPiece);
    } else { // Only draw game highlights in game mode
        drawHighlights(window, legalMoveHighlights, selectedRow, selectedCol, lastAiMove);
    }
}

// --- Private Helper Functions ---

// Draw the grid lines, coloring special squares (Uses getScreenPos)
void Graphics::drawGrid(sf::RenderWindow& window, const GameState& gameState) {
    sf::RectangleShape square(sf::Vector2f(SQUARE_SIZE, SQUARE_SIZE));
    square.setOutlineColor(NightColors::GridLine);
    square.setOutlineThickness(1.0f);

    for (int r = 0; r < BOARD_ROWS; ++r) {
        for (int c = 0; c < BOARD_COLS; ++c) {
            square.setPosition(getScreenPos(r, c)); // Use helper function for position
            // Color determination logic remains the same, based on internal (r, c)
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


// Draw the pieces onto the board (Handles 3 display modes, uses getScreenPos)
void Graphics::drawPieces(sf::RenderWindow& window, const GameState& gameState) {
    sf::Text mainText; sf::Text subText;
    if (!font.loadFromFile("assets/arial.ttf")) { return; }
    mainText.setFont(font); subText.setFont(font);
    const unsigned int mainSize = static_cast<unsigned int>(SQUARE_SIZE * 0.55f);
    const unsigned int subSize = static_cast<unsigned int>(SQUARE_SIZE * 0.25f);
    mainText.setCharacterSize(mainSize); subText.setCharacterSize(subSize);
    const float subPadding = 3.0f;

    for (int r = 0; r < BOARD_ROWS; ++r) {
        for (int c = 0; c < BOARD_COLS; ++c) {
            Piece currentPiece = gameState.getPiece(r, c);
            if (currentPiece.type != PieceType::EMPTY) {
                std::string letterChar = "?"; std::string numberChar = "?";
                switch (currentPiece.type) {
                    case PieceType::RAT: letterChar = "R"; break; case PieceType::CAT: letterChar = "C"; break;
                    case PieceType::DOG: letterChar = "D"; break; case PieceType::WOLF: letterChar = "W"; break;
                    case PieceType::LEOPARD: letterChar = "P"; break; case PieceType::TIGER: letterChar = "T"; break;
                    case PieceType::LION: letterChar = "L"; break; case PieceType::ELEPHANT: letterChar = "E"; break;
                    default: break;
                }
                numberChar = std::to_string(currentPiece.rank);
                switch (pieceDisplayMode) {
                    case 0: mainText.setString(letterChar); subText.setString(numberChar); break;
                    case 1: mainText.setString(numberChar); subText.setString(letterChar); break;
                    case 2: mainText.setString(letterChar); subText.setString(""); break;
                }
                sf::Color pieceColor = (currentPiece.owner == Player::PLAYER1) ? NightColors::P1_Piece : NightColors::P2_Piece;
                mainText.setFillColor(pieceColor); subText.setFillColor(pieceColor);

                // Get screen position for this internal board square
                sf::Vector2f screenPos = getScreenPos(r, c);

                // Position Main Text (Centered within the screen square)
                sf::FloatRect mainBounds = mainText.getLocalBounds();
                mainText.setOrigin(mainBounds.left + mainBounds.width / 2.0f, mainBounds.top + mainBounds.height / 2.0f);
                mainText.setPosition(screenPos.x + SQUARE_SIZE / 2.0f, screenPos.y + SQUARE_SIZE / 2.0f);
                window.draw(mainText);

                // Position and Draw Sub Text (Only if not mode 2)
                if (pieceDisplayMode != 2) {
                    sf::FloatRect subBounds = subText.getLocalBounds();
                    subText.setOrigin(subBounds.left + subBounds.width, subBounds.top + subBounds.height);
                    // Position relative to the screen square's bottom-right
                    subText.setPosition(screenPos.x + SQUARE_SIZE - subPadding, screenPos.y + SQUARE_SIZE - subPadding);
                    window.draw(subText);
                }
            }
        }
    }
}


// Draw highlights (Uses getScreenPos)
void Graphics::drawHighlights(sf::RenderWindow& window,
                              const std::vector<Move>& legalMoveHighlights,
                              int selectedRow, int selectedCol, // These are internal board coords
                              const Move& lastAiMove)           // This contains internal board coords
{
    sf::RectangleShape highlightShape(sf::Vector2f(SQUARE_SIZE, SQUARE_SIZE));

    // 1. Highlight Last AI Move (Use internal coords with getScreenPos)
    if (selectedRow == -1 && lastAiMove.fromRow != -1) {
         highlightShape.setFillColor(sf::Color::Transparent);
         highlightShape.setOutlineColor(NightColors::LastAiOutline);
         highlightShape.setOutlineThickness(3.0f);
         highlightShape.setPosition(getScreenPos(lastAiMove.fromRow, lastAiMove.fromCol));
         window.draw(highlightShape);
         highlightShape.setPosition(getScreenPos(lastAiMove.toRow, lastAiMove.toCol));
         window.draw(highlightShape);
    }

    // 2. Highlight Selected Piece (Use internal coords with getScreenPos)
    if (selectedRow != -1 && selectedCol != -1) {
        highlightShape.setPosition(getScreenPos(selectedRow, selectedCol));
        highlightShape.setFillColor(sf::Color::Transparent);
        highlightShape.setOutlineColor(NightColors::SelectedOutline);
        highlightShape.setOutlineThickness(3.0f);
        window.draw(highlightShape);
    }

    // 3. Highlight Legal Moves (Use internal coords with getScreenPos)
    highlightShape.setFillColor(NightColors::LegalMoveFill);
    highlightShape.setOutlineThickness(0);
    for (const auto& move : legalMoveHighlights) {
        highlightShape.setPosition(getScreenPos(move.toRow, move.toCol));
        window.draw(highlightShape);
    }
}


// Draw Setup UI (No changes needed, UI is fixed position)
void Graphics::drawSetupUI(sf::RenderWindow& window, Player setupPlayer, PieceType selectedSetupPiece) {
    window.draw(clearButton.shape); window.draw(clearButton.label);
    sideButton.shape.setFillColor(setupPlayer == Player::PLAYER1 ? NightColors::P1_Piece : NightColors::P2_Piece);
    sideButton.label.setString(setupPlayer == Player::PLAYER1 ? "P1" : "P2");
    sf::FloatRect sbounds = sideButton.label.getLocalBounds();
    sideButton.label.setOrigin(sbounds.left + sbounds.width / 2.0f, sbounds.top + sbounds.height / 2.0f);
    sideButton.label.setPosition(sideButton.shape.getPosition().x + UI_BUTTON_WIDTH / 2.0f, sideButton.shape.getPosition().y + UI_BUTTON_HEIGHT / 2.0f);
    window.draw(sideButton.shape); window.draw(sideButton.label);
    for (auto const& [type, button] : pieceButtons) {
        sf::RectangleShape currentShape = button.shape;
        currentShape.setFillColor(setupPlayer == Player::PLAYER1 ? NightColors::P1_Piece : NightColors::P2_Piece);
        if (type == selectedSetupPiece) { currentShape.setOutlineColor(sf::Color::Yellow); currentShape.setOutlineThickness(3.0f); }
        else { currentShape.setOutlineThickness(0); }
        window.draw(currentShape); window.draw(button.label);
    }
    window.draw(finishButton.shape); window.draw(finishButton.label);
}

// Draw Turn Indicator Implementation
void Graphics::drawTurnIndicator(sf::RenderWindow& window, const GameState& gameState) {
    Player currentPlayer = gameState.getCurrentPlayer();
    if (currentPlayer == Player::PLAYER1) {
        turnIndicatorDot.setFillColor(NightColors::P1_Piece); // Blue dot
    } else if (currentPlayer == Player::PLAYER2) {
        turnIndicatorDot.setFillColor(NightColors::P2_Piece); // Orange/Red dot
    } else {
        turnIndicatorDot.setFillColor(sf::Color::Transparent); // Hide if no player
    }
    window.draw(turnIndicatorDot);
}


