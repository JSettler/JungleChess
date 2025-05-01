#include <SFML/Graphics.hpp>
#include "GameState.h"
#include "Graphics.h"
#include "AI.h" // Include AI.h
#include "Common.h" // Include Common.h for AppMode and Piece struct
#include <iostream>
#include <vector>
#include <string>
#include <chrono>
#include <cstring> // Required for strcmp
#include <fstream>   // Required for file operations
#include <sstream>   // Required for std::to_string in window title
#include <stdexcept> // For std::stoi exceptions
#include <limits>    // For numeric_limits
#include <iomanip>   // For std::fixed, std::setprecision

// --- Forward Declarations for Save/Load ---
bool saveGame(const std::vector<GameState>& history, const std::string& filename);
bool loadGame(GameState& currentGameState, const std::string& filename, std::vector<GameState>& history); // Needs history to populate it


int main(int argc, char* argv[]) {

    // --- Argument Parsing ---
    bool showHelp = false;
    bool debugMode = false;
    bool quietMode = false; // Default to normal output
    bool unknownArgumentFound = false; // Flag for invalid args
    std::string unknownArg = "";        // Store the first unknown arg
    int searchDepth = 6; // Default depth
    AppMode currentMode = AppMode::GAME; // Default to game mode

    const char* progName = (argc > 0 && argv[0] != nullptr) ? argv[0] : "jungle_chess";
    if (progName == nullptr) progName = "jungle_chess";
    std::string usageSyntax = "Usage: " + std::string(progName) + " [--depth N] [--setup] [-n | -d | -h | --help | -?]";


    for (int i = 1; i < argc; ++i) {
        if (strcmp(argv[i], "-h") == 0 || strcmp(argv[i], "--help") == 0 ||
            strcmp(argv[i], "-?") == 0)
        {
            showHelp = true;
            // Don't break immediately, let other flags potentially set modes
        } else if (strcmp(argv[i], "-d") == 0) {
            debugMode = true;
            quietMode = false; // Debug overrides quiet
        } else if (strcmp(argv[i], "-n") == 0 && !debugMode) { // Only set quiet if debug isn't set
            quietMode = true;
        } else if (strcmp(argv[i], "--setup") == 0) {
            currentMode = AppMode::SETUP;
            if (!quietMode) std::cout << "Starting in Setup Mode." << std::endl;
        } else if (strcmp(argv[i], "--depth") == 0) {
            if (i + 1 < argc) { // Check if a value follows the flag
                try {
                    int depthValue = std::stoi(argv[i + 1]);
                    // Add a slightly higher reasonable upper limit for depth
                    if (depthValue > 0 && depthValue < 20) {
                         searchDepth = depthValue;
                         i++; // Consume the number argument
                         // Announce depth change only if not quiet
                         if (!quietMode) std::cout << "Search depth set to " << searchDepth << " plies." << std::endl;
                    } else {
                         std::cerr << "Error: Invalid search depth value '" << argv[i + 1] << "'. Must be positive and reasonable (< 20)." << std::endl;
                         return 1;
                    }
                } catch (const std::invalid_argument& e) {
                    std::cerr << "Error: Invalid number format for --depth: '" << argv[i + 1] << "'" << std::endl;
                    return 1;
                } catch (const std::out_of_range& e) {
                     std::cerr << "Error: Search depth value out of range: '" << argv[i + 1] << "'" << std::endl;
                     return 1;
                }
            } else {
                std::cerr << "Error: Missing value after --depth flag." << std::endl;
                std::cerr << usageSyntax << std::endl;
                return 1;
            }
        } else { // Handle Unknown Argument
            if (!unknownArgumentFound) { // Store only the first unknown one
                unknownArgumentFound = true;
                unknownArg = argv[i];
            }
        }
    }

    // --- Handle Help Flag OR Unknown Argument ---
    if (showHelp) {
        // Print full help text
        std::cout << usageSyntax << "\n\n"; // Print basic syntax first
        std::cout << "Options:\n"; // Changed Modes to Options
        std::cout << "  --depth N : Set AI search depth to N plies (integer, default: 6).\n"; // Show default
        std::cout << "  --setup   : Start in board setup mode instead of a new game.\n"; // Added setup flag
        std::cout << "  -n        : Quiet mode (minimal console output).\n";
        std::cout << "  -d        : Debug mode (verbose AI move evaluation output).\n";
        std::cout << "  -h, --help, -? : Show this help message and exit.\n\n";
        std::cout << "In-Game Keys (Game Mode):\n"; // Clarified section
        std::cout << "  <Backspace>       : Take back last half-move (Undo).\n";
        std::cout << "  <Shift+Backspace> : Redo last undone half-move.\n";
        std::cout << "  S                 : Save current game state to dsq-game.sav.\n";
        std::cout << "  L                 : Load game state from dsq-game.sav (clears undo/redo history).\n";
        std::cout << "  P                 : Cycle piece display emphasis (Letters <-> Numbers).\n";
        std::cout << "  G                 : Make AI move (if it's AI's turn or start of game).\n";
        std::cout << "  R                 : Rotate board view 180 degrees.\n";
        std::cout << "  <Escape>          : Quit game.\n\n"; // Updated Escape description
        std::cout << "In-Game Keys (Setup Mode):\n"; // Added section
        std::cout << "  Left Click  : Place selected piece / Select UI button.\n";
        std::cout << "  Right Click : Remove piece from board square.\n";
        std::cout << "  1-8         : Select piece type (Rat=1 to Elephant=8).\n";
        std::cout << "  S           : Switch player side for piece placement.\n";
        std::cout << "  P           : Cycle piece display emphasis.\n";
        std::cout << "  F           : Finish setup and start game.\n";
        std::cout << "  R           : Rotate board view 180 degrees.\n";
        std::cout << "  <Escape>    : Quit game.\n"; // Updated Escape description
        return 0; // Exit after printing help
    }
    else if (unknownArgumentFound) {
        // Handle Unknown Argument Error
        std::cerr << "Error: Unknown parameter '" << unknownArg << "'\n"; // Use cerr for errors
        std::cerr << usageSyntax << std::endl;
        return 1; // Exit with an error code
    }


    // Announce operational mode (if not quiet and no errors/help shown)
    if (debugMode) {
        std::cout << "Debug mode enabled." << std::endl;
    } else if (quietMode) {
        std::cout << "Quiet mode enabled." << std::endl;
    }


    // --- Initialization ---
    std::string windowTitle = "Jungle Chess v1.0  [depth = " + std::to_string(searchDepth) + "]"; // Updated title format
    sf::RenderWindow window(sf::VideoMode(800, 700), windowTitle);
    window.setFramerateLimit(60);

    GameState gameState; // Current active game state
    Graphics graphics;   // Graphics object
    graphics.loadAssets();
    Player humanPlayer = Player::PLAYER1; // Default human player
    Player aiPlayer = Player::PLAYER2;    // Default AI player

    // Game State History
    std::vector<GameState> history;
    history.push_back(gameState); // Store the initial state
    std::vector<GameState> redoHistory; // For redo functionality

    // UI State
    Move selectedMove = {-1,-1,-1,-1}; bool pieceSelected = false;
    std::vector<Move> selectedPieceLegalMoves; Move lastAiMove = {-1,-1,-1,-1};
    bool gameOver = false; Player winner = Player::NONE; std::string winReason = "";

    // Setup Mode State
    Player setupPlayer = Player::PLAYER1;
    PieceType selectedSetupPiece = PieceType::EMPTY;

    // Quit Confirmation State
    bool confirmingQuit = false;

    // Force AI Move Flag
    bool forceAiMove = false;

    // Wait for Go command flag
    bool waitingForGo = false;

    // Save file name
    const std::string saveFilename = "dsq-game.sav";

    // --- Main Game Loop ---
    while (window.isOpen()) {

        // Reset force flag at start of loop iteration
        forceAiMove = false;

        // --- Event Handling ---
        sf::Event event;
        while (window.pollEvent(event)) {
            if (event.type == sf::Event::Closed) window.close();

            // --- Handle Quit Confirmation First ---
            if (confirmingQuit) {
                if (event.type == sf::Event::KeyPressed) {
                    if (event.key.code == sf::Keyboard::Y) { window.close(); }
                    else { confirmingQuit = false; } // Any other key cancels
                } else if (event.type == sf::Event::MouseButtonPressed) { confirmingQuit = false; }
                continue; // Skip all other event processing
            }

            // --- Normal Event Processing ---
            if (gameOver && event.type == sf::Event::MouseButtonPressed && event.mouseButton.button == sf::Mouse::Left) { window.close(); continue; }

            // --- Global Key Presses ---
            if (event.type == sf::Event::KeyPressed) {
                if (event.key.code == sf::Keyboard::Escape) { confirmingQuit = true; continue; }
                if (event.key.code == sf::Keyboard::P) { graphics.togglePieceDisplay(); continue; }
                if (event.key.code == sf::Keyboard::R) { // Handle 'R' key for rotation
                    graphics.toggleBoardFlip();
                    pieceSelected = false; selectedMove = {-1,-1,-1,-1}; selectedPieceLegalMoves.clear(); // Deselect on flip
                    continue;
                }
            }

            // --- Mode-Specific Event Handling ---
            if (currentMode == AppMode::SETUP) {
                // --- SETUP MODE EVENTS ---
                if (event.type == sf::Event::KeyPressed) {
                    if (event.key.code >= sf::Keyboard::Num1 && event.key.code <= sf::Keyboard::Num8) { selectedSetupPiece = static_cast<PieceType>(event.key.code - sf::Keyboard::Num1 + 1); if (!quietMode) std::cout << "Setup: Selected piece type " << static_cast<int>(selectedSetupPiece) << std::endl; }
                    else if (event.key.code == sf::Keyboard::S) { setupPlayer = (setupPlayer == Player::PLAYER1) ? Player::PLAYER2 : Player::PLAYER1; if (!quietMode) std::cout << "Setup: Switched to placing pieces for Player " << static_cast<int>(setupPlayer) << std::endl; selectedSetupPiece = PieceType::EMPTY; }
                    else if (event.key.code == sf::Keyboard::F) {
                        if (gameState.validateSetup()) {
                            currentMode = AppMode::GAME; gameState.setCurrentPlayer(Player::PLAYER1); // P1 always starts after setup
                            gameState.recalculateHash(); history.clear(); redoHistory.clear(); history.push_back(gameState); // CRITICAL: Reset history with current setup
                            window.setTitle("JungleChess v1.0  [depth = " + std::to_string(searchDepth) + "]"); // Update title
                            if (!quietMode) std::cout << "Setup finished. Player 1 to move." << std::endl;
                            waitingForGo = false; // Ensure not waiting after setup
                        } else { if (!quietMode) std::cerr << "Setup Error: Invalid board position." << std::endl; }
                    }
                } else if (event.type == sf::Event::MouseButtonPressed) {
                    sf::Vector2i mousePos = sf::Mouse::getPosition(window);
                    if (graphics.isClickOnClearButton(mousePos)) { gameState.clearBoard(); setupPlayer = Player::PLAYER1; selectedSetupPiece = PieceType::EMPTY; if (!quietMode) std::cout << "Setup: Board cleared." << std::endl; }
                    else if (graphics.isClickOnSideButton(mousePos)) { setupPlayer = (setupPlayer == Player::PLAYER1) ? Player::PLAYER2 : Player::PLAYER1; if (!quietMode) std::cout << "Setup: Switched to placing pieces for Player " << static_cast<int>(setupPlayer) << std::endl; selectedSetupPiece = PieceType::EMPTY; }
                    else if (graphics.isClickOnFinishButton(mousePos)) {
                         if (gameState.validateSetup()) {
                            currentMode = AppMode::GAME; gameState.setCurrentPlayer(Player::PLAYER1); gameState.recalculateHash();
                            history.clear(); redoHistory.clear(); history.push_back(gameState); // CRITICAL: Reset history with current setup
                            window.setTitle("JungleChess v1.0  [depth = " + std::to_string(searchDepth) + "]"); // Update title
                            if (!quietMode) std::cout << "Setup finished. Player 1 to move." << std::endl;
                            waitingForGo = false; // Ensure not waiting after setup
                        } else { if (!quietMode) std::cerr << "Setup Error: Invalid board position." << std::endl; }
                    } else {
                        PieceType clickedPieceType = graphics.getClickedSetupPieceButton(mousePos);
                        if (clickedPieceType != PieceType::EMPTY) { selectedSetupPiece = clickedPieceType; if (!quietMode) std::cout << "Setup: Selected piece type " << static_cast<int>(selectedSetupPiece) << std::endl; }
                        else {
                            sf::Vector2i boardPos = graphics.getClickedSquare(mousePos);
                            if (gameState.isValidPosition(boardPos.y, boardPos.x)) {
                                if (event.mouseButton.button == sf::Mouse::Left) { if (selectedSetupPiece != PieceType::EMPTY) { if (!gameState.setPieceAt(boardPos.y, boardPos.x, selectedSetupPiece, setupPlayer)) { if (!quietMode) std::cerr << "Setup Error: Cannot place piece there." << std::endl; } } }
                                else if (event.mouseButton.button == sf::Mouse::Right) { gameState.clearSquare(boardPos.y, boardPos.x); }
                            }
                        }
                    }
                } // End MouseButton Pressed (Setup)

            } else { // currentMode == AppMode::GAME
                // --- GAME MODE EVENTS ---
                // Handle Takeback (Undo) - Undoes ONE half-move
                if (!gameOver && event.type == sf::Event::KeyPressed && event.key.code == sf::Keyboard::BackSpace && !event.key.shift) {
                    if (history.size() >= 2) { // Need at least one previous state
                        redoHistory.push_back(history.back()); history.pop_back();
                        gameState = history.back(); // Restore previous state
                        waitingForGo = (gameState.getCurrentPlayer() == aiPlayer); // Check if AI's turn now
                        pieceSelected = false; selectedMove = {-1,-1,-1,-1}; selectedPieceLegalMoves.clear(); lastAiMove = {-1,-1,-1,-1};
                        if (!quietMode) std::cout << "<<< Undo! " << (waitingForGo ? "Press 'G' for AI move." : "") << " >>>" << std::endl;
                    } else { // history.size() is 1
                        if (!quietMode) { std::cout << "Nothing further to undo." << std::endl; }
                    }
                }
                // Handle Redo - Redoes ONE half-move
                else if (!gameOver && event.type == sf::Event::KeyPressed && event.key.code == sf::Keyboard::BackSpace && event.key.shift) {
                    if (!redoHistory.empty()) {
                        history.push_back(redoHistory.back()); redoHistory.pop_back();
                        gameState = history.back(); // Update current state
                        waitingForGo = (gameState.getCurrentPlayer() == aiPlayer); // Check if AI's turn now
                        pieceSelected = false; selectedMove = {-1,-1,-1,-1}; selectedPieceLegalMoves.clear(); lastAiMove = {-1,-1,-1,-1};
                        if (!quietMode) std::cout << ">>> Redo! " << (waitingForGo ? "Press 'G' for AI move." : "") << " >>>" << std::endl;
                    } else if (!quietMode) { std::cout << "Nothing to redo." << std::endl; }
                }
                // Handle Save (Only in Game Mode)
                else if (!gameOver && event.type == sf::Event::KeyPressed && event.key.code == sf::Keyboard::S) { if (saveGame(history, saveFilename)) { if (!quietMode) std::cout << "Game state saved to " << saveFilename << std::endl; } }
                // Handle Load (Only in Game Mode)
                else if (!gameOver && event.type == sf::Event::KeyPressed && event.key.code == sf::Keyboard::L) {
                    if (loadGame(gameState, saveFilename, history)) {
                         redoHistory.clear();
                         waitingForGo = (gameState.getCurrentPlayer() == aiPlayer); // Check turn after load
                         pieceSelected = false; selectedMove = {-1,-1,-1,-1}; selectedPieceLegalMoves.clear(); lastAiMove = {-1,-1,-1,-1};
                         if (!quietMode) std::cout << "Game state loaded from " << saveFilename << (waitingForGo ? " Press 'G' for AI move." : "") << std::endl;
                    }
                }
                // Handle 'G' key (AI Go / Force AI Move)
                else if (!gameOver && event.type == sf::Event::KeyPressed && event.key.code == sf::Keyboard::G) {
                    // Case 1: Start of game (P1's turn, history size 1) -> Make AI move first
                    if (gameState.getCurrentPlayer() == Player::PLAYER1 && history.size() == 1) {
                        if (!quietMode) std::cout << "Player chose AI (Red) to move first." << std::endl;
                        gameState.setCurrentPlayer(aiPlayer);
                        gameState.recalculateHash();
                        forceAiMove = true;
                        waitingForGo = false; // AI will move immediately
                    }
                    // Case 2: AI's turn AND we are waiting -> Force AI move
                    else if (gameState.getCurrentPlayer() == aiPlayer && waitingForGo) {
                         if (!quietMode) std::cout << "'G' pressed - Proceeding with AI move." << std::endl;
                         forceAiMove = true;
                         waitingForGo = false; // No longer waiting
                    }
                    // Case 3: Not applicable
                    else if (!quietMode) {
                        if (gameState.getCurrentPlayer() == aiPlayer && !waitingForGo) {
                             std::cout << "'G' pressed, but AI is already set to move." << std::endl;
                        } else { // Must be P1's turn but not the start
                             std::cout << "'G' key only works on the very first turn or when it is AI's turn after an Undo/Redo." << std::endl;
                        }
                    }
                }
                // Handle Human Player Input (Game Mode)
                else if (!gameOver && gameState.getCurrentPlayer() == humanPlayer && event.type == sf::Event::MouseButtonPressed && event.mouseButton.button == sf::Mouse::Left) {
                    sf::Vector2i mousePos = sf::Mouse::getPosition(window);
                    sf::Vector2i boardPos = graphics.getClickedSquare(mousePos);
                    if (gameState.isValidPosition(boardPos.y, boardPos.x)) {
                        if (!pieceSelected) { // Select
                            Piece clickedPiece = gameState.getPiece(boardPos.y, boardPos.x);
                            if (clickedPiece.owner == humanPlayer) { selectedMove.fromRow = boardPos.y; selectedMove.fromCol = boardPos.x; selectedMove.toRow = -1; selectedMove.toCol = -1; pieceSelected = true; lastAiMove = {-1,-1,-1,-1}; selectedPieceLegalMoves = gameState.getLegalMovesForPiece(selectedMove.fromRow, selectedMove.fromCol); }
                        } else { // Move/Deselect
                            if (boardPos.y == selectedMove.fromRow && boardPos.x == selectedMove.fromCol) { pieceSelected = false; selectedMove = {-1, -1, -1, -1}; selectedPieceLegalMoves.clear(); }
                            else { // Attempt move
                                bool isValidTarget = false; Move attemptedMove = {selectedMove.fromRow, selectedMove.fromCol, boardPos.y, boardPos.x};
                                for(const auto& legalMove : selectedPieceLegalMoves) if (legalMove.toRow == attemptedMove.toRow && legalMove.toCol == attemptedMove.toCol) { isValidTarget = true; break; }
                                if (isValidTarget) { gameState.applyMove(attemptedMove); gameState.switchPlayer(); history.push_back(gameState); redoHistory.clear(); waitingForGo = false; pieceSelected = false; selectedMove = {-1, -1, -1, -1}; selectedPieceLegalMoves.clear(); } // Clear waiting flag
                                else { if (!quietMode) std::cout << "Invalid move target." << std::endl; pieceSelected = false; selectedMove = {-1, -1, -1, -1}; selectedPieceLegalMoves.clear(); }
                            }
                        }
                    } else if (pieceSelected) { pieceSelected = false; selectedMove = {-1, -1, -1, -1}; selectedPieceLegalMoves.clear(); }
                } // End Human Input (Game Mode)
            } // End GAME MODE EVENTS
        } // End event loop



        // --- AI Turn Logic (Only in Game Mode and if not confirming quit) ---
        // Execute if it's AI's turn AND we are NOT waiting for 'G', OR if force flag is set
        if (currentMode == AppMode::GAME && !gameOver && !confirmingQuit &&
           ( (gameState.getCurrentPlayer() == aiPlayer && !waitingForGo) || forceAiMove ) ) {

            // Ensure player is AI if forced (handles the 'G' at start case)
            if (forceAiMove && gameState.getCurrentPlayer() != aiPlayer) {
                 gameState.setCurrentPlayer(aiPlayer);
                 gameState.recalculateHash(); // Recalculate hash again just in case
            }

            std::vector<Move> aiLegalMovesCheck = gameState.getAllLegalMoves(aiPlayer);
            if (aiLegalMovesCheck.empty()) { gameOver = true; winner = humanPlayer; winReason = "AI (Red) has no legal moves!"; if (!quietMode) std::cout << winReason << std::endl; }
            else {
                auto startTime = std::chrono::high_resolution_clock::now();
                AIMoveInfo aiResult = AI::getBestMove(gameState, searchDepth, debugMode, quietMode);
                auto stopTime = std::chrono::high_resolution_clock::now();
                auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(stopTime - startTime);

                // Check if AI returned a valid move before applying
                if (aiResult.bestMove.fromRow != -1) { // Check if move is valid
                    gameState.applyMove(aiResult.bestMove);
                    lastAiMove = aiResult.bestMove; // Store move for highlight

                    // Print stats (Score is now handled inside AI::getBestMove log)
                    if (!quietMode) {
                        double durationSeconds = duration.count() / 1000.0;
                        double nodesPerSecond = (durationSeconds > 0.0001) ? (static_cast<double>(aiResult.nodesSearched) / durationSeconds) : 0.0; // Avoid division by zero

                        std::cout << "AI calculation time: " << duration.count() << " ms | "
                                  << "Nodes: " << aiResult.nodesSearched << " | "
                                  << std::fixed << std::setprecision(0) << nodesPerSecond << " N/s";
                        #ifdef USE_TRANSPOSITION_TABLE
                        std::cout << " | " << std::fixed << std::setprecision(1) << "TT Util: " << aiResult.ttUtilizationPercent << "%";
                        #endif
                        std::cout << std::resetiosflags(std::ios::fixed) << std::endl;
                    }

                    gameState.switchPlayer(); // Switch back to Player 1
                    history.push_back(gameState);
                    redoHistory.clear(); // Clear redo on new move
                    waitingForGo = false; // AI has moved, no longer waiting
                } else {
                     if (!quietMode) std::cerr << "Error: AI failed to return a valid move!" << std::endl;
                     // If AI fails, maybe just let human try again? Don't switch player.
                     // gameState.switchPlayer(); // Maybe don't switch?
                     waitingForGo = false; // No longer waiting, but AI didn't move.
                }
            }
        } // End AI Turn


        // --- Check Game Over Conditions (Only in Game Mode) ---
        if (currentMode == AppMode::GAME && !gameOver) {
            winner = gameState.checkWinner(); // Check Den
            if (winner != Player::NONE) { gameOver = true; winReason = (winner == Player::PLAYER1 ? "Player 1 (Blue)" : "Player 2 (Red)") + std::string(" reached the Den!"); if (!quietMode) std::cout << winReason << std::endl; }
            else if (gameState.getCurrentPlayer() == humanPlayer) { // Check Human No Moves
                 if (gameState.getAllLegalMoves(humanPlayer).empty()) { gameOver = true; winner = aiPlayer; winReason = "Human (Blue) has no legal moves!"; if (!quietMode) std::cout << winReason << std::endl; }
            }
        } // End Game Over Check


        // --- Drawing ---
        window.clear(sf::Color::White);
        graphics.drawBoard(window, gameState, currentMode, setupPlayer, selectedSetupPiece,
                           selectedPieceLegalMoves, pieceSelected ? selectedMove.fromRow : -1,
                           pieceSelected ? selectedMove.fromCol : -1, lastAiMove);

        // Draw Quit Confirmation Overlay (if active)
        if (confirmingQuit) {
            sf::Font font;
            if (!font.loadFromFile("assets/arial.ttf")) { std::cerr << "Error loading font for quit confirm!" << std::endl; }
            else {
                sf::Text confirmText; confirmText.setFont(font); confirmText.setCharacterSize(30);
                confirmText.setFillColor(sf::Color(240, 240, 240)); confirmText.setStyle(sf::Text::Bold);
                confirmText.setString("Quit game (y/n)?");
                sf::FloatRect textRect = confirmText.getLocalBounds();
                confirmText.setOrigin(textRect.left + textRect.width / 2.0f, textRect.top + textRect.height / 2.0f);
                confirmText.setPosition(window.getSize().x / 2.0f, window.getSize().y / 2.0f);
                sf::RectangleShape background(sf::Vector2f(textRect.width + 60, textRect.height + 40));
                background.setFillColor(sf::Color(50, 50, 60, 235)); background.setOutlineColor(sf::Color::White);
                background.setOutlineThickness(2.0f); background.setOrigin(background.getSize().x / 2.0f, background.getSize().y / 2.0f);
                background.setPosition(window.getSize().x / 2.0f, window.getSize().y / 2.0f);
                window.draw(background); window.draw(confirmText);
            }
        }
        // Draw Game Over Message (Only if game is over AND not confirming quit)
        else if (gameOver) {
            sf::Font font;
            if (!font.loadFromFile("assets/arial.ttf")) { std::cerr << "Error loading font for game over!" << std::endl; }
            else {
                sf::Text gameOverText; gameOverText.setFont(font); gameOverText.setCharacterSize(40);
                gameOverText.setFillColor(sf::Color(220, 220, 230)); gameOverText.setStyle(sf::Text::Bold);
                std::string winnerStr = "Winner: " + std::string((winner == Player::PLAYER1) ? "Player 1 (Blue)" : "Player 2 (Red)");
                gameOverText.setString("Game Over!\n" + winnerStr + "\n" + winReason + "\n\nClick to Exit");
                sf::FloatRect textRect = gameOverText.getLocalBounds();
                gameOverText.setOrigin(textRect.left + textRect.width / 2.0f, textRect.top + textRect.height / 2.0f);
                gameOverText.setPosition(window.getSize().x / 2.0f, window.getSize().y / 2.0f);
                sf::RectangleShape background(sf::Vector2f(textRect.width + 40, textRect.height + 40));
                background.setFillColor(sf::Color(30, 30, 40, 230)); background.setOrigin(background.getSize().x / 2.0f, background.getSize().y / 2.0f);
                background.setPosition(window.getSize().x / 2.0f, window.getSize().y / 2.0f);
                window.draw(background); window.draw(gameOverText);
            }
        }
        window.display();

    } // End game loop

    if (!quietMode) std::cout << "Exiting game." << std::endl;
    return 0;
}


// --- Save Game Implementation (Saves History including weakened flag) ---
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


// --- Load Game Implementation (Loads History including weakened flag) ---
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


