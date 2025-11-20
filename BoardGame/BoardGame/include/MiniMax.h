#pragma once

#include "GameState.h"
#include <limits>
#include <utility>

class MiniMax
{
public:
    MiniMax();
    ~MiniMax();


    Move findBestMove(const GameState& state, int depth);

    std::pair<int, int> findBestPlacement(const GameState& state, Piece* piece);

    void setDepth(int depth) { m_depth = depth; }

    // Statistics for debugging/display
    int getNodesEvaluated() const { return m_nodesEvaluated; }
    int getPruneCount() const { return m_pruneCount; }

private:
    int alphaBeta(const GameState& state, int depth, int alpha, int beta,
        bool maximizing, PieceOwner aiPlayer);

    int evaluatePlacement(const GameState& state, int col, int row,
        Piece* piece, PieceOwner aiPlayer);

    int m_depth;
    int m_nodesEvaluated;
    int m_pruneCount;
};

