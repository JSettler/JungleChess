#pragma once // Prevents multiple inclusions

#include <vector>

enum class PieceType { EMPTY, RAT, CAT, DOG, WOLF, LEOPARD, TIGER, LION, ELEPHANT };
enum class Player { NONE, PLAYER1, PLAYER2 }; // Or RED, BLUE

struct Piece {
    PieceType type = PieceType::EMPTY;
    Player owner = Player::NONE;
    int rank = 0; // 1 for Rat, 8 for Elephant, 0 for Empty
};

struct Move {
    int fromRow, fromCol;
    int toRow, toCol;
    // Add operator== if needed for comparing moves
    bool operator==(const Move& other) const {
        return fromRow == other.fromRow && fromCol == other.fromCol &&
               toRow == other.toRow && toCol == other.toCol;
    }
};

const int BOARD_ROWS = 9;
const int BOARD_COLS = 7;

// Define special terrain squares (optional for simple version, but needed for full rules)
// enum class Terrain { NORMAL, TRAP, WATER, DEN };