#include "GameState.h"
#include "Common.h"
#include "Hashing.h" // Include Zobrist hashing
#include <vector>
#include <cmath>
#include <stdexcept>
#include <iostream>
#include <string>
#include <map> // For piece counting

// --- Constructor Implementation ---
GameState::GameState() {
    // Ensure Zobrist keys are initialized (safe to call multiple times)
    Zobrist::initializeKeys();
    currentPlayer = Player::PLAYER1;
    setupInitialBoard(); // This will now calculate the initial hash
    // std::cout << "GameState initialized. Player " << static_cast<int>(currentPlayer) << " starts." << std::endl;
}

// --- setupInitialBoard Implementation ---
void GameState::setupInitialBoard() {
    // Create the board grid and fill with EMPTY pieces
    board.assign(BOARD_ROWS, std::vector<Piece>(BOARD_COLS, {PieceType::EMPTY, Player::NONE, 0}));

    // --- Place Player 1 (Bottom, often Blue) Pieces ---
    board[0][0] = {PieceType::LION, Player::PLAYER1, 7}; board[0][6] = {PieceType::TIGER, Player::PLAYER1, 6};
    board[1][1] = {PieceType::DOG, Player::PLAYER1, 3}; board[1][5] = {PieceType::CAT, Player::PLAYER1, 2};
    board[2][0] = {PieceType::RAT, Player::PLAYER1, 1}; board[2][2] = {PieceType::LEOPARD, Player::PLAYER1, 5};
    board[2][4] = {PieceType::WOLF, Player::PLAYER1, 4}; board[2][6] = {PieceType::ELEPHANT, Player::PLAYER1, 8};
    // --- Place Player 2 (Top, often Red) Pieces ---
    board[8][6] = {PieceType::LION, Player::PLAYER2, 7}; board[8][0] = {PieceType::TIGER, Player::PLAYER2, 6};
    board[7][5] = {PieceType::DOG, Player::PLAYER2, 3}; board[7][1] = {PieceType::CAT, Player::PLAYER2, 2};
    board[6][6] = {PieceType::RAT, Player::PLAYER2, 1}; board[6][4] = {PieceType::LEOPARD, Player::PLAYER2, 5};
    board[6][2] = {PieceType::WOLF, Player::PLAYER2, 4}; board[6][0] = {PieceType::ELEPHANT, Player::PLAYER2, 8};

    recalculateHash(); // Calculate initial hash after board is set up
    // std::cout << "Initial board set up. Hash: " << currentHashKey << std::endl; // Debug
}

// --- getPiece Implementation ---
Piece GameState::getPiece(int row, int col) const {
    if (isValidPosition(row, col)) { return board[row][col]; }
    return {PieceType::EMPTY, Player::NONE, 0}; // Return empty piece for invalid coords
}

// --- isMoveLegal Implementation ---
bool GameState::isMoveLegal(const Move& move, Player player) const {
    // 1. Basic Validity Checks
    if (!isValidPosition(move.fromRow, move.fromCol) || !isValidPosition(move.toRow, move.toCol)) return false;
    Piece movingPiece = getPiece(move.fromRow, move.fromCol);
    Piece destinationPiece = getPiece(move.toRow, move.toCol);
    if (movingPiece.owner != player || movingPiece.type == PieceType::EMPTY) return false;
    if (destinationPiece.owner == player) return false; // Cannot capture own piece
    if (isOwnDen(move.toRow, move.toCol, player)) return false; // Cannot move into own den

    // 2. Calculate Movement Details
    int rowDiff = std::abs(move.toRow - move.fromRow);
    int colDiff = std::abs(move.toCol - move.fromCol);
    bool toSquareIsRiver = isRiver(move.toRow, move.toCol);
    bool fromSquareIsRiver = isRiver(move.fromRow, move.fromCol);

    // 3. Check River Rules (General)
    // Only Rat can enter the river
    if (toSquareIsRiver && movingPiece.type != PieceType::RAT) return false;

    // 4. Check Piece-Specific Movement Rules
    // --- 4a. Lion/Tiger Jump ---
    if (movingPiece.type == PieceType::LION || movingPiece.type == PieceType::TIGER) {
        // Jump cannot start or end in river
        if (!fromSquareIsRiver && !toSquareIsRiver) {
            // Horizontal Jump Check
            if (rowDiff == 0 && colDiff == 3) {
                int stepDir = (move.toCol > move.fromCol) ? 1 : -1;
                // Check if intermediate squares are river and empty
                if (isRiver(move.fromRow, move.fromCol + stepDir) &&
                    isRiver(move.fromRow, move.fromCol + 2*stepDir) &&
                    getPiece(move.fromRow, move.fromCol + stepDir).type == PieceType::EMPTY &&
                    getPiece(move.fromRow, move.fromCol + 2*stepDir).type == PieceType::EMPTY)
                {
                    // Check capture validity at destination
                    if (destinationPiece.type != PieceType::EMPTY && !canCapture(movingPiece, destinationPiece, move.toRow, move.toCol)) return false;
                    return true; // Valid jump
                }
            }
            // Vertical Jump Check
            else if (colDiff == 0 && rowDiff == 4) {
                 int stepDir = (move.toRow > move.fromRow) ? 1 : -1;
                 // Check if intermediate squares are river and empty
                if (isRiver(move.fromRow + stepDir, move.fromCol) &&
                    isRiver(move.fromRow + 2*stepDir, move.fromCol) &&
                    isRiver(move.fromRow + 3*stepDir, move.fromCol) &&
                    getPiece(move.fromRow + stepDir, move.fromCol).type == PieceType::EMPTY &&
                    getPiece(move.fromRow + 2*stepDir, move.fromCol).type == PieceType::EMPTY &&
                    getPiece(move.fromRow + 3*stepDir, move.fromCol).type == PieceType::EMPTY)
                {
                    // Check capture validity at destination
                     if (destinationPiece.type != PieceType::EMPTY && !canCapture(movingPiece, destinationPiece, move.toRow, move.toCol)) return false;
                    return true; // Valid jump
                }
            }
        }
        // If jump conditions not met, fall through to normal move check
    }

    // --- 4b. Rat Movement ---
    if (movingPiece.type == PieceType::RAT) {
        // Can move one step orthogonally
        if ((rowDiff == 1 && colDiff == 0) || (rowDiff == 0 && colDiff == 1)) {
             // Special capture rules for Rat
             if (destinationPiece.type != PieceType::EMPTY) { // Attempting capture
                 // Rat cannot capture Elephant from water
                 if (destinationPiece.type == PieceType::ELEPHANT && fromSquareIsRiver) return false;
                 // Rat cannot capture between water/land
                 if (fromSquareIsRiver != toSquareIsRiver) return false;
                 // Standard capture check (includes Rat vs Elephant on land)
                 if (!canCapture(movingPiece, destinationPiece, move.toRow, move.toCol)) return false;
             }
             // If not capturing or capture is valid, the move is legal
             return true;
        }
        // If not orthogonal move, illegal for Rat
        return false;
    }

    // --- 4c. General Land Movement (Non-Rat, Non-Jump) ---
    // Must be one step orthogonal, cannot end in water
    if (!toSquareIsRiver && (rowDiff + colDiff == 1)) {
        // Check capture validity
        if (destinationPiece.type != PieceType::EMPTY && !canCapture(movingPiece, destinationPiece, move.toRow, move.toCol)) return false;
        return true; // Valid land move
    }

    // 5. If none of the above rules matched, the move is illegal
    return false;
}

// --- Hash update helper (Renamed and uses Player) ---
// XORs the key for a given piece type AND player at a given square
void GameState::updateHashForPieceChange(PieceType type, Player player, int r, int c) {
    if (type != PieceType::EMPTY && player != Player::NONE) {
        int ppi = Zobrist::getPiecePlayerIndex(type, player);
        // Basic bounds check
        if (ppi != -1 && ppi < Zobrist::piecePlayerKeys.size() &&
            r >= 0 && r < BOARD_ROWS && c >= 0 && c < BOARD_COLS)
        {
            currentHashKey ^= Zobrist::piecePlayerKeys[ppi][r][c];
        } else {
             // Use std::cerr for errors, make sure <iostream> is included where Hashing.h is used
             std::cerr << "Warning: Invalid index during hash update. Type: " << static_cast<int>(type)
                       << " Player: " << static_cast<int>(player) << " R: " << r << " C: " << c << std::endl;
        }
    }
}

// --- applyMove Implementation (Update Hash using new helper) ---
void GameState::applyMove(const Move& move) {
    Piece movingPiece = getPiece(move.fromRow, move.fromCol);
    Piece capturedPiece = getPiece(move.toRow, move.toCol); // Get piece before overwriting

    // --- Update Hash ---
    // 1. XOR out the moving piece (with its player) from its original square
    updateHashForPieceChange(movingPiece.type, movingPiece.owner, move.fromRow, move.fromCol);
    // 2. XOR out the captured piece (with its player, if any) from the destination square
    updateHashForPieceChange(capturedPiece.type, capturedPiece.owner, move.toRow, move.toCol);
    // 3. XOR in the moving piece (with its player) at the destination square
    updateHashForPieceChange(movingPiece.type, movingPiece.owner, move.toRow, move.toCol);
    // Side to move hash is updated in switchPlayer()

    // --- Update Board ---
    board[move.toRow][move.toCol] = movingPiece;
    board[move.fromRow][move.fromCol] = {PieceType::EMPTY, Player::NONE, 0};
}

// --- getAllLegalMoves Implementation ---
std::vector<Move> GameState::getAllLegalMoves(Player player) const {
    std::vector<Move> legalMoves;
    legalMoves.reserve(40); // Pre-allocate some space
    for (int r = 0; r < BOARD_ROWS; ++r) {
        for (int c = 0; c < BOARD_COLS; ++c) {
            if (board[r][c].owner == player) {
                Piece piece = board[r][c];
                // Generate potential orthogonal moves
                int potentialDests[][2] = {{r + 1, c}, {r - 1, c}, {r, c + 1}, {r, c - 1}};
                for (auto& dest : potentialDests) {
                    Move testMove = {r, c, dest[0], dest[1]};
                    if (isMoveLegal(testMove, player)) { legalMoves.push_back(testMove); }
                }
                // Generate potential jump moves for L/T
                if (piece.type == PieceType::LION || piece.type == PieceType::TIGER) {
                    int jumpDests[][2] = {{r, c+3}, {r, c-3}, {r+4, c}, {r-4, c}};
                     for (auto& dest : jumpDests) {
                        Move testMove = {r, c, dest[0], dest[1]};
                        // Check bounds before isMoveLegal for jumps
                        if (isValidPosition(dest[0], dest[1])) {
                            if (isMoveLegal(testMove, player)) { legalMoves.push_back(testMove); }
                        }
                    }
                }
            }
        }
    }
    return legalMoves;
}

// --- getLegalMovesForPiece Implementation ---
std::vector<Move> GameState::getLegalMovesForPiece(int fromRow, int fromCol) const {
    std::vector<Move> legalMoves;
    Player player = getCurrentPlayer(); // Use the current player

    if (!isValidPosition(fromRow, fromCol)) return legalMoves;
    Piece piece = getPiece(fromRow, fromCol);
    if (piece.owner != player || piece.type == PieceType::EMPTY) return legalMoves;

    // Iterate through ALL possible destination squares
    // This is less efficient than targeted generation but simpler for this helper
    for (int toRow = 0; toRow < BOARD_ROWS; ++toRow) {
        for (int toCol = 0; toCol < BOARD_COLS; ++toCol) {
            Move testMove = {fromRow, fromCol, toRow, toCol};
            if (isMoveLegal(testMove, player)) {
                legalMoves.push_back(testMove);
            }
        }
    }
    return legalMoves;
}


// --- getCurrentPlayer Implementation ---
Player GameState::getCurrentPlayer() const { return currentPlayer; }

// --- switchPlayer Implementation (Update Hash) ---
void GameState::switchPlayer() {
    currentPlayer = (currentPlayer == Player::PLAYER1) ? Player::PLAYER2 : Player::PLAYER1;
    // Update hash for side change
    currentHashKey ^= Zobrist::sideToMoveKey;
}

// --- checkWinner Implementation ---
Player GameState::checkWinner() const {
    // Check Den Reached
    // Player 1 Den is at (0, 3)
    if (isValidPosition(0, 3) && board[0][3].owner == Player::PLAYER2) return Player::PLAYER2;
    // Player 2 Den is at (8, 3)
    if (isValidPosition(8, 3) && board[8][3].owner == Player::PLAYER1) return Player::PLAYER1;

    // Note: No-moves win condition is checked in main loop by calling getAllLegalMoves
    return Player::NONE;
}

// --- Hash getter implementation ---
uint64_t GameState::getHashKey() const {
    return currentHashKey;
}


// --- Public Helper Functions ---
bool GameState::isValidPosition(int r, int c) const { return r >= 0 && r < BOARD_ROWS && c >= 0 && c < BOARD_COLS; }
bool GameState::isRiver(int r, int c) const { return (r >= 3 && r <= 5) && (c == 1 || c == 2 || c == 4 || c == 5); }
bool GameState::isOwnTrap(int r, int c, Player player) const {
     if (!isValidPosition(r,c)) return false;
     // Player 1 traps (near row 0)
     if (player == Player::PLAYER1) return (r == 0 && c == 2) || (r == 0 && c == 4) || (r == 1 && c == 3);
     // Player 2 traps (near row 8)
     if (player == Player::PLAYER2) return (r == 8 && c == 2) || (r == 8 && c == 4) || (r == 7 && c == 3);
     return false;
}
 bool GameState::isOwnDen(int r, int c, Player player) const {
     if (!isValidPosition(r,c)) return false;
      // Player 1 Den
      if (player == Player::PLAYER1) return (r == 0 && c == 3);
      // Player 2 Den
      if (player == Player::PLAYER2) return (r == 8 && c == 3);
      return false;
 }

// --- getRank Implementation (Moved to public) ---
int GameState::getRank(PieceType type) const {
     switch(type) {
        case PieceType::RAT: return 1; case PieceType::CAT: return 2; case PieceType::DOG: return 3;
        case PieceType::WOLF: return 4; case PieceType::LEOPARD: return 5; case PieceType::TIGER: return 6;
        case PieceType::LION: return 7; case PieceType::ELEPHANT: return 8; default: return 0;
     }
}

// --- Private Helper Implementations ---
bool GameState::canCapture(const Piece& attacker, const Piece& defender, int defenderRow, int defenderCol) const {
     if (defender.type == PieceType::EMPTY || attacker.owner == defender.owner) return false;

     Player opponentPlayer = (attacker.owner == Player::PLAYER1) ? Player::PLAYER2 : Player::PLAYER1;

     // Check if defender is in one of opponent's traps (relative to attacker)
     if (isOwnTrap(defenderRow, defenderCol, opponentPlayer)) {
         // std::cout << "Debug: Capture allowed due to trap at (" << defenderRow << "," << defenderCol << ")" << std::endl;
         return true; // Any piece can capture a piece in an opponent's trap
     }

     // Specific Rat/Elephant rules
     if (attacker.type == PieceType::RAT && defender.type == PieceType::ELEPHANT) {
         // Rat cannot capture Elephant from water is handled in isMoveLegal
         // std::cout << "Debug: Capture allowed Rat->Elephant" << std::endl;
         return true; // Rat captures Elephant (on land)
     }
     if (attacker.type == PieceType::ELEPHANT && defender.type == PieceType::RAT) {
         // std::cout << "Debug: Capture denied Elephant->Rat" << std::endl;
         return false; // Elephant cannot capture Rat
     }

     // General rank capture rule
     bool canCap = attacker.rank >= defender.rank;
     // if (!canCap) std::cout << "Debug: Capture denied Rank " << attacker.rank << " vs " << defender.rank << std::endl;
     // else std::cout << "Debug: Capture allowed Rank " << attacker.rank << " vs " << defender.rank << std::endl;
     return canCap;
}

// --- Setter Implementations ---
void GameState::setBoard(const std::vector<std::vector<Piece>>& newBoard) {
    if (newBoard.size() == BOARD_ROWS && (!newBoard.empty() && newBoard[0].size() == BOARD_COLS)) {
        board = newBoard;
        // Hash must be recalculated or set explicitly after this
    } else {
        std::cerr << "Error: Attempted to set board with invalid dimensions." << std::endl;
    }
}

void GameState::setCurrentPlayer(Player player) {
    // Consider updating hash if player changes without switchPlayer?
    // Best practice: Recalculate hash after manually setting player/board.
    currentPlayer = player;
}

void GameState::setHashKey(uint64_t key) {
    // Allows manually setting the hash key after loading board and player
    currentHashKey = key;
}

// --- Setup Mode Method Implementations ---

// Counts pieces for a given player
std::map<PieceType, int> GameState::countPieces(Player player) const {
    std::map<PieceType, int> counts;
    for (int r = 0; r < BOARD_ROWS; ++r) {
        for (int c = 0; c < BOARD_COLS; ++c) {
            if (board[r][c].owner == player && board[r][c].type != PieceType::EMPTY) {
                counts[board[r][c].type]++;
            }
        }
    }
    return counts;
}

// Places a piece if valid during setup
bool GameState::setPieceAt(int r, int c, PieceType type, Player player) {
    if (!isValidPosition(r, c)) return false;
    if (type == PieceType::EMPTY) { // Effectively clearing the square
        clearSquare(r, c);
        return true;
    }
    if (player == Player::NONE) return false; // Must assign to a player

    // Rule Checks:
    // 1. Cannot place in opponent's den
    Player opponent = (player == Player::PLAYER1) ? Player::PLAYER2 : Player::PLAYER1;
    if (isOwnDen(r, c, opponent)) {
         std::cerr << "Setup Error: Cannot place piece in opponent's den." << std::endl;
         return false;
    }
    // 2. Cannot place non-rat in river
    if (isRiver(r, c) && type != PieceType::RAT) {
         std::cerr << "Setup Error: Only Rat can be placed in the river." << std::endl;
         return false;
    }
    // 3. Check piece count limit (max 1 of each type per player)
    auto counts = countPieces(player);
    Piece existingPiece = board[r][c];
    // Allow placing over self, otherwise check count
    if (!(existingPiece.type == type && existingPiece.owner == player)) {
        if (counts[type] >= 1) {
            std::cerr << "Setup Error: Player " << static_cast<int>(player)
                      << " already has a " << static_cast<int>(type) << std::endl;
            return false;
        }
    }

    // Clear the target square first (no hash update needed here)
    board[r][c] = {PieceType::EMPTY, Player::NONE, 0};

    // Place the new piece
    board[r][c] = {type, player, getRank(type)};
    // Hash will be recalculated when finishing setup.
    return true;
}

// Removes piece at location
void GameState::clearSquare(int r, int c) {
    if (isValidPosition(r, c)) {
        board[r][c] = {PieceType::EMPTY, Player::NONE, 0};
        // Hash will be recalculated when finishing setup.
    }
}

// Removes all pieces
void GameState::clearBoard() {
     board.assign(BOARD_ROWS, std::vector<Piece>(BOARD_COLS, {PieceType::EMPTY, Player::NONE, 0}));
     // Hash will be recalculated when finishing setup.
}

// Recalculates the hash from the current board state and player
void GameState::recalculateHash() {
    currentHashKey = Zobrist::calculateInitialHash(board, currentPlayer);
    // std::cout << "Hash recalculated: " << currentHashKey << std::endl; // Debug
}

// Basic validation for setup completion
bool GameState::validateSetup() const {
     // Check if any piece is in opponent's den
     if (isOwnDen(0, 3, Player::PLAYER1) && board[0][3].owner == Player::PLAYER2) return false;
     if (isOwnDen(8, 3, Player::PLAYER2) && board[8][3].owner == Player::PLAYER1) return false;

     // Could add more checks (e.g., exactly 8 pieces per side if desired)
     auto p1counts = countPieces(Player::PLAYER1);
     auto p2counts = countPieces(Player::PLAYER2);
     int p1total = 0; for(auto const& [key, val] : p1counts) p1total += val;
     int p2total = 0; for(auto const& [key, val] : p2counts) p2total += val;
     if (p1total == 0 || p2total == 0) {
         std::cerr << "Setup Error: Each player must have at least one piece." << std::endl;
         return false; // Must have some pieces
     }

     return true;
}


