#pragma once

#include "Piece.h"
#include <vector>
#include <limits>
#include <unordered_map>
#include <string>
#include <random>

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
    void applyMove(const Move& move, bool updatePiecePosition = true);
    void applyPlacement(int col, int row, Piece* piece);

    // Win condition checking
    bool isWinningState(PieceOwner player) const;
    PieceOwner getWinner() const;

    // Evaluation for AI
    int evaluate(PieceOwner player) const;

    // Game phase management
    GamePhase getCurrentPhase() const { return m_currentPhase; }
    void setPhase(GamePhase phase) { m_currentPhase = phase; }

    // Position history for repetition detection
    uint64_t getBoardHash() const;
    void recordPosition();
    void updateZobrist(Piece* piece, int col, int row, bool add);
    int getPositionRepetitionCount(uint64_t key) const;
    void clearPositionHistory();

private:
    // Add non owning pointer to board and the locals
    Piece* m_board[5][5];
    GamePhase m_currentPhase;
    PieceOwner m_currentPlayer;
    PieceOwner m_winner;

    // Position history tracking
    void initializeZobrist();
    std::unordered_map<uint64_t, int> m_positionHistory;
    uint64_t m_zobristKey;

    // To check a line directly
    bool checkLine(int startCol, int startRow, int dCol, int dRow, PieceOwner player) const;
    // Helper functions for evaluation
    int evaluateLines(PieceOwner player) const;
    int evaluateLine(int startCol, int startRow, int deltaCol, int deltaRow, PieceOwner player) const;
    int evaluateCenterControl(PieceOwner player) const;
};