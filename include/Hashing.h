#pragma once

#include "Common.h"
#include <vector>
#include <cstdint>
#include <random>
#include <limits>
#include <iostream>
#include <stdexcept>

namespace Zobrist {

    // --- Zobrist Keys ---
    //vvv MODIFIED vvv --- Keys for each piece *and* player --- vvv
    // Dimension: [PieceType * 2 + PlayerOffset][Rows][Cols]
    // PlayerOffset: 0 for Player1, 1 for Player2
    extern std::vector<std::vector<std::vector<uint64_t>>> piecePlayerKeys;
    //^^^ MODIFIED ^^^-----------------------------------------^^^
    extern uint64_t sideToMoveKey; // Key for Player 2 to move
    extern bool initialized;

    // --- Helper to get the index for piecePlayerKeys ---
    inline int getPiecePlayerIndex(PieceType type, Player player) {
        if (type == PieceType::EMPTY || player == Player::NONE) {
            return -1; // Invalid index for empty or no player
        }
        // Map Player1 -> offset 0, Player2 -> offset 1
        int playerOffset = (player == Player::PLAYER1) ? 0 : 1;
        // Max enum value determines size needed. Assuming ELEPHANT is max.
        int numTypes = static_cast<int>(PieceType::ELEPHANT) + 1;
        return static_cast<int>(type) * 2 + playerOffset;
    }

    // --- Initialization Function ---
    inline void initializeKeys() {
        if (initialized) return;

        std::mt19937_64 rng(0xdeadbeefcafebabe); // Fixed seed
        std::uniform_int_distribution<uint64_t> dist(0, std::numeric_limits<uint64_t>::max());

        //vvv MODIFIED vvv --- Resize for piece+player keys --- vvv
        // Need space for (NumPieceTypes * 2) entries in the first dimension
        int numTypes = static_cast<int>(PieceType::ELEPHANT) + 1;
        int numPiecePlayerKeys = numTypes * 2;
        piecePlayerKeys.resize(numPiecePlayerKeys,
                               std::vector<std::vector<uint64_t>>(BOARD_ROWS,
                               std::vector<uint64_t>(BOARD_COLS)));

        // Assign random keys for each piece-player combo at each square
        for (int ppi = 0; ppi < numPiecePlayerKeys; ++ppi) {
             // Skip indices that don't correspond to valid PieceType * 2 + PlayerOffset ?
             // No, just generate for all possible indices up to max needed.
            for (int r = 0; r < BOARD_ROWS; ++r) {
                for (int c = 0; c < BOARD_COLS; ++c) {
                    piecePlayerKeys[ppi][r][c] = dist(rng);
                }
            }
        }
        //^^^ MODIFIED ^^^------------------------------------^^^

        sideToMoveKey = dist(rng);
        initialized = true;
        std::cout << "Zobrist keys initialized (with player distinction)." << std::endl;
    }

    // --- Hash Calculation Helper ---
    inline uint64_t calculateInitialHash(const std::vector<std::vector<Piece>>& board, Player currentPlayer) {
        if (!initialized) {
             throw std::runtime_error("Zobrist keys not initialized!");
        }
        uint64_t hash = 0;
        for (int r = 0; r < BOARD_ROWS; ++r) {
            for (int c = 0; c < BOARD_COLS; ++c) {
                const Piece& piece = board[r][c];
                if (piece.type != PieceType::EMPTY) {
                    //vvv MODIFIED vvv --- Use piecePlayerKeys --- vvv
                    int ppi = getPiecePlayerIndex(piece.type, piece.owner);
                    if (ppi != -1 && ppi < piecePlayerKeys.size() &&
                        r >= 0 && r < BOARD_ROWS && c >= 0 && c < BOARD_COLS)
                    {
                         hash ^= piecePlayerKeys[ppi][r][c];
                    } else {
                         std::cerr << "Warning: Invalid index during initial hash. Type: "
                                   << static_cast<int>(piece.type) << " Player: " << static_cast<int>(piece.owner)
                                   << " R: " << r << " C: " << c << std::endl;
                    }
                    //^^^ MODIFIED ^^^----------------------------^^^
                }
            }
        }
        if (currentPlayer == Player::PLAYER2) {
            hash ^= sideToMoveKey;
        }
        return hash;
    }

} // namespace Zobrist


