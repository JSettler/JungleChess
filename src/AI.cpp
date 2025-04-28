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

// --- Helper Structure for Scored Moves (Defined in AI.h) ---

// --- Helper Function to Score a Single Move Statically ---
int scoreMoveStatic(const Move& move, const GameState& gameState) {
    Player aiPlayer = gameState.getCurrentPlayer();
    Player opponentPlayer = (aiPlayer == Player::PLAYER1) ? Player::PLAYER2 : Player::PLAYER1;
    if (gameState.isOwnDen(move.toRow, move.toCol, opponentPlayer)) return Evaluation::WIN_SCORE * 2;
    Piece targetPiece = gameState.getPiece(move.toRow, move.toCol);
    if (targetPiece.owner == opponentPlayer) { // Capture
        Piece attackerPiece = gameState.getPiece(move.fromRow, move.fromCol);
        return 1000000 + Evaluation::getPieceValue(targetPiece.type) - Evaluation::getPieceValue(attackerPiece.type);
    }
    return 0; // Non-capture, non-win
}

// Helper for debug indentation
std::string indent(int depth, int maxDepth) {
    return std::string((maxDepth - depth) * 2, ' ');
}


// Define static TT members
std::vector<TTEntry> AI::transpositionTable;
bool AI::ttInitialized = false;

// Initialize TT
void AI::initializeTT() {
    if (!ttInitialized) {
        transpositionTable.resize(TT_SIZE);
        ttInitialized = true;
         std::cout << "Transposition Table initialized (Size: " << TT_SIZE << " entries)." << std::endl;
    }
     for (TTEntry& entry : transpositionTable) {
         entry.depth = -1; entry.key = 0;
     }
}


// --- Alpha-Beta Recursive Helper Function ---
int AI::alphaBeta(GameState gameState, int depth, int maxDepth, int alpha, int beta, bool isMaximizingPlayer, bool debugMode) {

    int originalAlpha = alpha;
    int originalBeta = beta;

    // 0. TT Lookup
    uint64_t currentHash = gameState.getHashKey();
    size_t ttIndex = currentHash % TT_SIZE;
    TTEntry& ttEntry = transpositionTable[ttIndex]; // Use reference
    Move ttBestMove = {-1,-1,-1,-1};
    if (ttEntry.key == currentHash && ttEntry.depth >= depth) {
        switch (ttEntry.bound) {
            case TTBound::EXACT:       return ttEntry.score;
            case TTBound::LOWER_BOUND: alpha = std::max(alpha, ttEntry.score); break;
            case TTBound::UPPER_BOUND: beta = std::min(beta, ttEntry.score); break;
        }
        if (beta <= alpha) return ttEntry.score;
        if (ttEntry.bestMove.fromRow != -1) ttBestMove = ttEntry.bestMove;
    }

    // 1. Terminal States
    Player winner = gameState.checkWinner();
    if (winner == Player::PLAYER2) return Evaluation::WIN_SCORE + depth;
    if (winner == Player::PLAYER1) return -Evaluation::WIN_SCORE - depth;
    if (depth == 0) return Evaluation::evaluateBoard(gameState);
    Player currentPlayer = gameState.getCurrentPlayer();
    std::vector<Move> legalMoves = gameState.getAllLegalMoves(currentPlayer);
    if (legalMoves.empty()) return isMaximizingPlayer ? (-Evaluation::WIN_SCORE - depth) : (Evaluation::WIN_SCORE + depth);

    // 2. Score and Sort Moves
    std::vector<ScoredMove> scoredMoves; scoredMoves.reserve(legalMoves.size());
    int ttMoveScore = -1;
    if(ttBestMove.fromRow != -1) {
        bool ttMoveIsLegal = false;
        for(const auto& legal : legalMoves) if (legal == ttBestMove) { ttMoveIsLegal = true; break; }
        if (ttMoveIsLegal) { ttMoveScore = scoreMoveStatic(ttBestMove, gameState) + 10000000; scoredMoves.push_back(ScoredMove{ttBestMove, ttMoveScore}); }
        else { ttBestMove = {-1,-1,-1,-1}; }
    }
    for (const auto& move : legalMoves) {
        if (ttBestMove.fromRow != -1 && move == ttBestMove) continue;
        scoredMoves.push_back(ScoredMove{move, scoreMoveStatic(move, gameState)});
    }
    std::sort(scoredMoves.begin(), scoredMoves.end(), std::greater<ScoredMove>());


    // 3. Recursive Exploration
    int bestScoreForNode = isMaximizingPlayer ? -std::numeric_limits<int>::max() : std::numeric_limits<int>::max();
    Move bestMoveForNode = scoredMoves[0].move;
    TTBound calculatedBound; // Determined after loop

    if (isMaximizingPlayer) { // AI Maximize
        calculatedBound = TTBound::UPPER_BOUND; // Assume fail low initially
        for (const auto& scoredMove : scoredMoves) {
            GameState nextState = gameState; nextState.applyMove(scoredMove.move); nextState.switchPlayer();
            int eval = alphaBeta(nextState, depth - 1, maxDepth, alpha, beta, false, debugMode);
            if (eval > bestScoreForNode) { bestScoreForNode = eval; bestMoveForNode = scoredMove.move; }
            alpha = std::max(alpha, bestScoreForNode);
            if (beta <= alpha) { calculatedBound = TTBound::LOWER_BOUND; break; } // Beta cutoff
            else { calculatedBound = TTBound::EXACT; } // Might be exact if loop completes
        }
        // Determine final bound type more accurately
        if (bestScoreForNode <= originalAlpha) calculatedBound = TTBound::UPPER_BOUND;
        // else if (bestScoreForNode >= beta) calculatedBound = TTBound::LOWER_BOUND; // Covered by break
        else calculatedBound = TTBound::EXACT;

        ttEntry = {currentHash, depth, bestScoreForNode, calculatedBound, bestMoveForNode};
        return bestScoreForNode;

    } else { // Opponent Minimize
        calculatedBound = TTBound::LOWER_BOUND; // Assume fail high initially
         for (const auto& scoredMove : scoredMoves) {
            GameState nextState = gameState; nextState.applyMove(scoredMove.move); nextState.switchPlayer();
            int eval = alphaBeta(nextState, depth - 1, maxDepth, alpha, beta, true, debugMode);
             if (eval < bestScoreForNode) { bestScoreForNode = eval; bestMoveForNode = scoredMove.move; }
            beta = std::min(beta, bestScoreForNode);
            if (beta <= alpha) { calculatedBound = TTBound::UPPER_BOUND; break; } // Alpha cutoff
            else { calculatedBound = TTBound::EXACT; }
        }
         // Determine final bound type more accurately
         if (bestScoreForNode >= originalBeta) calculatedBound = TTBound::LOWER_BOUND;
         // else if (bestScoreForNode <= alpha) calculatedBound = TTBound::UPPER_BOUND; // Covered by break
         else calculatedBound = TTBound::EXACT;

        ttEntry = {currentHash, depth, bestScoreForNode, calculatedBound, bestMoveForNode};
        return bestScoreForNode;
    }
}


// --- Main AI Function: Uses Alpha-Beta ---
Move AI::getBestMove(const GameState& currentGameState, int searchDepth, bool debugMode, bool quietMode) {
    initializeTT(); // Clear/Initialize TT before search

    Player aiPlayer = currentGameState.getCurrentPlayer();
    std::vector<Move> legalMoves = currentGameState.getAllLegalMoves(aiPlayer);
    if (legalMoves.empty()) throw std::runtime_error("AI has no legal moves in getBestMove!");

    // Score and Sort Initial Moves
    std::vector<ScoredMove> scoredInitialMoves; scoredInitialMoves.reserve(legalMoves.size());
    for (const auto& move : legalMoves) scoredInitialMoves.push_back(ScoredMove{move, scoreMoveStatic(move, currentGameState)});
    std::sort(scoredInitialMoves.begin(), scoredInitialMoves.end(), std::greater<ScoredMove>());

    Move bestMove = scoredInitialMoves[0].move;
    int bestScore = -std::numeric_limits<int>::max();
    // Use passed searchDepth parameter

    int alpha = -std::numeric_limits<int>::max();
    int beta = std::numeric_limits<int>::max();
    // int initialAlpha = alpha; // Store if needed for root TT entry

    // Print thinking message using passed depth
    if (debugMode) {
         std::cout << "AI Thinking (TT Depth " << searchDepth << ")... Evaluating " << scoredInitialMoves.size() << " initial moves." << std::endl;
    } else if (!quietMode) {
        std::cout << "AI Thinking (Depth " << searchDepth << ")..." << std::endl;
    }

    // Iterate through SORTED initial moves
    for (const auto& scoredMove : scoredInitialMoves) {
        const Move& move = scoredMove.move;
        GameState nextState = currentGameState; nextState.applyMove(move);
        Player winner = nextState.checkWinner();
        int currentMoveScore;

        if (winner == aiPlayer) {
            currentMoveScore = Evaluation::WIN_SCORE;
            if (!quietMode) std::cout << "  Found Immediate Winning Move (Den): (" << move.fromRow << "," << move.fromCol << ")->(" << move.toRow << "," << move.toCol << ")" << std::endl;
            bestMove = move; bestScore = currentMoveScore; // Update before returning
            // Store TT entry for root?
            // transpositionTable[currentGameState.getHashKey() % TT_SIZE] = {currentGameState.getHashKey(), searchDepth, currentMoveScore, TTBound::LOWER_BOUND, move};
            return move; // Take immediate win
        } else {
            nextState.switchPlayer();
            // Pass searchDepth parameter
            currentMoveScore = alphaBeta(nextState, searchDepth - 1, searchDepth, alpha, beta, false, debugMode);
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
        std::cout << "AI Chose Best Move (Alpha-Beta " << searchDepth << "-ply, Ordered, TT): ("
                  << bestMove.fromRow << "," << bestMove.fromCol << ")->(" << bestMove.toRow << "," << bestMove.toCol << ")"
                  << " (Piece: " << static_cast<int>(bestMovedPiece.type) << ")"
                  << (bestCapturedPiece.type != PieceType::EMPTY ? " Captures: " + std::to_string(static_cast<int>(bestCapturedPiece.type)) : "")
                  << " | Final Score: " << bestScore << std::endl;
    }

    // Optional: Store the final best move in the TT for the root position
    // ...

    return bestMove;
}


