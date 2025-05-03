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
    // <<< NEW: Public getter for the board >>>
    const std::vector<std::vector<Piece>>& getBoard() const;
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
    int getRank(PieceType type) const;

    // --- Setters needed for loading state ---
    void setBoard(const std::vector<std::vector<Piece>>& newBoard);
    void setCurrentPlayer(Player player);
    void setHashKey(uint64_t key);

    // --- Methods for Setup Mode ---
    bool setPieceAt(int r, int c, PieceType type, Player player);
    void clearSquare(int r, int c);
    void clearBoard();
    void recalculateHash();
    std::map<PieceType, int> countPieces(Player player) const;
    bool validateSetup() const;


private:
    // --- Internal State ---
    std::vector<std::vector<Piece>> board; // <<< Still private
    Player currentPlayer;
    uint64_t currentHashKey;

    // --- Private Helper Functions ---
    bool canCapture(const Piece& attacker, const Piece& defender, int defenderRow, int defenderCol) const;
    void updateHashForPieceChange(PieceType type, Player player, int r, int c);
};


