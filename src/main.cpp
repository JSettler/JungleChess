#include <SFML/Graphics.hpp>
#include "GameState.h"
#include "Graphics.h"
#include "AI.h"
#include "Common.h"
#include "Book.h"       // Include Book.h for opening book functionality & editor saving
#include <iostream>
#include <vector>
#include <string>
#include <chrono>
#include <cstring>      // Required for strcmp
#include <fstream>      // Required for file operations
#include <sstream>      // Required for std::to_string in window title
#include <stdexcept>    // For std::stoi exceptions
#include <limits>       // For numeric_limits
#include <iomanip>      // For std::fixed, std::setprecision

// --- Forward Declarations for Save/Load ---
bool saveGame(const std::vector<GameState>& history, const std::string& filename);
bool loadGame(GameState& currentGameState, const std::string& filename, std::vector<GameState>& history);

// Helper function to reset game state to initial position
void resetToInitialState(GameState& gs, std::vector<GameState>& hist, std::vector<GameState>& redoHist, std::vector<Move>& moveSeq, bool& aiFirst) {
    gs.setupInitialBoard(); // Resets board and player to P1, recalculates hash
    hist.clear(); // Clear GameState history
    redoHist.clear(); // Clear redo history
    hist.push_back(gs); // Add initial state as the first entry
    moveSeq.clear(); // Clear move sequence
    aiFirst = false; // Reset AI first move flag
}


int main(int argc, char* argv[]) {

    // --- Argument Parsing ---
    bool showHelp = false;
    bool debugMode = false;
    bool quietMode = false;
    bool unknownArgumentFound = false;
    std::string unknownArg = "";
    int searchDepth = 6;
    AppMode currentMode = AppMode::GAME; // Default mode
    bool setupFlag = false;
    bool bookFlag = false;

    const char* progName = (argc > 0 && argv[0] != nullptr) ? argv[0] : "jungle_chess";
    if (progName == nullptr) progName = "jungle_chess";
    std::string usageSyntax = "Usage: " + std::string(progName) + " [--depth N] [--setup | --book] [-n | -d | -h | --help | -?]";


    for (int i = 1; i < argc; ++i) {
        if (strcmp(argv[i], "-h") == 0 || strcmp(argv[i], "--help") == 0 || strcmp(argv[i], "-?") == 0) {
            showHelp = true;
        } else if (strcmp(argv[i], "-d") == 0) {
            debugMode = true; quietMode = false;
        } else if (strcmp(argv[i], "-n") == 0 && !debugMode) {
            quietMode = true;
        } else if (strcmp(argv[i], "--setup") == 0) {
            setupFlag = true;
        } else if (strcmp(argv[i], "--book") == 0) { // Book flag
            bookFlag = true;
        } else if (strcmp(argv[i], "--depth") == 0) {
            if (i + 1 < argc) {
                try {
                    int depthValue = std::stoi(argv[i + 1]);
                    if (depthValue > 0 && depthValue < 20) {
                         searchDepth = depthValue; i++;
                         if (!quietMode) std::cout << "Search depth set to " << searchDepth << " plies." << std::endl;
                    } else {
                         std::cerr << "Error: Invalid search depth value '" << argv[i + 1] << "'. Must be positive and reasonable (< 20)." << std::endl; return 1;
                    }
                } catch (const std::invalid_argument& e) {
                    std::cerr << "Error: Invalid number format for --depth: '" << argv[i + 1] << "'" << std::endl; return 1;
                } catch (const std::out_of_range& e) {
                     std::cerr << "Error: Search depth value out of range: '" << argv[i + 1] << "'" << std::endl; return 1;
                }
            } else {
                std::cerr << "Error: Missing value after --depth flag." << std::endl; std::cerr << usageSyntax << std::endl; return 1;
            }
        } else {
             if (!unknownArgumentFound) { unknownArgumentFound = true; unknownArg = argv[i]; }
        }
    }

    // Check for conflicting modes
    if (setupFlag && bookFlag) {
        std::cerr << "Error: Cannot use --setup and --book flags together." << std::endl;
        std::cerr << usageSyntax << std::endl;
        return 1;
    }
    // Set the starting mode based on flags
    if (setupFlag) {
        currentMode = AppMode::SETUP;
        if (!quietMode) std::cout << "Starting in Setup Mode." << std::endl;
    } else if (bookFlag) {
        currentMode = AppMode::BOOK_EDITOR;
        if (!quietMode) std::cout << "Starting in Book Editor Mode." << std::endl;
    }


    // Handle Help or Unknown Argument
    if (showHelp) {
        std::cout << usageSyntax << "\n\n";
        std::cout << "Options:\n";
        std::cout << "  --depth N : Set AI search depth to N plies (default: 6).\n";
        std::cout << "  --setup   : Start in board setup mode.\n";
        std::cout << "  --book    : Start in opening book editor mode.\n";
        std::cout << "  -n        : Quiet mode (minimal console output).\n";
        std::cout << "  -d        : Debug mode (verbose AI output).\n";
        std::cout << "  -h, --help, -? : Show this help message and exit.\n\n";
        std::cout << "In-Game Keys (Game Mode):\n";
        std::cout << "  <Backspace>       : Take back last half-move (Undo).\n";
        std::cout << "  <Shift+Backspace> : Redo last undone half-move.\n";
        std::cout << "  S                 : Save current game state to dsq-game.sav.\n";
        std::cout << "  L                 : Load game state from dsq-game.sav (clears undo/redo history).\n";
        std::cout << "  P                 : Cycle piece display emphasis (Letters <-> Numbers).\n";
        std::cout << "  G                 : Make AI move (if it's AI's turn or start of game).\n";
        std::cout << "  R                 : Rotate board view 180 degrees.\n";
        std::cout << "  <Escape>          : Quit game.\n\n";
        std::cout << "In-Game Keys (Setup Mode):\n";
        std::cout << "  Left Click  : Place selected piece / Select UI button.\n";
        std::cout << "  Right Click : Remove piece from board square.\n";
        std::cout << "  1-8         : Select piece type (Rat=1 to Elephant=8).\n";
        std::cout << "  S           : Switch player side for piece placement.\n";
        std::cout << "  P           : Cycle piece display emphasis.\n";
        std::cout << "  F           : Finish setup and start game.\n";
        std::cout << "  R           : Rotate board view 180 degrees.\n";
        std::cout << "  <Escape>    : Quit game.\n\n";
        std::cout << "In-Game Keys (Book Editor Mode):\n";
        std::cout << "  Left Click  : Select piece / Make move for current player.\n";
        std::cout << "  Right Click : Deselect piece.\n";
        std::cout << "  P           : Cycle piece display emphasis.\n";
        std::cout << "  R           : Rotate board view 180 degrees.\n";
        std::cout << "  (UI Buttons): Save Line, Reset Board, Undo Move, Exit Editor.\n";
        std::cout << "  <Escape>    : Quit program.\n";
        return 0;
    }
    else if (unknownArgumentFound) {
        std::cerr << "Error: Unknown parameter '" << unknownArg << "'\n";
        std::cerr << usageSyntax << std::endl;
        return 1;
    }


    // Announce operational mode
    if (debugMode) std::cout << "Debug mode enabled." << std::endl;
    else if (quietMode) { /* no output */ }


    // --- Initialization ---
    std::string windowTitle = "JungleChess v1.0";
    if (currentMode == AppMode::GAME) windowTitle += " [depth = " + std::to_string(searchDepth) + "]";
    else if (currentMode == AppMode::SETUP) windowTitle += " [Setup Mode]";
    else if (currentMode == AppMode::BOOK_EDITOR) windowTitle += " [Book Editor Mode]";

    sf::RenderWindow window(sf::VideoMode(800, 700), windowTitle);
    window.setFramerateLimit(60);
    GameState gameState; // Will be reset if starting in editor/setup
    Graphics graphics;
    graphics.loadAssets();
    Player humanPlayer = Player::PLAYER1; // Not relevant in editor mode
    Player aiPlayer = Player::PLAYER2;    // Not relevant in editor mode
    std::vector<GameState> history; // GameState history (used for Undo in Game & Editor)
    std::vector<GameState> redoHistory; // GameState redo history (used only in Game mode)
    std::vector<Move> moveHistorySequence; // Move sequence history (used for book & editor)
    bool bookAvailable = Book::load(); // Load book
    Move selectedMove = {-1,-1,-1,-1}; bool pieceSelected = false;
    std::vector<Move> selectedPieceLegalMoves; Move lastAiMove = {-1,-1,-1,-1};
    bool gameOver = false; Player winner = Player::NONE; std::string winReason = "";
    Player setupPlayer = Player::PLAYER1; PieceType selectedSetupPiece = PieceType::EMPTY;
    bool confirmingQuit = false;
    bool forceAiMove = false; bool waitingForGo = false; bool aiMadeFirstMove = false;
    const std::string saveFilename = "dsq-game.sav";

    // Reset board if starting in Setup or Book Editor mode
    if (currentMode == AppMode::SETUP || currentMode == AppMode::BOOK_EDITOR) {
        resetToInitialState(gameState, history, redoHistory, moveHistorySequence, aiMadeFirstMove);
    } else {
        // If starting in Game mode, ensure initial state is in history
        if (history.empty()) { history.push_back(gameState); }
    }


    // --- Main Loop ---
    while (window.isOpen()) {
        forceAiMove = false; // Reset per frame
        sf::Event event;
        while (window.pollEvent(event)) {
            if (event.type == sf::Event::Closed) window.close();
            if (confirmingQuit) { // Handle Quit Confirmation
                 if (event.type == sf::Event::KeyPressed) { if (event.key.code == sf::Keyboard::Y) { window.close(); } else { confirmingQuit = false; } }
                 else if (event.type == sf::Event::MouseButtonPressed) { confirmingQuit = false; }
                 continue;
            }
            if (currentMode == AppMode::GAME && gameOver && event.type == sf::Event::MouseButtonPressed && event.mouseButton.button == sf::Mouse::Left) { window.close(); continue; } // Click to exit on game over

            // Global Key Presses
            if (event.type == sf::Event::KeyPressed) {
                if (event.key.code == sf::Keyboard::Escape) { confirmingQuit = true; continue; }
                if (event.key.code == sf::Keyboard::P) { graphics.togglePieceDisplay(); continue; }
                if (event.key.code == sf::Keyboard::R) { graphics.toggleBoardFlip(); pieceSelected = false; selectedMove = {-1,-1,-1,-1}; selectedPieceLegalMoves.clear(); continue; }

                // Game-Specific Keys only in Game Mode
                bool allowGameKeys = (currentMode == AppMode::GAME && !gameOver);
                if (allowGameKeys) {
                    // Handle Takeback (Undo)
                    if (event.key.code == sf::Keyboard::BackSpace && !event.key.shift) {
                        if (history.size() >= 2) { redoHistory.push_back(history.back()); history.pop_back(); gameState = history.back(); waitingForGo = (gameState.getCurrentPlayer() == aiPlayer); pieceSelected = false; selectedMove = {-1,-1,-1,-1}; selectedPieceLegalMoves.clear(); lastAiMove = {-1,-1,-1,-1}; if (!moveHistorySequence.empty()) { moveHistorySequence.pop_back(); } aiMadeFirstMove = false; if (!quietMode) std::cout << "<<< Undo! (Book seq len: " << moveHistorySequence.size() << ") >>>" << std::endl; }
                        else { if (!quietMode) std::cout << "Nothing further to undo." << std::endl; }
                    }
                    // Handle Redo
                    else if (event.key.code == sf::Keyboard::BackSpace && event.key.shift) {
                         if (!redoHistory.empty()) { moveHistorySequence.clear(); history.push_back(redoHistory.back()); redoHistory.pop_back(); gameState = history.back(); waitingForGo = (gameState.getCurrentPlayer() == aiPlayer); pieceSelected = false; selectedMove = {-1,-1,-1,-1}; selectedPieceLegalMoves.clear(); lastAiMove = {-1,-1,-1,-1}; aiMadeFirstMove = false; if (!quietMode) std::cout << ">>> Redo! (Book disabled) >>>" << std::endl; }
                         else if (!quietMode) { std::cout << "Nothing to redo." << std::endl; }
                    }
                    // Handle Save
                    else if (event.key.code == sf::Keyboard::S) {
                        if (saveGame(history, saveFilename)) { if (!quietMode) std::cout << "Game state saved to " << saveFilename << std::endl; }
                    }
                    // Handle Load
                    else if (event.key.code == sf::Keyboard::L) {
                        if (loadGame(gameState, saveFilename, history)) { redoHistory.clear(); waitingForGo = (gameState.getCurrentPlayer() == aiPlayer); pieceSelected = false; selectedMove = {-1,-1,-1,-1}; selectedPieceLegalMoves.clear(); lastAiMove = {-1,-1,-1,-1}; moveHistorySequence.clear(); aiMadeFirstMove = false; if (!quietMode) std::cout << "Game state loaded from " << saveFilename << " (Book disabled after Load)" << std::endl; }
                    }
                    // Handle 'G' key (AI Go / Force AI Move)
                    else if (event.key.code == sf::Keyboard::G) {
                        if (gameState.getCurrentPlayer() == Player::PLAYER1 && history.size() == 1) { if (!quietMode) std::cout << "Player chose AI (Red) to move first." << std::endl; gameState.setCurrentPlayer(aiPlayer); gameState.recalculateHash(); aiMadeFirstMove = true; forceAiMove = true; waitingForGo = false; }
                        else if (gameState.getCurrentPlayer() == aiPlayer && waitingForGo) { if (!quietMode) std::cout << "'G' pressed - Proceeding with AI move." << std::endl; forceAiMove = true; waitingForGo = false; }
                        else if (!quietMode) { if (gameState.getCurrentPlayer() == aiPlayer && !waitingForGo) std::cout << "'G' pressed, but AI is already set to move." << std::endl; else std::cout << "'G' key only works on the very first turn or when it is AI's turn after an Undo/Redo." << std::endl; }
                    }
                } // End allowGameKeys
            } // End KeyPressed

            // --- Mode-Specific Event Handling ---
            if (currentMode == AppMode::SETUP) {
                // --- SETUP MODE ---
                 if (event.type == sf::Event::KeyPressed) {
                    if (event.key.code >= sf::Keyboard::Num1 && event.key.code <= sf::Keyboard::Num8) { selectedSetupPiece = static_cast<PieceType>(event.key.code - sf::Keyboard::Num1 + 1); if (!quietMode) std::cout << "Setup: Selected piece type " << static_cast<int>(selectedSetupPiece) << std::endl; }
                    else if (event.key.code == sf::Keyboard::S) { setupPlayer = (setupPlayer == Player::PLAYER1) ? Player::PLAYER2 : Player::PLAYER1; if (!quietMode) std::cout << "Setup: Switched to placing pieces for Player " << static_cast<int>(setupPlayer) << std::endl; selectedSetupPiece = PieceType::EMPTY; }
                    else if (event.key.code == sf::Keyboard::F) { // Finish Key
                        if (gameState.validateSetup()) {
                            currentMode = AppMode::GAME; GameState currentSetup = gameState;
                            resetToInitialState(gameState, history, redoHistory, moveHistorySequence, aiMadeFirstMove);
                            gameState.setBoard(currentSetup.getBoard()); gameState.setCurrentPlayer(Player::PLAYER1); gameState.recalculateHash();
                            history.clear(); history.push_back(gameState);
                            windowTitle = "JungleChess v1.0 [depth = " + std::to_string(searchDepth) + "]"; window.setTitle(windowTitle);
                            if (!quietMode) std::cout << "Setup finished. Player 1 to move." << std::endl; waitingForGo = false;
                        } else { if (!quietMode) std::cerr << "Setup Error: Invalid board position." << std::endl; }
                    }
                } else if (event.type == sf::Event::MouseButtonPressed) {
                     sf::Vector2i mousePos = sf::Mouse::getPosition(window);
                     if (graphics.isClickOnClearButton(mousePos)) { gameState.clearBoard(); setupPlayer = Player::PLAYER1; selectedSetupPiece = PieceType::EMPTY; if (!quietMode) std::cout << "Setup: Board cleared." << std::endl; }
                     else if (graphics.isClickOnSideButton(mousePos)) { setupPlayer = (setupPlayer == Player::PLAYER1) ? Player::PLAYER2 : Player::PLAYER1; if (!quietMode) std::cout << "Setup: Switched to placing pieces for Player " << static_cast<int>(setupPlayer) << std::endl; selectedSetupPiece = PieceType::EMPTY; }
                     else if (graphics.isClickOnFinishButton(mousePos)) { // Finish Button
                          if (gameState.validateSetup()) {
                             currentMode = AppMode::GAME; GameState currentSetup = gameState;
                             resetToInitialState(gameState, history, redoHistory, moveHistorySequence, aiMadeFirstMove);
                             gameState.setBoard(currentSetup.getBoard()); gameState.setCurrentPlayer(Player::PLAYER1); gameState.recalculateHash();
                             history.clear(); history.push_back(gameState);
                             windowTitle = "JungleChess v1.0 [depth = " + std::to_string(searchDepth) + "]"; window.setTitle(windowTitle);
                             if (!quietMode) std::cout << "Setup finished. Player 1 to move." << std::endl; waitingForGo = false;
                          } else { if (!quietMode) std::cerr << "Setup Error: Invalid board position." << std::endl; }
                     }
                     // --- CORRECTED Setup Board Click Logic ---
                     else { // Handle clicks not on the main UI buttons
                        PieceType clickedPieceType = graphics.getClickedSetupPieceButton(mousePos);
                        if (clickedPieceType != PieceType::EMPTY) {
                            // Clicked on a piece selection button
                            selectedSetupPiece = clickedPieceType;
                            if (!quietMode) std::cout << "Setup: Selected piece type " << static_cast<int>(selectedSetupPiece) << std::endl;
                        } else {
                            // Clicked somewhere else (potentially the board)
                            sf::Vector2i boardPos = graphics.getClickedSquare(mousePos);
                            if (gameState.isValidPosition(boardPos.y, boardPos.x)) {
                                // Clicked on a valid board square
                                if (event.mouseButton.button == sf::Mouse::Left) {
                                    // Left click: Place selected piece
                                    if (selectedSetupPiece != PieceType::EMPTY) {
                                        if (!gameState.setPieceAt(boardPos.y, boardPos.x, selectedSetupPiece, setupPlayer)) {
                                            if (!quietMode) std::cerr << "Setup Error: Cannot place piece there." << std::endl;
                                        }
                                    }
                                } else if (event.mouseButton.button == sf::Mouse::Right) {
                                    // Right click: Remove piece from square
                                    gameState.clearSquare(boardPos.y, boardPos.x);
                                    if (!quietMode) std::cout << "Setup: Cleared square (" << boardPos.y << "," << boardPos.x << ")" << std::endl;
                                }
                            }
                        }
                    } // --- End CORRECTED Setup Board Click Logic ---
                 } // End MouseButton Pressed (Setup)

            } else if (currentMode == AppMode::BOOK_EDITOR) {
                // --- BOOK EDITOR MODE ---
                if (event.type == sf::Event::MouseButtonPressed) {
                    sf::Vector2i mousePos = sf::Mouse::getPosition(window);
                    // --- UI Button Clicks ---
                    if (graphics.isClickOnSaveLineButton(mousePos)) {
                        Book::SaveResult result = Book::saveVariation(moveHistorySequence);
                        // <<< NO LONGER RESETTING BOARD HERE >>>
                        if (!quietMode) { // Provide feedback only if not quiet
                            switch(result) {
                                case Book::SaveResult::APPENDED: std::cout << "Book Editor: New variation saved." << std::endl; break;
                                case Book::SaveResult::UPDATED: std::cout << "Book Editor: Existing line updated." << std::endl; break;
                                case Book::SaveResult::ALREADY_EXISTS: std::cout << "Book Editor: (Sub)line exists. Not saved." << std::endl; break;
                                case Book::SaveResult::ERROR_EMPTY: std::cout << "Book Editor: Cannot save empty line." << std::endl; break;
                                case Book::SaveResult::ERROR_FILE: std::cout << "Book Editor: File error during save." << std::endl; break;
                            }
                        }
                    } else if (graphics.isClickOnResetBoardButton(mousePos)) {
                        resetToInitialState(gameState, history, redoHistory, moveHistorySequence, aiMadeFirstMove);
                        pieceSelected = false; selectedMove = {-1,-1,-1,-1}; selectedPieceLegalMoves.clear();
                        if (!quietMode) std::cout << "Book Editor: Board reset." << std::endl;
                    }
                    // <<< NEW: Handle Undo Button >>>
                    else if (graphics.isClickOnUndoEditorButton(mousePos)) {
                        if (history.size() > 1) { // Can only undo if not at the initial state (history[0])
                            history.pop_back(); // Remove the last GameState
                            gameState = history.back(); // Revert to the previous GameState
                            if (!moveHistorySequence.empty()) {
                                moveHistorySequence.pop_back(); // Remove the last move from sequence
                            }
                            pieceSelected = false; // Clear UI selection
                            selectedMove = {-1,-1,-1,-1};
                            selectedPieceLegalMoves.clear();
                             if (!quietMode) std::cout << "Book Editor: Undo last move." << std::endl;
                        } else {
                             if (!quietMode) std::cout << "Book Editor: Nothing to undo." << std::endl;
                        }
                    }
                    // ---
                    else if (graphics.isClickOnExitEditorButton(mousePos)) {
                        currentMode = AppMode::GAME; // Switch to game mode
                        resetToInitialState(gameState, history, redoHistory, moveHistorySequence, aiMadeFirstMove);
                        pieceSelected = false; selectedMove = {-1,-1,-1,-1}; selectedPieceLegalMoves.clear();
                        windowTitle = "JungleChess v1.0 [depth = " + std::to_string(searchDepth) + "]"; window.setTitle(windowTitle);
                        if (!quietMode) std::cout << "Exited Book Editor. Starting new game." << std::endl;
                    } else {
                        // --- Board Clicks (Move Input) ---
                        sf::Vector2i boardPos = graphics.getClickedSquare(mousePos);
                        if (gameState.isValidPosition(boardPos.y, boardPos.x)) {
                             if (event.mouseButton.button == sf::Mouse::Left) {
                                if (!pieceSelected) { // Select Piece
                                    Piece clickedPiece = gameState.getPiece(boardPos.y, boardPos.x);
                                    if (clickedPiece.owner == gameState.getCurrentPlayer()) {
                                        selectedMove.fromRow = boardPos.y; selectedMove.fromCol = boardPos.x; selectedMove.toRow = -1; selectedMove.toCol = -1;
                                        pieceSelected = true;
                                        selectedPieceLegalMoves = gameState.getLegalMovesForPiece(selectedMove.fromRow, selectedMove.fromCol);
                                    }
                                } else { // Move/Deselect
                                    if (boardPos.y == selectedMove.fromRow && boardPos.x == selectedMove.fromCol) { // Deselect
                                        pieceSelected = false; selectedMove = {-1, -1, -1, -1}; selectedPieceLegalMoves.clear();
                                    } else { // Attempt Move
                                        bool isValidTarget = false; Move attemptedMove = {selectedMove.fromRow, selectedMove.fromCol, boardPos.y, boardPos.x};
                                        for(const auto& legalMove : selectedPieceLegalMoves) if (legalMove == attemptedMove) { isValidTarget = true; break; }
                                        if (isValidTarget) {
                                            moveHistorySequence.push_back(attemptedMove); // Add to sequence
                                            gameState.applyMove(attemptedMove);
                                            gameState.switchPlayer(); // Switch player for next move
                                            history.push_back(gameState); // <<< Add new state to history for undo
                                            // redoHistory.clear(); // Don't need redo in editor? Clear it.
                                            pieceSelected = false; // Deselect after move
                                            selectedMove = {-1, -1, -1, -1};
                                            selectedPieceLegalMoves.clear();
                                            if (!quietMode) std::cout << "Book Move: " << Book::moveToAlgebraic(attemptedMove) << std::endl;
                                        } else {
                                             if (!quietMode) std::cout << "Book Editor: Invalid move target." << std::endl;
                                             pieceSelected = false; selectedMove = {-1,-1,-1,-1}; selectedPieceLegalMoves.clear();
                                        }
                                    }
                                }
                             } else if (event.mouseButton.button == sf::Mouse::Right) { // Deselect
                                 pieceSelected = false; selectedMove = {-1,-1,-1,-1}; selectedPieceLegalMoves.clear();
                             }
                        } else if (pieceSelected) { // Click outside board while piece selected
                             pieceSelected = false; selectedMove = {-1,-1,-1,-1}; selectedPieceLegalMoves.clear();
                        }
                    } // End Board Click Handling
                } // End MouseButtonPressed (Book Editor)

            } else { // currentMode == AppMode::GAME
                // --- GAME MODE --- (Human Input)
                if (!gameOver && gameState.getCurrentPlayer() == humanPlayer && event.type == sf::Event::MouseButtonPressed && event.mouseButton.button == sf::Mouse::Left) {
                     sf::Vector2i mousePos = sf::Mouse::getPosition(window); sf::Vector2i boardPos = graphics.getClickedSquare(mousePos);
                     if (gameState.isValidPosition(boardPos.y, boardPos.x)) {
                         if (!pieceSelected) { /* Select */ Piece p = gameState.getPiece(boardPos.y, boardPos.x); if (p.owner == humanPlayer) {selectedMove={boardPos.y, boardPos.x, -1, -1}; pieceSelected=true; lastAiMove={-1,-1,-1,-1}; selectedPieceLegalMoves=gameState.getLegalMovesForPiece(selectedMove.fromRow, selectedMove.fromCol);} }
                         else { /* Move/Deselect */ if (boardPos.y==selectedMove.fromRow && boardPos.x==selectedMove.fromCol) { pieceSelected=false; selectedMove={-1,-1,-1,-1}; selectedPieceLegalMoves.clear(); }
                             else { /* Attempt Move */ bool valid=false; Move attempt={selectedMove.fromRow, selectedMove.fromCol, boardPos.y, boardPos.x}; for(const auto& legal : selectedPieceLegalMoves) if (legal==attempt) { valid=true; break; }
                                 if (valid) { moveHistorySequence.push_back(attempt); gameState.applyMove(attempt); gameState.switchPlayer(); history.push_back(gameState); redoHistory.clear(); waitingForGo=false; pieceSelected=false; selectedMove={-1,-1,-1,-1}; selectedPieceLegalMoves.clear(); }
                                 else { if (!quietMode) std::cout << "Invalid move target." << std::endl; pieceSelected=false; selectedMove={-1,-1,-1,-1}; selectedPieceLegalMoves.clear(); }
                             }
                         }
                     } else if (pieceSelected) { pieceSelected=false; selectedMove={-1,-1,-1,-1}; selectedPieceLegalMoves.clear(); }
                } // End Human Input (Game Mode)
            } // End Mode Handling
        } // End event loop


        // --- Check Game Over Conditions (Only in Game Mode) ---
        if (currentMode == AppMode::GAME && !gameOver) {
             winner = gameState.checkWinner();
            if (winner != Player::NONE) { gameOver = true; winReason = (winner == Player::PLAYER1 ? "P1(Blue)" : "P2(Red)") + std::string(" reached Den!"); if (!quietMode) std::cout << winReason << std::endl; }
            else if (gameState.getCurrentPlayer() == humanPlayer) { if (gameState.getAllLegalMoves(humanPlayer).empty()) { gameOver = true; winner = aiPlayer; winReason = "Human(Blue) no legal moves!"; if (!quietMode) std::cout << winReason << std::endl; } }
            else if (gameState.getCurrentPlayer() == aiPlayer) { if (gameState.getAllLegalMoves(aiPlayer).empty()) { gameOver = true; winner = humanPlayer; winReason = "AI(Red) no legal moves!"; if (!quietMode) std::cout << winReason << std::endl; } }
        }


        // --- Drawing ---
        window.clear(sf::Color(40, 40, 50)); // Use background color consistent with theme
        graphics.drawBoard(window, gameState, currentMode, // Pass mode
                           setupPlayer, selectedSetupPiece, // For setup UI
                           gameOver, // For game UI/state
                           selectedPieceLegalMoves, pieceSelected ? selectedMove.fromRow : -1, // Highlights
                           pieceSelected ? selectedMove.fromCol : -1, lastAiMove);             // Highlights

        // Draw Quit Confirmation Overlay
        if (confirmingQuit) {
            sf::Font font; if (!font.loadFromFile("assets/arial.ttf")) { std::cerr << "Error loading font!" << std::endl; }
            else { sf::Text text("Quit game (y/n)?", font, 30); text.setFillColor(sf::Color(240, 240, 240)); text.setStyle(sf::Text::Bold); sf::FloatRect r = text.getLocalBounds(); text.setOrigin(r.left+r.width/2.f, r.top+r.height/2.f); text.setPosition(window.getSize().x/2.f, window.getSize().y/2.f); sf::RectangleShape bg(sf::Vector2f(r.width+60, r.height+40)); bg.setFillColor(sf::Color(50,50,60,235)); bg.setOutlineColor(sf::Color::White); bg.setOutlineThickness(2.f); bg.setOrigin(bg.getSize()/2.f); bg.setPosition(window.getSize().x/2.f, window.getSize().y/2.f); window.draw(bg); window.draw(text); }
        }
        // Draw Game Over Message (Only in Game Mode)
        else if (currentMode == AppMode::GAME && gameOver) {
            sf::Font font; if (!font.loadFromFile("assets/arial.ttf")) { std::cerr << "Error loading font!" << std::endl; }
            else { sf::Text text; text.setFont(font); text.setCharacterSize(40); text.setFillColor(sf::Color(220,220,230)); text.setStyle(sf::Text::Bold); std::string winnerStr = "Winner: " + std::string((winner == Player::PLAYER1) ? "P1(Blue)" : "P2(Red)"); text.setString("Game Over!\n" + winnerStr + "\n" + winReason + "\n\nClick to Exit"); sf::FloatRect r = text.getLocalBounds(); text.setOrigin(r.left+r.width/2.f, r.top+r.height/2.f); text.setPosition(window.getSize().x/2.f, window.getSize().y/2.f); sf::RectangleShape bg(sf::Vector2f(r.width+40, r.height+40)); bg.setFillColor(sf::Color(30,30,40,230)); bg.setOrigin(bg.getSize()/2.f); bg.setPosition(window.getSize().x/2.f, window.getSize().y/2.f); window.draw(bg); window.draw(text); }
        }
        window.display();


        // --- AI Turn Logic (Only in Game Mode) ---
        if (currentMode == AppMode::GAME && !gameOver && !confirmingQuit &&
           ( (gameState.getCurrentPlayer() == aiPlayer && !waitingForGo) || forceAiMove ) ) {

            if (forceAiMove && gameState.getCurrentPlayer() != aiPlayer) { gameState.setCurrentPlayer(aiPlayer); gameState.recalculateHash(); }

            std::vector<Move> aiLegalMovesCheck = gameState.getAllLegalMoves(aiPlayer);
            if (aiLegalMovesCheck.empty()) { if (!gameOver) { gameOver = true; winner = humanPlayer; winReason = "AI(Red) no legal moves!"; if (!quietMode) std::cout << winReason << std::endl; } }
            else {
                // Check Book
                Move bookMove = {-1,-1,-1,-1};
                if (bookAvailable && Book::isLoaded()) bookMove = Book::findBookMove(moveHistorySequence);
                bool playedBookMove = false;
                if (bookMove.fromRow != -1) {
                    if (aiMadeFirstMove) { // Rotate if AI started
                        const int MR = BOARD_ROWS - 1; const int MC = BOARD_COLS - 1;
                        bookMove = {MR - bookMove.fromRow, MC - bookMove.fromCol, MR - bookMove.toRow, MC - bookMove.toCol};
                    }
                    bool legal = false; for(const auto& m : aiLegalMovesCheck) if (m == bookMove) { legal = true; break; }
                    if (legal) {
                        if (!quietMode) std::cout << "AI plays book move: " << Book::moveToAlgebraic(bookMove) << std::endl;
                        gameState.applyMove(bookMove); lastAiMove = bookMove;
                        moveHistorySequence.push_back(bookMove); // Add book move
                        gameState.switchPlayer(); history.push_back(gameState); redoHistory.clear();
                        waitingForGo = false; playedBookMove = true;
                    } else {
                        if (!quietMode) std::cerr << "Warning: Book move " << Book::moveToAlgebraic(bookMove) << " illegal!" << std::endl;
                    }
                }
                // Search if no book move
                if (!playedBookMove) {
                    auto start = std::chrono::high_resolution_clock::now();
                    AIMoveInfo aiResult = AI::getBestMove(gameState, searchDepth, debugMode, quietMode);
                    auto stop = std::chrono::high_resolution_clock::now();
                    auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(stop - start);
                    if (aiResult.bestMove.fromRow != -1) {
                        gameState.applyMove(aiResult.bestMove); lastAiMove = aiResult.bestMove;
                        moveHistorySequence.push_back(aiResult.bestMove); // Add search move
                        if (!quietMode) { // Print stats
                            double durS = duration.count()/1000.0; double nps = (durS > 0.0001) ? (aiResult.nodesSearched/durS) : 0.0;
                            std::cout << "AI time: " << duration.count() << "ms | Nodes: " << aiResult.nodesSearched << " | " << std::fixed << std::setprecision(0) << nps << " N/s";
                            #ifdef USE_TRANSPOSITION_TABLE
                            std::cout << " | TT Util: " << std::fixed << std::setprecision(1) << aiResult.ttUtilizationPercent << "%";
                            #endif
                            std::cout << std::resetiosflags(std::ios::fixed) << std::endl;
                        }
                        gameState.switchPlayer(); history.push_back(gameState); redoHistory.clear(); waitingForGo = false;
                    } else { if (!quietMode) std::cerr << "Error: AI failed to return valid move!" << std::endl; waitingForGo = false; }
                } // End if (!playedBookMove)
            } // End else (AI has moves)
        } // End AI Turn
    } // End game loop

    if (!quietMode) std::cout << "Exiting game." << std::endl;
    return 0;
}


// --- Save Game Implementation ---
bool saveGame(const std::vector<GameState>& history, const std::string& filename) {
    std::ofstream outFile(filename, std::ios::binary | std::ios::trunc);
    if (!outFile.is_open()) { std::cerr << "Error opening file for saving: " << filename << std::endl; return false; }
    size_t historySize = history.size();
    outFile.write(reinterpret_cast<const char*>(&historySize), sizeof(historySize));
    for (const auto& state : history) {
        Player player = state.getCurrentPlayer(); outFile.write(reinterpret_cast<const char*>(&player), sizeof(player));
        uint64_t hashKey = state.getHashKey(); outFile.write(reinterpret_cast<const char*>(&hashKey), sizeof(hashKey));
        for (int r = 0; r < BOARD_ROWS; ++r) for (int c = 0; c < BOARD_COLS; ++c) {
            Piece piece = state.getPiece(r, c);
            outFile.write(reinterpret_cast<const char*>(&piece.type), sizeof(piece.type));
            outFile.write(reinterpret_cast<const char*>(&piece.owner), sizeof(piece.owner));
            outFile.write(reinterpret_cast<const char*>(&piece.rank), sizeof(piece.rank));
            outFile.write(reinterpret_cast<const char*>(&piece.weakened), sizeof(piece.weakened));
        }
        if (outFile.fail()) { std::cerr << "Error writing game state during history save: " << filename << std::endl; outFile.close(); return false; }
    }
    outFile.close(); return !outFile.fail();
}


// --- Load Game Implementation ---
bool loadGame(GameState& currentGameState, const std::string& filename, std::vector<GameState>& history) {
    std::ifstream inFile(filename, std::ios::binary);
    if (!inFile.is_open()) { std::cerr << "Error opening file for loading: " << filename << std::endl; return false; }
    size_t historySize = 0; inFile.read(reinterpret_cast<char*>(&historySize), sizeof(historySize));
    if (inFile.fail() || historySize == 0) { std::cerr << "Error reading history size or invalid size (0)." << std::endl; inFile.close(); return false; }
    std::vector<GameState> loadedHistory; loadedHistory.reserve(historySize);
    for (size_t i = 0; i < historySize; ++i) {
        Player loadedPlayer; uint64_t loadedHashKey; std::vector<std::vector<Piece>> loadedBoard(BOARD_ROWS, std::vector<Piece>(BOARD_COLS));
        inFile.read(reinterpret_cast<char*>(&loadedPlayer), sizeof(loadedPlayer));
        if (inFile.fail()) { std::cerr << "Error reading player data state " << i << "." << std::endl; inFile.close(); return false; }
        inFile.read(reinterpret_cast<char*>(&loadedHashKey), sizeof(loadedHashKey));
        if (inFile.fail()) { std::cerr << "Error reading hash key data state " << i << "." << std::endl; inFile.close(); return false; }
        for (int r = 0; r < BOARD_ROWS; ++r) for (int c = 0; c < BOARD_COLS; ++c) {
            Piece loadedPiece;
            inFile.read(reinterpret_cast<char*>(&loadedPiece.type), sizeof(loadedPiece.type));
            inFile.read(reinterpret_cast<char*>(&loadedPiece.owner), sizeof(loadedPiece.owner));
            inFile.read(reinterpret_cast<char*>(&loadedPiece.rank), sizeof(loadedPiece.rank));
            inFile.read(reinterpret_cast<char*>(&loadedPiece.weakened), sizeof(loadedPiece.weakened));
            if (inFile.fail()) { std::cerr << "Error reading board data (" << r << "," << c << ") state " << i << "." << std::endl; inFile.close(); return false; }
            loadedBoard[r][c] = loadedPiece;
        }
        GameState tempState; tempState.setBoard(loadedBoard); tempState.setCurrentPlayer(loadedPlayer); tempState.setHashKey(loadedHashKey);
        loadedHistory.push_back(tempState);
    }
    inFile.peek(); if (!inFile.eof()) { std::cerr << "Warning: Save file contains extra data." << std::endl; }
    inFile.close();
    history = loadedHistory; // Replace main history
    currentGameState = history.back(); // Set current state
    return true;
}


