#include "GameState.h"

GameState::GameState()
    : m_currentPhase(GamePhase::PLACEMENT)
    , m_currentPlayer(PieceOwner::PLAYER)
    , m_winner(PieceOwner::NONE)
{
    // Initialize board to empty
    for (int col = 0; col < 5; col++) {
        for (int row = 0; row < 5; row++) {
            m_board[col][row] = nullptr;
        }
    }
}

GameState::~GameState() {
}

bool GameState::isPositionEmpty(int col, int row) const {
    if (col < 0 || col >= 5 || row < 0 || row >= 5) return false;
    return m_board[col][row] == nullptr;
}

Piece* GameState::getPieceAt(int col, int row) const {
    if (col < 0 || col >= 5 || row < 0 || row >= 5) return nullptr;
    return m_board[col][row];
}

void GameState::setPieceAt(int col, int row, Piece* piece) {
    if (col >= 0 && col < 5 && row >= 0 && row < 5) {
        m_board[col][row] = piece;
    }
}

void GameState::removePieceAt(int col, int row) {
    if (col >= 0 && col < 5 && row >= 0 && row < 5) {
        m_board[col][row] = nullptr;
    }
}

bool GameState::isValidPlacement(int col, int row) const {
    return isPositionEmpty(col, row);
}

bool GameState::isValidMove(const Move& move) const {
    if (!move.piece) return false;

    // Check if starting position matches piece location
    if (getPieceAt(move.fromCol, move.fromRow) != move.piece) return false;

    // Check if destination is empty
    if (!isPositionEmpty(move.toCol, move.toRow)) return false;

    // Use piece's movement validation
    return move.piece->isValidMove(*this, move.fromCol, move.fromRow, move.toCol, move.toRow);
}
