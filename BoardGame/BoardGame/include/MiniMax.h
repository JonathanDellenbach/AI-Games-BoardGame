#pragma once

#include "GameState.h"
#include <limits>
#include <vector>

class MiniMax
{
public:
    MiniMax();
    MiniMax(PieceOwner player);
    ~MiniMax();

    // Public API
    Move findBestMove(const GameState& state, int depth);
    std::pair<int, int> findBestPlacement(const GameState& state, Piece* piece);

private:
    // Evaluation
    int evaluatePosition(const GameState& state, int col, int row, Piece* piece);

    // Minimax algorithm
    int alphaBeta(const GameState& state, int depth, int alpha, int beta,
        bool isMaximizingPlayer, PieceOwner aiPlayer);

    int maximizeScore(const GameState& state, const std::vector<Move>& moves,
        int depth, int alpha, int beta, PieceOwner aiPlayer);

    int minimizeScore(const GameState& state, const std::vector<Move>& moves,
        int depth, int alpha, int beta, PieceOwner aiPlayer);

    // Utilities
    PieceOwner getOpponent(PieceOwner player) const;
    bool isValidPosition(int col, int row) const;
    void resetStatistics();

    // Members
    int m_depth;
    int m_nodesEvaluated;
    int m_pruneCount;
    PieceOwner m_player; //which player the player AI represents

    // Constants
    static constexpr int MIN_SCORE = std::numeric_limits<int>::min();
    static constexpr int MAX_SCORE = std::numeric_limits<int>::max();
    static constexpr int WIN_SCORE = 10000;
    static constexpr int LOSS_SCORE = -10000;
    static constexpr int NON_TERMINAL = 0;
};

