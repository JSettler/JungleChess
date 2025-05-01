#pragma once
#include "GameState.h" // Includes Common.h indirectly
#include "Common.h"    // Include directly for Move struct definition
#include <vector>
#include <limits>
#include <cstdint>   // For uint64_t
#include <vector>    // For std::vector

//vvv NEW vvv --- Control Macro for TT --- vvv
// Comment out this line to disable TTs completely at compile time
#define USE_TRANSPOSITION_TABLE
//^^^ NEW ^^^------------------------------^^^


// Helper Structure for Scored Moves (Defined here)
struct ScoredMove {
    Move move;
    int score; // Static score based on capture/win potential

    // Define comparison for sorting (we want higher scores first)
    bool operator>(const ScoredMove& other) const {
        return score > other.score;
    }
};


#ifdef USE_TRANSPOSITION_TABLE // Only define TT types if using TTs
// Transposition Table Entry
enum class TTBound : uint8_t { // Use uint8_t for smaller size
    EXACT,       // Score is the exact value for the node
    LOWER_BOUND, // Score is at least this value (failed high / alpha update)
    UPPER_BOUND  // Score is at most this value (failed low / beta cutoff)
};

struct TTEntry {
    uint64_t key = 0;         // Zobrist key verification
    int depth = -1;       // Depth this entry was calculated at
    int score = 0;        // The evaluated score
    TTBound bound = TTBound::EXACT; // Type of score (exact, lower, upper)
    Move bestMove = {-1,-1,-1,-1}; // Best move found from this position (for move ordering)
};
#endif // USE_TRANSPOSITION_TABLE


// Struct to return AI results
struct AIMoveInfo {
    Move bestMove = {-1,-1,-1,-1};
    uint64_t nodesSearched = 0;
    double ttUtilizationPercent = 0.0; // Will be 0 if TT is disabled
    int finalScore = 0; // The raw evaluation score of the chosen move
};


class AI {
public:
    // Finds the best move using Alpha-Beta Pruning search
    static AIMoveInfo getBestMove(const GameState& currentGameState, int searchDepth, bool debugMode = false, bool quietMode = false);

private:
#ifdef USE_TRANSPOSITION_TABLE // Only declare TT members if using TTs
    // TT stuff
    static const size_t TT_SIZE_POWER_OF_2 = 22; // 2^22 = ~4 Million entries
    static const size_t TT_SIZE = 1 << TT_SIZE_POWER_OF_2;
    static std::vector<TTEntry> transpositionTable;
    static bool ttInitialized;
    static void initializeTT();
    static double getTTUtilization();
#endif // USE_TRANSPOSITION_TABLE

    // Node counter (always needed)
    static uint64_t nodesSearched;

    // AlphaBeta signature (unchanged)
    static int alphaBeta(GameState gameState, int depth, int maxDepth, int alpha, int beta, bool isMaximizingPlayer, bool debugMode);
};


