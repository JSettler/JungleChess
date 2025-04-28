#pragma once
#include "GameState.h" // Includes Common.h indirectly
#include "Common.h"    // Include directly for Move struct definition
#include <vector>
#include <limits>
#include <cstdint>
#include <vector>

// Helper Structure for Scored Moves (Defined here)
struct ScoredMove {
    Move move;
    int score;
    bool operator>(const ScoredMove& other) const { return score > other.score; }
};

// Transposition Table Entry (Unchanged)
enum class TTBound : uint8_t { EXACT, LOWER_BOUND, UPPER_BOUND };
struct TTEntry {
    uint64_t key = 0; int depth = -1; int score = 0;
    TTBound bound = TTBound::EXACT; Move bestMove = {-1,-1,-1,-1};
};

class AI {
public:
    //vvv REMOVED vvv --- Removed static constant --- vvv
    // static const int SEARCH_DEPTH = 6;
    //^^^ REMOVED ^^^---------------------------------^^^

    //vvv MODIFIED vvv --- Added searchDepth parameter --- vvv
    // Finds the best move using Alpha-Beta Pruning search
    static Move getBestMove(const GameState& currentGameState, int searchDepth, bool debugMode = false, bool quietMode = false);
    //^^^ MODIFIED ^^^------------------------------------^^^

private:
    // TT stuff (Unchanged)
    static const size_t TT_SIZE_POWER_OF_2 = 20;
    static const size_t TT_SIZE = 1 << TT_SIZE_POWER_OF_2;
    static std::vector<TTEntry> transpositionTable;
    static bool ttInitialized;
    static void initializeTT();

    // AlphaBeta signature (Unchanged)
    static int alphaBeta(GameState gameState, int depth, int maxDepth, int alpha, int beta, bool isMaximizingPlayer, bool debugMode);
};


