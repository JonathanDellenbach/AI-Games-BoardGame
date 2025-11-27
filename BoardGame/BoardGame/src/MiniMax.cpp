#include "MiniMax.h"
#include <algorithm>
#include <iostream>

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
    // Need to reset otherwise evals previous moves
    resetStatistics();

    // Get all the moves can make
    std::vector<Move> legalMoves = state.getLegalMoves(PieceOwner::AI);

    if (legalMoves.empty()) {
        std::cout << "MinMax: No legal moves available!" << std::endl;
        return Move();
    }

    std::cout << "MinMax: Evaluating " << legalMoves.size() << " moves at depth " << depth << std::endl;

    Move bestMove;
    int bestScore = MIN_SCORE; // Start -infin
    int alpha = MIN_SCORE; // Best Ai
    int beta = MAX_SCORE; // Best Player

    // Evaluate each move
    for (const Move& move : legalMoves) {
        GameState simulatedState = state;
        simulatedState.applyMove(move, false); // Simulate moving a piece but dont actually move it

        // Look ahead with alphabeta for players min
        /*int moveScore = alphaBeta(simulatedState, depth - 1, alpha, beta, //replacing this as we were hardcoding the AI
            false, PieceOwner::AI);*/ // Keep false, want to mini the player

        int moveScore = alphaBeta(simulatedState, depth - 1, alpha, beta, false, m_player);

        if (moveScore > bestScore) {
            bestScore = moveScore;
            bestMove = move;
        }

        alpha = std::max(alpha, bestScore); // Update best for ai
    }

    std::cout << "MiniMax: Nodes evaluated = " << m_nodesEvaluated
        << " | Branches pruned = " << m_pruneCount << std::endl;

    return bestMove;
}

// Main entry point for placement phase
std::pair<int, int> MiniMax::findBestPlacement(const GameState& state, Piece* piece)
{
    if (!piece) {
        return { -1, -1 };
    }

    auto availablePositions = state.getLegalPlacements(); // Get all the boards empty positions

    if (availablePositions.empty()) {
        return { -1, -1 };
    }

    // Start with the first one for best initally
    std::pair<int, int> bestPosition = availablePositions[0]; // Pair a position to a score
    int bestScore = MIN_SCORE;

    // Evaluate each position with simple heuristic
    for (const auto& [col, row] : availablePositions) {
        int score = evaluatePosition(state, col, row, piece); // Score this cell

        if (score > bestScore) {
            bestScore = score;
            bestPosition = { col, row }; // Use the position evaled by bestScore
        }
    }

    return bestPosition;
}

// Scores a position based on center control, board state, and adjacency
int MiniMax::evaluatePosition(const GameState& state, int col, int row, Piece* piece)
{
    int score = 0;

    // F1
    // Center control (Using Manhattan distance to keep in line with grid constrains)
    // Make it so center is best and corner is worse, like what we did for the flow vector, same ideaology
    int distanceFromCenter = std::abs(col - 2) + std::abs(row - 2);
    // Note: 4 is max distance from center so we have to use thad, 15 is just to make the center control a wright of 0-60, can be anything above 10 really.
    score += (4 - distanceFromCenter) * 15;

    // F2
    // Simulate placement and evaluate board
    GameState simulatedState = state; 
    simulatedState.applyPlacement(col, row, piece);
    //score += simulatedState.evaluate(PieceOwner::AI) / 2; // how good is board after placement?
    score += simulatedState.evaluate(m_player) / 2;

    // F3
    // Adjacency for pieces near friendly pieces
    const int DIRECTIONS[8][2] = {
        {-1, -1}, {-1, 0}, {-1, 1},
        { 0, -1},          { 0, 1},
        { 1, -1}, { 1, 0}, { 1, 1}
    };

    int friendlyNeighbors = 0;
    // Go through board, get friendly piece holders and count for scoring
    for (const auto& [dx, dy] : DIRECTIONS) {
        int neighborCol = col + dx;
        int neighborRow = row + dy;

        if (isValidPosition(neighborCol, neighborRow)) {
            Piece* neighbor = state.getPieceAt(neighborCol, neighborRow);
            if (neighbor && neighbor->getOwner() == PieceOwner::AI) {
                friendlyNeighbors++;
            }
        }
    }
    score += friendlyNeighbors * 10;

    return score;
}

// minimax with alpha-beta pruning
int MiniMax::alphaBeta(const GameState& state, int depth, int alpha, int beta,
    bool isMaximizingPlayer, PieceOwner aiPlayer)
{
    m_nodesEvaluated++;

    // Game condition: game over
    PieceOwner winner = state.getWinner();
    if (winner == aiPlayer) {
        return WIN_SCORE + depth;
    }
    else if (winner != PieceOwner::NONE) {
        return LOSS_SCORE - depth;
    }

    // depth limit reached
    if (depth == 0) {
        return state.evaluate(aiPlayer);
    }

    // Get moves for current player (determine player to mini)
    PieceOwner currentPlayer = isMaximizingPlayer ? aiPlayer : getOpponent(aiPlayer);
    std::vector<Move> possibleMoves = state.getLegalMoves(currentPlayer);

    if (possibleMoves.empty()) {
        return state.evaluate(aiPlayer);
    }

    // Mini or max depending on whose turn
    if (isMaximizingPlayer) {
        return maximizeScore(state, possibleMoves, depth, alpha, beta, aiPlayer);
    }
    else {
        return minimizeScore(state, possibleMoves, depth, alpha, beta, aiPlayer);
    }
}

// AI's turn: maximize score
int MiniMax::maximizeScore(const GameState& state,
    const std::vector<Move>& moves,
    int depth, int alpha, int beta,
    PieceOwner aiPlayer)
{
    int maxScore = MIN_SCORE;

    for (const Move& move : moves) {
        // Simulate
        GameState testState = state;
        testState.applyMove(move, false);

        // Next turn is player so keep mini them
        int score = alphaBeta(testState, depth - 1, alpha, beta, false, aiPlayer);

        // Track score
        maxScore = std::max(maxScore, score);
        alpha = std::max(alpha, score);

        if (beta <= alpha) { // If found a move the player will blcok then just skip it
            m_pruneCount++;
            break;
        }
    }

    return maxScore;
}

// Opponent's turn: minimize score
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

        if (beta <= alpha) { // Ai wont allow this, just skip
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
