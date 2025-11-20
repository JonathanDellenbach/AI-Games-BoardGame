#include "MiniMax.h"
#include <algorithm>
#include <iostream>

MiniMax::MiniMax()
    : m_depth(3)
    , m_nodesEvaluated(0)
    , m_pruneCount(0)
{
}

MiniMax::~MiniMax() {}

Move MiniMax::findBestMove(const GameState& state, int depth)
{
    m_nodesEvaluated = 0;
    m_pruneCount = 0;

    std::vector<Move> legalMoves = state.getLegalMoves(PieceOwner::AI);

    if (legalMoves.empty()) {
        std::cout << "MinMax: No legal moves!" << std::endl;
        return Move();
    }

    Move bestMove;
    int bestScore = std::numeric_limits<int>::min();
    int alpha = std::numeric_limits<int>::min();
    int beta = std::numeric_limits<int>::max();

    std::cout << "MinMax: Evaluating " << legalMoves.size() << " moves at depth " << depth << std::endl;

    for (const Move& move : legalMoves) {
        // Create copy of state and apply move
        GameState testState = state;
        testState.applyMove(move);

        // Evaluate with alpha-beta (opponent's turn, so minimizing)
        int score = alphaBeta(testState, depth - 1, alpha, beta, false, PieceOwner::AI);

        if (score > bestScore) {
            bestScore = score;
            bestMove = move;
        }

        alpha = std::max(alpha, bestScore);
    }

    std::cout << "MiniMax: Best score=" << bestScore
        << " Nodes=" << m_nodesEvaluated
        << " Pruned=" << m_pruneCount << std::endl;

    return bestMove;
}

int MiniMax::alphaBeta(const GameState& state, int depth, int alpha, int beta,
    bool maximizing, PieceOwner aiPlayer)
{
    m_nodesEvaluated++;

    // Terminal check: winner found
    PieceOwner winner = state.getWinner();
    if (winner == aiPlayer) {
        return 10000 + depth;  // Prefer faster wins
    }
    else if (winner != PieceOwner::NONE) {
        return -10000 - depth; // Avoid faster losses
    }

    // Depth limit reached: use heuristic evaluation
    if (depth == 0) {
        return state.evaluate(aiPlayer);
    }

    PieceOwner currentPlayer = maximizing ? aiPlayer :
        (aiPlayer == PieceOwner::AI ? PieceOwner::PLAYER : PieceOwner::AI);

    std::vector<Move> moves = state.getLegalMoves(currentPlayer);

    if (moves.empty()) {
        return state.evaluate(aiPlayer);
    }

    if (maximizing) {
        int maxEval = std::numeric_limits<int>::min();

        for (const Move& move : moves) {
            GameState testState = state;
            testState.applyMove(move);

            int eval = alphaBeta(testState, depth - 1, alpha, beta, false, aiPlayer);
            maxEval = std::max(maxEval, eval);
            alpha = std::max(alpha, eval);

            // Beta cutoff (prune)
            if (beta <= alpha) {
                m_pruneCount++;
                break;
            }
        }
        return maxEval;
    }
    else {
        int minEval = std::numeric_limits<int>::max();

        for (const Move& move : moves) {
            GameState testState = state;
            testState.applyMove(move);

            int eval = alphaBeta(testState, depth - 1, alpha, beta, true, aiPlayer);
            minEval = std::min(minEval, eval);
            beta = std::min(beta, eval);

            // Alpha cutoff (prune)
            if (beta <= alpha) {
                m_pruneCount++;
                break;
            }
        }
        return minEval;
    }
}

std::pair<int, int> MiniMax::findBestPlacement(const GameState& state, Piece* piece)
{
    if (!piece) {
        return { -1, -1 };
    }

    auto placements = state.getLegalPlacements();

    if (placements.empty()) {
        return { -1, -1 };
    }

    std::pair<int, int> bestPos = placements[0];
    int bestScore = std::numeric_limits<int>::min();

    for (const auto& [col, row] : placements) {
        int score = evaluatePlacement(state, col, row, piece, PieceOwner::AI);

        if (score > bestScore) {
            bestScore = score;
            bestPos = { col, row };
        }
    }

    return bestPos;
}

int MiniMax::evaluatePlacement(const GameState& state, int col, int row,
    Piece* piece, PieceOwner aiPlayer)
{
    int score = 0;

    // Center control is valuable (Manhattan distance from center)
    int distFromCenter = std::abs(col - 2) + std::abs(row - 2);
    score += (4 - distFromCenter) * 15;  // Max 60 for center

    // Simulate placement and evaluate resulting state
    GameState testState = state;
    testState.applyPlacement(col, row, piece);
    score += testState.evaluate(aiPlayer) / 2;

    // Bonus for positions that could form lines
    // Check how many friendly pieces are adjacent
    const int dirs[8][2] = { {-1,-1},{-1,0},{-1,1},{0,-1},{0,1},{1,-1},{1,0},{1,1} };
    int adjacentFriendly = 0;

    for (auto& dir : dirs) {
        int nc = col + dir[0];
        int nr = row + dir[1];
        if (nc >= 0 && nc < 5 && nr >= 0 && nr < 5) {
            Piece* adj = state.getPieceAt(nc, nr);
            if (adj && adj->getOwner() == aiPlayer) {
                adjacentFriendly++;
            }
        }
    }
    score += adjacentFriendly * 10;

    return score;
}