#include "GameState.h"
#include <sstream>

// Pre-compile line check data for win detection and evaluation
static constexpr int WIN_LINES[24][4] = {
    // Horizontal lines
    {0,0,1,0}, {1,0,1,0}, {0,1,1,0}, {1,1,1,0}, {0,2,1,0},
    {1,2,1,0}, {0,3,1,0}, {1,3,1,0}, {0,4,1,0}, {1,4,1,0},
    // Vertical lines
    {0,0,0,1}, {0,1,0,1}, {1,0,0,1}, {1,1,0,1}, {2,0,0,1},
    {2,1,0,1}, {3,0,0,1}, {3,1,0,1}, {4,0,0,1}, {4,1,0,1},
    // Diagonal TL-BR (4 total)
    {0,0,1,1}, {1,0,1,1}, {0,1,1,1}, {1,1,1,1}
};

// Anti-diagonal lines (TR-BL)
static constexpr int ANTI_DIAG_LINES[4][4] = {
    {3,0,-1,1}, {4,0,-1,1}, {3,1,-1,1}, {4,1,-1,1}
};

// CRITICAL FIX: This should be static, not global per-instance
static uint64_t ZOBRIST[5][5][3][2];
static bool zobristInitialized = false;

GameState::GameState()
    : m_currentPhase(GamePhase::PLACEMENT)
    , m_currentPlayer(PieceOwner::PLAYER)
    , m_winner(PieceOwner::NONE)
    , m_zobristKey(0)  // Initialize member variable
{
    for (int col = 0; col < 5; col++) {
        for (int row = 0; row < 5; row++) {
            m_board[col][row] = nullptr;
        }
    }

    initializeZobrist();
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
    return col >= 0 && col < 5 && row >= 0 && row < 5 && isPositionEmpty(col, row);
}

bool GameState::isValidMove(const Move& move) const {
    if (!move.piece) return false;

    if (move.toCol < 0 || move.toCol >= 5 || move.toRow < 0 || move.toRow >= 5) {
        return false;
    }

    if (getPieceAt(move.fromCol, move.fromRow) != move.piece) return false;

    if (!isPositionEmpty(move.toCol, move.toRow)) return false;

    if (move.fromCol == move.toCol && move.fromRow == move.toRow) return false;

    return move.piece->isValidMove(*this, move.fromCol, move.fromRow, move.toCol, move.toRow);
}

std::vector<Move> GameState::getLegalMoves(PieceOwner player) const {
    std::vector<Move> moves;
    moves.reserve(50);

    for (int fromCol = 0; fromCol < 5; fromCol++) {
        for (int fromRow = 0; fromRow < 5; fromRow++) {
            Piece* piece = m_board[fromCol][fromRow];
            if (!piece || piece->getOwner() != player) continue;

            PieceType type = piece->getType();

            if (type == PieceType::DONKEY) {
                static constexpr int cardinalDirs[4][2] = { {0,1},{0,-1},{1,0},{-1,0} };
                for (auto& d : cardinalDirs) {
                    int tc = fromCol + d[0], tr = fromRow + d[1];
                    if (tc >= 0 && tc < 5 && tr >= 0 && tr < 5 && !m_board[tc][tr]) {
                        moves.emplace_back(fromCol, fromRow, tc, tr, piece);
                    }
                }
            }
            else if (type == PieceType::SNAKE) {
                static constexpr int allDirs[8][2] = { {-1,-1},{-1,0},{-1,1},{0,-1},{0,1},{1,-1},{1,0},{1,1} };
                for (auto& d : allDirs) {
                    int tc = fromCol + d[0], tr = fromRow + d[1];
                    if (tc >= 0 && tc < 5 && tr >= 0 && tr < 5 && !m_board[tc][tr]) {
                        moves.emplace_back(fromCol, fromRow, tc, tr, piece);
                    }
                }
            }
            else if (type == PieceType::FROG) {
                static constexpr int allDirs[8][2] = { {-1,-1},{-1,0},{-1,1},{0,-1},{0,1},{1,-1},{1,0},{1,1} };

                for (auto& d : allDirs) {
                    int tc = fromCol + d[0], tr = fromRow + d[1];
                    if (tc >= 0 && tc < 5 && tr >= 0 && tr < 5 && !m_board[tc][tr]) {
                        moves.emplace_back(fromCol, fromRow, tc, tr, piece);
                    }
                }

                for (auto& d : allDirs) {
                    int adjCol = fromCol + d[0], adjRow = fromRow + d[1];

                    if (adjCol < 0 || adjCol >= 5 || adjRow < 0 || adjRow >= 5) continue;
                    if (!m_board[adjCol][adjRow]) continue;

                    int landCol = adjCol + d[0], landRow = adjRow + d[1];

                    while (landCol >= 0 && landCol < 5 && landRow >= 0 && landRow < 5
                        && m_board[landCol][landRow]) {
                        landCol += d[0];
                        landRow += d[1];
                    }

                    if (landCol >= 0 && landCol < 5 && landRow >= 0 && landRow < 5
                        && !m_board[landCol][landRow]) {
                        moves.emplace_back(fromCol, fromRow, landCol, landRow, piece);
                    }
                }
            }
        }
    }

    return moves;
}

std::vector<std::pair<int, int>> GameState::getLegalPlacements() const {
    std::vector<std::pair<int, int>> placements;
    placements.reserve(25);

    for (int col = 0; col < 5; col++) {
        for (int row = 0; row < 5; row++) {
            if (!m_board[col][row]) {
                placements.emplace_back(col, row);
            }
        }
    }
    return placements;
}

void GameState::applyMove(const Move& move, bool updatePiecePosition) {
    if (!move.piece) return;
    Piece* piece = move.piece;
    updateZobrist(piece, move.fromCol, move.fromRow, false);
    m_board[move.fromCol][move.fromRow] = nullptr; // Remove old pos

    // Add new position
    updateZobrist(piece, move.toCol, move.toRow, true);
    m_board[move.toCol][move.toRow] = piece;

    if (updatePiecePosition)
    {
        move.piece->setGridPosition(move.toCol, move.toRow); // only move when true (for simulation make sure its false)
    }
}

void GameState::applyPlacement(int col, int row, Piece* piece) {
    if (piece && col >= 0 && col < 5 && row >= 0 && row < 5 && !m_board[col][row]) {
        updateZobrist(piece, col, row, true);
        m_board[col][row] = piece;
        piece->setGridPosition(col, row);
    }
}

bool GameState::isWinningState(PieceOwner player) const {
    // Check horizontal lines
    for (int row = 0; row < 5; row++) {
        for (int col = 0; col <= 1; col++) {
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
        for (int row = 0; row <= 1; row++) {
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

void GameState::initializeZobrist()
{
    // Only initialize once globally
    if (!zobristInitialized) {
        std::mt19937_64 rng(12345);

        for (int col = 0; col < 5; col++) {
            for (int row = 0; row < 5; row++) {
                for (int type = 0; type < 3; type++) {    // Frog, Snake, Donkey
                    for (int owner = 0; owner < 2; owner++) { // Player, AI
                        ZOBRIST[col][row][type][owner] = rng();
                    }
                }
            }
        }
        zobristInitialized = true;
    }
}

bool GameState::checkLine(int startCol, int startRow, int dCol, int dRow, PieceOwner player) const
{
    for (int i = 0; i < 4; i++) {
        Piece* p = m_board[startCol + i * dCol][startRow + i * dRow];
        if (!p || p->getOwner() != player) return false;
    }
    return true;
}


int GameState::evaluate(PieceOwner player) const {
    // Check win/loss
    PieceOwner winner = getWinner();
    if (winner == player) return 10000;
    if (winner != PieceOwner::NONE) return -10000;

    int score = 0;
    PieceOwner opponent = (player == PieceOwner::PLAYER) ? PieceOwner::AI : PieceOwner::PLAYER;

    // Offensive score (player winning lines)
    score += evaluateLines(player) * 15;

    // Defensive score (block opponent's winning lines) - make sure Ai makes this priority
    score -= evaluateLines(opponent) * 20;

    // Center control differential
    score += evaluateCenterControl(player) * 3;
    score -= evaluateCenterControl(opponent) * 3;

    return score;
}

int GameState::evaluateLines(PieceOwner player) const {
    int score = 0;

    // All 4 in a rows
    for (const auto& line : WIN_LINES) {
        score += evaluateLine(line[0], line[1], line[2], line[3], player);
    }

    // Anti - diagonal checks
    for (const auto& line : ANTI_DIAG_LINES) {
        score += evaluateLine(line[0], line[1], line[2], line[3], player);
    }

    return score;
}


int GameState::evaluateLine(int startCol, int startRow, int dCol, int dRow, PieceOwner player) const {
    int playerCount = 0;
    int opponentCount = 0;
    int emptyCount = 0;

    for (int i = 0; i < 4; i++) {
        Piece* p = m_board[startCol + i * dCol][startRow + i * dRow];
        if (p) {
            if (p->getOwner() == player)
                playerCount++;
            else
                opponentCount++;
        }
        else {
            emptyCount++;
        }
    }

    // Line blocked by oppoentn
    if (opponentCount > 0 && playerCount > 0) return 0;

    // Score based on the player and op counts for lines
    if (playerCount > 0 && opponentCount == 0) {
        if (playerCount == 3 && emptyCount == 1) return 150;  // very valuable r
        if (playerCount == 2 && emptyCount == 2) return 20;   // mid
        if (playerCount == 1 && emptyCount == 3) return 2;    // least
    }

    return 0;
}


int GameState::evaluateCenterControl(PieceOwner player) const {
    int score = 0;

    // Scoring map for board positions
    static constexpr int POSITION_VALUES[5][5] = {
        {1, 2, 3, 2, 1},
        {2, 4, 5, 4, 2},
        {3, 5, 8, 5, 3},
        {2, 4, 5, 4, 2},
        {1, 2, 3, 2, 1}
    };

    for (int col = 0; col < 5; col++) {
        for (int row = 0; row < 5; row++) {
            Piece* p = m_board[col][row];
            if (p && p->getOwner() == player) {
                score += POSITION_VALUES[row][col];
            }
        }
    }

    return score;
}

uint64_t GameState::getBoardHash() const {
    return m_zobristKey;
}

void GameState::recordPosition() {
    m_positionHistory[m_zobristKey]++;
}

int GameState::getPositionRepetitionCount(uint64_t key) const {
    auto it = m_positionHistory.find(key);
    return it == m_positionHistory.end() ? 0 : it->second;
}


void GameState::updateZobrist(Piece* piece, int col, int row, bool add)
{
    int type = (int)piece->getType();    // 0–2
    int owner = (piece->getOwner() == PieceOwner::PLAYER) ? 0 : 1;

    m_zobristKey ^= ZOBRIST[col][row][type][owner];
}

void GameState::clearPositionHistory() {
    m_positionHistory.clear();
}