#include <SFML/Graphics.hpp>
#include "GameState.h"
#include "Graphics.h"
#include "AI.h" // Include AI.h to access AI::SEARCH_DEPTH
#include "Common.h"
#include <iostream>
#include <vector>
#include <string>
#include <chrono>
#include <cstring> // Required for strcmp
#include <fstream>   // Required for file operations
#include <sstream>   // Required for std::to_string in window title
#include <stdexcept> // For std::stoi exceptions
#include <limits>    // For numeric_limits

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

    const char* progName = (argc > 0 && argv[0] != nullptr) ? argv[0] : "jungle_chess";
    if (progName == nullptr) progName = "jungle_chess";
    std::string usageSyntax = "Usage: " + std::string(progName) + " [--depth N] [-n | -d | -h | --help | -?]";


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
        std::cout << "  -n        : Quiet mode (minimal console output).\n";
        std::cout << "  -d        : Debug mode (verbose AI move evaluation output).\n";
        std::cout << "  -h, --help, -? : Show this help message and exit.\n\n";
        std::cout << "In-Game Keys:\n";
        std::cout << "  <Backspace>       : Take back last full move (Undo).\n";
        std::cout << "  <Shift+Backspace> : Redo last undone move.\n";
        std::cout << "  S                 : Save current game state to dsq-game.sav.\n";
        std::cout << "  L                 : Load game state from dsq-game.sav (clears undo/redo history).\n";
        std::cout << "  <Escape>          : Quit the game immediately.\n";
        // Removed the explicit depth mention here as it's in Options now
        // std::cout << "Other Info:\n";
        // std::cout << "  Search depth is set at " << searchDepth << " plies.\n";
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
    std::string windowTitle = "Jungle Chess - AlphaBeta AI (Depth " + std::to_string(searchDepth) + " + Undo/Redo)"; // Use variable depth
    sf::RenderWindow window(sf::VideoMode(800, 700), windowTitle);
    window.setFramerateLimit(60);

    GameState gameState; // Current active game state
    Graphics graphics;
    graphics.loadAssets();
    Player humanPlayer = Player::PLAYER1;
    Player aiPlayer = Player::PLAYER2;

    // Game State History
    std::vector<GameState> history;
    history.push_back(gameState); // Store the initial state
    std::vector<GameState> redoHistory; // For redo functionality

    // UI State
    Move selectedMove = {-1,-1,-1,-1}; bool pieceSelected = false;
    std::vector<Move> selectedPieceLegalMoves; Move lastAiMove = {-1,-1,-1,-1};
    bool gameOver = false; Player winner = Player::NONE; std::string winReason = "";

    // Save file name
    const std::string saveFilename = "dsq-game.sav";

    // --- Main Game Loop ---
    while (window.isOpen()) {

        // --- Event Handling ---
        sf::Event event;
        while (window.pollEvent(event)) {
            if (event.type == sf::Event::Closed) window.close();

            // Handle Game Over Click
            if (gameOver && event.type == sf::Event::MouseButtonPressed && event.mouseButton.button == sf::Mouse::Left) {
                window.close();
            }
            // Handle Escape Key
            else if (event.type == sf::Event::KeyPressed && event.key.code == sf::Keyboard::Escape) {
                window.close(); // Quit immediately
            }
            // Handle Takeback (Undo)
            else if (!gameOver && event.type == sf::Event::KeyPressed && event.key.code == sf::Keyboard::BackSpace && !event.key.shift) { // Check shift is NOT pressed
                 if (history.size() >= 3) { // Undo full Human+AI move
                    redoHistory.push_back(history.back()); history.pop_back(); // Move AI state to redo
                    redoHistory.push_back(history.back()); history.pop_back(); // Move Human state to redo
                    gameState = history.back(); // Restore state before last Human move
                    pieceSelected = false; selectedMove = {-1,-1,-1,-1}; selectedPieceLegalMoves.clear(); lastAiMove = {-1,-1,-1,-1};
                    if (!quietMode) std::cout << "<<< Undo Full Move! >>>" << std::endl;
                 } else if (history.size() == 2) { // Undo first human move
                      redoHistory.push_back(history.back()); history.pop_back();
                      gameState = history.back(); // Restore initial state
                      pieceSelected = false; selectedMove = {-1,-1,-1,-1}; selectedPieceLegalMoves.clear(); lastAiMove = {-1,-1,-1,-1};
                      if (!quietMode) std::cout << "<<< Undo! Restored initial state. >>>" << std::endl;
                 } else if (!quietMode) { // history.size() <= 1
                     std::cout << "Nothing to undo." << std::endl;
                 }
            }
            // Handle Redo
            else if (!gameOver && event.type == sf::Event::KeyPressed && event.key.code == sf::Keyboard::BackSpace && event.key.shift) { // Check shift IS pressed
                if (!redoHistory.empty()) {
                    // Redo involves 2 states (Human move, then AI move) if available
                    history.push_back(redoHistory.back()); // Restore state after Human move
                    redoHistory.pop_back();
                    if (!redoHistory.empty()) { // Check if there's an AI move state to redo
                         history.push_back(redoHistory.back()); // Restore state after AI move
                         redoHistory.pop_back();
                    }
                    gameState = history.back(); // Update current state to the last one restored
                    pieceSelected = false; selectedMove = {-1,-1,-1,-1}; selectedPieceLegalMoves.clear(); lastAiMove = {-1,-1,-1,-1}; // Reset UI
                     if (!quietMode) std::cout << ">>> Redo Move! >>>" << std::endl;
                } else if (!quietMode) {
                    std::cout << "Nothing to redo." << std::endl;
                }
            }
            // Handle Save
            else if (!gameOver && event.type == sf::Event::KeyPressed && event.key.code == sf::Keyboard::S) {
                // Save only the current history, not the redo history
                if (saveGame(history, saveFilename)) {
                    if (!quietMode) std::cout << "Game state saved to " << saveFilename << std::endl;
                } else { /* Error printed in saveGame */ }
            }
            // Handle Load
            else if (!gameOver && event.type == sf::Event::KeyPressed && event.key.code == sf::Keyboard::L) {
                if (loadGame(gameState, saveFilename, history)) { // Pass history to be populated
                     redoHistory.clear(); // Clear redo history on load
                     pieceSelected = false; selectedMove = {-1,-1,-1,-1}; selectedPieceLegalMoves.clear(); lastAiMove = {-1,-1,-1,-1};
                     if (!quietMode) std::cout << "Game state loaded from " << saveFilename << std::endl;
                } else { /* Error printed in loadGame */ }
            }
            // Handle Human Player Input
            else if (!gameOver && gameState.getCurrentPlayer() == humanPlayer && event.type == sf::Event::MouseButtonPressed && event.mouseButton.button == sf::Mouse::Left) {
                sf::Vector2i mousePos = sf::Mouse::getPosition(window);
                sf::Vector2i boardPos = graphics.getClickedSquare(mousePos);
                if (gameState.isValidPosition(boardPos.y, boardPos.x)) {
                    if (!pieceSelected) { // Try to Select piece
                        Piece clickedPiece = gameState.getPiece(boardPos.y, boardPos.x);
                        if (clickedPiece.owner == humanPlayer) {
                            selectedMove.fromRow = boardPos.y; selectedMove.fromCol = boardPos.x; selectedMove.toRow = -1; selectedMove.toCol = -1;
                            pieceSelected = true; lastAiMove = {-1,-1,-1,-1}; // Clear AI highlight on selection
                            selectedPieceLegalMoves = gameState.getLegalMovesForPiece(selectedMove.fromRow, selectedMove.fromCol);
                        }
                    } else { // Piece already selected: Try to Move or Deselect
                        if (boardPos.y == selectedMove.fromRow && boardPos.x == selectedMove.fromCol) { // Clicked same piece: Deselect
                            pieceSelected = false; selectedMove = {-1, -1, -1, -1}; selectedPieceLegalMoves.clear();
                        } else { // Clicked different square: Attempt move
                            bool isValidTarget = false; Move attemptedMove = {selectedMove.fromRow, selectedMove.fromCol, boardPos.y, boardPos.x};
                            // Check if the target square is in the list of legal destinations
                            for(const auto& legalMove : selectedPieceLegalMoves) {
                                if (legalMove.toRow == attemptedMove.toRow && legalMove.toCol == attemptedMove.toCol) {
                                    isValidTarget = true; break;
                                }
                            }
                            if (isValidTarget) { // Valid move target found
                                gameState.applyMove(attemptedMove); gameState.switchPlayer();
                                history.push_back(gameState); // Store state after human move
                                redoHistory.clear(); // Clear redo history after new move
                                // Reset UI state
                                pieceSelected = false; selectedMove = {-1, -1, -1, -1}; selectedPieceLegalMoves.clear();
                            } else { // Invalid move target
                                if (!quietMode) std::cout << "Invalid move target." << std::endl;
                                // Deselect on invalid target click
                                pieceSelected = false; selectedMove = {-1, -1, -1, -1}; selectedPieceLegalMoves.clear();
                            }
                        }
                    }
                } else if (pieceSelected) { // Clicked outside board - deselect
                    pieceSelected = false; selectedMove = {-1, -1, -1, -1}; selectedPieceLegalMoves.clear();
                }
            } // End Human Input
        } // End event loop

        // --- AI Turn Logic ---
        if (!gameOver && gameState.getCurrentPlayer() == aiPlayer) {
            std::vector<Move> aiLegalMovesCheck = gameState.getAllLegalMoves(aiPlayer);
            if (aiLegalMovesCheck.empty()) {
                gameOver = true; winner = humanPlayer; winReason = "AI (Red) has no legal moves!";
                if (!quietMode) std::cout << winReason << std::endl;
            } else {
                auto startTime = std::chrono::high_resolution_clock::now();
                // Pass searchDepth variable
                Move aiMove = AI::getBestMove(gameState, searchDepth, debugMode, quietMode);
                auto stopTime = std::chrono::high_resolution_clock::now();
                auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(stopTime - startTime);

                gameState.applyMove(aiMove); lastAiMove = aiMove; // Store move for highlight
                if (!quietMode) std::cout << "AI calculation time: " << duration.count() << " ms" << std::endl; // Print time if not quiet
                gameState.switchPlayer();
                history.push_back(gameState); // Store state after AI move
                redoHistory.clear(); // Clear redo history after new move
            }
        } // End AI Turn

        // --- Check Game Over Conditions ---
        if (!gameOver) {
            winner = gameState.checkWinner(); // Check Den
            if (winner != Player::NONE) {
                gameOver = true; winReason = (winner == Player::PLAYER1 ? "Player 1 (Blue)" : "Player 2 (Red)") + std::string(" reached the Den!");
                if (!quietMode) std::cout << winReason << std::endl;
            } else if (gameState.getCurrentPlayer() == humanPlayer) { // Check Human No Moves
                 if (gameState.getAllLegalMoves(humanPlayer).empty()) {
                     gameOver = true; winner = aiPlayer; winReason = "Human (Blue) has no legal moves!";
                     if (!quietMode) std::cout << winReason << std::endl;
                 }
            }
        } // End Game Over Check

        // --- Drawing ---
        window.clear(sf::Color::White); // Clear with a default color (Graphics draws background over it)
        graphics.drawBoard(window, gameState, selectedPieceLegalMoves, pieceSelected ? selectedMove.fromRow : -1, pieceSelected ? selectedMove.fromCol : -1, lastAiMove);

        if (gameOver) { // Display Game Over Message
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
                background.setFillColor(sf::Color(30, 30, 40, 230));
                background.setOrigin(background.getSize().x / 2.0f, background.getSize().y / 2.0f);
                background.setPosition(window.getSize().x / 2.0f, window.getSize().y / 2.0f);
                window.draw(background); window.draw(gameOverText);
            }
        }
        window.display();

    } // End game loop

    if (!quietMode) std::cout << "Exiting game." << std::endl;
    return 0;
}


// --- Save Game Implementation (Saves History) ---
bool saveGame(const std::vector<GameState>& history, const std::string& filename) {
    std::ofstream outFile(filename, std::ios::binary | std::ios::trunc);
    if (!outFile.is_open()) {
        std::cerr << "Error opening file for saving: " << filename << std::endl;
        return false;
    }
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
        }
        if (outFile.fail()) { std::cerr << "Error writing game state during history save: " << filename << std::endl; outFile.close(); return false; }
    }
    outFile.close(); return !outFile.fail();
}


// --- Load Game Implementation (Loads History) ---
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


