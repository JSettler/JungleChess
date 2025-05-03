#pragma once

#include "GameState.h" // Includes Common.h indirectly
#include "Common.h"
#include <limits>
#include <array>
#include <map>
#include <vector>
#include <cmath>  // Needed for std::abs
#include <iostream>
#include <algorithm> // Needed for std::max

namespace Evaluation {

    // --- Define Material Values ---
    inline int getPieceValue(PieceType type) {
        switch (type) {
            case PieceType::RAT:      return 6500; case PieceType::CAT:      return 3000;
            case PieceType::DOG:      return 4000; case PieceType::WOLF:     return 5000;
            case PieceType::LEOPARD:  return 6000; case PieceType::TIGER:    return 7500;
            case PieceType::LION:     return 8500; case PieceType::ELEPHANT: return 9000;
            case PieceType::EMPTY:    return 0;    default:                  return 0;
        }
    }

    // --- Define Score for Winning ---
    const int WIN_SCORE = 1000000;

    // --- Define Piece-Square Tables (PSTs) ---
    // Defined from Player 1's perspective (bottom is row 0). Will be flipped for Player 2.
    using PieceSquareTable = std::array<std::array<int, BOARD_COLS>, BOARD_ROWS>;

    // Rat: Encourage river, advancing (Unchanged)
    const PieceSquareTable pst_rat = {{
        {{-5,-5, 0, 0, 0,-5,-5}}, {{ 0, 0, 5, 5, 5, 0, 0}}, {{ 5, 5,10,10,10, 5, 5}},
        {{10,50,50,15,50,50,10}}, {{15,60,60,20,60,60,15}}, {{10,50,50,15,50,50,10}},
        {{ 5,10,15,20,15,10, 5}}, {{ 0, 5,10,15,10, 5, 0}}, {{ 0, 0, 5, 10, 5, 0, 0}}
    }};
    // Cat/Dog: Encourage staying near own traps/den (Unchanged)
    const PieceSquareTable pst_cat_dog = {{
        {{15,10,20,25,20,10,15}}, {{10,15,15,20,15,15,10}}, {{ 5, 5, 5, 5, 5, 5, 5}},
        {{ 0, 0, 0, 0, 0, 0, 0}}, {{-5,-5,-5,-5,-5,-5,-5}}, {{-5,-5,-5,-5,-5,-5,-5}},
        {{-10,-10,-5,-5,-5,-10,-10}}, {{-10,-10,-10,-10,-10,-10,-10}}, {{-15,-15,-10,-10,-10,-15,-15}}
    }};
    // Wolf: Defensive posture (Unchanged)
     const PieceSquareTable pst_wolf = {{
        {{ 5, 5, 5, 5, 5, 5, 5}}, {{10,10,10,10,10,10,10}}, {{15,15,15,15,15,15,15}},
        {{ 5, 5, 5, 5, 5, 5, 5}}, {{ 0, 0, 0, 0, 0, 0, 0}}, {{-5,-5,-5,-5,-5,-5,-5}},
        {{-10,-10,-10,-10,-10,-10,-10}}, {{-15,-15,-15,-15,-15,-15,-15}}, {{-20,-20,-15,-15,-15,-20,-20}}
    }};
    // Leopard: Encourage advancing, central control (Unchanged)
    const PieceSquareTable pst_leopard = {{
        {{ 0, 0, 0, 0, 0, 0, 0}}, {{ 0, 5, 5, 5, 5, 5, 0}}, {{ 0, 5,10,10,10, 5, 0}},
        {{ 5,10,15,15,15,10, 5}}, {{ 5,10,15,15,15,10, 5}}, {{10,15,20,20,20,15,10}},
        {{10,15,20,25,20,15,10}}, {{ 5,10,15,20,15,10, 5}}, {{ 0, 5,10,15,10, 5, 0}}
    }};

    //vvv MODIFIED vvv --- Separate LION PST with strong den attack values --- vvv
    // Row 8 = Opponent's Row 0 (Back Rank)
    // Row 7 = Opponent's Row 1 (Trap Row)
    // Row 6 = Opponent's Row 2
    const PieceSquareTable pst_lion = {{
        {{-10,-10,-10, -1,-10,-10,-10}}, // Row 0 (Own Back Rank) - Bad
        {{ -5, -5, -5, -5, -5, -5, -5}}, // Row 1
        {{  0,  0,  5,  5,  5,  0,  0}}, // Row 2
        {{  5, -1, -1, 20, -1, -1,  5}}, // Row 3 (River Edge)
        {{ 10, -1, -1, 25, -1, -1, 10}}, // Row 4 (River Middle)
        {{ 15, -1, -1, 90, -1, -1, 15}}, // Row 5 (River Edge - Attack Side)
        {{ 20, 30,100,220,100, 30, 20}}, // Row 6 (Opponent Territory)
        {{ 50,100,270,550,270,100, 50}}, // Row 7 (Opponent Trap Row - High Value) - Central trap (1,3) is table[7][3]
        {{  0,200,500,999,500,200,  0}}  // Row 8 (Opponent Back Rank - Very High Value) - Den (0,3) is table[8][3]
        // Note: The +3000 for the central trap is handled by the TrapControl logic now, not PST.
    }};
    //^^^ MODIFIED ^^^--------------------------------------------------------^^^

    //vvv MODIFIED vvv --- Separate TIGER PST, strong but less den-focused than Lion --- vvv
    const PieceSquareTable pst_tiger = {{
        {{-10,-10,-10, -1,-10,-10,-10}}, // Row 0 (Own Back Rank) - Bad
        {{ -5, -5, -5, -5, -5, -5, -5}}, // Row 1
        {{  0,  0,  5,  5,  5,  0,  0}}, // Row 2
        {{  5, -1, -1, 20, -1, -1,  5}}, // Row 3 (River Edge)
        {{ 10, -1, -1, 25, -1, -1, 10}}, // Row 4 (River Middle)
        {{ 20, -1, -1,100, -1, -1, 20}}, // Row 5 (River Edge - Attack Side - Stronger than Lion here? [Nope - the tiger is inferior!])
        {{ 25, 40, 95,200, 95, 40, 25}}, // Row 6 (Opponent Territory)
        {{ 40, 80,230,450,230, 80, 40}}, // Row 7 (Opponent Trap Row - Still high value)
        {{  0,100,400,999,400,100,  0}}  // Row 8 (Opponent Back Rank - High but less than Lion)
    }};
    //^^^ MODIFIED ^^^-----------------------------------------------------------^^^

    //vvv MODIFIED vvv --- Revised Elephant PST (Stronger Center) --- vvv
    const PieceSquareTable pst_elephant = {{
        {{-30,-30,-25, -1,-25,-30,-30}}, // Row 0 (Own Back Rank) - Very Bad
        {{-15,-15,-10, -5,-10,-15,-15}}, // Row 1 - Still bad
        {{ -5,  0, 10, 20, 10,  0, -5}}, // Row 2 - Encouraging forward/center
        {{  5, -1, -1, 60, -1, -1,  5}}, // Row 3 (River Edge) - Strong central presence
        {{ 10, -1, -1, 70, -1, -1, 10}}, // Row 4 (River Middle) - Peak central control value (significant bonus)
        {{ 10, -1, -1, 80, -1, -1, 10}}, // Row 5 (River Edge - Attack Side) - Still very strong center
        {{ 10, 15, 50,150, 50, 15, 10}}, // Row 6 (Opponent Territory) - Positive, but less than center
        {{  0, 25,200,400,200, 25,  0}}, // Row 7 (Opponent Trap Row) - Modest value, avoid traps
        {{  0, 50,350,999,350, 50,  0}}  // Row 8 (Opponent Back Rank) - Low value, generally avoid
    }};
    //^^^ MODIFIED ^^^------------------------------------------------^^^


    // --- Function to get PST value ---
    //vvv MODIFIED vvv --- Use separate L/T tables --- vvv
    inline int getPstValue(PieceType type, int r, int c, Player player) {
        int table_r = (player == Player::PLAYER1) ? r : (BOARD_ROWS - 1 - r);
        if (table_r < 0 || table_r >= BOARD_ROWS || c < 0 || c >= BOARD_COLS) return 0;
        switch (type) {
            case PieceType::RAT:      return pst_rat[table_r][c];
            case PieceType::CAT:      case PieceType::DOG: return pst_cat_dog[table_r][c];
            case PieceType::WOLF:     return pst_wolf[table_r][c];
            case PieceType::LEOPARD:  return pst_leopard[table_r][c];
            case PieceType::TIGER:    return pst_tiger[table_r][c]; // Use Tiger PST
            case PieceType::LION:     return pst_lion[table_r][c];  // Use Lion PST
            case PieceType::ELEPHANT: return pst_elephant[table_r][c];
            default:                  return 0;
        }
    }
    //^^^ MODIFIED ^^^---------------------------------^^^


    // --- Evaluation Weights ---
    const int MATERIAL_WEIGHT_MULTIPLIER = 2;
    const int MOBILITY_WEIGHT = 5; // Still defined, just not used in evaluateBoard below
    const int LION_PROXIMITY_WEIGHT = 40; // Specific bonus for AI Lion near opponent den (redundant now? Keep for now)
    const int ELEPHANT_TRAP_PENALTY = 3000;
    const int ELEPHANT_EDGE_THRESHOLD = 1;
    const int RAT_PROXIMITY_THRESHOLD = 3;
    const double TRAPPED_CORNER_MALUS_PCT = 0.75;
    const double TRAPPED_DIST1_MALUS_PCT = 0.60;
    const double TRAPPED_DIST2_MALUS_PCT = 0.50;
    const int RAT_INTERCEPT_MAX_BONUS = 1000;
    const int DEN_SAFETY_MAX_DIST = 4;
    const int DEN_SAFETY_BASE_SCORE = 100;
    const double DEN_SAFETY_COUNT_MULTIPLIER = 1.5;
    const int TRAP_CONTROL_SUICIDE_PENALTY_PCT = 95; // Pct of piece value
    const int TRAP_CONTROL_WINNING_BONUS = 2500;     // Bonus if supporter >= defender
    const int TRAP_CONTROL_SAFE_BONUS = 1500;        // Bonus if no defenders adjacent


    // --- Static Board Evaluation Function ---
    inline int evaluateBoard(const GameState& gameState) {
        int materialScore = 0; int positionalScorePST = 0;
        int lionProximityScore = 0; int elephantTrapPenalty = 0;
        int trappedPieceMalus = 0; int ratInterceptBonus = 0;
        int ai_den_threat_score = 0; int opponent_den_threat_score = 0;
        int ai_pieces_near_opponent_den = 0; int opponent_pieces_near_ai_den = 0;
        int trapControlScore = 0;

        int aiElephantRow = -1, aiElephantCol = -1;
        int humanRatRow = -1, humanRatCol = -1;

        // --- Calculate Component Scores ---
        for (int r = 0; r < BOARD_ROWS; ++r) {
            for (int c = 0; c < BOARD_COLS; ++c) {
                Piece piece = gameState.getPiece(r, c);
                if (piece.type != PieceType::EMPTY) {
                    int basePieceValue = getPieceValue(piece.type);
                    int pstValue = getPstValue(piece.type, r, c, piece.owner); // Uses updated getPstValue
                    Player owner = piece.owner;
                    Player opponent = (owner == Player::PLAYER1) ? Player::PLAYER2 : Player::PLAYER1;

                    // Check if piece is in an opponent's trap for Trap Control Score
                    bool isInOpponentTrap = gameState.isOwnTrap(r, c, opponent);
                    int currentTrapAdjustment = 0;
                    if (isInOpponentTrap) {
                        int defenderCount = 0; int maxDefenderRank = 0; int maxSupportRank = 0;
                        int adjacentOffsets[][2] = {{0, 1}, {0, -1}, {1, 0}, {-1, 0}};
                        for (const auto& offset : adjacentOffsets) {
                            int adj_r = r + offset[0]; int adj_c = c + offset[1];
                            if (gameState.isValidPosition(adj_r, adj_c) && !gameState.isRiver(adj_r, adj_c)) {
                                Piece adjPiece = gameState.getPiece(adj_r, adj_c);
                                if (adjPiece.owner == opponent) { defenderCount++; maxDefenderRank = std::max(maxDefenderRank, gameState.getRank(adjPiece.type)); }
                                else if (adjPiece.owner == owner) { maxSupportRank = std::max(maxSupportRank, gameState.getRank(adjPiece.type)); }
                            }
                        }
                        // Apply logic based on defenders/supporters
                        if (defenderCount >= 2) { currentTrapAdjustment = -(basePieceValue * TRAP_CONTROL_SUICIDE_PENALTY_PCT / 100); }
                        else if (defenderCount == 1) {
                            if (maxSupportRank > 0) { // Support exists
                                if (maxSupportRank >= maxDefenderRank) { currentTrapAdjustment = TRAP_CONTROL_WINNING_BONUS; }
                                else { currentTrapAdjustment = -(basePieceValue * 90 / 100); } // Supporter weaker
                            } else { currentTrapAdjustment = -(basePieceValue * 90 / 100); } // No support
                        } else { currentTrapAdjustment = TRAP_CONTROL_SAFE_BONUS; } // No defenders

                        if (owner == Player::PLAYER2) { trapControlScore += currentTrapAdjustment; }
                        else { trapControlScore -= currentTrapAdjustment; }
                    } // End if isInOpponentTrap

                    // --- Other Evaluation Terms ---
                    if (owner == Player::PLAYER2) { // AI's piece
                        materialScore += basePieceValue;
                        positionalScorePST += pstValue;
                        // Lion Proximity Bonus (Still keep this specific one? Maybe reduce weight?)
                        if (piece.type == PieceType::LION) { lionProximityScore += LION_PROXIMITY_WEIGHT * (BOARD_ROWS - 1 - r); }
                        else if (piece.type == PieceType::ELEPHANT) { aiElephantRow = r; aiElephantCol = c; }
                        // Trapped Piece Malus (Near corner, diagonally blocked, NOT in trap)
                        else if (!isInOpponentTrap && (piece.type == PieceType::LION || piece.type == PieceType::TIGER || piece.type == PieceType::ELEPHANT)) {
                            int corner_r = 0; int corner_c = -1; int trap_r = r + 1; int trap_c = -1;
                            if (c <= 3) { corner_c = 0; trap_c = c + 1; } else { corner_c = 6; trap_c = c - 1; }
                            int dist = std::abs(r - corner_r) + std::abs(c - corner_c);
                            if (dist <= 2) {
                                if (gameState.isValidPosition(trap_r, trap_c)) {
                                    Piece trapperPiece = gameState.getPiece(trap_r, trap_c);
                                    if (trapperPiece.owner == Player::PLAYER1 && gameState.getRank(trapperPiece.type) >= gameState.getRank(piece.type)) {
                                        double penalty_pct = 0.0;
                                        if (dist == 0) penalty_pct = TRAPPED_CORNER_MALUS_PCT;
                                        else if (dist == 1) penalty_pct = TRAPPED_DIST1_MALUS_PCT;
                                        else if (dist == 2) penalty_pct = TRAPPED_DIST2_MALUS_PCT;
                                        trappedPieceMalus -= static_cast<int>(basePieceValue * penalty_pct);
                                    }
                                }
                            }
                        }
                        // Rat Intercept Bonus
                        else if (piece.type == PieceType::RAT && !gameState.isRiver(r, c)) { /* ... Rat intercept logic ... */ }
                        // Den Safety Threat (AI attacking opponent den)
                        if (piece.type == PieceType::LION || piece.type == PieceType::TIGER || piece.type == PieceType::ELEPHANT || piece.type == PieceType::RAT) {
                            int dist_to_opponent_den = std::abs(r - 0) + std::abs(c - 3);
                            if (dist_to_opponent_den <= DEN_SAFETY_MAX_DIST) {
                                ai_pieces_near_opponent_den++;
                                int piece_threat = DEN_SAFETY_BASE_SCORE * (DEN_SAFETY_MAX_DIST - dist_to_opponent_den + 1);
                                opponent_den_threat_score += piece_threat;
                            }
                        }

                    } else { // Human's piece (Player::PLAYER1)
                        materialScore -= basePieceValue;
                        positionalScorePST -= pstValue;
                        // Lion Proximity Penalty
                        if (piece.type == PieceType::LION) { lionProximityScore += -LION_PROXIMITY_WEIGHT * r; }
                        else if (piece.type == PieceType::RAT) { humanRatRow = r; humanRatCol = c; }
                        // Den Safety Threat (Human attacking AI den)
                        if (piece.type == PieceType::LION || piece.type == PieceType::TIGER || piece.type == PieceType::ELEPHANT || piece.type == PieceType::RAT) {
                            int dist_to_ai_den = std::abs(r - (BOARD_ROWS - 1)) + std::abs(c - 3);
                            if (dist_to_ai_den <= DEN_SAFETY_MAX_DIST) {
                                opponent_pieces_near_ai_den++;
                                int piece_threat = DEN_SAFETY_BASE_SCORE * (DEN_SAFETY_MAX_DIST - dist_to_ai_den + 1);
                                ai_den_threat_score += piece_threat;
                            }
                        }
                    }
                }
            }
        }

        // --- Mobility Score (COMMENTED OUT FOR PERFORMANCE) ---
        // int aiMoves = gameState.getAllLegalMoves(Player::PLAYER2).size();
        // int humanMoves = gameState.getAllLegalMoves(Player::PLAYER1).size();
        // int mobilityScore = MOBILITY_WEIGHT * (aiMoves - humanMoves);

        // --- Elephant Trap Penalty Calculation ---
        if (aiElephantRow != -1 && humanRatRow != -1) { /* ... elephant trap logic ... */ }

        // --- Scale Den Threat Scores by Count ---
        if (opponent_pieces_near_ai_den > 1) { ai_den_threat_score = static_cast<int>(ai_den_threat_score * (1.0 + DEN_SAFETY_COUNT_MULTIPLIER * (opponent_pieces_near_ai_den - 1))); }
        if (ai_pieces_near_opponent_den > 1) { opponent_den_threat_score = static_cast<int>(opponent_den_threat_score * (1.0 + DEN_SAFETY_COUNT_MULTIPLIER * (ai_pieces_near_opponent_den - 1))); }


        // --- Combine Scores ---
        return (materialScore * MATERIAL_WEIGHT_MULTIPLIER)
               + positionalScorePST // Now includes separate L/T values
               // + mobilityScore // COMMENTED OUT FOR PERFORMANCE
               // + lionProximityScore // Maybe remove this now PSTs are stronger? Keep for now.
               + elephantTrapPenalty
               + trappedPieceMalus
               + ratInterceptBonus
               + trapControlScore // Added new trap control term
               + opponent_den_threat_score // Den Safety bonus for AI
               - ai_den_threat_score;      // Den Safety penalty for Human
    }

} // namespace Evaluation


