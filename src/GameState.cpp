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
}

// --- setupInitialBoard Implementation ---
void GameState::setupInitialBoard() {
    // Assign board with default pieces (weakened=false)
    board.assign(BOARD_ROWS, std::vector<Piece>(BOARD_COLS, {PieceType::EMPTY, Player::NONE, 0, false}));

    // --- Place Player 1 (Bottom, often Blue) Pieces ---
    board[0][0] = {PieceType::LION, Player::PLAYER1, 7, false}; board[0][6] = {PieceType::TIGER, Player::PLAYER1, 6, false};
    board[1][1] = {PieceType::DOG, Player::PLAYER1, 3, false}; board[1][5] = {PieceType::CAT, Player::PLAYER1, 2, false};
    board[2][0] = {PieceType::RAT, Player::PLAYER1, 1, false}; board[2][2] = {PieceType::LEOPARD, Player::PLAYER1, 5, false};
    board[2][4] = {PieceType::WOLF, Player::PLAYER1, 4, false}; board[2][6] = {PieceType::ELEPHANT, Player::PLAYER1, 8, false};
    // --- Place Player 2 (Top, often Red) Pieces ---
    board[8][6] = {PieceType::LION, Player::PLAYER2, 7, false}; board[8][0] = {PieceType::TIGER, Player::PLAYER2, 6, false};
    board[7][5] = {PieceType::DOG, Player::PLAYER2, 3, false}; board[7][1] = {PieceType::CAT, Player::PLAYER2, 2, false};
    board[6][6] = {PieceType::RAT, Player::PLAYER2, 1, false}; board[6][4] = {PieceType::LEOPARD, Player::PLAYER2, 5, false};
    board[6][2] = {PieceType::WOLF, Player::PLAYER2, 4, false}; board[6][0] = {PieceType::ELEPHANT, Player::PLAYER2, 8, false};

    recalculateHash(); // Calculate initial hash after board is set up
}

// --- getPiece Implementation ---
Piece GameState::getPiece(int row, int col) const {
    if (isValidPosition(row, col)) { return board[row][col]; }
    return {PieceType::EMPTY, Player::NONE, 0, false}; // Include weakened flag default
}

// <<< NEW: getBoard Implementation >>>
const std::vector<std::vector<Piece>>& GameState::getBoard() const {
    return board;
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
                 // Rat cannot capture anything from water if target is on land, or vice versa
                 if (fromSquareIsRiver != toSquareIsRiver) return false;
                 // Rat cannot capture Elephant from water (explicit check)
                 if (destinationPiece.type == PieceType::ELEPHANT && fromSquareIsRiver) return false;
                 // General capture check (uses updated canCapture)
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

// --- Hash update helper ---
void GameState::updateHashForPieceChange(PieceType type, Player player, int r, int c) {
    // Hash doesn't depend on 'weakened' status, so this function is unchanged
    if (type != PieceType::EMPTY && player != Player::NONE) {
        int ppi = Zobrist::getPiecePlayerIndex(type, player);
        // Basic bounds check
        if (ppi != -1 && ppi < Zobrist::piecePlayerKeys.size() &&
            r >= 0 && r < BOARD_ROWS && c >= 0 && c < BOARD_COLS)
        {
            currentHashKey ^= Zobrist::piecePlayerKeys[ppi][r][c];
        } else {
             // Suppress warning in non-debug? Or keep it? Let's keep it.
             std::cerr << "Warning: Invalid index during hash update. ppi=" << ppi << " R=" << r << " C=" << c << std::endl;
        }
    }
}

// --- applyMove Implementation ---
void GameState::applyMove(const Move& move) {
    Piece movingPiece = getPiece(move.fromRow, move.fromCol);
    Piece capturedPiece = getPiece(move.toRow, move.toCol); // Get piece before overwriting

    // --- Update Hash (BEFORE modifying board state) ---
    updateHashForPieceChange(movingPiece.type, movingPiece.owner, move.fromRow, move.fromCol);
    updateHashForPieceChange(capturedPiece.type, capturedPiece.owner, move.toRow, move.toCol);

    // --- Update Board ---
    // Check if moving onto an opponent's trap to set weakened flag
    Player opponent = (movingPiece.owner == Player::PLAYER1) ? Player::PLAYER2 : Player::PLAYER1;
    if (isOwnTrap(move.toRow, move.toCol, opponent)) {
        movingPiece.weakened = true; // Set weakened flag permanently
    }
    // Note: The weakened status persists even if it moves off the trap later.

    board[move.toRow][move.toCol] = movingPiece; // Place the (potentially weakened) piece
    board[move.fromRow][move.fromCol] = {PieceType::EMPTY, Player::NONE, 0, false}; // Clear original square

    // --- Update Hash (Part 2: Add moving piece in new location) ---
    // Must use the piece info *after* potential weakening status change, though hash doesn't use weakened flag
    updateHashForPieceChange(movingPiece.type, movingPiece.owner, move.toRow, move.toCol);
    // Side to move hash is updated in switchPlayer()
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
    // Use the owner of the piece at the square, not necessarily the current player
    if (!isValidPosition(fromRow, fromCol)) return legalMoves;
    Piece piece = getPiece(fromRow, fromCol);
    Player player = piece.owner;
    if (player == Player::NONE || piece.type == PieceType::EMPTY) return legalMoves;

    // Iterate through ALL possible destination squares
    // This is less efficient than targeted generation but simpler for this UI helper
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

// --- switchPlayer Implementation ---
void GameState::switchPlayer() {
    Player oldPlayer = currentPlayer;
    currentPlayer = (currentPlayer == Player::PLAYER1) ? Player::PLAYER2 : Player::PLAYER1;
    // Update hash for side change only if the player actually changed
    if (oldPlayer != currentPlayer) {
        currentHashKey ^= Zobrist::sideToMoveKey;
    }
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

// --- getRank Implementation ---
int GameState::getRank(PieceType type) const {
     switch(type) {
        case PieceType::RAT: return 1; case PieceType::CAT: return 2; case PieceType::DOG: return 3;
        case PieceType::WOLF: return 4; case PieceType::LEOPARD: return 5; case PieceType::TIGER: return 6;
        case PieceType::LION: return 7; case PieceType::ELEPHANT: return 8; default: return 0;
     }
}

// --- Private Helper Implementations ---
bool GameState::canCapture(const Piece& attacker, const Piece& defender, int defenderRow, int defenderCol) const {
     // Basic checks: cannot capture empty square or own piece
     if (defender.type == PieceType::EMPTY || attacker.owner == defender.owner) {
         return false;
     }

     // 1. Trap Rule Check (Highest Priority): Is the DEFENDER on the ATTACKER'S trap?
     if (isOwnTrap(defenderRow, defenderCol, attacker.owner)) {
         return true;
     }

     // 2. Permanent Weakening Check (Second Highest Priority): Has the DEFENDER been weakened previously?
     if (defender.weakened) {
         return true;
     }

     // 3. Specific Rat/Elephant rules (Only if not captured via trap or weakening)
     if (attacker.type == PieceType::RAT && defender.type == PieceType::ELEPHANT) {
         // Note: Rule about Rat not capturing Elephant from water is handled in isMoveLegal
         return true; // Rat captures Elephant (on land)
     }
     if (attacker.type == PieceType::ELEPHANT && defender.type == PieceType::RAT) {
         return false; // Elephant cannot capture Rat (unless Rat is weakened, checked above)
     }

     // 4. General rank capture rule (if no other rule applied)
     return attacker.rank >= defender.rank;
}


// --- Setter Implementations ---
void GameState::setBoard(const std::vector<std::vector<Piece>>& newBoard) {
    if (newBoard.size() == BOARD_ROWS && (!newBoard.empty() && newBoard[0].size() == BOARD_COLS)) {
        board = newBoard;
        // WARNING: Hash is NOT updated here. Caller must call recalculateHash or setHashKey.
    } else {
        std::cerr << "Error: Attempted to set board with invalid dimensions." << std::endl;
    }
}

void GameState::setCurrentPlayer(Player player) {
    // Update hash only if player actually changes
    // This prevents XORing the side key unnecessarily if setting to the same player
    if (player != currentPlayer && player != Player::NONE) {
        // XOR out the key for the OLD player if it was P2
        if (currentPlayer == Player::PLAYER2) {
            currentHashKey ^= Zobrist::sideToMoveKey;
        }
        // XOR in the key for the NEW player if it is P2
        if (player == Player::PLAYER2) {
            currentHashKey ^= Zobrist::sideToMoveKey;
        }
    }
    currentPlayer = player; // Update the player state
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
    Player opponent = (player == Player::PLAYER1) ? Player::PLAYER2 : Player::PLAYER1;
    if (isOwnDen(r, c, opponent)) {
         std::cerr << "Setup Error: Cannot place piece in opponent's den." << std::endl;
         return false;
    }
    if (isRiver(r, c) && type != PieceType::RAT) {
         std::cerr << "Setup Error: Only Rat can be placed in the river." << std::endl;
         return false;
    }
    auto counts = countPieces(player);
    Piece existingPiece = board[r][c];
    // Allow placing over self, otherwise check count
    if (!(existingPiece.type == type && existingPiece.owner == player)) {
        if (counts.count(type) && counts[type] >= 1) { // Check if key exists before accessing
            std::cerr << "Setup Error: Player " << static_cast<int>(player)
                      << " already has a " << static_cast<int>(type) << std::endl;
            return false;
        }
    }

    // Create the new piece (start not weakened)
    Piece newPiece = {type, player, getRank(type), false}; // Ensure weakened is false on setup placement
    board[r][c] = newPiece; // Place new (overwrites old)
    // Hash will be recalculated when finishing setup.
    return true;
}

// Removes piece at location
void GameState::clearSquare(int r, int c) {
    if (isValidPosition(r, c)) {
        board[r][c] = {PieceType::EMPTY, Player::NONE, 0, false}; // Ensure weakened is false
        // Hash will be recalculated when finishing setup.
    }
}

// Removes all pieces
void GameState::clearBoard() {
     board.assign(BOARD_ROWS, std::vector<Piece>(BOARD_COLS, {PieceType::EMPTY, Player::NONE, 0, false})); // Ensure weakened is false
     // Hash will be recalculated when finishing setup.
}

// Recalculates the hash from the current board state and player
void GameState::recalculateHash() {
    currentHashKey = Zobrist::calculateInitialHash(board, currentPlayer);
    // Optional Debug: std::cout << "Hash recalculated: " << currentHashKey << std::endl;
}

// Basic validation for setup completion
bool GameState::validateSetup() const {
     // Check if any piece is in opponent's den
     if (getPiece(0, 3).owner == Player::PLAYER2) {
         std::cerr << "Setup Error: Player 2 piece in Player 1 den." << std::endl;
         return false;
     }
     if (getPiece(8, 3).owner == Player::PLAYER1) {
          std::cerr << "Setup Error: Player 1 piece in Player 2 den." << std::endl;
          return false;
     }

     // Could add more checks (e.g., exactly 8 pieces per side if desired)
     auto p1counts = countPieces(Player::PLAYER1);
     auto p2counts = countPieces(Player::PLAYER2);
     int p1total = 0; for(auto const& [key, val] : p1counts) p1total += val;
     int p2total = 0; for(auto const& [key, val] : p2counts) p2total += val;
     if (p1total == 0 || p2total == 0) {
         std::cerr << "Setup Error: Each player must have at least one piece." << std::endl;
         return false; // Must have some pieces
     }
     // Add check for max 1 piece of each type per player
     for(auto const& [key, val] : p1counts) if (val > 1) { std::cerr << "Setup Error: Player 1 has >1 of piece type " << (int)key << std::endl; return false; }
     for(auto const& [key, val] : p2counts) if (val > 1) { std::cerr << "Setup Error: Player 2 has >1 of piece type " << (int)key << std::endl; return false; }

     return true;
}


