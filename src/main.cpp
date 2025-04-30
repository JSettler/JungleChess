#include <SFML/Graphics.hpp>
#include "GameState.h"
#include "Graphics.h"
#include "AI.h" // Include AI.h
#include "Common.h" // Include Common.h for AppMode
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
bool loadGame(GameState& currentGameState, const std::string& filename, std::vector<GameState>& history);


int main(int argc, char* argv[]) {

    // --- Argument Parsing ---
    bool showHelp = false; bool debugMode = false; bool quietMode = false;
    bool unknownArgumentFound = false; std::string unknownArg = "";
    int searchDepth = 6; // Default depth
    //vvv NEW vvv --- Mode variable --- vvv
    AppMode currentMode = AppMode::GAME; // Default to game mode
    //^^^ NEW ^^^-----------------------^^^

    const char* progName = (argc > 0 && argv[0] != nullptr) ? argv[0] : "jungle_chess";
    if (progName == nullptr) progName = "jungle_chess";
    std::string usageSyntax = "Usage: " + std::string(progName) + " [--depth N] [--setup] [-n | -d | -h | --help | -?]";


    for (int i = 1; i < argc; ++i) {
        if (strcmp(argv[i], "-h") == 0 || strcmp(argv[i], "--help") == 0 || strcmp(argv[i], "-?") == 0) { showHelp = true; }
        else if (strcmp(argv[i], "-d") == 0) { debugMode = true; quietMode = false; }
        else if (strcmp(argv[i], "-n") == 0 && !debugMode) { quietMode = true; }
        //vvv NEW vvv --- Check for --setup --- vvv
        else if (strcmp(argv[i], "--setup") == 0) {
            currentMode = AppMode::SETUP;
            if (!quietMode) std::cout << "Starting in Setup Mode." << std::endl;
        }
        //^^^ NEW ^^^-------------------------^^^
        else if (strcmp(argv[i], "--depth") == 0) {
            if (i + 1 < argc) {
                try {
                    int depthValue = std::stoi(argv[i + 1]);
                    if (depthValue > 0 && depthValue < 20) { searchDepth = depthValue; i++; if (!quietMode) std::cout << "Search depth set to " << searchDepth << " plies." << std::endl; }
                    else { std::cerr << "Error: Invalid search depth value '" << argv[i + 1] << "'. Must be positive and reasonable (< 20)." << std::endl; return 1; }
                } catch (const std::exception& e) { std::cerr << "Error parsing depth value '" << argv[i+1] << "': " << e.what() << std::endl; return 1; }
            } else { std::cerr << "Error: Missing value after --depth flag." << std::endl; std::cerr << usageSyntax << std::endl; return 1; }
        } else if (!unknownArgumentFound) { unknownArgumentFound = true; unknownArg = argv[i]; }
    }

    // --- Handle Help Flag OR Unknown Argument ---
    if (showHelp) {
        std::cout << usageSyntax << "\n\n";
        std::cout << "Options:\n";
        std::cout << "  --depth N : Set AI search depth to N plies (integer, default: 6).\n";
        std::cout << "  --setup   : Start in board setup mode instead of a new game.\n"; // Added setup flag
        std::cout << "  -n        : Quiet mode (minimal console output).\n";
        std::cout << "  -d        : Debug mode (verbose AI move evaluation output).\n";
        std::cout << "  -h, --help, -? : Show this help message and exit.\n\n";
        std::cout << "In-Game Keys (Game Mode):\n"; // Clarified section
        std::cout << "  <Backspace>       : Take back last full move (Undo).\n";
        std::cout << "  <Shift+Backspace> : Redo last undone move.\n";
        std::cout << "  S                 : Save current game state to dsq-game.sav.\n";
        std::cout << "  L                 : Load game state from dsq-game.sav (clears undo/redo history).\n";
        std::cout << "  P                 : Cycle piece display emphasis (Letters <-> Numbers).\n";
        std::cout << "  <Escape>          : Quit the game immediately.\n\n";
        std::cout << "In-Game Keys (Setup Mode):\n"; // Added section
        std::cout << "  Left Click  : Place selected piece / Select UI button.\n";
        std::cout << "  Right Click : Remove piece from board square.\n";
        std::cout << "  1-8         : Select piece type (Rat=1 to Elephant=8).\n";
        std::cout << "  S           : Switch player side for piece placement.\n";
        std::cout << "  P           : Cycle piece display emphasis.\n";
        std::cout << "  F           : Finish setup and start game.\n";
        std::cout << "  <Escape>    : Quit the game immediately.\n";
        return 0;
    }
    else if (unknownArgumentFound) {
        std::cerr << "Error: Unknown parameter '" << unknownArg << "'\n" << usageSyntax << std::endl;
        return 1;
    }

    // Announce operational mode
    if (debugMode && !quietMode) std::cout << "Debug mode enabled." << std::endl;
    else if (quietMode) std::cout << "Quiet mode enabled." << std::endl;


    // --- Initialization ---
    std::string windowTitle = "Jungle Chess - AlphaBeta AI (Depth " + std::to_string(searchDepth) + ")"; // Base title
    if (currentMode == AppMode::SETUP) windowTitle += " - SETUP MODE";
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
    std::vector<GameState> redoHistory;

    // UI State
    Move selectedMove = {-1,-1,-1,-1}; bool pieceSelected = false;
    std::vector<Move> selectedPieceLegalMoves; Move lastAiMove = {-1,-1,-1,-1};
    bool gameOver = false; Player winner = Player::NONE; std::string winReason = "";

    //vvv NEW vvv --- Setup Mode State --- vvv
    Player setupPlayer = Player::PLAYER1; // Player whose pieces are being placed
    PieceType selectedSetupPiece = PieceType::EMPTY; // Piece type selected for placement
    //^^^ NEW ^^^--------------------------^^^

    // Save file name
    const std::string saveFilename = "dsq-game.sav";

    // --- Main Game Loop ---
    while (window.isOpen()) {

        // --- Event Handling ---
        sf::Event event;
        while (window.pollEvent(event)) {
            if (event.type == sf::Event::Closed) window.close();

            // --- Global Key Presses (Work in both modes) ---
            if (event.type == sf::Event::KeyPressed) {
                if (event.key.code == sf::Keyboard::Escape) {
                    window.close(); continue; // Handle Escape first
                }
                // Toggle piece display works in both modes
                if (event.key.code == sf::Keyboard::P) {
                    graphics.togglePieceDisplay(); continue;
                }
            }

            // --- Mode-Specific Event Handling ---
            if (currentMode == AppMode::SETUP) {
                // --- SETUP MODE EVENTS ---
                if (event.type == sf::Event::KeyPressed) {
                    // Select piece type with number keys
                    if (event.key.code >= sf::Keyboard::Num1 && event.key.code <= sf::Keyboard::Num8) {
                        selectedSetupPiece = static_cast<PieceType>(event.key.code - sf::Keyboard::Num1 + 1);
                        if (!quietMode) std::cout << "Setup: Selected piece type " << static_cast<int>(selectedSetupPiece) << std::endl;
                    }
                    // Switch setup side with 'S'
                    else if (event.key.code == sf::Keyboard::S) {
                        setupPlayer = (setupPlayer == Player::PLAYER1) ? Player::PLAYER2 : Player::PLAYER1;
                         if (!quietMode) std::cout << "Setup: Switched to placing pieces for Player " << static_cast<int>(setupPlayer) << std::endl;
                         selectedSetupPiece = PieceType::EMPTY; // Deselect piece on side switch
                    }
                    // Finish setup with 'F'
                    else if (event.key.code == sf::Keyboard::F) {
                        if (gameState.validateSetup()) {
                            currentMode = AppMode::GAME;
                            gameState.setCurrentPlayer(setupPlayer); // Set who moves first
                            gameState.recalculateHash(); // Recalculate hash for the final position
                            history.clear(); redoHistory.clear(); // Reset history
                            history.push_back(gameState); // Start history with setup position
                            window.setTitle("Jungle Chess - AlphaBeta AI (Depth " + std::to_string(searchDepth) + ")"); // Update title
                            if (!quietMode) std::cout << "Setup finished. Player " << static_cast<int>(gameState.getCurrentPlayer()) << " to move." << std::endl;
                        } else {
                             if (!quietMode) std::cerr << "Setup Error: Invalid board position (e.g., piece in opponent den)." << std::endl;
                        }
                    }
                } else if (event.type == sf::Event::MouseButtonPressed) {
                    sf::Vector2i mousePos = sf::Mouse::getPosition(window);
                    // Check UI button clicks first
                    if (graphics.isClickOnClearButton(mousePos)) {
                        gameState.clearBoard();
                        setupPlayer = Player::PLAYER1; // Reset side
                        selectedSetupPiece = PieceType::EMPTY;
                        if (!quietMode) std::cout << "Setup: Board cleared." << std::endl;
                    } else if (graphics.isClickOnSideButton(mousePos)) {
                        setupPlayer = (setupPlayer == Player::PLAYER1) ? Player::PLAYER2 : Player::PLAYER1;
                        if (!quietMode) std::cout << "Setup: Switched to placing pieces for Player " << static_cast<int>(setupPlayer) << std::endl;
                        selectedSetupPiece = PieceType::EMPTY;
                    } else if (graphics.isClickOnFinishButton(mousePos)) {
                         if (gameState.validateSetup()) {
                            currentMode = AppMode::GAME;
                            gameState.setCurrentPlayer(setupPlayer); // Set who moves first
                            gameState.recalculateHash();
                            history.clear(); redoHistory.clear();
                            history.push_back(gameState);
                            window.setTitle("Jungle Chess - AlphaBeta AI (Depth " + std::to_string(searchDepth) + ")");
                            if (!quietMode) std::cout << "Setup finished. Player " << static_cast<int>(gameState.getCurrentPlayer()) << " to move." << std::endl;
                        } else {
                             if (!quietMode) std::cerr << "Setup Error: Invalid board position." << std::endl;
                        }
                    } else {
                        PieceType clickedPieceType = graphics.getClickedSetupPieceButton(mousePos);
                        if (clickedPieceType != PieceType::EMPTY) {
                            selectedSetupPiece = clickedPieceType;
                             if (!quietMode) std::cout << "Setup: Selected piece type " << static_cast<int>(selectedSetupPiece) << std::endl;
                        } else {
                            // Click was not on a button, check board
                            sf::Vector2i boardPos = graphics.getClickedSquare(mousePos);
                            if (gameState.isValidPosition(boardPos.y, boardPos.x)) {
                                if (event.mouseButton.button == sf::Mouse::Left) { // Place piece
                                    if (selectedSetupPiece != PieceType::EMPTY) {
                                        if (!gameState.setPieceAt(boardPos.y, boardPos.x, selectedSetupPiece, setupPlayer)) {
                                            if (!quietMode) std::cerr << "Setup Error: Cannot place piece there." << std::endl;
                                        }
                                    }
                                } else if (event.mouseButton.button == sf::Mouse::Right) { // Remove piece
                                    gameState.clearSquare(boardPos.y, boardPos.x);
                                }
                            }
                        }
                    }
                } // End MouseButton Pressed (Setup)

            } else { // currentMode == AppMode::GAME
                // --- GAME MODE EVENTS ---
                if (gameOver && event.type == sf::Event::MouseButtonPressed && event.mouseButton.button == sf::Mouse::Left) { window.close(); }
                // Handle Takeback (Undo)
                else if (!gameOver && event.type == sf::Event::KeyPressed && event.key.code == sf::Keyboard::BackSpace && !event.key.shift) {
                    if (history.size() >= 3) { redoHistory.push_back(history.back()); history.pop_back(); redoHistory.push_back(history.back()); history.pop_back(); gameState = history.back(); pieceSelected = false; selectedMove = {-1,-1,-1,-1}; selectedPieceLegalMoves.clear(); lastAiMove = {-1,-1,-1,-1}; if (!quietMode) std::cout << "<<< Undo Full Move! >>>" << std::endl; }
                    else if (history.size() == 2) { redoHistory.push_back(history.back()); history.pop_back(); gameState = history.back(); pieceSelected = false; selectedMove = {-1,-1,-1,-1}; selectedPieceLegalMoves.clear(); lastAiMove = {-1,-1,-1,-1}; if (!quietMode) std::cout << "<<< Undo! Restored initial state. >>>" << std::endl; }
                    else if (!quietMode) { std::cout << "Nothing to undo." << std::endl; }
                }
                // Handle Redo
                else if (!gameOver && event.type == sf::Event::KeyPressed && event.key.code == sf::Keyboard::BackSpace && event.key.shift) {
                    if (!redoHistory.empty()) {
                        history.push_back(redoHistory.back()); redoHistory.pop_back();
                        if (!redoHistory.empty()) { history.push_back(redoHistory.back()); redoHistory.pop_back(); }
                        gameState = history.back(); pieceSelected = false; selectedMove = {-1,-1,-1,-1}; selectedPieceLegalMoves.clear(); lastAiMove = {-1,-1,-1,-1};
                        if (!quietMode) std::cout << ">>> Redo Move! >>>" << std::endl;
                    } else if (!quietMode) { std::cout << "Nothing to redo." << std::endl; }
                }
                // Handle Save (Only in Game Mode)
                else if (!gameOver && event.type == sf::Event::KeyPressed && event.key.code == sf::Keyboard::S) { if (saveGame(history, saveFilename)) { if (!quietMode) std::cout << "Game state saved to " << saveFilename << std::endl; } }
                // Handle Load (Only in Game Mode)
                else if (!gameOver && event.type == sf::Event::KeyPressed && event.key.code == sf::Keyboard::L) { if (loadGame(gameState, saveFilename, history)) { redoHistory.clear(); pieceSelected = false; selectedMove = {-1,-1,-1,-1}; selectedPieceLegalMoves.clear(); lastAiMove = {-1,-1,-1,-1}; if (!quietMode) std::cout << "Game state loaded from " << saveFilename << std::endl; } }
                // Handle Human Player Input (Game Mode)
                else if (!gameOver && gameState.getCurrentPlayer() == humanPlayer && event.type == sf::Event::MouseButtonPressed && event.mouseButton.button == sf::Mouse::Left) {
                    sf::Vector2i mousePos = sf::Mouse::getPosition(window);
                    sf::Vector2i boardPos = graphics.getClickedSquare(mousePos);
                    if (gameState.isValidPosition(boardPos.y, boardPos.x)) {
                        if (!pieceSelected) { // Select
                            Piece clickedPiece = gameState.getPiece(boardPos.y, boardPos.x);
                            if (clickedPiece.owner == humanPlayer) {
                                selectedMove.fromRow = boardPos.y; selectedMove.fromCol = boardPos.x; selectedMove.toRow = -1; selectedMove.toCol = -1;
                                pieceSelected = true; lastAiMove = {-1,-1,-1,-1};
                                selectedPieceLegalMoves = gameState.getLegalMovesForPiece(selectedMove.fromRow, selectedMove.fromCol);
                            }
                        } else { // Move/Deselect
                            if (boardPos.y == selectedMove.fromRow && boardPos.x == selectedMove.fromCol) { // Deselect
                                pieceSelected = false; selectedMove = {-1, -1, -1, -1}; selectedPieceLegalMoves.clear();
                            } else { // Attempt move
                                bool isValidTarget = false; Move attemptedMove = {selectedMove.fromRow, selectedMove.fromCol, boardPos.y, boardPos.x};
                                for(const auto& legalMove : selectedPieceLegalMoves) if (legalMove.toRow == attemptedMove.toRow && legalMove.toCol == attemptedMove.toCol) { isValidTarget = true; break; }
                                if (isValidTarget) { // Valid move
                                    gameState.applyMove(attemptedMove); gameState.switchPlayer();
                                    history.push_back(gameState); redoHistory.clear();
                                    pieceSelected = false; selectedMove = {-1, -1, -1, -1}; selectedPieceLegalMoves.clear();
                                } else { // Invalid target
                                    if (!quietMode) std::cout << "Invalid move target." << std::endl;
                                    pieceSelected = false; selectedMove = {-1, -1, -1, -1}; selectedPieceLegalMoves.clear();
                                }
                            }
                        }
                    } else if (pieceSelected) { // Clicked outside - deselect
                        pieceSelected = false; selectedMove = {-1, -1, -1, -1}; selectedPieceLegalMoves.clear();
                    }
                } // End Human Input (Game Mode)
            } // End GAME MODE EVENTS
        } // End event loop


        // --- AI Turn Logic (Only in Game Mode) ---
        if (currentMode == AppMode::GAME && !gameOver && gameState.getCurrentPlayer() == aiPlayer) {
            std::vector<Move> aiLegalMovesCheck = gameState.getAllLegalMoves(aiPlayer);
            if (aiLegalMovesCheck.empty()) { gameOver = true; winner = humanPlayer; winReason = "AI (Red) has no legal moves!"; if (!quietMode) std::cout << winReason << std::endl; }
            else {
                auto startTime = std::chrono::high_resolution_clock::now();
                AIMoveInfo aiResult = AI::getBestMove(gameState, searchDepth, debugMode, quietMode);
                auto stopTime = std::chrono::high_resolution_clock::now();
                auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(stopTime - startTime);
                gameState.applyMove(aiResult.bestMove); lastAiMove = aiResult.bestMove;
                if (!quietMode) {
                    double durationSeconds = duration.count() / 1000.0;
                    double nodesPerSecond = (durationSeconds > 0.0001) ? (static_cast<double>(aiResult.nodesSearched) / durationSeconds) : 0.0;
                    std::cout << "AI calculation time: " << duration.count() << " ms | "
                              << "Nodes: " << aiResult.nodesSearched << " | "
                              << std::fixed << std::setprecision(0) << nodesPerSecond << " N/s | "
                              << std::fixed << std::setprecision(1) << "TT Util: " << aiResult.ttUtilizationPercent << "%"
                              << std::resetiosflags(std::ios::fixed) << std::endl;
                }
                gameState.switchPlayer();
                history.push_back(gameState); redoHistory.clear();
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
        //vvv MODIFIED vvv --- Pass setup mode info to drawBoard --- vvv
        graphics.drawBoard(window, gameState, currentMode, setupPlayer, selectedSetupPiece,
                           selectedPieceLegalMoves, pieceSelected ? selectedMove.fromRow : -1,
                           pieceSelected ? selectedMove.fromCol : -1, lastAiMove);
        //^^^ MODIFIED ^^^-----------------------------------------^^^

        if (gameOver) { // Display Game Over Message (Only shows if gameOver is true)
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


