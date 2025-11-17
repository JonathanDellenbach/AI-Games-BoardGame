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
    // Must be within bounds and empty
    return col >= 0 && col < 5 && row >= 0 && row < 5 && isPositionEmpty(col, row);
}

bool GameState::isValidMove(const Move& move) const {
    if (!move.piece) return false;

    // Check bounds
    if (move.toCol < 0 || move.toCol >= 5 || move.toRow < 0 || move.toRow >= 5) {
        return false;
    }

    // Check if starting position matches piece location
    if (getPieceAt(move.fromCol, move.fromRow) != move.piece) return false;

    // Check if destination is empty
    if (!isPositionEmpty(move.toCol, move.toRow)) return false;

    // Can't stay in same place
    if (move.fromCol == move.toCol && move.fromRow == move.toRow) return false;

    // Use piece's movement validation
    return move.piece->isValidMove(*this, move.fromCol, move.fromRow, move.toCol, move.toRow);
}

std::vector<Move> GameState::getLegalMoves(PieceOwner player) const {
    std::vector<Move> legalMoves;

    // Find all pieces belonging to the player
    for (int fromCol = 0; fromCol < 5; fromCol++) {
        for (int fromRow = 0; fromRow < 5; fromRow++) {
            Piece* piece = getPieceAt(fromCol, fromRow);
            if (piece && piece->getOwner() == player) {
                // Try all possible destinations
                for (int toCol = 0; toCol < 5; toCol++) {
                    for (int toRow = 0; toRow < 5; toRow++) {
                        Move move(fromCol, fromRow, toCol, toRow, piece);
                        if (isValidMove(move)) {
                            legalMoves.push_back(move);
                        }
                    }
                }
            }
        }
    }

    return legalMoves;
}

std::vector<std::pair<int, int>> GameState::getLegalPlacements() const {
    std::vector<std::pair<int, int>> placements;

    for (int col = 0; col < 5; col++) {
        for (int row = 0; row < 5; row++) {
            if (isValidPlacement(col, row)) {
                placements.push_back({ col, row });
            }
        }
    }

    return placements;
}

void GameState::applyMove(const Move& move) {
    if (!move.piece) return;

    // Remove piece from old position
    removePieceAt(move.fromCol, move.fromRow);

    // Place piece at new position
    setPieceAt(move.toCol, move.toRow, move.piece);

    // Update piece's grid position
    move.piece->setGridPosition(move.toCol, move.toRow);
}

void GameState::applyPlacement(int col, int row, Piece* piece) {
    if (piece && isValidPlacement(col, row)) {
        setPieceAt(col, row, piece);
        piece->setGridPosition(col, row);
    }
}

bool GameState::isWinningState(PieceOwner player) const {
    // Check horizontal lines
    for (int row = 0; row < 5; row++) {
        for (int col = 0; col <= 1; col++) {  // Can start at 0 or 1
            int count = 0;
            for (int i = 0; i < 4; i++) {
                Piece* piece = getPieceAt(col + i, row);
                if (piece && piece->getOwner() == player) {
                    count++;
                }
                else {
                    break;
                }
            }
            if (count == 4) return true;
        }
    }

    // Check vertical lines
    for (int col = 0; col < 5; col++) {
        for (int row = 0; row <= 1; row++) {  // Can start at 0 or 1
            int count = 0;
            for (int i = 0; i < 4; i++) {
                Piece* piece = getPieceAt(col, row + i);
                if (piece && piece->getOwner() == player) {
                    count++;
                }
                else {
                    break;
                }
            }
            if (count == 4) return true;
        }
    }

    // Check diagonal (top-left to bottom-right)
    for (int col = 0; col <= 1; col++) {
        for (int row = 0; row <= 1; row++) {
            int count = 0;
            for (int i = 0; i < 4; i++) {
                Piece* piece = getPieceAt(col + i, row + i);
                if (piece && piece->getOwner() == player) {
                    count++;
                }
                else {
                    break;
                }
            }
            if (count == 4) return true;
        }
    }

    // Check diagonal (top-right to bottom-left)
    for (int col = 3; col < 5; col++) {
        for (int row = 0; row <= 1; row++) {
            int count = 0;
            for (int i = 0; i < 4; i++) {
                Piece* piece = getPieceAt(col - i, row + i);
                if (piece && piece->getOwner() == player) {
                    count++;
                }
                else {
                    break;
                }
            }
            if (count == 4) return true;
        }
    }

    return false;
}

PieceOwner GameState::getWinner() const {
    if (isWinningState(PieceOwner::PLAYER)) return PieceOwner::PLAYER;
    if (isWinningState(PieceOwner::AI)) return PieceOwner::AI;
    return PieceOwner::NONE;
}

int GameState::evaluate(PieceOwner player) const {
    // If there's a winner, return extreme values
    PieceOwner winner = getWinner();
    if (winner == player) return 10000;
    if (winner != PieceOwner::NONE && winner != player) return -10000;

    int score = 0;
    PieceOwner opponent = (player == PieceOwner::PLAYER) ? PieceOwner::AI : PieceOwner::PLAYER;

    // Evaluate potential lines of 4 for both players
    score += evaluateLines(player) * 10;
    score -= evaluateLines(opponent) * 10;

    // Center control bonus
    score += evaluateCenterControl(player) * 5;
    score -= evaluateCenterControl(opponent) * 5;

    // Mobility (number of legal moves)
    // Note: This could be expensive, so we might weight it less or optimize

    return score;
}

int GameState::evaluateLines(PieceOwner player) const {
    int score = 0;

    // Check all possible lines of 4 and count how many pieces player has in each
    // Horizontal
    for (int row = 0; row < 5; row++) {
        for (int col = 0; col <= 1; col++) {
            score += evaluateLine(col, row, 1, 0, player);
        }
    }

    // Vertical
    for (int col = 0; col < 5; col++) {
        for (int row = 0; row <= 1; row++) {
            score += evaluateLine(col, row, 0, 1, player);
        }
    }

    // Diagonal (top-left to bottom-right)
    for (int col = 0; col <= 1; col++) {
        for (int row = 0; row <= 1; row++) {
            score += evaluateLine(col, row, 1, 1, player);
        }
    }

    // Diagonal (top-right to bottom-left)
    for (int col = 3; col < 5; col++) {
        for (int row = 0; row <= 1; row++) {
            score += evaluateLine(col, row, -1, 1, player);
        }
    }

    return score;
}

int GameState::evaluateLine(int startCol, int startRow, int deltaCol, int deltaRow, PieceOwner player) const {
    int playerPieces = 0;
    int opponentPieces = 0;

    for (int i = 0; i < 4; i++) {
        Piece* piece = getPieceAt(startCol + i * deltaCol, startRow + i * deltaRow);
        if (piece) {
            if (piece->getOwner() == player) {
                playerPieces++;
            }
            else {
                opponentPieces++;
            }
        }
    }

    // If opponent has pieces in this line, it's not viable for us
    if (opponentPieces > 0 && playerPieces > 0) return 0;

    // Score based on how many pieces we have in a line
    if (playerPieces == 3) return 100;  // Three in a row is very valuable
    if (playerPieces == 2) return 10;   // Two in a row
    if (playerPieces == 1) return 1;    // One piece

    return 0;
}

int GameState::evaluateCenterControl(PieceOwner player) const {
    int score = 0;

    // Center square (2,2) is most valuable
    Piece* center = getPieceAt(2, 2);
    if (center && center->getOwner() == player) score += 3;

    // Adjacent to center
    const int adjacent[][2] = { {1,1}, {1,2}, {1,3}, {2,1}, {2,3}, {3,1}, {3,2}, {3,3} };
    for (auto& pos : adjacent) {
        Piece* piece = getPieceAt(pos[0], pos[1]);
        if (piece && piece->getOwner() == player) score += 1;
    }

    return score;
}
