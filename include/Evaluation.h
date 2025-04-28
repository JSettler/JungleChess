#pragma once

#include "GameState.h"
#include "Common.h"
#include <limits>
#include <array>
#include <map>
#include <vector>
#include <cmath>  // Needed for std::abs

namespace Evaluation {

    // --- Define Material Values ---
    inline int getPieceValue(PieceType type) {
        //vvv RESTORED BODY vvv
        switch (type) {
            case PieceType::RAT:      return 6500;
            case PieceType::CAT:      return 3000;
            case PieceType::DOG:      return 4000;
            case PieceType::WOLF:     return 5000;
            case PieceType::LEOPARD:  return 6000;
            case PieceType::TIGER:    return 7500;
            case PieceType::LION:     return 8500;
            case PieceType::ELEPHANT: return 9000;
            case PieceType::EMPTY:    return 0;
            default:                  return 0; // Default case ensures return
        }
        //^^^ RESTORED BODY ^^^
    }

    // --- Define Score for Winning ---
    const int WIN_SCORE = 1000000;

    // --- Define Piece-Square Tables (PSTs) ---
    using PieceSquareTable = std::array<std::array<int, BOARD_COLS>, BOARD_ROWS>;
    // (Assuming these are defined correctly as in the previous working step)
    const PieceSquareTable pst_rat = {{
        {{-5,-5, 0, 0, 0,-5,-5}}, {{ 0, 0, 5, 5, 5, 0, 0}}, {{ 5, 5,10,10,10, 5, 5}},
        {{10,50,50,15,50,50,10}}, {{15,60,60,20,60,60,15}}, {{10,50,50,15,50,50,10}},
        {{ 5,10,15,20,15,10, 5}}, {{ 0, 5,10,15,10, 5, 0}}, {{ 0, 0, 5, 10, 5, 0, 0}}
    }};
    const PieceSquareTable pst_cat_dog = {{
        {{15,10,20,25,20,10,15}}, {{10,15,15,20,15,15,10}}, {{ 5, 5, 5, 5, 5, 5, 5}},
        {{ 0, 0, 0, 0, 0, 0, 0}}, {{-5,-5,-5,-5,-5,-5,-5}}, {{-5,-5,-5,-5,-5,-5,-5}},
        {{-10,-10,-5,-5,-5,-10,-10}}, {{-10,-10,-10,-10,-10,-10,-10}}, {{-15,-15,-10,-10,-10,-15,-15}}
    }};
     const PieceSquareTable pst_wolf = {{
        {{ 5, 5, 5, 5, 5, 5, 5}}, {{10,10,10,10,10,10,10}}, {{15,15,15,15,15,15,15}},
        {{ 5, 5, 5, 5, 5, 5, 5}}, {{ 0, 0, 0, 0, 0, 0, 0}}, {{-5,-5,-5,-5,-5,-5,-5}},
        {{-10,-10,-10,-10,-10,-10,-10}}, {{-15,-15,-15,-15,-15,-15,-15}}, {{-20,-20,-15,-15,-15,-20,-20}}
    }};
    const PieceSquareTable pst_leopard = {{
        {{ 0, 0, 0, 0, 0, 0, 0}}, {{ 0, 5, 5, 5, 5, 5, 0}}, {{ 0, 5,10,10,10, 5, 0}},
        {{ 5,10,15,15,15,10, 5}}, {{ 5,10,15,15,15,10, 5}}, {{10,15,20,20,20,15,10}},
        {{10,15,20,25,20,15,10}}, {{ 5,10,15,20,15,10, 5}}, {{ 0, 5,10,15,10, 5, 0}}
    }};
    const PieceSquareTable pst_lion_tiger = {{
        {{-5,-5,-5,-5,-5,-5,-5}}, {{ 0, 0, 0, 0, 0, 0, 0}}, {{ 5, 5, 5, 5, 5, 5, 5}},
        {{10,10,10,10,10,10,10}}, {{10,10,10,10,10,10,10}}, {{15,15,15,15,15,15,15}},
        {{20,20,25,25,25,20,20}}, {{20,25,30,35,30,25,20}}, {{15,20,25,30,25,20,15}}
    }};
    const PieceSquareTable pst_elephant = {{
        {{-10,-10,-5, 0,-5,-10,-10}}, {{ -5, -5, 0, 5, 0, -5, -5}}, {{  0,  0, 5,10, 5,  0,  0}},
        {{  0,  5,10,15,10,  5,  0}}, {{  5, 10,15,20,15, 10,  5}}, {{  5, 10,15,20,15, 10,  5}},
        {{  0,  5,10,15,10,  5,  0}}, {{  0,  0, 5,10, 5,  0,  0}}, {{ -5,  0, 0, 5, 0,  0, -5}}
    }};


    // --- Function to get PST value ---
    inline int getPstValue(PieceType type, int r, int c, Player player) {
        //vvv RESTORED BODY vvv
        int table_r = (player == Player::PLAYER1) ? r : (BOARD_ROWS - 1 - r);
        if (table_r < 0 || table_r >= BOARD_ROWS || c < 0 || c >= BOARD_COLS) return 0;

        switch (type) {
            case PieceType::RAT:      return pst_rat[table_r][c];
            case PieceType::CAT:      // Fallthrough
            case PieceType::DOG:      return pst_cat_dog[table_r][c];
            case PieceType::WOLF:     return pst_wolf[table_r][c];
            case PieceType::LEOPARD:  return pst_leopard[table_r][c];
            case PieceType::TIGER:    // Fallthrough
            case PieceType::LION:     return pst_lion_tiger[table_r][c];
            case PieceType::ELEPHANT: return pst_elephant[table_r][c];
            case PieceType::EMPTY:    // Fallthrough
            default:                  return 0; // Default case ensures return
        }
        //^^^ RESTORED BODY ^^^
    }

    // --- Evaluation Weights ---
    const int MOBILITY_WEIGHT = 5;
    const int LION_PROXIMITY_WEIGHT = 40;
    const int ELEPHANT_TRAP_PENALTY = 3000;
    const int ELEPHANT_EDGE_THRESHOLD = 1;
    const int RAT_PROXIMITY_THRESHOLD = 3;


    // --- Static Board Evaluation Function (Includes Elephant Trap Logic) ---
    inline int evaluateBoard(const GameState& gameState) {
        int materialScore = 0;
        int positionalScorePST = 0;
        int lionProximityScore = 0;
        int aiElephantRow = -1, aiElephantCol = -1;
        int humanRatRow = -1, humanRatCol = -1;

        // Material, PST, Lion Proximity, Find Key Pieces
        for (int r = 0; r < BOARD_ROWS; ++r) {
            for (int c = 0; c < BOARD_COLS; ++c) {
                Piece piece = gameState.getPiece(r, c);
                if (piece.type != PieceType::EMPTY) {
                    int pieceValue = getPieceValue(piece.type);
                    int pstValue = getPstValue(piece.type, r, c, piece.owner);
                    if (piece.owner == Player::PLAYER2) { // AI
                        materialScore += pieceValue; positionalScorePST += pstValue;
                        if (piece.type == PieceType::LION) lionProximityScore += LION_PROXIMITY_WEIGHT * (BOARD_ROWS - 1 - r);
                        else if (piece.type == PieceType::ELEPHANT) { aiElephantRow = r; aiElephantCol = c; }
                    } else { // Human
                        materialScore -= pieceValue; positionalScorePST -= pstValue;
                        if (piece.type == PieceType::LION) lionProximityScore += -LION_PROXIMITY_WEIGHT * r;
                        else if (piece.type == PieceType::RAT) { humanRatRow = r; humanRatCol = c; }
                    }
                }
            }
        }

        // Mobility Score
        int aiMoves = gameState.getAllLegalMoves(Player::PLAYER2).size();
        int humanMoves = gameState.getAllLegalMoves(Player::PLAYER1).size();
        int mobilityScore = MOBILITY_WEIGHT * (aiMoves - humanMoves);

        // Elephant Trap Penalty Calculation
        int elephantTrapPenalty = 0;
        if (aiElephantRow != -1 && humanRatRow != -1) {
            bool nearEdge = (aiElephantRow <= ELEPHANT_EDGE_THRESHOLD || aiElephantRow >= BOARD_ROWS - 1 - ELEPHANT_EDGE_THRESHOLD ||
                             aiElephantCol <= ELEPHANT_EDGE_THRESHOLD || aiElephantCol >= BOARD_COLS - 1 - ELEPHANT_EDGE_THRESHOLD);
            if (nearEdge) {
                int dist = std::abs(aiElephantRow - humanRatRow) + std::abs(aiElephantCol - humanRatCol);
                if (dist <= RAT_PROXIMITY_THRESHOLD) {
                    elephantTrapPenalty = -ELEPHANT_TRAP_PENALTY;
                }
            }
        }

        // Combine Scores
        return materialScore + positionalScorePST + mobilityScore + lionProximityScore + elephantTrapPenalty;
    }

} // namespace Evaluation


