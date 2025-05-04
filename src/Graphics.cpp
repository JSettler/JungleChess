#include "Graphics.h"
#include "Common.h"
#include <SFML/Graphics.hpp>
#include <iostream>
#include <string>
#include <vector>
#include <map>
#include <cmath> // For std::floor
#include <set>   // Required by drawHighlights logic (optional optimization)

// Define and initialize static const floats
const float Graphics::INDICATOR_RADIUS = 10.0f;
const float Graphics::INDICATOR_X = Graphics::BOARD_OFFSET_X;
const float Graphics::INDICATOR_Y = Graphics::BOARD_OFFSET_Y - Graphics::INDICATOR_RADIUS * 2.5f;

// Define Night Mode Colors (or your preferred theme)
namespace NightColors {
    const sf::Color Background = sf::Color(40, 40, 50);
    const sf::Color GridLine = sf::Color(100, 100, 110);
    const sf::Color Land = sf::Color(0, 80, 20);
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
    const sf::Color ButtonFill = sf::Color(80, 80, 90);
    const sf::Color ButtonText = sf::Color::White;
    // Book highlight colors
    const sf::Color BookBorder = sf::Color(0, 200, 0, 220); // Brighter green border
    const sf::Color BookTargetFill = sf::Color(0, 220, 0, 120); // Brighter green fill
    // Game UI button colors (optional)
    const sf::Color BookButtonOn = sf::Color(50, 150, 50);
    const sf::Color BookButtonOff = sf::Color(150, 50, 50);
}


// Constructor
Graphics::Graphics() : pieceDisplayMode(0), boardFlipped(true) {
    if (!font.loadFromFile("assets/arial.ttf")) {
         std::cerr << "Warning: Could not load default font 'assets/arial.ttf'." << std::endl;
    }
    // setupUIElements is called AFTER font loading attempt
    setupUIElements();
    turnIndicatorDot.setRadius(INDICATOR_RADIUS);
    turnIndicatorDot.setOrigin(INDICATOR_RADIUS, INDICATOR_RADIUS);
    turnIndicatorDot.setPosition(INDICATOR_X + INDICATOR_RADIUS, INDICATOR_Y);
    turnIndicatorDot.setOutlineColor(sf::Color(200, 200, 200));
    turnIndicatorDot.setOutlineThickness(1.0f);
}

// Load graphical assets
void Graphics::loadAssets() {
    if (!font.loadFromFile("assets/arial.ttf")) {
         std::cerr << "Error loading font assets/arial.ttf in loadAssets!" << std::endl;
         // Attempt to setup UI again in case font failed before
         setupUIElements();
    } else if (font.getInfo().family.empty()) { // Check if font loaded correctly
         std::cerr << "Warning: Font loaded but appears invalid." << std::endl;
         setupUIElements(); // Still try
    } else {
        // Font loaded successfully, ensure UI is set up
        setupUIElements();
    }
}

// Helper to create a standard button (Member function)
void Graphics::setupButton(ButtonUI& button, const std::string& text, float x, float y, float width, float height) {
    // Use the member font 'this->font' (or just 'font')
    // Check if the member font is valid before using it
    if (this->font.getInfo().family.empty()) {
         // Don't print error repeatedly if font load failed initially
         // std::cerr << "Error in setupButton: Font not loaded or invalid." << std::endl;
         button.bounds = sf::FloatRect(0,0,0,0); // Ensure bounds are zeroed if setup fails
         return; // Don't setup if font is invalid
    }

    button.shape.setSize(sf::Vector2f(width, height));
    button.shape.setPosition(x, y);
    button.shape.setFillColor(NightColors::ButtonFill); // Default fill
    button.label.setFont(this->font); // Use member font
    button.label.setString(text);
    button.label.setCharacterSize(16);
    button.label.setFillColor(NightColors::ButtonText);
    sf::FloatRect bounds = button.label.getLocalBounds();
    // Use integer division for pixel-perfect centering if desired, but float is usually fine
    button.label.setOrigin(bounds.left + bounds.width / 2.0f, bounds.top + bounds.height / 2.0f);
    button.label.setPosition(x + width / 2.0f, y + height / 2.0f);
    button.bounds = button.shape.getGlobalBounds();
}


// Helper to create UI buttons
void Graphics::setupUIElements() {
    if (font.getInfo().family.empty()) {
        if (!font.loadFromFile("assets/arial.ttf")) {
             std::cerr << "Critical Error: Cannot set up UI elements without font." << std::endl;
             return;
        }
        if (font.getInfo().family.empty()) {
             std::cerr << "Critical Error: Font loading failed in setupUIElements." << std::endl;
             return;
        }
    }

    // Determine Panel X based on potentially flipped board offset?
    // For now, assume UI panel is always on the right relative to the window.
    const float panelX = BOARD_OFFSET_X + BOARD_COLS * SQUARE_SIZE + 20;
    const float panelWidth = UI_PANEL_WIDTH; // Use constant defined in header

    // --- Setup Mode UI ---
    float currentY_setup = BOARD_OFFSET_Y;
    const float setupModeButtonWidth = 100.0f; // Specific width for setup buttons
    setupButton(clearButton, "Clear", panelX, currentY_setup, setupModeButtonWidth, UI_BUTTON_HEIGHT);
    sideButton.shape.setSize(sf::Vector2f(setupModeButtonWidth, UI_BUTTON_HEIGHT));
    sideButton.shape.setPosition(panelX + setupModeButtonWidth + UI_BUTTON_PADDING, currentY_setup);
    sideButton.label.setFont(font); sideButton.label.setCharacterSize(16); sideButton.label.setFillColor(NightColors::ButtonText);
    sideButton.bounds = sideButton.shape.getGlobalBounds();
    currentY_setup += UI_BUTTON_HEIGHT + UI_BUTTON_PADDING * 2;
    PieceType pieceOrder[] = { PieceType::RAT, PieceType::CAT, PieceType::DOG, PieceType::WOLF, PieceType::LEOPARD, PieceType::TIGER, PieceType::LION, PieceType::ELEPHANT };
    int buttonsPerRow = 2; float pieceButtonWidth = (panelWidth - UI_BUTTON_PADDING * (buttonsPerRow -1) - 30) / buttonsPerRow;
    int currentX_setup = panelX;
    for (int i = 0; i < 8; ++i) {
        PieceType type = pieceOrder[i]; ButtonUI& btn = pieceButtons[type];
        setupButton(btn, std::to_string(GameState().getRank(type)), currentX_setup, currentY_setup, pieceButtonWidth, UI_BUTTON_HEIGHT);
        btn.label.setFillColor(sf::Color::Black);
        if ((i + 1) % buttonsPerRow == 0) { currentX_setup = panelX; currentY_setup += UI_BUTTON_HEIGHT + UI_BUTTON_PADDING; }
        else { currentX_setup += pieceButtonWidth + UI_BUTTON_PADDING; }
    }
    if (8 % buttonsPerRow != 0) { currentY_setup += UI_BUTTON_HEIGHT + UI_BUTTON_PADDING; }
    currentY_setup += UI_BUTTON_PADDING;
    setupButton(finishButton, "Finish (F)", panelX, currentY_setup, panelWidth - 30, UI_BUTTON_HEIGHT);
    finishButton.shape.setFillColor(sf::Color(50, 150, 50));


    // --- Book Editor UI ---
    float currentY_editor = BOARD_OFFSET_Y; // Start editor UI from top
    setupButton(saveLineButton, "Save Line", panelX, currentY_editor, UI_BUTTON_WIDTH, UI_BUTTON_HEIGHT);
    currentY_editor += UI_BUTTON_HEIGHT + UI_BUTTON_PADDING;
    setupButton(resetBoardButton, "Reset Board", panelX, currentY_editor, UI_BUTTON_WIDTH, UI_BUTTON_HEIGHT);
    currentY_editor += UI_BUTTON_HEIGHT + UI_BUTTON_PADDING;
    setupButton(undoEditorButton, "Undo Move", panelX, currentY_editor, UI_BUTTON_WIDTH, UI_BUTTON_HEIGHT);
    currentY_editor += UI_BUTTON_HEIGHT + UI_BUTTON_PADDING;
    setupButton(exitEditorButton, "Exit Editor", panelX, currentY_editor, UI_BUTTON_WIDTH, UI_BUTTON_HEIGHT);
    exitEditorButton.shape.setFillColor(sf::Color(150, 50, 50));


    // --- Game Mode UI ---
    float currentY_game = BOARD_OFFSET_Y; // Start from top for game UI buttons too
    // Book toggle button - text/color set dynamically
    setupButton(bookToggleButton, "Book ?", panelX, currentY_game, UI_BUTTON_WIDTH, UI_BUTTON_HEIGHT);
    currentY_game += UI_BUTTON_HEIGHT + UI_BUTTON_PADDING;
    // Depth adjust button - text set dynamically
    setupButton(depthAdjustButton, "Depth ?", panelX, currentY_game, UI_BUTTON_WIDTH, UI_BUTTON_HEIGHT);

}


// Convert mouse pixel coordinates to board coordinates (Handles flip)
sf::Vector2i Graphics::getClickedSquare(const sf::Vector2i& mousePos) const {
    float relativeX = static_cast<float>(mousePos.x) - BOARD_OFFSET_X;
    float relativeY = static_cast<float>(mousePos.y) - BOARD_OFFSET_Y;
    if (relativeX < 0 || relativeY < 0 || relativeX >= BOARD_COLS * SQUARE_SIZE || relativeY >= BOARD_ROWS * SQUARE_SIZE) {
        return sf::Vector2i(-1, -1);
    }
    int apparentCol = static_cast<int>(std::floor(relativeX / SQUARE_SIZE));
    int apparentRow = static_cast<int>(std::floor(relativeY / SQUARE_SIZE));
    if (boardFlipped) {
        int internalRow = BOARD_ROWS - 1 - apparentRow; int internalCol = BOARD_COLS - 1 - apparentCol;
        return sf::Vector2i(internalCol, internalRow);
    } else {
        return sf::Vector2i(apparentCol, apparentRow);
    }
}


// --- Click detection for UI buttons ---
PieceType Graphics::getClickedSetupPieceButton(const sf::Vector2i& mousePos) const {
    for (const auto& pair : pieceButtons) { if (pair.second.bounds.width > 0 && pair.second.bounds.contains(static_cast<float>(mousePos.x), static_cast<float>(mousePos.y))) return pair.first; } return PieceType::EMPTY; }
bool Graphics::isClickOnClearButton(const sf::Vector2i& mousePos) const { return clearButton.bounds.width > 0 && clearButton.bounds.contains(static_cast<float>(mousePos.x), static_cast<float>(mousePos.y)); }
bool Graphics::isClickOnSideButton(const sf::Vector2i& mousePos) const { return sideButton.bounds.width > 0 && sideButton.bounds.contains(static_cast<float>(mousePos.x), static_cast<float>(mousePos.y)); }
bool Graphics::isClickOnFinishButton(const sf::Vector2i& mousePos) const { return finishButton.bounds.width > 0 && finishButton.bounds.contains(static_cast<float>(mousePos.x), static_cast<float>(mousePos.y)); }
bool Graphics::isClickOnSaveLineButton(const sf::Vector2i& mousePos) const { return saveLineButton.bounds.width > 0 && saveLineButton.bounds.contains(static_cast<float>(mousePos.x), static_cast<float>(mousePos.y)); }
bool Graphics::isClickOnResetBoardButton(const sf::Vector2i& mousePos) const { return resetBoardButton.bounds.width > 0 && resetBoardButton.bounds.contains(static_cast<float>(mousePos.x), static_cast<float>(mousePos.y)); }
bool Graphics::isClickOnExitEditorButton(const sf::Vector2i& mousePos) const { return exitEditorButton.bounds.width > 0 && exitEditorButton.bounds.contains(static_cast<float>(mousePos.x), static_cast<float>(mousePos.y)); }
bool Graphics::isClickOnUndoEditorButton(const sf::Vector2i& mousePos) const { return undoEditorButton.bounds.width > 0 && undoEditorButton.bounds.contains(static_cast<float>(mousePos.x), static_cast<float>(mousePos.y)); }

// <<< NEW: Game UI Click Detectors >>>
bool Graphics::isClickOnBookToggleButton(const sf::Vector2i& mousePos) const {
    return bookToggleButton.bounds.width > 0 && bookToggleButton.bounds.contains(static_cast<float>(mousePos.x), static_cast<float>(mousePos.y));
}
bool Graphics::isClickOnDepthAdjustButton(const sf::Vector2i& mousePos) const {
    return depthAdjustButton.bounds.width > 0 && depthAdjustButton.bounds.contains(static_cast<float>(mousePos.x), static_cast<float>(mousePos.y));
}


// Toggle display implementation
void Graphics::togglePieceDisplay() { pieceDisplayMode = (pieceDisplayMode + 1) % 3; }
// Toggle board flip implementation
void Graphics::toggleBoardFlip() { boardFlipped = !boardFlipped; }
// Helper to get screen position based on flip
sf::Vector2f Graphics::getScreenPos(int r, int c) const {
    int displayRow = r; int displayCol = c;
    if (boardFlipped) { displayRow = BOARD_ROWS - 1 - r; displayCol = BOARD_COLS - 1 - c; }
    return sf::Vector2f(BOARD_OFFSET_X + displayCol * SQUARE_SIZE, BOARD_OFFSET_Y + displayRow * SQUARE_SIZE);
}


// Main drawing function - UPDATED
void Graphics::drawBoard(sf::RenderWindow& window,
                         const GameState& gameState,
                         AppMode currentMode,
                         Player setupPlayer,
                         PieceType selectedSetupPiece,
                         bool gameOver,
                         const std::vector<Move>& legalMoveHighlights,
                         int selectedRow, int selectedCol,
                         const Move& lastAiMove,
                         const std::vector<sf::Vector2i>& bookStartingSquares,
                         const std::vector<sf::Vector2i>& bookTargetSquares,
                         // <<< NEW: Pass Game UI state >>>
                         bool isBookEnabled,
                         int currentSearchDepth
                        ) {
    window.clear(NightColors::Background);

    if ((currentMode == AppMode::GAME || currentMode == AppMode::BOOK_EDITOR) && !gameOver) {
         drawTurnIndicator(window, gameState);
    }

    drawGrid(window, gameState);
    drawPieces(window, gameState);

    // Draw UI based on mode
    if (currentMode == AppMode::SETUP) {
        drawSetupUI(window, setupPlayer, selectedSetupPiece);
        // No highlights needed in setup mode usually
    } else if (currentMode == AppMode::BOOK_EDITOR) {
        drawBookEditorUI(window); // Draw book editor UI
        // <<< Pass book data to drawHighlights >>>
        drawHighlights(window, currentMode, legalMoveHighlights, selectedRow, selectedCol, {-1,-1,-1,-1}, bookStartingSquares, bookTargetSquares);
    } else { // GAME mode
        // <<< NEW: Draw Game UI >>>
        drawGameUI(window, isBookEnabled, currentSearchDepth);
        // <<< Pass empty book data to drawHighlights >>>
        drawHighlights(window, currentMode, legalMoveHighlights, selectedRow, selectedCol, lastAiMove, {}, {});
    }
}

// --- Private Helper Functions ---

// Draw the grid lines, coloring special squares
void Graphics::drawGrid(sf::RenderWindow& window, const GameState& gameState) {
    sf::RectangleShape square(sf::Vector2f(SQUARE_SIZE, SQUARE_SIZE));
    square.setOutlineColor(NightColors::GridLine);
    square.setOutlineThickness(1.0f);

    for (int r = 0; r < BOARD_ROWS; ++r) {
        for (int c = 0; c < BOARD_COLS; ++c) {
            square.setPosition(getScreenPos(r, c));
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


// Draw the pieces onto the board
void Graphics::drawPieces(sf::RenderWindow& window, const GameState& gameState) {
    // Check font validity at the start
    if (font.getInfo().family.empty()) return;

    sf::Text mainText; sf::Text subText;
    mainText.setFont(font); subText.setFont(font); // Set font for both
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
                if (pieceDisplayMode != 2 && !subText.getString().isEmpty()) { // Check if subtext is not empty
                    sf::FloatRect subBounds = subText.getLocalBounds();
                    // Adjust origin for bottom-right alignment
                    subText.setOrigin(subBounds.left + subBounds.width, subBounds.top + subBounds.height);
                    // Position relative to the screen square's bottom-right
                    subText.setPosition(screenPos.x + SQUARE_SIZE - subPadding, screenPos.y + SQUARE_SIZE - subPadding);
                    window.draw(subText);
                }
            }
        }
    }
}


// Draw highlights - MODIFIED
void Graphics::drawHighlights(sf::RenderWindow& window,
                              AppMode currentMode, // Need mode
                              const std::vector<Move>& legalMoveHighlights, // Normal legal moves (used in Game, potentially Editor)
                              int selectedRow, int selectedCol,             // Currently selected piece
                              const Move& lastAiMove,                       // Last AI move (Game mode only)
                              const std::vector<sf::Vector2i>& bookStartingSquares, // Pieces with book moves (Editor mode)
                              const std::vector<sf::Vector2i>& bookTargetSquares    // Target squares for selected piece (Editor mode)
                             )
{
    sf::RectangleShape highlightShape(sf::Vector2f(SQUARE_SIZE, SQUARE_SIZE));
    const float borderThickness = 3.0f;

    // --- Common Highlights (Selected Piece) ---
    if (selectedRow != -1 && selectedCol != -1) {
        highlightShape.setPosition(getScreenPos(selectedRow, selectedCol));
        highlightShape.setFillColor(sf::Color::Transparent);
        highlightShape.setOutlineColor(NightColors::SelectedOutline);
        highlightShape.setOutlineThickness(borderThickness);
        window.draw(highlightShape);
    }

    // --- Mode-Specific Highlights ---
    if (currentMode == AppMode::GAME) {
        // Highlight Last AI Move (Only if no piece is selected)
        if (selectedRow == -1 && lastAiMove.fromRow != -1) {
             highlightShape.setFillColor(sf::Color::Transparent); highlightShape.setOutlineColor(NightColors::LastAiOutline); highlightShape.setOutlineThickness(borderThickness);
             highlightShape.setPosition(getScreenPos(lastAiMove.fromRow, lastAiMove.fromCol)); window.draw(highlightShape);
             highlightShape.setPosition(getScreenPos(lastAiMove.toRow, lastAiMove.toCol)); window.draw(highlightShape);
        }
        // Highlight Normal Legal Moves
        highlightShape.setFillColor(NightColors::LegalMoveFill); highlightShape.setOutlineThickness(0);
        for (const auto& move : legalMoveHighlights) { highlightShape.setPosition(getScreenPos(move.toRow, move.toCol)); window.draw(highlightShape); }
    } else if (currentMode == AppMode::BOOK_EDITOR) {
        if (selectedRow == -1) {
            // No piece selected: Highlight pieces that have book moves available
            highlightShape.setFillColor(sf::Color::Transparent); highlightShape.setOutlineColor(NightColors::BookBorder); highlightShape.setOutlineThickness(borderThickness);
            for (const auto& pos : bookStartingSquares) { highlightShape.setPosition(getScreenPos(pos.y, pos.x)); window.draw(highlightShape); } // Use y for row, x for col
        } else {
            // Piece selected: Highlight book target squares for that piece
            highlightShape.setFillColor(NightColors::BookTargetFill); highlightShape.setOutlineThickness(0);
            for (const auto& pos : bookTargetSquares) { highlightShape.setPosition(getScreenPos(pos.y, pos.x)); window.draw(highlightShape); } // Use y for row, x for col
        }
    }
}


// Draw Setup UI
void Graphics::drawSetupUI(sf::RenderWindow& window, Player setupPlayer, PieceType selectedSetupPiece) {
    if (clearButton.bounds.width <= 0) return; // Basic check if UI is initialized

    window.draw(clearButton.shape); window.draw(clearButton.label);

    // Update dynamic side button
    sideButton.shape.setFillColor(setupPlayer == Player::PLAYER1 ? NightColors::P1_Piece : NightColors::P2_Piece);
    sideButton.label.setString(setupPlayer == Player::PLAYER1 ? "P1" : "P2");
    sf::FloatRect sbounds = sideButton.label.getLocalBounds();
    sideButton.label.setOrigin(sbounds.left + sbounds.width / 2.0f, sbounds.top + sbounds.height / 2.0f);
    sideButton.label.setPosition(sideButton.shape.getPosition().x + sideButton.shape.getSize().x / 2.0f, sideButton.shape.getPosition().y + UI_BUTTON_HEIGHT / 2.0f);
    window.draw(sideButton.shape); window.draw(sideButton.label);

    // Draw piece buttons with correct color and highlight
    for (auto const& [type, button] : pieceButtons) {
         if (button.bounds.width <= 0) continue;
        sf::RectangleShape currentShape = button.shape;
        currentShape.setFillColor(setupPlayer == Player::PLAYER1 ? NightColors::P1_Piece : NightColors::P2_Piece);
        if (type == selectedSetupPiece) { currentShape.setOutlineColor(sf::Color::Yellow); currentShape.setOutlineThickness(3.0f); }
        else { currentShape.setOutlineThickness(0); }
        window.draw(currentShape); window.draw(button.label);
    }
    if (finishButton.bounds.width > 0) {
        window.draw(finishButton.shape); window.draw(finishButton.label);
    }
}

// Draw Turn Indicator
void Graphics::drawTurnIndicator(sf::RenderWindow& window, const GameState& gameState) {
    Player currentPlayer = gameState.getCurrentPlayer();
    if (currentPlayer == Player::PLAYER1) turnIndicatorDot.setFillColor(NightColors::P1_Piece);
    else if (currentPlayer == Player::PLAYER2) turnIndicatorDot.setFillColor(NightColors::P2_Piece);
    else turnIndicatorDot.setFillColor(sf::Color::Transparent);
    window.draw(turnIndicatorDot);
}

// Draw Book Editor UI
void Graphics::drawBookEditorUI(sf::RenderWindow& window) {
    if (saveLineButton.bounds.width <= 0) return; // Basic check if UI is initialized
    window.draw(saveLineButton.shape); window.draw(saveLineButton.label);
    window.draw(resetBoardButton.shape); window.draw(resetBoardButton.label);
    window.draw(undoEditorButton.shape); window.draw(undoEditorButton.label);
    window.draw(exitEditorButton.shape); window.draw(exitEditorButton.label);
}

// <<< NEW: Draw Game UI >>>
void Graphics::drawGameUI(sf::RenderWindow& window, bool isBookEnabled, int currentSearchDepth) {
    // Check if buttons have been initialized
    if (bookToggleButton.bounds.width <= 0 || depthAdjustButton.bounds.width <= 0 || font.getInfo().family.empty()) {
        return; // Avoid drawing if UI not set up or font missing
    }

    // --- Draw Book Toggle Button ---
    bookToggleButton.label.setString(isBookEnabled ? "Book ON" : "Book OFF");
    bookToggleButton.shape.setFillColor(isBookEnabled ? NightColors::BookButtonOn : NightColors::BookButtonOff);
    sf::FloatRect bookBounds = bookToggleButton.label.getLocalBounds();
    bookToggleButton.label.setOrigin(bookBounds.left + bookBounds.width / 2.0f, bookBounds.top + bookBounds.height / 2.0f);
    bookToggleButton.label.setPosition(bookToggleButton.shape.getPosition().x + UI_BUTTON_WIDTH / 2.0f, bookToggleButton.shape.getPosition().y + UI_BUTTON_HEIGHT / 2.0f);
    window.draw(bookToggleButton.shape);
    window.draw(bookToggleButton.label);


    // --- Draw Depth Adjust Button ---
    depthAdjustButton.label.setString("Depth " + std::to_string(currentSearchDepth));
    depthAdjustButton.shape.setFillColor(NightColors::ButtonFill); // Use default color
    sf::FloatRect depthBounds = depthAdjustButton.label.getLocalBounds();
    depthAdjustButton.label.setOrigin(depthBounds.left + depthBounds.width / 2.0f, depthBounds.top + depthBounds.height / 2.0f);
    depthAdjustButton.label.setPosition(depthAdjustButton.shape.getPosition().x + UI_BUTTON_WIDTH / 2.0f, depthAdjustButton.shape.getPosition().y + UI_BUTTON_HEIGHT / 2.0f);
    window.draw(depthAdjustButton.shape);
    window.draw(depthAdjustButton.label);
}


