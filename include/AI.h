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

//vvv NEW vvv --- Struct to return AI results --- vvv
struct AIMoveInfo {
    Move bestMove = {-1,-1,-1,-1};
    uint64_t nodesSearched = 0;
    double ttUtilizationPercent = 0.0;
};
//^^^ NEW ^^^------------------------------------^^^


class AI {
public:
    // Finds the best move using Alpha-Beta Pruning search
    //vvv MODIFIED vvv --- Return AIMoveInfo --- vvv
    static AIMoveInfo getBestMove(const GameState& currentGameState, int searchDepth, bool debugMode = false, bool quietMode = false);
    //^^^ MODIFIED ^^^---------------------------^^^

private:
    // TT stuff
    //vvv MODIFIED vvv --- Increased TT size for Depth 7/8 --- vvv
    static const size_t TT_SIZE_POWER_OF_2 = 22; // 2^22 = ~4 Million entries (~128-160MB)
    //^^^ MODIFIED ^^^---------------------------------------^^^
    static const size_t TT_SIZE = 1 << TT_SIZE_POWER_OF_2;
    static std::vector<TTEntry> transpositionTable;
    static bool ttInitialized;
    static void initializeTT();

    //vvv NEW vvv --- Node counter --- vvv
    static uint64_t nodesSearched;
    //^^^ NEW ^^^--------------------^^^

    //vvv NEW vvv --- TT Utilization helper --- vvv
    static double getTTUtilization();
    //^^^ NEW ^^^-----------------------------^^^


    // AlphaBeta signature (Unchanged)
    static int alphaBeta(GameState gameState, int depth, int maxDepth, int alpha, int beta, bool isMaximizingPlayer, bool debugMode);
};


