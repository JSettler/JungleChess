#pragma once

#include "Common.h"
#include "Hashing.h"
#include <vector>
#include <cstdint>
#include <map> // For piece counting

class GameState {
public:
    // --- Constructor and Setup ---
    GameState();
    void setupInitialBoard();

    // --- Core Game Actions & Information ---
    Piece getPiece(int row, int col) const;
    bool isMoveLegal(const Move& move, Player player) const;
    void applyMove(const Move& move);
    std::vector<Move> getAllLegalMoves(Player player) const;
    std::vector<Move> getLegalMovesForPiece(int fromRow, int fromCol) const;
    Player getCurrentPlayer() const;
    void switchPlayer();
    Player checkWinner() const;

    // --- Hashing ---
    uint64_t getHashKey() const;

    // --- Public Helper Functions ---
    bool isValidPosition(int r, int c) const;
    bool isRiver(int r, int c) const;
    bool isOwnTrap(int r, int c, Player player) const;
    bool isOwnDen(int r, int c, Player player) const;
    int getRank(PieceType type) const; // Now public

    // --- Setters needed for loading state ---
    void setBoard(const std::vector<std::vector<Piece>>& newBoard);
    void setCurrentPlayer(Player player);
    void setHashKey(uint64_t key);

    //vvv NEW vvv --- Methods for Setup Mode --- vvv
    // Places a piece if valid (checks count, location rules), returns true on success
    bool setPieceAt(int r, int c, PieceType type, Player player);
    // Removes piece at location
    void clearSquare(int r, int c);
    // Removes all pieces
    void clearBoard();
    // Recalculates the hash from the current board state and player
    void recalculateHash();
    // Counts pieces for validation
    std::map<PieceType, int> countPieces(Player player) const;
    // Basic validation for setup completion (e.g., no pieces in opponent den)
    bool validateSetup() const;
    //^^^ NEW ^^^------------------------------^^^


private:
    // --- Internal State ---
    std::vector<std::vector<Piece>> board;
    Player currentPlayer;
    uint64_t currentHashKey;

    // --- Private Helper Functions ---
    // int getRank(PieceType type) const; // Moved to public
    bool canCapture(const Piece& attacker, const Piece& defender, int defenderRow, int defenderCol) const;
    void updateHashForPieceChange(PieceType type, Player player, int r, int c);
};


