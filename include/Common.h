#pragma once

#include <cstdint> // For fixed-width integers like int8_t if needed
#include <vector>  // Often needed

// --- Constants ---
const int BOARD_ROWS = 9;
const int BOARD_COLS = 7;

// --- Enums ---
enum class Player : int8_t { // Use int8_t for smaller size in save files/structs
    NONE = 0,
    PLAYER1 = 1, // Often Blue, starts at bottom
    PLAYER2 = 2  // Often Red, starts at top
};

enum class PieceType : int8_t {
    EMPTY = 0,
    RAT = 1, CAT = 2, DOG = 3, WOLF = 4,
    LEOPARD = 5, TIGER = 6, LION = 7, ELEPHANT = 8
};

//vvv NEW vvv --- Application Mode Enum --- vvv
enum class AppMode {
    GAME,
    SETUP
};
//^^^ NEW ^^^-------------------------------^^^


// --- Structs ---
struct Piece {
    PieceType type = PieceType::EMPTY;
    Player owner = Player::NONE;
    int rank = 0; // Rank (1=Rat to 8=Elephant)
};

struct Move {
    int fromRow, fromCol;
    int toRow, toCol;

    // Add comparison operator for use in TT etc.
    bool operator==(const Move& other) const {
        return fromRow == other.fromRow && fromCol == other.fromCol &&
               toRow == other.toRow && toCol == other.toCol;
    }
};


