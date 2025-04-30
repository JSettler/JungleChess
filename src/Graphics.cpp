#include "Graphics.h"
#include "Common.h"
#include <SFML/Graphics.hpp>
#include <iostream>
#include <string>
#include <vector>
#include <map>

//vvv RESTORED vvv --- Define Night Mode Colors --- vvv
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
//^^^ RESTORED ^^^------------------------------------^^^


// Constructor
Graphics::Graphics() : pieceDisplayMode(0) {
    if (!font.loadFromFile("assets/arial.ttf")) {
         std::cerr << "Warning: Could not load default font 'assets/arial.ttf'." << std::endl;
    }
    setupUIElements(); // Setup UI elements
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

// Convert mouse pixel coordinates
//vvv RESTORED vvv --- Implementation for getClickedSquare --- vvv
sf::Vector2i Graphics::getClickedSquare(const sf::Vector2i& mousePos) const {
    float relativeX = static_cast<float>(mousePos.x) - BOARD_OFFSET_X;
    float relativeY = static_cast<float>(mousePos.y) - BOARD_OFFSET_Y;
    // Check if click is within board bounds before dividing
    if (relativeX < 0 || relativeY < 0 ||
        relativeX >= BOARD_COLS * SQUARE_SIZE || relativeY >= BOARD_ROWS * SQUARE_SIZE)
    {
        return sf::Vector2i(-1, -1); // Indicate click outside board
    }
    int col = static_cast<int>(relativeX / SQUARE_SIZE);
    int row = static_cast<int>(relativeY / SQUARE_SIZE);
    return sf::Vector2i(col, row);
}
//^^^ RESTORED ^^^------------------------------------------^^^

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


// Main drawing function
void Graphics::drawBoard(sf::RenderWindow& window,
                         const GameState& gameState,
                         AppMode currentMode,
                         Player setupPlayer,
                         PieceType selectedSetupPiece,
                         const std::vector<Move>& legalMoveHighlights,
                         int selectedRow, int selectedCol,
                         const Move& lastAiMove) {
    window.clear(NightColors::Background); // Use restored namespace
    drawGrid(window, gameState);
    drawPieces(window, gameState);

    if (currentMode == AppMode::SETUP) {
        drawSetupUI(window, setupPlayer, selectedSetupPiece);
    } else {
        drawHighlights(window, legalMoveHighlights, selectedRow, selectedCol, lastAiMove);
    }
}

// --- Private Helper Functions ---

// Draw the grid lines
void Graphics::drawGrid(sf::RenderWindow& window, const GameState& gameState) {
    sf::RectangleShape square(sf::Vector2f(SQUARE_SIZE, SQUARE_SIZE));
    square.setOutlineColor(NightColors::GridLine); // Use restored namespace
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

// Draw the pieces
void Graphics::drawPieces(sf::RenderWindow& window, const GameState& gameState) {
    sf::Text mainText;
    sf::Text subText;
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
                sf::Color pieceColor = (currentPiece.owner == Player::PLAYER1) ? NightColors::P1_Piece : NightColors::P2_Piece; // Use restored namespace
                mainText.setFillColor(pieceColor); subText.setFillColor(pieceColor);
                sf::FloatRect mainBounds = mainText.getLocalBounds();
                mainText.setOrigin(mainBounds.left + mainBounds.width / 2.0f, mainBounds.top + mainBounds.height / 2.0f);
                mainText.setPosition(BOARD_OFFSET_X + c * SQUARE_SIZE + SQUARE_SIZE / 2.0f, BOARD_OFFSET_Y + r * SQUARE_SIZE + SQUARE_SIZE / 2.0f);
                window.draw(mainText);
                if (pieceDisplayMode != 2) {
                    sf::FloatRect subBounds = subText.getLocalBounds();
                    subText.setOrigin(subBounds.left + subBounds.width, subBounds.top + subBounds.height);
                    subText.setPosition(BOARD_OFFSET_X + (c + 1) * SQUARE_SIZE - subPadding, BOARD_OFFSET_Y + (r + 1) * SQUARE_SIZE - subPadding);
                    window.draw(subText);
                }
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
    if (selectedRow == -1 && lastAiMove.fromRow != -1) {
         highlightShape.setFillColor(sf::Color::Transparent);
         highlightShape.setOutlineColor(NightColors::LastAiOutline); // Use restored namespace
         highlightShape.setOutlineThickness(3.0f);
         highlightShape.setPosition(BOARD_OFFSET_X + lastAiMove.fromCol * SQUARE_SIZE, BOARD_OFFSET_Y + lastAiMove.fromRow * SQUARE_SIZE);
         window.draw(highlightShape);
         highlightShape.setPosition(BOARD_OFFSET_X + lastAiMove.toCol * SQUARE_SIZE, BOARD_OFFSET_Y + lastAiMove.toRow * SQUARE_SIZE);
         window.draw(highlightShape);
    }
    if (selectedRow != -1 && selectedCol != -1) {
        highlightShape.setPosition(BOARD_OFFSET_X + selectedCol * SQUARE_SIZE, BOARD_OFFSET_Y + selectedRow * SQUARE_SIZE);
        highlightShape.setFillColor(sf::Color::Transparent);
        highlightShape.setOutlineColor(NightColors::SelectedOutline); // Use restored namespace
        highlightShape.setOutlineThickness(3.0f);
        window.draw(highlightShape);
    }
    highlightShape.setFillColor(NightColors::LegalMoveFill); // Use restored namespace
    highlightShape.setOutlineThickness(0);
    for (const auto& move : legalMoveHighlights) {
        highlightShape.setPosition(BOARD_OFFSET_X + move.toCol * SQUARE_SIZE, BOARD_OFFSET_Y + move.toRow * SQUARE_SIZE);
        window.draw(highlightShape);
    }
}

// Draw Setup UI
void Graphics::drawSetupUI(sf::RenderWindow& window, Player setupPlayer, PieceType selectedSetupPiece) {
    window.draw(clearButton.shape); window.draw(clearButton.label);
    sideButton.shape.setFillColor(setupPlayer == Player::PLAYER1 ? NightColors::P1_Piece : NightColors::P2_Piece); // Use restored namespace
    sideButton.label.setString(setupPlayer == Player::PLAYER1 ? "P1" : "P2");
    sf::FloatRect sbounds = sideButton.label.getLocalBounds();
    sideButton.label.setOrigin(sbounds.left + sbounds.width / 2.0f, sbounds.top + sbounds.height / 2.0f);
    sideButton.label.setPosition(sideButton.shape.getPosition().x + UI_BUTTON_WIDTH / 2.0f, sideButton.shape.getPosition().y + UI_BUTTON_HEIGHT / 2.0f);
    window.draw(sideButton.shape); window.draw(sideButton.label);
    for (auto const& [type, button] : pieceButtons) {
        sf::RectangleShape currentShape = button.shape;
        currentShape.setFillColor(setupPlayer == Player::PLAYER1 ? NightColors::P1_Piece : NightColors::P2_Piece); // Use restored namespace
        if (type == selectedSetupPiece) { currentShape.setOutlineColor(sf::Color::Yellow); currentShape.setOutlineThickness(3.0f); }
        else { currentShape.setOutlineThickness(0); }
        window.draw(currentShape); window.draw(button.label);
    }
    window.draw(finishButton.shape); window.draw(finishButton.label);
}


