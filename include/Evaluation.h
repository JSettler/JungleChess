#pragma once

#include "GameState.h" // Includes Common.h indirectly
#include "Common.h"
#include <limits>
#include <array>
#include <map>    // Keep includes, though map isn't used directly here
#include <vector>
#include <cmath>  // Needed for std::abs
#include <iostream> // Keep for potential debug/error output

namespace Evaluation {

    // --- Define Material Values (BASE VALUES - DO NOT CHANGE THESE) ---
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
    const int WIN_SCORE = 1000000; // Keep this large

    // --- Define Piece-Square Tables (PSTs) ---
    // (Values here are positional bonuses/penalties, NOT scaled by material multiplier)
    using PieceSquareTable = std::array<std::array<int, BOARD_COLS>, BOARD_ROWS>;
    const PieceSquareTable pst_rat = {{ // Encourage river, advancing
        {{-5,-5, 0, 0, 0,-5,-5}}, {{ 0, 0, 5, 5, 5, 0, 0}}, {{ 5, 5,10,10,10, 5, 5}},
        {{10,50,50,15,50,50,10}}, {{15,60,60,20,60,60,15}}, {{10,50,50,15,50,50,10}},
        {{ 5,10,15,20,15,10, 5}}, {{ 0, 5,10,15,10, 5, 0}}, {{ 0, 0, 5, 10, 5, 0, 0}}
    }};
    const PieceSquareTable pst_cat_dog = {{ // Encourage staying near own traps/den
        {{15,10,20,25,20,10,15}}, {{10,15,15,20,15,15,10}}, {{ 5, 5, 5, 5, 5, 5, 5}},
        {{ 0, 0, 0, 0, 0, 0, 0}}, {{-5,-5,-5,-5,-5,-5,-5}}, {{-5,-5,-5,-5,-5,-5,-5}},
        {{-10,-10,-5,-5,-5,-10,-10}}, {{-10,-10,-10,-10,-10,-10,-10}}, {{-15,-15,-10,-10,-10,-15,-15}}
    }};
     const PieceSquareTable pst_wolf = {{ // Defensive posture
        {{ 5, 5, 5, 5, 5, 5, 5}}, {{10,10,10,10,10,10,10}}, {{15,15,15,15,15,15,15}},
        {{ 5, 5, 5, 5, 5, 5, 5}}, {{ 0, 0, 0, 0, 0, 0, 0}}, {{-5,-5,-5,-5,-5,-5,-5}},
        {{-10,-10,-10,-10,-10,-10,-10}}, {{-15,-15,-15,-15,-15,-15,-15}}, {{-20,-20,-15,-15,-15,-20,-20}}
    }};
    const PieceSquareTable pst_leopard = {{ // Encourage advancing, central control
        {{ 0, 0, 0, 0, 0, 0, 0}}, {{ 0, 5, 5, 5, 5, 5, 0}}, {{ 0, 5,10,10,10, 5, 0}},
        {{ 5,10,15,15,15,10, 5}}, {{ 5,10,15,15,15,10, 5}}, {{10,15,20,20,20,15,10}},
        {{10,15,20,25,20,15,10}}, {{ 5,10,15,20,15,10, 5}}, {{ 0, 5,10,15,10, 5, 0}}
    }};
    const PieceSquareTable pst_lion_tiger = {{ // Encourage aggressive advance towards enemy den/traps
        {{-5,-5,-5,-5,-5,-5,-5}}, {{ 0, 0, 0, 0, 0, 0, 0}}, {{ 5, 5, 5, 5, 5, 5, 5}},
        {{10,10,10,10,10,10,10}}, {{10,10,10,10,10,10,10}}, {{15,15,15,15,15,15,15}},
        {{20,20,25,25,25,20,20}}, {{20,25,30,35,30,25,20}}, {{15,20,25,30,25,20,15}}
    }};
    const PieceSquareTable pst_elephant = {{ // Encourage advancing, avoid own back rank
        {{-10,-10,-5, 0,-5,-10,-10}}, {{ -5, -5, 0, 5, 0, -5, -5}}, {{  0,  0, 5,10, 5,  0,  0}},
        {{  0,  5,10,15,10,  5,  0}}, {{  5, 10,15,20,15, 10,  5}}, {{  5, 10,15,20,15, 10,  5}},
        {{  0,  5,10,15,10,  5,  0}}, {{  0,  0, 5,10, 5,  0,  0}}, {{ -5,  0, 0, 5, 0,  0, -5}}
    }};


    // --- Function to get PST value ---
    inline int getPstValue(PieceType type, int r, int c, Player player) {
        // Implementation unchanged, returns base PST values
        int table_r = (player == Player::PLAYER1) ? r : (BOARD_ROWS - 1 - r);
        if (table_r < 0 || table_r >= BOARD_ROWS || c < 0 || c >= BOARD_COLS) return 0;
        switch (type) {
            case PieceType::RAT:      return pst_rat[table_r][c];
            case PieceType::CAT:      case PieceType::DOG: return pst_cat_dog[table_r][c];
            case PieceType::WOLF:     return pst_wolf[table_r][c];
            case PieceType::LEOPARD:  return pst_leopard[table_r][c];
            case PieceType::TIGER:    case PieceType::LION: return pst_lion_tiger[table_r][c];
            case PieceType::ELEPHANT: return pst_elephant[table_r][c];
            default:                  return 0;
        }
    }

    // --- Evaluation Weights ---
    const int MATERIAL_WEIGHT_MULTIPLIER = 2; // Double the impact of material score
    const int MOBILITY_WEIGHT = 5;
    const int LION_PROXIMITY_WEIGHT = 40; // Specific bonus for AI Lion near opponent den
    const int ELEPHANT_TRAP_PENALTY = 3000; // For Rat near Elephant near edge
    const int ELEPHANT_EDGE_THRESHOLD = 1;
    const int RAT_PROXIMITY_THRESHOLD = 3;
    const double TRAPPED_CORNER_MALUS_PCT = 0.75; // Uses BASE piece value
    const double TRAPPED_DIST1_MALUS_PCT = 0.60;  // Uses BASE piece value
    const double TRAPPED_DIST2_MALUS_PCT = 0.50;  // Uses BASE piece value
    const int RAT_INTERCEPT_MAX_BONUS = 1000;     // For AI Rat safely blocking Elephant
    const int DEN_SAFETY_MAX_DIST = 4;      // Max Manhattan distance to check
    const int DEN_SAFETY_BASE_SCORE = 100;  // Base score for piece at dist 0
    const double DEN_SAFETY_COUNT_MULTIPLIER = 1.5; // Multiplier for >1 threatening piece


    // --- Static Board Evaluation Function ---
    inline int evaluateBoard(const GameState& gameState) {
        int materialScore = 0; // Calculated using BASE values
        int positionalScorePST = 0;
        int lionProximityScore = 0;
        int elephantTrapPenalty = 0;
        int trappedPieceMalus = 0; // Calculated using BASE values
        int ratInterceptBonus = 0;
        int ai_den_threat_score = 0;      // Accumulated threat score against AI den (negative impact)
        int opponent_den_threat_score = 0;// Accumulated threat score against Opponent den (positive impact)
        int ai_pieces_near_opponent_den = 0;
        int opponent_pieces_near_ai_den = 0;

        int aiElephantRow = -1, aiElephantCol = -1;
        int humanRatRow = -1, humanRatCol = -1;

        // --- Calculate Component Scores ---
        for (int r = 0; r < BOARD_ROWS; ++r) {
            for (int c = 0; c < BOARD_COLS; ++c) {
                Piece piece = gameState.getPiece(r, c);
                if (piece.type != PieceType::EMPTY) {
                    // Use BASE value for material and potentially dependent terms
                    int basePieceValue = getPieceValue(piece.type);
                    int pstValue = getPstValue(piece.type, r, c, piece.owner);

                    if (piece.owner == Player::PLAYER2) { // AI's piece (Assumed attacking towards row 0)
                        materialScore += basePieceValue; // Add base value
                        positionalScorePST += pstValue;

                        // Lion Proximity Bonus (Specific for AI Lion near Opponent Den at (0,3))
                        if (piece.type == PieceType::LION) { lionProximityScore += LION_PROXIMITY_WEIGHT * (BOARD_ROWS - 1 - r); }
                        // Store Elephant location
                        else if (piece.type == PieceType::ELEPHANT) { aiElephantRow = r; aiElephantCol = c; }
                        // Check for Trapped AI Piece Pattern (L/T/E near opponent corner)
                        else if (piece.type == PieceType::LION || piece.type == PieceType::TIGER || piece.type == PieceType::ELEPHANT) {
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
                                        trappedPieceMalus -= static_cast<int>(basePieceValue * penalty_pct); // Use base value
                                    }
                                }
                            }
                        }
                        // Check for Rat Intercepting Elephant
                        else if (piece.type == PieceType::RAT && !gameState.isRiver(r, c)) {
                            int rat_r = r; int rat_c = c; int elephant_r = -1; bool elephantFound = false;
                            int check_r = rat_r + 1;
                            for (int check_c_offset = -1; check_c_offset <= 1; ++check_c_offset) {
                                int check_c = rat_c + check_c_offset;
                                if (gameState.isValidPosition(check_r, check_c)) {
                                    Piece potentialElephant = gameState.getPiece(check_r, check_c);
                                    if (potentialElephant.owner == Player::PLAYER1 && potentialElephant.type == PieceType::ELEPHANT) {
                                        elephant_r = check_r; elephantFound = true; break;
                                    }
                                }
                            }
                            if (elephantFound) {
                                bool isSafe = true;
                                // Check adjacent threats
                                for (int dr = -1; dr <= 1 && isSafe; ++dr) for (int dc = -1; dc <= 1 && isSafe; ++dc) {
                                    if (dr == 0 && dc == 0) continue; int adj_r = rat_r + dr; int adj_c = rat_c + dc;
                                    if (gameState.isValidPosition(adj_r, adj_c)) {
                                        Piece adjPiece = gameState.getPiece(adj_r, adj_c);
                                        if (adjPiece.owner == Player::PLAYER1 && adjPiece.type != PieceType::ELEPHANT && gameState.getRank(adjPiece.type) > gameState.getRank(PieceType::RAT)) { isSafe = false; }
                                    }
                                }
                                // Check jump threats
                                if (isSafe) {
                                    int jumpSources[][2] = {{rat_r, rat_c - 3}, {rat_r, rat_c + 3}, {rat_r - 4, rat_c}, {rat_r + 4, rat_c}};
                                    for(const auto& src : jumpSources) {
                                        int jump_r = src[0]; int jump_c = src[1];
                                        if (gameState.isValidPosition(jump_r, jump_c)) {
                                            Piece jumper = gameState.getPiece(jump_r, jump_c);
                                            if (jumper.owner == Player::PLAYER1 && (jumper.type == PieceType::LION || jumper.type == PieceType::TIGER)) {
                                                bool pathClear = true;
                                                if (jump_r == rat_r) { int step = (jump_c < rat_c) ? 1 : -1; for (int i = 1; i <= 2; ++i) { int path_c = jump_c + i*step; if (!gameState.isValidPosition(rat_r, path_c) || !gameState.isRiver(rat_r, path_c) || gameState.getPiece(rat_r, path_c).type != PieceType::EMPTY) { pathClear = false; break; } } }
                                                else { int step = (jump_r < rat_r) ? 1 : -1; for (int i = 1; i <= 3; ++i) { int path_r = jump_r + i*step; if (!gameState.isValidPosition(path_r, rat_c) || !gameState.isRiver(path_r, rat_c) || gameState.getPiece(path_r, rat_c).type != PieceType::EMPTY) { pathClear = false; break; } } }
                                                if (pathClear) { isSafe = false; break; }
                                            }
                                        }
                                    }
                                }
                                // Apply bonus if safe
                                if (isSafe) {
                                    int proximity = (BOARD_ROWS - 1 - elephant_r);
                                    double bonus = RAT_INTERCEPT_MAX_BONUS * (static_cast<double>(proximity) / (BOARD_ROWS > 1 ? static_cast<double>(BOARD_ROWS - 1) : 1.0));
                                    ratInterceptBonus += static_cast<int>(bonus);
                                }
                            }
                        } // End Rat Intercept Check

                        // Check for AI Den Threat/Attack
                        if (piece.type == PieceType::LION || piece.type == PieceType::TIGER ||
                            piece.type == PieceType::ELEPHANT || piece.type == PieceType::RAT)
                        {
                            int dist_to_opponent_den = std::abs(r - 0) + std::abs(c - 3); // Opponent den at (0,3)
                            if (dist_to_opponent_den <= DEN_SAFETY_MAX_DIST) {
                                ai_pieces_near_opponent_den++;
                                int piece_threat = DEN_SAFETY_BASE_SCORE * (DEN_SAFETY_MAX_DIST - dist_to_opponent_den + 1);
                                opponent_den_threat_score += piece_threat;
                            }
                        }


                    } else { // Human's piece (Player::PLAYER1)
                        materialScore -= basePieceValue; // Subtract base value
                        positionalScorePST -= pstValue;

                        // Lion Proximity Penalty (Specific for Human Lion near AI Den at (8,3))
                        if (piece.type == PieceType::LION) { lionProximityScore += -LION_PROXIMITY_WEIGHT * r; }
                        // Store Rat location
                        else if (piece.type == PieceType::RAT) { humanRatRow = r; humanRatCol = c; }
                        // Check for Human Den Threat/Attack
                        if (piece.type == PieceType::LION || piece.type == PieceType::TIGER ||
                            piece.type == PieceType::ELEPHANT || piece.type == PieceType::RAT)
                        {
                            int dist_to_ai_den = std::abs(r - (BOARD_ROWS - 1)) + std::abs(c - 3); // AI den at (8,3)
                            if (dist_to_ai_den <= DEN_SAFETY_MAX_DIST) {
                                opponent_pieces_near_ai_den++;
                                int piece_threat = DEN_SAFETY_BASE_SCORE * (DEN_SAFETY_MAX_DIST - dist_to_ai_den + 1);
                                ai_den_threat_score += piece_threat; // Add to score that will be subtracted
                            }
                        }
                    }
                }
            }
        }

        // --- Mobility Score ---
        int aiMoves = gameState.getAllLegalMoves(Player::PLAYER2).size();
        int humanMoves = gameState.getAllLegalMoves(Player::PLAYER1).size();
        int mobilityScore = MOBILITY_WEIGHT * (aiMoves - humanMoves);

        // --- Elephant Trap Penalty Calculation ---
        if (aiElephantRow != -1 && humanRatRow != -1) {
            bool nearEdge = (aiElephantRow <= ELEPHANT_EDGE_THRESHOLD || aiElephantRow >= BOARD_ROWS - 1 - ELEPHANT_EDGE_THRESHOLD ||
                             aiElephantCol <= ELEPHANT_EDGE_THRESHOLD || aiElephantCol >= BOARD_COLS - 1 - ELEPHANT_EDGE_THRESHOLD);
            if (nearEdge) {
                int dist = std::abs(aiElephantRow - humanRatRow) + std::abs(aiElephantCol - humanRatCol);
                if (dist <= RAT_PROXIMITY_THRESHOLD) { elephantTrapPenalty = -ELEPHANT_TRAP_PENALTY; }
            }
        }

        // --- Scale Den Threat Scores by Count ---
        if (opponent_pieces_near_ai_den > 1) { ai_den_threat_score = static_cast<int>(ai_den_threat_score * (1.0 + DEN_SAFETY_COUNT_MULTIPLIER * (opponent_pieces_near_ai_den - 1))); }
        if (ai_pieces_near_opponent_den > 1) { opponent_den_threat_score = static_cast<int>(opponent_den_threat_score * (1.0 + DEN_SAFETY_COUNT_MULTIPLIER * (ai_pieces_near_opponent_den - 1))); }


        // --- Combine Scores ---
        // Score is from AI's perspective (Player 2)
        return (materialScore * MATERIAL_WEIGHT_MULTIPLIER) // Apply Material Multiplier HERE
               + positionalScorePST
               + mobilityScore
               + lionProximityScore    // Bonus for AI lion near opponent den, penalty for human lion near AI den
               + elephantTrapPenalty   // Penalty for AI elephant potentially trapped by human rat
               + trappedPieceMalus     // Penalty for AI L/T/E trapped near opponent corner (uses base piece value)
               + ratInterceptBonus     // Bonus for AI rat safely blocking human elephant
               + opponent_den_threat_score // Den Safety bonus for AI
               - ai_den_threat_score;      // Den Safety penalty for Human
    }

} // namespace Evaluation


