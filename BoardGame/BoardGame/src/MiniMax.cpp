#include "MiniMax.h"
#include <algorithm>
#include <iostream>
#include <cstdlib>
#include <ctime>

MiniMax::MiniMax()
    : m_depth(3)
    , m_nodesEvaluated(0)
    , m_pruneCount(0)
    , m_player(PieceOwner::AI)
{
}

MiniMax::MiniMax(PieceOwner player)
    : m_depth(3)
    , m_nodesEvaluated(0)
    , m_pruneCount(0)
    , m_player(player)
{
}

MiniMax::~MiniMax() {}

// Main entry point for movement phase
Move MiniMax::findBestMove(const GameState& state, int depth)
{
    resetStatistics();

    std::vector<Move> legalMoves = state.getLegalMoves(m_player);

    if (legalMoves.empty()) {
        std::cout << "MinMax: No legal moves available" << std::endl;
        return Move();
    }

    std::cout << "MinMax: Evaluating " << legalMoves.size() << " moves at depth " << depth << std::endl;

    // Track best moves & repeated moves
    Move bestMove;
    int bestScore = MIN_SCORE;
    Move bestNonRepeatingMove;
    int bestNonRepeatingScore = MIN_SCORE;
    bool foundNonRepeating = false;

    int alpha = MIN_SCORE;
    int beta = MAX_SCORE;

    // Evaluate each move
    for (const Move& move : legalMoves) {
        GameState simulatedState = state;
        simulatedState.applyMove(move, false);

        // Check if this position was seen before
        uint64_t positionHash = simulatedState.getBoardHash();
        int repetitionCount = state.getPositionRepetitionCount(positionHash);

        // Calculate score using minimax
        int moveScore = alphaBeta(simulatedState, depth - 1, alpha, beta, false, m_player);

        // Apply penalty if position repeats
        if (repetitionCount > 0) {
            int penalty = 2000 * repetitionCount;
            moveScore -= penalty;
            std::cout << "  Move to (" << move.toCol << "," << move.toRow
                << ") repeats (seen " << repetitionCount << " times, -" << penalty << ")" << std::endl;
        }

        // Track overall best move
        if (moveScore > bestScore) {
            bestScore = moveScore;
            bestMove = move;
        }

        // Track best non-repeating move separately
        if (repetitionCount == 0) {
            if (moveScore > bestNonRepeatingScore) {
                bestNonRepeatingScore = moveScore;
                bestNonRepeatingMove = move;
                foundNonRepeating = true;
            }
            alpha = std::max(alpha, moveScore);
        }
    }

    // Prefer non-repeating moves if available
    if (foundNonRepeating) {
        std::cout << "Selected non-repeating move (score: " << bestNonRepeatingScore << ")" << std::endl;
        bestMove = bestNonRepeatingMove;
        bestScore = bestNonRepeatingScore;
    }
    else {
        std::cout << "All moves repeat - chose least bad (score: " << bestScore << ")" << std::endl;
    }

    std::cout << "MiniMax: Nodes evaluated = " << m_nodesEvaluated
        << " | Branches pruned = " << m_pruneCount << std::endl;

    return bestMove;
}

std::pair<int, int> MiniMax::findBestPlacement(const GameState& state, Piece* piece)
{
    if (!piece) {
        return { -1, -1 };
    }

    auto availablePositions = state.getLegalPlacements();

    if (availablePositions.empty()) {
        return { -1, -1 };
    }

    std::pair<int, int> bestPosition = availablePositions[0];
    int bestScore = MIN_SCORE;

    for (const auto& [col, row] : availablePositions) {
        int score = evaluatePosition(state, col, row, piece);

        if (score > bestScore) {
            bestScore = score;
            bestPosition = { col, row };
        }
    }

    return bestPosition;
}

int MiniMax::evaluatePosition(const GameState& state, int col, int row, Piece* piece)
{
    int score = 0;
    PieceOwner opponent = getOpponent(m_player);

    // F1: Strategic position value
    static constexpr int POSITION_VALUES[5][5] = {
        {2, 3, 4, 3, 2},
        {3, 5, 6, 5, 3},
        {4, 6, 10, 6, 4},
        {3, 5, 6, 5, 3},
        {2, 3, 4, 3, 2}
    };
    score += POSITION_VALUES[row][col] * 8;

    // F2: Simulate placement and evaluate board state
    GameState simulatedState = state;
    simulatedState.applyPlacement(col, row, piece);
    score += simulatedState.evaluate(m_player) / 3;

    // F3: Friendly piece adjacency
    const int DIRECTIONS[8][2] = {
        {-1, -1}, {-1, 0}, {-1, 1},
        { 0, -1},          { 0, 1},
        { 1, -1}, { 1, 0}, { 1, 1}
    };

    int friendlyNeighbors = 0;
    int opponentNeighbors = 0;

    for (const auto& [dx, dy] : DIRECTIONS) {
        int neighborCol = col + dx;
        int neighborRow = row + dy;

        if (isValidPosition(neighborCol, neighborRow)) {
            Piece* neighbor = state.getPieceAt(neighborCol, neighborRow);
            if (neighbor) {
                if (neighbor->getOwner() == m_player) {
                    friendlyNeighbors++;
                }
                else {
                    opponentNeighbors++;
                }
            }
        }
    }

    score += friendlyNeighbors * 12;
    score += opponentNeighbors * 8;

    // F4: Blocking potential
    int blockingValue = evaluateBlockingPotential(state, col, row, opponent);
    score += blockingValue * 25;

    // F5: Offensive potential
    int offensiveValue = evaluateOffensivePotential(state, col, row, piece);
    score += offensiveValue * 15;

    return score;
}

int MiniMax::evaluateBlockingPotential(const GameState& state, int col, int row, PieceOwner opponent) const
{
    int blockValue = 0;

    const int DIRECTIONS[4][2] = {
        {1, 0},   // Horizontal
        {0, 1},   // Vertical
        {1, 1},   // Diagonal TL-BR
        {1, -1}   // Diagonal TR-BL
    };

    for (const auto& [dx, dy] : DIRECTIONS) {
        int opponentCount = 0;
        int emptyCount = 0;
        int positions = 0;

        for (int i = -3; i <= 3; i++) {
            if (i == 0) continue;

            int checkCol = col + i * dx;
            int checkRow = row + i * dy;

            if (!isValidPosition(checkCol, checkRow)) continue;

            Piece* p = state.getPieceAt(checkCol, checkRow);
            if (p && p->getOwner() == opponent) {
                opponentCount++;
            }
            else if (!p) {
                emptyCount++;
            }

            positions++;
            if (positions >= 3) break;
        }

        if (opponentCount == 3) blockValue += 100;
        else if (opponentCount == 2 && emptyCount >= 1) blockValue += 30;
    }

    return blockValue;
}

int MiniMax::evaluateOffensivePotential(const GameState& state, int col, int row, Piece* piece) const
{
    if (!piece) return 0;

    int offensiveValue = 0;
    PieceOwner player = piece->getOwner();

    const int DIRECTIONS[4][2] = {
        {1, 0}, {0, 1}, {1, 1}, {1, -1}
    };

    for (const auto& [dx, dy] : DIRECTIONS) {
        int friendlyCount = 0;
        int emptyCount = 0;

        for (int i = -3; i <= 3; i++) {
            if (i == 0) continue;

            int checkCol = col + i * dx;
            int checkRow = row + i * dy;

            if (!isValidPosition(checkCol, checkRow)) continue;

            Piece* p = state.getPieceAt(checkCol, checkRow);
            if (p && p->getOwner() == player) {
                friendlyCount++;
            }
            else if (!p) {
                emptyCount++;
            }
        }

        if (friendlyCount == 2 && emptyCount >= 1) offensiveValue += 20;
        if (friendlyCount >= 1 && emptyCount >= 2) offensiveValue += 5;
    }

    return offensiveValue;
}

int MiniMax::alphaBeta(const GameState& state, int depth, int alpha, int beta,
    bool isMaximizingPlayer, PieceOwner aiPlayer)
{
    m_nodesEvaluated++;

    PieceOwner winner = state.getWinner();
    if (winner == aiPlayer) {
        return WIN_SCORE + depth;
    }
    else if (winner != PieceOwner::NONE) {
        return LOSS_SCORE - depth;
    }

    if (depth == 0) {
        return state.evaluate(aiPlayer);
    }

    PieceOwner currentPlayer = isMaximizingPlayer ? aiPlayer : getOpponent(aiPlayer);
    std::vector<Move> possibleMoves = state.getLegalMoves(currentPlayer);

    if (possibleMoves.empty()) {
        return state.evaluate(aiPlayer);
    }

    if (isMaximizingPlayer) {
        return maximizeScore(state, possibleMoves, depth, alpha, beta, aiPlayer);
    }
    else {
        return minimizeScore(state, possibleMoves, depth, alpha, beta, aiPlayer);
    }
}

int MiniMax::maximizeScore(const GameState& state,
    const std::vector<Move>& moves,
    int depth, int alpha, int beta,
    PieceOwner aiPlayer)
{
    int maxScore = MIN_SCORE;

    for (const Move& move : moves) {
        GameState testState = state;
        testState.applyMove(move, false);

        int score = alphaBeta(testState, depth - 1, alpha, beta, false, aiPlayer);

        maxScore = std::max(maxScore, score);
        alpha = std::max(alpha, score);

        if (beta <= alpha) {
            m_pruneCount++;
            break;
        }
    }

    return maxScore;
}

int MiniMax::minimizeScore(const GameState& state,
    const std::vector<Move>& moves,
    int depth, int alpha, int beta,
    PieceOwner aiPlayer)
{
    int minScore = MAX_SCORE;

    for (const Move& move : moves) {
        GameState testState = state;
        testState.applyMove(move, false);

        int score = alphaBeta(testState, depth - 1, alpha, beta, true, aiPlayer);
        minScore = std::min(minScore, score);
        beta = std::min(beta, score);

        if (beta <= alpha) {
            m_pruneCount++;
            break;
        }
    }

    return minScore;
}

PieceOwner MiniMax::getOpponent(PieceOwner player) const
{
    return (player == PieceOwner::AI) ? PieceOwner::PLAYER : PieceOwner::AI;
}

bool MiniMax::isValidPosition(int col, int row) const
{
    return col >= 0 && col < 5 && row >= 0 && row < 5;
}

void MiniMax::resetStatistics()
{
    m_nodesEvaluated = 0;
    m_pruneCount = 0;
}