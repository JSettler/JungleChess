#include "AI.h"
#include "Evaluation.h"
#include <vector>
#include <limits>
#include <stdexcept>
#include <iostream>
#include <algorithm> // Required for std::sort, std::max, std::min
#include <string>
#include <functional> // Required for std::greater
#include <vector> // Ensure vector is included
#include <iomanip> // For std::fixed, std::setprecision

// --- Helper Structure for Scored Moves (Defined in AI.h) ---

// --- Helper Function to Score a Single Move Statically ---
// Scores moves for ordering: Winning > TT Move > Captures > Others
int scoreMoveStatic(const Move& move, const GameState& gameState) {
    Player aiPlayer = gameState.getCurrentPlayer();
    Player opponentPlayer = (aiPlayer == Player::PLAYER1) ? Player::PLAYER2 : Player::PLAYER1;
    // 1. Immediate Win
    if (gameState.isOwnDen(move.toRow, move.toCol, opponentPlayer)) {
        return 2000000000; // Highest priority
    }
    // 2. Capture
    Piece targetPiece = gameState.getPiece(move.toRow, move.toCol);
    if (targetPiece.owner == opponentPlayer) {
        Piece attackerPiece = gameState.getPiece(move.fromRow, move.fromCol);
        // MVV-LVA inspired scoring (simple version)
        return 10000000 + (Evaluation::getPieceValue(targetPiece.type) * 10) - Evaluation::getPieceValue(attackerPiece.type);
    }
    // 3. TODO: Add other heuristic scores
    // 4. Default score
    return 0;
}

// Helper for debug indentation
std::string indent(int depth, int maxDepth) {
    // Indentation logic remains useful if verbose debugging is re-enabled later
    return std::string((maxDepth - depth) * 2, ' ');
}


// Define static members
uint64_t AI::nodesSearched = 0; // Always defined

#ifdef USE_TRANSPOSITION_TABLE // Only define TT members if using TTs
std::vector<TTEntry> AI::transpositionTable;
bool AI::ttInitialized = false;

// Initialize TT
void AI::initializeTT() {
    if (!ttInitialized) {
        try {
            transpositionTable.resize(TT_SIZE);
            ttInitialized = true;
            // Avoid cout if quietMode could be a factor here, maybe pass quietMode?
            // For now, assume initialization message is acceptable.
            std::cout << "Transposition Table initialized (Size: " << TT_SIZE << " entries)." << std::endl;
        } catch (const std::bad_alloc& e) {
            std::cerr << "FATAL ERROR: Failed to allocate Transposition Table (Size: " << TT_SIZE << "). " << e.what() << std::endl;
             throw; // Re-throw exception
        }
    }
    // Clear entries efficiently before each search
     for (TTEntry& entry : transpositionTable) {
         entry.depth = -1; // Mark as invalid/empty by setting depth
         entry.key = 0;
     }
}

// Calculate TT Utilization
double AI::getTTUtilization() {
    if (!ttInitialized || TT_SIZE == 0) { return 0.0; }
    size_t usedCount = 0;
    for (const auto& entry : transpositionTable) {
        if (entry.depth != -1) { // Check if entry seems valid
            usedCount++;
        }
    }
    return (static_cast<double>(usedCount) / TT_SIZE) * 100.0;
}
#endif // USE_TRANSPOSITION_TABLE


// --- Alpha-Beta Recursive Helper Function ---
int AI::alphaBeta(GameState gameState, int depth, int maxDepth, int alpha, int beta, bool isMaximizingPlayer, bool debugMode) {

    int originalAlpha = alpha;
    int originalBeta = beta;
    Move ttBestMove = {-1,-1,-1,-1}; // Keep this declaration outside TT block

#ifdef USE_TRANSPOSITION_TABLE
    // 0. Transposition Table Lookup
    uint64_t currentHash = gameState.getHashKey();
    size_t ttIndex = currentHash % TT_SIZE;
    TTEntry& ttEntry = transpositionTable[ttIndex]; // Use reference for potential update
    if (ttEntry.key == currentHash && ttEntry.depth >= depth) {
        switch (ttEntry.bound) {
            case TTBound::EXACT:       return ttEntry.score;
            case TTBound::LOWER_BOUND: alpha = std::max(alpha, ttEntry.score); break;
            case TTBound::UPPER_BOUND: beta = std::min(beta, ttEntry.score); break;
        }
        if (beta <= alpha) return ttEntry.score; // Cutoff based on TT info
        if (ttEntry.bestMove.fromRow != -1) ttBestMove = ttEntry.bestMove; // Use stored move hint
    }
#endif // USE_TRANSPOSITION_TABLE

    // 1. Terminal States & Base Case
    Player winner = gameState.checkWinner();
    if (winner == Player::PLAYER2) return Evaluation::WIN_SCORE + depth;
    if (winner == Player::PLAYER1) return -Evaluation::WIN_SCORE - depth;
    if (depth <= 0) { nodesSearched++; return Evaluation::evaluateBoard(gameState); }
    Player currentPlayer = gameState.getCurrentPlayer();
    std::vector<Move> legalMoves = gameState.getAllLegalMoves(currentPlayer);
    if (legalMoves.empty()) { return isMaximizingPlayer ? (-Evaluation::WIN_SCORE - depth) : (Evaluation::WIN_SCORE + depth); }

    nodesSearched++; // Count internal nodes

    // 2. Score and Sort Moves
    std::vector<ScoredMove> scoredMoves; scoredMoves.reserve(legalMoves.size());
#ifdef USE_TRANSPOSITION_TABLE
    // Try TT Move first if available and legal
    if(ttBestMove.fromRow != -1) {
        bool ttMoveIsLegal = false;
        for(const auto& legal : legalMoves) if (legal == ttBestMove) { ttMoveIsLegal = true; break; }
        if (ttMoveIsLegal) {
            // Give TT move the highest score for ordering
            scoredMoves.push_back(ScoredMove{ttBestMove, 2000000000});
        } else {
            ttBestMove = {-1,-1,-1,-1}; // Invalidate if not legal in this position
        }
    }
#endif // USE_TRANSPOSITION_TABLE
    // Score remaining moves
    for (const auto& move : legalMoves) {
#ifdef USE_TRANSPOSITION_TABLE
        if (ttBestMove.fromRow != -1 && move == ttBestMove) continue; // Don't add TT move twice
#endif // USE_TRANSPOSITION_TABLE
        scoredMoves.push_back(ScoredMove{move, scoreMoveStatic(move, gameState)});
    }
    // Sort moves (TT move first if present, then by score descending)
#ifdef USE_TRANSPOSITION_TABLE
    // Sort starting after the potential TT move
    std::sort(scoredMoves.begin() + (ttBestMove.fromRow != -1 ? 1 : 0), scoredMoves.end(), std::greater<ScoredMove>());
#else
    std::sort(scoredMoves.begin(), scoredMoves.end(), std::greater<ScoredMove>()); // Sort all if no TT move
#endif // USE_TRANSPOSITION_TABLE


    // 3. Recursive Exploration
    int bestScoreInNode = isMaximizingPlayer ? -std::numeric_limits<int>::max() : std::numeric_limits<int>::max();
    Move bestMoveForNode = scoredMoves[0].move; // Default best move is the first after sorting
#ifdef USE_TRANSPOSITION_TABLE
    TTBound resultBound = isMaximizingPlayer ? TTBound::UPPER_BOUND : TTBound::LOWER_BOUND; // Assume the worst initially
#endif // USE_TRANSPOSITION_TABLE

    for (const auto& scoredMove : scoredMoves) {
        GameState nextState = gameState; nextState.applyMove(scoredMove.move); nextState.switchPlayer();
        int eval = alphaBeta(nextState, depth - 1, maxDepth, alpha, beta, !isMaximizingPlayer, debugMode); // Pass correct maximizing flag

        if (isMaximizingPlayer) {
            if (eval > bestScoreInNode) { bestScoreInNode = eval; bestMoveForNode = scoredMove.move; }
            alpha = std::max(alpha, bestScoreInNode);
            if (beta <= alpha) {
                #ifdef USE_TRANSPOSITION_TABLE
                resultBound = TTBound::LOWER_BOUND; // Failed high
                #endif
                break; // Beta cutoff
            }
        } else { // Minimizing Player
            if (eval < bestScoreInNode) { bestScoreInNode = eval; bestMoveForNode = scoredMove.move; }
            beta = std::min(beta, bestScoreInNode);
            if (beta <= alpha) {
                #ifdef USE_TRANSPOSITION_TABLE
                resultBound = TTBound::UPPER_BOUND; // Failed low
                #endif
                break; // Alpha cutoff
            }
        }
    }

#ifdef USE_TRANSPOSITION_TABLE
    // --- Store Result in TT ---
    // Determine final bound type more accurately based on original alpha/beta
    if (isMaximizingPlayer) {
        if (bestScoreInNode <= originalAlpha) resultBound = TTBound::UPPER_BOUND;
        else if (bestScoreInNode >= beta) resultBound = TTBound::LOWER_BOUND;
        else resultBound = TTBound::EXACT;
    } else { // Minimizing
        if (bestScoreInNode >= originalBeta) resultBound = TTBound::LOWER_BOUND;
        else if (bestScoreInNode <= alpha) resultBound = TTBound::UPPER_BOUND;
        else resultBound = TTBound::EXACT;
    }
    // Replace if the new result is from the same or deeper search
    if (ttEntry.depth <= depth) {
         ttEntry = {currentHash, depth, bestScoreInNode, resultBound, bestMoveForNode};
    }
#endif // USE_TRANSPOSITION_TABLE

    return bestScoreInNode;
}


// --- Main AI Function: Uses Alpha-Beta ---
AIMoveInfo AI::getBestMove(const GameState& currentGameState, int searchDepth, bool debugMode, bool quietMode) {
#ifdef USE_TRANSPOSITION_TABLE
    initializeTT(); // Clear/Initialize TT before search
#else
    // Optionally print a message if TT is disabled and not in quiet mode
    // if (!quietMode) std::cout << "Note: Transposition Table disabled." << std::endl;
#endif // USE_TRANSPOSITION_TABLE

    nodesSearched = 0; // Reset node counter for this search

    Player aiPlayer = currentGameState.getCurrentPlayer();
    std::vector<Move> legalMoves = currentGameState.getAllLegalMoves(aiPlayer);
    if (legalMoves.empty()) {
        if (!quietMode) std::cerr << "Error: AI called with no legal moves!" << std::endl;
        return AIMoveInfo(); // Return default/empty info
    }

    // Score and Sort Initial Moves
    std::vector<ScoredMove> scoredInitialMoves; scoredInitialMoves.reserve(legalMoves.size());
    for (const auto& move : legalMoves) scoredInitialMoves.push_back(ScoredMove{move, scoreMoveStatic(move, currentGameState)});
    std::sort(scoredInitialMoves.begin(), scoredInitialMoves.end(), std::greater<ScoredMove>());

    Move bestMove = scoredInitialMoves[0].move; // Initialize with the heuristically best move
    int bestScore = -std::numeric_limits<int>::max();

    int alpha = -std::numeric_limits<int>::max();
    int beta = std::numeric_limits<int>::max();
    // int initialAlpha = alpha; // Store if needed for root TT entry

    // Print thinking message using passed depth
#ifdef USE_TRANSPOSITION_TABLE
    const char* ttStatus = "TT";
#else
    const char* ttStatus = "NoTT";
#endif
    if (debugMode) {
         std::cout << "AI Thinking (" << ttStatus << " Depth " << searchDepth << ")... Evaluating " << scoredInitialMoves.size() << " initial moves." << std::endl;
    } else if (!quietMode) {
        std::cout << "AI Thinking (Depth " << searchDepth << ")..." << std::endl;
    }

    // Iterate through initial moves
    for (const auto& scoredMove : scoredInitialMoves) {
        const Move& move = scoredMove.move;
        GameState nextState = currentGameState; nextState.applyMove(move);
        Player winner = nextState.checkWinner();
        int currentMoveScore;

        if (winner == aiPlayer) {
            currentMoveScore = Evaluation::WIN_SCORE;
            if (!quietMode) std::cout << "  Found Immediate Winning Move (Den): (" << move.fromRow << "," << move.fromCol << ")->(" << move.toRow << "," << move.toCol << ")" << std::endl;
            bestMove = move; bestScore = currentMoveScore; // Update before returning
            AIMoveInfo result; result.bestMove = bestMove; result.nodesSearched = nodesSearched;
            #ifdef USE_TRANSPOSITION_TABLE
            result.ttUtilizationPercent = getTTUtilization();
            #else
            result.ttUtilizationPercent = 0.0;
            #endif
            return result; // Return immediately
        } else {
            nextState.switchPlayer();
            // Start search for this move
            currentMoveScore = alphaBeta(nextState, searchDepth - 1, searchDepth, alpha, beta, false, debugMode); // false = minimizing player
        }

        // Debug Output
        if (debugMode) {
             Piece movedPiece = currentGameState.getPiece(move.fromRow, move.fromCol);
             Piece capturedPiece = currentGameState.getPiece(move.toRow, move.toCol);
             std::cout << "  AI Move (" << move.fromRow << "," << move.fromCol << ")->(" << move.toRow << "," << move.toCol << ")"
                       << " (P" << static_cast<int>(movedPiece.type) << ")"
                       << (capturedPiece.type != PieceType::EMPTY ? " Cap P" + std::to_string(static_cast<int>(capturedPiece.type)) : "")
                       << " -> AB Score: " << currentMoveScore << " (Static: " << scoredMove.score << ")" << std::endl;
        }

        // Update best move
        if (currentMoveScore > bestScore) {
             if (debugMode) std::cout << "    New best score! (" << currentMoveScore << " > " << bestScore << ")" << std::endl;
            bestScore = currentMoveScore; bestMove = move;
            alpha = std::max(alpha, bestScore); // Update alpha at the top level
        }
         // Optional top-level beta cutoff check
         // if (beta <= alpha) { break; }
    }

    // Log final choice using passed depth
    if (!quietMode) {
        Piece bestMovedPiece = currentGameState.getPiece(bestMove.fromRow, bestMove.fromCol);
        Piece bestCapturedPiece = currentGameState.getPiece(bestMove.toRow, bestMove.toCol);
        std::cout << "AI Chose Best Move (Alpha-Beta " << searchDepth << "-ply, Ordered, " << ttStatus << "): ("
                  << bestMove.fromRow << "," << bestMove.fromCol << ")->(" << bestMove.toRow << "," << bestMove.toCol << ")"
                  << " (Piece: " << static_cast<int>(bestMovedPiece.type) << ")"
                  << (bestCapturedPiece.type != PieceType::EMPTY ? " Captures: " + std::to_string(static_cast<int>(bestCapturedPiece.type)) : "")
                  << " | Final Score: " << bestScore << std::endl;
    }

    // Construct and return result struct
    AIMoveInfo result;
    result.bestMove = bestMove;
    result.nodesSearched = nodesSearched;
#ifdef USE_TRANSPOSITION_TABLE
    result.ttUtilizationPercent = getTTUtilization();
#else
    result.ttUtilizationPercent = 0.0; // No TT utilization if disabled
#endif

    return result;
}


