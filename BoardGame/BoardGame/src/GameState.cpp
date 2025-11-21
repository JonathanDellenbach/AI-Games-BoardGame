#include "GameState.h"


// Pre-compile line check data for win detection and evaluation
// Format: {startCol, startRow, deltaCol, deltaRow}
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


// NOTE WILL HAVE TO MAKE THIS MRORE READABLE AND SUCH, MIGHT PASS THE PIECE FOR EVALS INSTEAD OF REDOING THEM HERE!!!!!!!!!!!!!!!
std::vector<Move> GameState::getLegalMoves(PieceOwner player) const {
    std::vector<Move> moves;
    moves.reserve(50); 

    // First pass: find all pieces belonging to player
    for (int fromCol = 0; fromCol < 5; fromCol++) {
        for (int fromRow = 0; fromRow < 5; fromRow++) {
            Piece* piece = m_board[fromCol][fromRow];
            if (!piece || piece->getOwner() != player) continue;

            // Generate moves based on piece type for efficiency
            PieceType type = piece->getType();

            if (type == PieceType::DONKEY) {
                // Donkey: only 4 cardinal directions
                static constexpr int cardinalDirs[4][2] = { {0,1},{0,-1},{1,0},{-1,0} };
                for (auto& d : cardinalDirs) {
                    int tc = fromCol + d[0], tr = fromRow + d[1];
                    if (tc >= 0 && tc < 5 && tr >= 0 && tr < 5 && !m_board[tc][tr]) {
                        moves.emplace_back(fromCol, fromRow, tc, tr, piece);
                    }
                }
            }
            else if (type == PieceType::SNAKE) {
                // Snake: 8 directions, 1 step
                static constexpr int allDirs[8][2] = { {-1,-1},{-1,0},{-1,1},{0,-1},{0,1},{1,-1},{1,0},{1,1} };
                for (auto& d : allDirs) {
                    int tc = fromCol + d[0], tr = fromRow + d[1];
                    if (tc >= 0 && tc < 5 && tr >= 0 && tr < 5 && !m_board[tc][tr]) {
                        moves.emplace_back(fromCol, fromRow, tc, tr, piece);
                    }
                }
            }
            else if (type == PieceType::FROG) {
                // Frog: 8 directions for single step, plus jump moves
                static constexpr int allDirs[8][2] = { {-1,-1},{-1,0},{-1,1},{0,-1},{0,1},{1,-1},{1,0},{1,1} };

                // Single step moves
                for (auto& d : allDirs) {
                    int tc = fromCol + d[0], tr = fromRow + d[1];
                    if (tc >= 0 && tc < 5 && tr >= 0 && tr < 5 && !m_board[tc][tr]) {
                        moves.emplace_back(fromCol, fromRow, tc, tr, piece);
                    }
                }

                // Jump moves: check each direction for adjacent piece then find landing spot
                // Frog can only land on the FIRST empty square after jumping over pieces
                for (auto& d : allDirs) {
                    int adjCol = fromCol + d[0], adjRow = fromRow + d[1];

                    // Must have adjacent piece to jump over
                    if (adjCol < 0 || adjCol >= 5 || adjRow < 0 || adjRow >= 5) continue;
                    if (!m_board[adjCol][adjRow]) continue;  // No piece to jump

                    // Skip over consecutive pieces, then land on first empty
                    int landCol = adjCol + d[0], landRow = adjRow + d[1];

                    // Skip over any consecutive pieces
                    while (landCol >= 0 && landCol < 5 && landRow >= 0 && landRow < 5
                        && m_board[landCol][landRow]) {
                        landCol += d[0];
                        landRow += d[1];
                    }

                    // Land on the first empty square (if in bounds)
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
    m_board[move.fromCol][move.fromRow] = nullptr; //remove old pos
    m_board[move.toCol][move.toRow] = move.piece; //place at new position

    if (updatePiecePosition) //we added the flag to control whether we update the piece or not
    {
        move.piece->setGridPosition(move.toCol, move.toRow); //when making real moves its true when simulating moves its false in minimax
    }
}

void GameState::applyPlacement(int col, int row, Piece* piece) {
    if (piece && col >= 0 && col < 5 && row >= 0 && row < 5 && !m_board[col][row]) {
        m_board[col][row] = piece;
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
    if (winner == player) return 10000; // max for the current player moving
    if (winner != PieceOwner::NONE) return -10000; // min for the other player

    int score = 0;
    PieceOwner opponent = (player == PieceOwner::PLAYER) ? PieceOwner::AI : PieceOwner::PLAYER;

    // Note: these are just basic heuritics for fallbacks really
    // Score based on winning lines, 3 in a row, 2 in a wor etc
    score += evaluateLines(player) * 10;
    score -= evaluateLines(opponent) * 10;

    // Score based on center control
    score += evaluateCenterControl(player) * 5;
    score -= evaluateCenterControl(opponent) * 5;

    return score;
}

int GameState::evaluateLines(PieceOwner player) const {
    int score = 0;

    // Check all possible 4-in-a-row lines on the board
    for (const auto& line : WIN_LINES) {
        score += evaluateLine(line[0], line[1], line[2], line[3], player);
    }
    // Check anti-diagonal lines (top-right to bottom-left)
    for (const auto& line : ANTI_DIAG_LINES) {
        score += evaluateLine(line[0], line[1], line[2], line[3], player);
    }

    return score;
}

int GameState::evaluateLine(int startCol, int startRow, int dCol, int dRow, PieceOwner player) const {
    int playerCount = 0, opponentCount = 0; // Piece count

    for (int i = 0; i < 4; i++) {
        Piece* p = m_board[startCol + i * dCol][startRow + i * dRow];
        if (p) {
            if (p->getOwner() == player) 
                playerCount++;
            else opponentCount++;
        }
    }

    // Line blocked by opponent
    if (opponentCount > 0 && playerCount > 0) return 0;

    // Score based on how many pieces we have in line
    if (playerCount == 3) return 100;  // Very valuable
    if (playerCount == 2) return 10;
    if (playerCount == 1) return 1; // Least

    return 0;
}

int GameState::evaluateCenterControl(PieceOwner player) const {
    int score = 0;

    // Center (2,2) is most valuable
    if (Piece* p = m_board[2][2]; p && p->getOwner() == player) 
        score += 3;

    // Adjacent to center
    static constexpr int adj[8][2] = {
        {1,1}, {1,2}, {1,3},  
        {2,1},        {2,3},   
        {3,1}, {3,2}, {3,3}
    };
    for (const auto& pos : adj) { // Any piece near center ++
        if (Piece* p = m_board[pos[0]][pos[1]]; p && p->getOwner() == player) score++;
    }

    return score;
}
