#pragma once

#include "Piece.h"
#include <vector>
#include <limits>

// Represents a move in the game
struct Move {
    int fromCol;
    int fromRow;
    int toCol;
    int toRow;
    Piece* piece;

    Move() : fromCol(-1), fromRow(-1), toCol(-1), toRow(-1), piece(nullptr) {}
    Move(int fc, int fr, int tc, int tr, Piece* p)
        : fromCol(fc), fromRow(fr), toCol(tc), toRow(tr), piece(p) {
    }
};

// Game phases
enum class GamePhase {
    PLACEMENT,
    MOVEMENT,
    GAME_OVER
};

class GameState {
public:
    GameState();
    ~GameState();

    // Board management
    bool isPositionEmpty(int col, int row) const;
    Piece* getPieceAt(int col, int row) const;
    void setPieceAt(int col, int row, Piece* piece);
    void removePieceAt(int col, int row);

    // Move validation and generation
    bool isValidPlacement(int col, int row) const;
    bool isValidMove(const Move& move) const;
    std::vector<Move> getLegalMoves(PieceOwner player) const;
    std::vector<std::pair<int, int>> getLegalPlacements() const;

    // Move execution
    void applyMove(const Move& move);
    void applyPlacement(int col, int row, Piece* piece);

    // Win condition checking
    bool isWinningState(PieceOwner player) const;
    PieceOwner getWinner() const;

    // Evaluation for AI
    int evaluate(PieceOwner player) const;

    // Game phase management
    GamePhase getCurrentPhase() const { return m_currentPhase; }
    void setPhase(GamePhase phase) { m_currentPhase = phase; }

    // Turn management

private:
    // Add non owning pointer to board and the locals
    Piece* m_board[5][5];
    GamePhase m_currentPhase;
    PieceOwner m_currentPlayer;
    PieceOwner m_winner;

    // NOTE ADD THESE:
    // Helper functions for win checking

    // Helper functions for evaluation of pieces
 
};

